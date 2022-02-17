// mzhou@pinterest 2021-12-12
namespace mio {

class ArchiveBase { // not a virtual class
protected:
    ArchiveBase() {
    }
    // Archive is not copyable. But to allow move capture by function objects, check it at runtime rather than at compile time.
    ArchiveBase(const ArchiveBase&) {
        LOG(FATAL) << "Not supported";
    }
    ArchiveBase(ArchiveBase&& other) :
        _buffer(other._buffer),
        _cursor(other._cursor),
        _finish(other._finish),
        _limit(other._limit),
        _deleter(std::move(other._deleter)) {
        other._buffer = NULL;
        other._cursor = NULL;
        other._finish = NULL;
        other._limit = NULL;
        other._deleter = nullptr;
    }
    ~ArchiveBase() {
        free_buffer();
    }
public:
    ArchiveBase& operator=(const ArchiveBase&) {
        LOG(FATAL) << "Not supported";
        return *this;
    }
    ArchiveBase& operator=(ArchiveBase&& other) {
        if (this != &other) {
            free_buffer();
            _buffer = other._buffer;
            _cursor = other._cursor;
            _finish = other._finish;
            _limit = other._limit;
            _deleter = std::move(other._deleter);
            other._buffer = NULL;
            other._cursor = NULL;
            other._finish = NULL;
            other._limit = NULL;
            other._deleter = nullptr;
        }
        return *this;
    }
    char* buffer() {
        return _buffer;
    }
    void set_read_buffer(char* buffer, size_t length, std::function<void(char*)>&& deleter) {
        set_buffer(buffer, length, length, std::move(deleter));
    }
    void set_write_buffer(char* buffer, size_t capacity, std::function<void(char*)>&& deleter) {
        set_buffer(buffer, 0, capacity, std::move(deleter));
    }
    void set_buffer(char* buffer, size_t length, size_t capacity, std::function<void(char*)>&& deleter) {
        CHECK(length <= capacity);
        free_buffer();
        _buffer = buffer;
        _cursor = _buffer;
        _finish = buffer + length;
        _limit = buffer + capacity;
        _deleter = std::move(deleter);
    }
    char* cursor() {
        return _cursor;
    }
    void set_cursor(char* cursor) {
        DCHECK(cursor >= _buffer && cursor <= _finish);
        _cursor = cursor;
    }
    void advance_cursor(size_t offset) {
        DCHECK(offset <= size_t(_finish - _cursor));
        _cursor += offset;
    }
    char* finish() {
        return _finish;
    }
    void set_finish(char* finish) {
        DCHECK(finish >= _cursor && finish <= _limit);
        _finish = finish;
    }
    void advance_finish(size_t offset) {
        DCHECK(offset <= size_t(_limit - _finish));
        _finish += offset;
    }
    char* limit() {
        return _limit;
    }
    size_t position() {
        return _cursor - _buffer;
    }
    size_t length() {
        return _finish - _buffer;
    }
    size_t capacity() {
        return _limit - _buffer;
    }
    bool empty() {
        return _finish == _buffer;
    }
    void reset() {
        free_buffer();
        _buffer = NULL;
        _cursor = NULL;
        _finish = NULL;
        _limit = NULL;
    }
    void clear() {
        _cursor = _buffer;
        _finish = _buffer;
    }
    char* release() {
        char* buf = _buffer;
        _buffer = NULL;
        _cursor = NULL;
        _finish = NULL;
        _deleter = nullptr;
        return buf;
    }
    void resize(size_t newsize) {
        if (unlikely(newsize > capacity())) {
            reserve(std::max(capacity() * 2, newsize));
        }
        _finish = _buffer + newsize;
        _cursor = std::min(_cursor, _finish);
    }
    void reserve(size_t newcap) {
        if (newcap > capacity()) {
            char* newbuf;
            newbuf = new char[newcap];
            if (length() > 0) {
                memcpy(newbuf, _buffer, length());
            }
            _cursor = newbuf + (_cursor - _buffer);
            _finish = newbuf + (_finish - _buffer);
            _limit = newbuf + newcap;
            free_buffer();
            _buffer = newbuf;
            _deleter = std::default_delete<char[]>();
        }
    }
    void prepare_read(size_t size) {
        if (unlikely(!(size <= size_t(_finish - _cursor)))) {
            CHECK(size <= size_t(_finish - _cursor));
        }
    }
    void prepare_write(size_t size) {
        if (unlikely(size > size_t(_limit - _finish))) {
            reserve(std::max(capacity() * 2, length() + size));
        }
    }
    void read(void* data, size_t size) {
        if (size > 0) {
            prepare_read(size);
            memcpy(data, _cursor, size);
            advance_cursor(size);
        }
    }
    void read_back(void* data, size_t size) {
        if (size > 0) {
            CHECK(size <= size_t(_finish - _cursor));
            memcpy(data, _finish - size, size);
            _finish -= size;
        }
    }
    void write(const void* data, size_t size) {
        if (size > 0) {
            prepare_write(size);
            memcpy(_finish, data, size);
            advance_finish(size);
        }
    }
    template<class T>
    void get_raw(T& x) {
        prepare_read(sizeof(T));
        memcpy(&x, _cursor, sizeof(T));
        advance_cursor(sizeof(T));
    }
    template<class T>
    T get_raw() {
        T x;
        get_raw<T>(x);
        return std::move(x);
    }
    template<class T>
    void put_raw(const T& x) {
        prepare_write(sizeof(T));
        memcpy(_finish, &x, sizeof(T));
        advance_finish(sizeof(T));
    }
private:
    char* _buffer = NULL;
    char* _cursor = NULL;
    char* _finish = NULL;
    char* _limit = NULL;
    std::function<void(char*)> _deleter = nullptr;

    void free_buffer() {
        if (_deleter) {
            _deleter(_buffer);
        }
        _deleter = nullptr;
    }
};

template<class Type>
class Archive {
};

class BinaryArchiveType {
};

class TextArchiveType {
};

typedef Archive<BinaryArchiveType> BinaryArchive;
typedef Archive<TextArchiveType> TextArchive;

template<>
class Archive<BinaryArchiveType> : public ArchiveBase {
public:
    #define MIO_REPEAT_PATTERN(T) \
    BinaryArchive& operator>>(T& x) { \
        get_raw(x); \
        return *this; \
    } \
    BinaryArchive& operator<<(const T& x) { \
        put_raw(x); \
        return *this; \
    }
    // avoid using MIO_REPEAT10, which could produce very long output once compiliation error occurs
    MIO_REPEAT_PATTERN(int16_t)
    MIO_REPEAT_PATTERN(uint16_t)
    MIO_REPEAT_PATTERN(int32_t)
    MIO_REPEAT_PATTERN(uint32_t)
    MIO_REPEAT_PATTERN(int64_t)
    MIO_REPEAT_PATTERN(uint64_t)
    MIO_REPEAT_PATTERN(float)
    MIO_REPEAT_PATTERN(double)
    MIO_REPEAT_PATTERN(signed char)
    MIO_REPEAT_PATTERN(unsigned char)
    MIO_REPEAT_PATTERN(bool)
    #undef MIO_REPEAT_PATTERN
    template<class T>
    T get() {
        T x;
        *this >> x;
        return std::move(x);
    }
};


template<>
class Archive<TextArchiveType> : public ArchiveBase {
public:
    #define MIO_REPEAT_PATTERN(T) \
    TextArchive& operator>>(T& x) { \
        get_arithmetic(x); \
        return *this; \
    } \
    TextArchive& operator<<(const T& x) { \
        put_arithmetic(x); \
        return *this; \
    }
    MIO_REPEAT_PATTERN(int16_t)
    MIO_REPEAT_PATTERN(uint16_t)
    MIO_REPEAT_PATTERN(int32_t)
    MIO_REPEAT_PATTERN(uint32_t)
    MIO_REPEAT_PATTERN(int64_t)
    MIO_REPEAT_PATTERN(uint64_t)
    MIO_REPEAT_PATTERN(float)
    MIO_REPEAT_PATTERN(double)
    MIO_REPEAT_PATTERN(signed char)
    MIO_REPEAT_PATTERN(unsigned char)
    MIO_REPEAT_PATTERN(bool)
    #undef MIO_REPEAT_PATTERN
    char* next_delim() {
        char* next = cursor();
        while (next < finish() && *next != '\t') {
            next++;
        }
        return next;
    }
    template<class T>
    void get_arithmetic(T& x) {
        CHECK(cursor() < finish());
        char* next = next_delim();
        char* i = cursor();
        while (i < next && *i == ' ') {
            i++;
        }
        char* j = next;
        while (j > i && *(j - 1) == ' ') {
            j--;
        }
        x = boost::lexical_cast<T>(i, j - i);
        set_cursor(std::min(++next, finish()));
    }
    template<class T>
    void put_arithmetic(const T& x) {
        typedef std::array<char, 50> str_t;
        prepare_write(sizeof(str_t));
        *(str_t*)finish() = boost::lexical_cast<str_t>(x);
        char* i = finish();
        while (*i != 0) {
            i++;
        }
        *i = '\t';
        set_finish(i + 1);
    }
    template<class T>
    T get() {
        T x;
        *this >> x;
        return std::move(x);
    }
    template<class... ARGS>
    void printf(const char* fmt, ARGS&&... args) {
        size_t temp = limit() - finish();
        int len = snprintf(finish(), temp, fmt, args...);
        CHECK(len >= 0);
        if ((size_t)len >= temp) {
            prepare_write(len + 1);
            CHECK(snprintf(finish(), (size_t)len + 1, fmt, args...) == len);
        }
        advance_finish(len);
    }
};

template<class AR>
struct MioSerializer {
    explicit MioSerializer(Archive<AR>& ar) : ar(ar) {
    }
    template<class T>
    MioSerializer<AR>& operator,(const T& x) {
        ar << x;
        return *this;
    }
    Archive<AR>& ar;
};

template<class AR>
struct MioDeserializer {
    explicit MioDeserializer(Archive<AR>& ar) : ar(ar) {
    }
    template<class T>
    MioDeserializer<AR>& operator,(T& x) {
        ar >> x;
        return *this;
    }
    Archive<AR>& ar;
};

#define MIO_DEFINE_SIMPLE_SERIALIZER(CLASSNAME, FIELDS...) \
    template<class AR> \
    void _mio_serialize_internal_(::mio::Archive<AR>& _mio_ar_) const { \
        ::mio::MioSerializer<AR> _mio_serializer_(_mio_ar_); \
        _mio_serializer_, FIELDS; \
    } \
    template<class AR> \
    void _mio_deserialize_internal_(::mio::Archive<AR>& _mio_ar_) { \
        ::mio::MioDeserializer<AR> _mio_deserializer_(_mio_ar_); \
        _mio_deserializer_, FIELDS; \
    } \
    template<class AR> \
    friend ::mio::Archive<AR>& operator<<(::mio::Archive<AR>& ar, const CLASSNAME& x) { \
        x._mio_serialize_internal_(ar); \
        return ar; \
    } \
    template<class AR> \
    friend ::mio::Archive<AR>& operator>>(::mio::Archive<AR>& ar, CLASSNAME& x) { \
        x._mio_deserialize_internal_(ar); \
        return ar; \
    }

template<class AR, class T, size_t N>
Archive<AR>& operator<<(Archive<AR>& ar, const T (&p)[N]) {
    for (size_t i = 0; i < N; i++) {
        ar << p[i];
    }
    return ar;
}

template<class AR, class T, size_t N>
Archive<AR>& operator>>(Archive<AR>& ar, T (&p)[N]) {
    for (size_t i = 0; i < N; i++) {
        ar >> p[i];
    }
    return ar;
}

template<class AR, class T>
Archive<AR>& operator<<(Archive<AR>& ar, const std::vector<T>& p) {
    ar << (size_t)p.size();
    for (const auto& x : p) {
        ar << x;
    }
    return ar;
}

template<class AR, class T>
Archive<AR>& operator>>(Archive<AR>& ar, std::vector<T>& p) {
    p.resize(ar.template get<size_t>());
    for (auto& x : p) {
        ar >> x;
    }
    return ar;
}

template<class AR, class T>
Archive<AR>& operator<<(Archive<AR>& ar, const std::valarray<T>& p) {
    ar << (size_t)p.size();
    for (const auto& x : p) {
        ar << x;
    }
    return ar;
}

template<class AR, class T>
Archive<AR>& operator>>(Archive<AR>& ar, std::valarray<T>& p) {
    p.resize(ar.template get<size_t>());
    for (auto& x : p) {
        ar >> x;
    }
    return ar;
}

BinaryArchive& operator<<(BinaryArchive& ar, const std::string& s) {
    ar << (size_t)s.length();
    ar.write(&s[0], s.length());
    return ar;
}

BinaryArchive& operator>>(BinaryArchive& ar, std::string& s) {
    size_t len = ar.template get<size_t>();
    ar.prepare_read(len);
    s.assign(ar.cursor(), len);
    ar.advance_cursor(len);
    return ar;
}

TextArchive& operator<<(TextArchive& ar, const std::string& s) {
    ar.write(&s[0], s.length());
    ar.put_raw('\t');
    return ar;
}

TextArchive& operator>>(TextArchive& ar, std::string& s) {
    char* next = ar.next_delim();
    s.assign(ar.cursor(), next - ar.cursor());
    ar.set_cursor(std::min(++next, ar.finish()));
    return ar;
}

template<class AR, class T1, class T2>
Archive<AR>& operator<<(Archive<AR>& ar, const std::pair<T1, T2>& x) {
    return ar << x.first << x.second;
}

template<class AR, class T1, class T2>
Archive<AR>& operator>>(Archive<AR>& ar, std::pair<T1, T2>& x) {
    return ar >> x.first >> x.second;
}

template<class AR, class... T>
Archive<AR>& serialize_tuple(Archive<AR>& ar, const std::tuple<T...>& x, std::integral_constant<size_t, 0> n) {
    return ar;
}

template<class AR, class... T, size_t N>
Archive<AR>& serialize_tuple(Archive<AR>& ar, const std::tuple<T...>& x, std::integral_constant<size_t, N> n) {
    return serialize_tuple(ar, x, std::integral_constant<size_t, N - 1>()) << std::get<N - 1>(x);
}

template<class AR, class... T>
Archive<AR>& operator<<(Archive<AR>& ar, const std::tuple<T...>& x) {
    const size_t size = std::tuple_size<std::tuple<T...>>::value;
    return serialize_tuple(ar, x, std::integral_constant<size_t, size>());
}

template<class AR, class... T>
Archive<AR>& deserialize_tuple(Archive<AR>& ar, std::tuple<T...>& x, std::integral_constant<size_t, 0> n) {
    return ar;
}

template<class AR, class... T, size_t N>
Archive<AR>& deserialize_tuple(Archive<AR>& ar, std::tuple<T...>& x, std::integral_constant<size_t, N> n) {
    return deserialize_tuple(ar, x, std::integral_constant<size_t, N - 1>()) >> std::get<N - 1>(x);
}

template<class AR, class... T>
Archive<AR>& operator>>(Archive<AR>& ar, std::tuple<T...>& x) {
    const size_t size = std::tuple_size<std::tuple<T...>>::value;
    return deserialize_tuple(ar, x, std::integral_constant<size_t, size>());
}

#define MIO_REPEAT_PATTERN(MAP_TYPE, RESERVE_STATEMENT) \
    template<class AR, class KEY, class VALUE, class... ARGS> \
    Archive<AR>& operator<<(Archive<AR>& ar, const MAP_TYPE<KEY, VALUE, ARGS...>& p) { \
        ar << (size_t)p.size(); \
        for (auto it = p.begin(); it != p.end(); ++it) { \
            ar << *it; \
        } \
        return ar; \
    } \
    template<class AR, class KEY, class VALUE, class... ARGS> \
    Archive<AR>& operator>>(Archive<AR>& ar, MAP_TYPE<KEY, VALUE, ARGS...>& p) { \
        size_t size = ar.template get<size_t>(); \
        p.clear(); \
        RESERVE_STATEMENT; \
        for (size_t i = 0; i < size; i++) { \
            p.insert(ar.template get<std::pair<KEY, VALUE>>()); \
        } \
        return ar; \
    }

MIO_REPEAT_PATTERN(std::map, )
MIO_REPEAT_PATTERN(std::multimap, )
MIO_REPEAT_PATTERN(std::unordered_map, p.reserve(size))
MIO_REPEAT_PATTERN(std::unordered_multimap, p.reserve(size))
    
#undef MIO_REPEAT_PATTERN
    
#define MIO_REPEAT_PATTERN(SET_TYPE, RESERVE_STATEMENT) \
    template<class AR, class KEY, class... ARGS> \
    Archive<AR>& operator<<(Archive<AR>& ar, const SET_TYPE<KEY, ARGS...>& p) { \
        ar << (size_t)p.size(); \
        for (auto it = p.begin(); it != p.end(); ++it) { \
            ar << *it; \
        } \
        return ar; \
    } \
    template<class AR, class KEY, class... ARGS> \
    Archive<AR>& operator>>(Archive<AR>& ar, SET_TYPE<KEY, ARGS...>& p) { \
        size_t size = ar.template get<size_t>(); \
        p.clear(); \
        RESERVE_STATEMENT; \
        for (size_t i = 0; i < size; i++) { \
            p.insert(ar.template get<KEY>()); \
        } \
        return ar; \
    }


MIO_REPEAT_PATTERN(std::set, )
MIO_REPEAT_PATTERN(std::multiset, )
MIO_REPEAT_PATTERN(std::unordered_set, p.reserve(size))
MIO_REPEAT_PATTERN(std::unordered_multiset, p.reserve(size))
    
#undef MIO_REPEAT_PATTERN
    
}

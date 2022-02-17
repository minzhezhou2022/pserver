namespace mio {

inline size_t count_spaces(const char* s) {
    size_t count = 0;
    while (*s != 0 && isspace(*s++)) {
        count++;
    }
    return count;
}

inline size_t count_nonspaces(const char* s) {
    size_t count = 0;
    while (*s != 0 && !isspace(*s++)) {
        count++;
    }
    return count;
}

template<class... ARGS>
void format_string_append(std::string& str, const char* fmt, ARGS&&... args) { // use VA_ARGS may be better ?
    int len = snprintf(NULL, 0, fmt, args...);
    CHECK(len >= 0);
    size_t oldlen = str.length();
    str.resize(oldlen + len + 1);
    CHECK(snprintf(&str[oldlen], (size_t)len + 1, fmt, args...) == len);
    str.resize(oldlen + len);
}

template<class... ARGS>
void format_string_append(std::string& str, const std::string& fmt, ARGS&&... args) {
    format_string_append(str, fmt.c_str(), args...);
}

template<class... ARGS>
std::string format_string(const char* fmt, ARGS&&... args) {
    std::string str;
    format_string_append(str, fmt, args...);
    return std::move(str);
}

template<class... ARGS>
std::string format_string(const std::string& fmt, ARGS&&... args) {
    return format_string(fmt.c_str(), args...);
}

// remove leading and tailing spaces
inline std::string trim_spaces(const std::string& str) {
    const char* p = str.c_str();
    while (*p != 0 && isspace(*p)) {
        p++;
    }
    size_t len = strlen(p);
    while (len > 0 && isspace(p[len - 1])) {
        len--;
    }
    return std::string(p, len);
}

// split string by delim
template<class T = std::string>
std::vector<T> split_string(const std::string& str, char delim) {
    size_t num = 1;
    const char* p;
    for (p = str.c_str(); *p != 0; p++) {
        if (*p == delim) {
            num++;
        }
    }
    std::vector<T> list(num);
    const char* last = str.c_str();
    num = 0;
    for (p = str.c_str(); *p != 0; p++) {
        if (*p == delim) {
            list[num++] = boost::lexical_cast<T>(last, p - last);
            last = p + 1;
        }
    }
    list[num] = boost::lexical_cast<T>(last, p - last);
    return std::move(list);
}

// split string by spaces. Leading and tailing spaces are ignored. Consecutive spaces are treated as one delim.
template<class T = std::string>
std::vector<T> split_string(const std::string& str) {
    size_t num = 0;
    const char* p;
    for (p = str.c_str(); *p != 0; ) {
        if (!isspace(*p)) {
            num++;
            p++;
            while (*p != 0 && !isspace(*p)) {
                p++;
            }
        } else {
            p++;
        }
    }
    std::vector<T> list(num);
    num = 0;
    for (p = str.c_str(); *p != 0; ) {
        if (!isspace(*p)) {
            const char* last = p;
            p++;
            while (*p != 0 && !isspace(*p)) {
                p++;
            }
            list[num++] = boost::lexical_cast<T>(last, p - last);
        } else {
            p++;
        }
    }
    return std::move(list);
}

template<class T>
std::string join_strings(const std::vector<T>& strs, char delim) {
    std::string str;
    for (size_t i = 0; i < strs.size(); i++) {
        if (i > 0) {
            str += delim;
        }
        str += boost::lexical_cast<std::string>(strs[i]);
    }
    return std::move(str);
}

// A helper class for reading lines from file. A line buffer is maintained. It doesn't need to know the maximum possible length of a line.
class LineFileReader : public boost::noncopyable {
public:
    ~LineFileReader() {
        ::free(_buffer);
    }
    char* getline(FILE* f) {
        return this->getdelim(f, '\n');
    }
    char* getdelim(FILE* f, char delim) {
        ssize_t ret = ::getdelim(&_buffer, &_buf_size, delim, f);
        if (ret >= 0) {
            if (ret >= 1 && _buffer[ret - 1] == delim) {
                _buffer[--ret] = 0;
            }
            _length = (size_t)ret;
            return _buffer;
        } else {
            _length = 0;
            CHECK(feof(f));
            return NULL;
        }
    }
    char* get() {
        return _buffer;
    }
    size_t length() {
        return _length;
    }
private:
    char* _buffer = NULL;
    size_t _buf_size = 0;
    size_t _length = 0;
};

}


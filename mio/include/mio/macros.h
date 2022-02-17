namespace mio {

#ifndef likely
#define likely(x) __builtin_expect(!!(x), 1)
#endif

#ifndef unlikely
#define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#undef LOG
#define LOG(severity) COMPACT_GOOGLE_LOG_ ## severity.stream() << __func__ << "(): "

// repeat MIO_REPEAT_PATTERN for every argument
// avoid using recursive implementation, which could print a lot of informarion once compilation error happens
#define MIO_REPEAT1(a) MIO_REPEAT_PATTERN(a)
#define MIO_REPEAT2(a, b) MIO_REPEAT_PATTERN(a) MIO_REPEAT_PATTERN(b) 
#define MIO_REPEAT3(a, b, c) MIO_REPEAT_PATTERN(a) MIO_REPEAT_PATTERN(b) MIO_REPEAT_PATTERN(c) 
#define MIO_REPEAT4(a, b, c, d) MIO_REPEAT_PATTERN(a) MIO_REPEAT_PATTERN(b) MIO_REPEAT_PATTERN(c) MIO_REPEAT_PATTERN(d) 
#define MIO_REPEAT5(a, b, c, d, e) MIO_REPEAT_PATTERN(a) MIO_REPEAT_PATTERN(b) MIO_REPEAT_PATTERN(c) MIO_REPEAT_PATTERN(d)  MIO_REPEAT_PATTERN(e) 
#define MIO_REPEAT6(a, b, c, d, e, f) MIO_REPEAT_PATTERN(a) MIO_REPEAT_PATTERN(b) MIO_REPEAT_PATTERN(c) MIO_REPEAT_PATTERN(d)  MIO_REPEAT_PATTERN(e) \
    MIO_REPEAT_PATTERN(f) 
#define MIO_REPEAT7(a, b, c, d, e, f, g) MIO_REPEAT_PATTERN(a) MIO_REPEAT_PATTERN(b) MIO_REPEAT_PATTERN(c) MIO_REPEAT_PATTERN(d)  MIO_REPEAT_PATTERN(e) \
    MIO_REPEAT_PATTERN(f) MIO_REPEAT_PATTERN(g) 
#define MIO_REPEAT8(a, b, c, d, e, f, g, h) MIO_REPEAT_PATTERN(a) MIO_REPEAT_PATTERN(b) MIO_REPEAT_PATTERN(c) MIO_REPEAT_PATTERN(d)  MIO_REPEAT_PATTERN(e) \
    MIO_REPEAT_PATTERN(f) MIO_REPEAT_PATTERN(g) MIO_REPEAT_PATTERN(h) 
#define MIO_REPEAT9(a, b, c, d, e, f, g, h, i) MIO_REPEAT_PATTERN(a) MIO_REPEAT_PATTERN(b) MIO_REPEAT_PATTERN(c) MIO_REPEAT_PATTERN(d)  MIO_REPEAT_PATTERN(e) \
    MIO_REPEAT_PATTERN(f) MIO_REPEAT_PATTERN(g) MIO_REPEAT_PATTERN(h) MIO_REPEAT_PATTERN(i) 
#define MIO_REPEAT10(a, b, c, d, e, f, g, h, i, j) MIO_REPEAT_PATTERN(a) MIO_REPEAT_PATTERN(b) MIO_REPEAT_PATTERN(c) MIO_REPEAT_PATTERN(d)  MIO_REPEAT_PATTERN(e) \
    MIO_REPEAT_PATTERN(f) MIO_REPEAT_PATTERN(g) MIO_REPEAT_PATTERN(h) MIO_REPEAT_PATTERN(i) MIO_REPEAT_PATTERN(j) 

#define MIO_DEFINE_SIMPLE_HASH(CLASSNAME, FIELDS...) \
    static void _mio_hash_combine_internal_(::std::size_t& seed) { \
    } \
    template<class T, class... OTHER> \
    static void _mio_hash_combine_internal_(::std::size_t& seed, const T& x, const OTHER&... other) { \
        ::boost::hash_combine(seed, x); \
        _mio_hash_combine_internal_(seed, other...); \
    } \
    ::std::size_t _mio_hash_value_internal_() const { \
        ::std::size_t _mio_seed_ = 0; \
        _mio_hash_combine_internal_(_mio_seed, FIELDS); \
    } \
    friend ::std::size_t hash_value(const CLASSNAME& x) { \
        return x._mio_hash_value_internal_(); \
    }
    
}


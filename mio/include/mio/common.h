namespace mio {

using std::shared_ptr;
using std::weak_ptr;
using std::unique_ptr;

class ScopeExit : public boost::noncopyable {
public:
    explicit ScopeExit(std::function<void ()> f) : _f(std::move(f)) {
    }
    ~ScopeExit() {
        _f();
    }
private:
    std::function<void ()> _f;
};

// Get time in seconds.
inline double current_realtime() {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    return tp.tv_sec + tp.tv_nsec * 1e-9;
}

// Call func(args...). If interrupted by signal, recall the function.
template<class FUNC, class... ARGS>
auto ignore_signal_call(FUNC&& func, ARGS&&... args) -> typename std::result_of<FUNC(ARGS...)>::type {
    for (;;) {
        auto err = func(args...);
        if (err < 0 && errno == EINTR) {
            LOG(INFO) << "Signal is caught. Ignored.";
            continue;
        }
        return err;
    }
}

inline std::mutex& global_fork_mutex() {
    static std::mutex mutex;
    return mutex;
}

// popen and pclose are not thread-safe
inline FILE* guarded_popen(const char* command, const char* type) {
    std::lock_guard<std::mutex> lock(global_fork_mutex());
    return popen(command, type);
}

inline int guarded_pclose(FILE* stream) {
    std::lock_guard<std::mutex> lock(global_fork_mutex());
    return pclose(stream);
}

}


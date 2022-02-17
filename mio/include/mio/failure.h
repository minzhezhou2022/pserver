namespace mio {

struct FailureExit;

inline std::vector<FailureExit*>& local_failure_exits() {
    thread_local std::vector<FailureExit*> x;
    return x;
}

struct FailureExit : public boost::noncopyable {
    std::function<void()> func;

    explicit FailureExit(std::function<void()> func_) : func(std::move(func_)) {
        auto& items = local_failure_exits();
        items.push_back(this);
    }
    ~FailureExit() {
        auto& items = local_failure_exits();
        CHECK(!items.empty() && items.back() == this);
        items.pop_back();
        if (std::uncaught_exceptions()) {
            func();
        }
    }
};

#define MIO_FAILURE_LOG LOG(INFO) << __PRETTY_FUNCTION__ << "(): MIO FAILURE LOG: "

inline void mio_failure_writer(const char* data, int size) {
    auto& items = local_failure_exits();
    while (!items.empty()) {
        FailureExit* item = items.back();
        items.pop_back();
        item->func();
    }
    fwrite(data, size, 1, stderr);
}

inline void mio_failure_function() {
    abort(); // should invoke mio_failure_writer
}

inline void failure_init_internal() {
    google::InstallFailureSignalHandler();
    google::InstallFailureWriter(&mio_failure_writer);
    google::InstallFailureFunction(&mio_failure_function);
}

}

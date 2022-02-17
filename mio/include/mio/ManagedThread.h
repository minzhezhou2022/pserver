namespace mio {

class alignas(64) ManagedThread : public boost::noncopyable {
public:
    ManagedThread() {
        _thr = std::thread([this]() {
            run_thread();
        });
    }
    ~ManagedThread() {
        CHECK(!_active);
        _terminate = true;
        _sem_start.post();
        _thr.join();
    }
    bool active() {
        return _active;
    }
    void start(std::function<void()> func) {
        CHECK(!_active);
        _active = true;
        _func = std::move(func);
        _sem_start.post();
    }
    void join() {
        CHECK(_active);
        _sem_finish.wait();
        _active = false;
    }
private:
    bool _active = false;
    bool _terminate = false;
    std::function<void()> _func;
    Semaphore _sem_start;
    Semaphore _sem_finish;
    std::thread _thr;

    void run_thread() {
        while (!_terminate) {
            _sem_start.wait();
            if (_terminate) {
                break;
            };
            _func();
            _sem_finish.post();
        }
    }
};

}

namespace mio {

class WaitGroup : public boost::noncopyable {
public:
    void clear() {
        std::lock_guard<std::mutex> lock(_mutex);
        _counter = 0;
        _cond.notify_all();
    }
    void add(int delta) {
        if (delta == 0) {
            return;
        }
        std::lock_guard<std::mutex> lock(_mutex);
        _counter += delta;
        if (_counter == 0) {
            _cond.notify_all();
        }
    }
    void done() {
        add(-1);
    }
    void wait() {
        std::unique_lock<std::mutex> lock(_mutex);
        while (_counter != 0) {
            _cond.wait(lock);
        }
    }
private:
    std::mutex _mutex;
    std::condition_variable _cond;
    int _counter = 0;
};

}


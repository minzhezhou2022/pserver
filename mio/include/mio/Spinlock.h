namespace mio {

class Spinlock : public boost::noncopyable {
public:
    Spinlock() {
        PCHECK(0 == pthread_spin_init(&_lock, 0));
    }
    ~Spinlock() {
        PCHECK(0 == pthread_spin_destroy(&_lock));
    }
    void lock() {
        PCHECK(0 == pthread_spin_lock(&_lock));
    }
    void unlock() {
        PCHECK(0 == pthread_spin_unlock(&_lock));
    }
private:
    pthread_spinlock_t _lock;
};

}

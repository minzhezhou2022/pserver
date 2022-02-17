namespace mio {

class Semaphore : public boost::noncopyable {
public:
    Semaphore() {
        PCHECK(0 == sem_init(&_sem, 0, 0));
    }
    ~Semaphore() {
        PCHECK(0 == sem_destroy(&_sem));
    }
    void post() {
        PCHECK(0 == sem_post(&_sem));
    }
    void wait() {
        PCHECK(0 == ignore_signal_call(sem_wait, &_sem));
    }
    bool try_wait() {
        int err = 0;
        PCHECK((err = ignore_signal_call(sem_trywait, &_sem), err == 0 || errno == EAGAIN));
        return err == 0;
    }
private:
    sem_t _sem;
};

}

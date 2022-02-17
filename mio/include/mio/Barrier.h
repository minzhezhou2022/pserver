namespace mio {

class Barrier : public boost::noncopyable {
public:
    explicit Barrier(int count = 1) {
        CHECK(count >= 1);
        PCHECK(0 == pthread_barrier_init(&_barrier, NULL, count));
    }
    ~Barrier() {
        PCHECK(0 == pthread_barrier_destroy(&_barrier));
    }
    void reset(int count) {
        CHECK(count >= 1);
        PCHECK(0 == pthread_barrier_destroy(&_barrier));
        PCHECK(0 == pthread_barrier_init(&_barrier, NULL, count));
    }
    void wait() {
        int err = pthread_barrier_wait(&_barrier);
        PCHECK((err = pthread_barrier_wait(&_barrier), err == 0 || err == PTHREAD_BARRIER_SERIAL_THREAD));
    }
private:
    pthread_barrier_t _barrier;
};

}

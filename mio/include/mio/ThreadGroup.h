namespace mio {

class ThreadGroup : public boost::noncopyable {
public:
    explicit ThreadGroup(int thread_num = 0) {
        set_real_thread_num(thread_num);
    }
    ~ThreadGroup() {
        set_real_thread_num(0);
    }
    int real_thread_num() {
        return (int)_threads.size();
    }
    int parallel_num() {
        return _parallel_num;
    }
    void set_real_thread_num(int thread_num) {
        CHECK(thread_num >= 0);
        CHECK(!joinable());
        if (thread_num == (int)_threads.size()) {
            return;
        }
        _threads = std::vector<ManagedThread>(thread_num);
        _parallel_num = (thread_num == 0) ? 1 : thread_num;
        _barrier.reset(_parallel_num);
    }
    void set_parallel_num(int parallel_num) {
        CHECK(parallel_num >= 1);
        set_real_thread_num(parallel_num);
    }
    bool joinable() {
        return (bool)_func;
    }
    void run(std::function<void (int)> func) {
        start(std::move(func));
        join();
    }
    void start(std::function<void (int)> func) {
        CHECK(!joinable());
        if (_threads.empty()) {
            ScopeExit on_exit([old_id = thread_id(), old_grp = parent_group()]() {
                thread_id() = old_id;
                parent_group() = old_grp;
            });
            thread_id() = 0;
            parent_group() = this;
            func(0);
            return;
        }
        _func = std::move(func);
        for (int i = 0; i < _parallel_num; i++) {
            _threads[i].start([this, i]() {
                thread_id() = i;
                parent_group() = this;
                _func(i);
            });
        }
    }
    void join() {
        CHECK(joinable());
        for (int i = 0; i < (int)_threads.size(); i++) {
            _threads[i].join();
        }
        _func = nullptr;
    }
    static int& thread_id() {
        thread_local int x = 0;
        return x;
    }
    static ThreadGroup*& parent_group() {
        thread_local ThreadGroup* x = NULL;
        return x;
    }
    void barrier_wait() {
        _barrier.wait();
    }

private:
    int _parallel_num = 1;
    std::vector<ManagedThread> _threads;
    Barrier _barrier;
    std::function<void (int)> _func;
};

inline int parallel_run_id() {
    return ThreadGroup::thread_id();
}

inline ThreadGroup& local_thread_group() {
    thread_local ThreadGroup g;
    return g;
}

inline int parallel_run_num(ThreadGroup& thrgrp = local_thread_group()) {
    return thrgrp.parallel_num();
}

inline void parallel_run_barrier_wait() {
    ThreadGroup* thrgrp = ThreadGroup::parent_group();
    CHECK(thrgrp != NULL);
    thrgrp->barrier_wait();
}

inline void parallel_run_barrier_wait(ThreadGroup& thrgrp) {
    thrgrp.barrier_wait();
}

template<class THREAD_FUNC>
void parallel_run(THREAD_FUNC&& func, ThreadGroup& thrgrp = local_thread_group()) {
    thrgrp.run([&func](int i) {
        func(i);
    });
}

template<class THREAD_FUNC>
void parallel_run_range(uint64_t n, THREAD_FUNC&& func, ThreadGroup& thrgrp = local_thread_group()) {
    int thr_num = thrgrp.parallel_num();
    thrgrp.run([n, &func, thr_num](int i) {
        func(i, n * i / thr_num, n * (i + 1) / thr_num);
    });
}

template<class THREAD_FUNC>
void parallel_run_dynamic(int n, THREAD_FUNC&& func, ThreadGroup& thrgrp = local_thread_group()) {
    std::atomic<int> counter(0);
    thrgrp.run([n, &counter, &func](int thr_id) {
        int i;
        while (i = counter++, i < n) {
            func(thr_id, i);
        }
    });
}

}


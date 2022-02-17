namespace mio {

inline std::default_random_engine& local_random_engine() {
    struct engine_wrapper_t {
        std::default_random_engine engine;
        engine_wrapper_t() {
            static std::atomic<unsigned long> x(0);
            std::seed_seq sseq = {x++, x++, x++, (unsigned long)(current_realtime() * 1000)};
            engine.seed(sseq);
        }
    };
    thread_local engine_wrapper_t r;
    return r.engine;
}

template<class T>
std::uniform_real_distribution<T>& local_uniform_real_distribution() {
    thread_local std::uniform_real_distribution<T> distr;
    DCHECK(distr.a() == 0.0 && distr.b() == 1.0);
    return distr;
}

}


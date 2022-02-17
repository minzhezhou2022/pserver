// mzhou@pinterest 2021-12-15
namespace mio {

template<class T>
struct mpi_type_trait {
};

template<>
struct mpi_type_trait<double> {
    static MPI_Datatype type() {
        return MPI_DOUBLE;
    }
};

template<>
struct mpi_type_trait<float> {
    static MPI_Datatype type() {
        return MPI_FLOAT;
    }
};

template<>
struct mpi_type_trait<int32_t> {
    static MPI_Datatype type() {
        return MPI_INT;
    }
};

template<>
struct mpi_type_trait<uint32_t> {
    static MPI_Datatype type() {
        return MPI_UNSIGNED;
    }
};

template<>
struct mpi_type_trait<int64_t> {
    static MPI_Datatype type() {
        return MPI_LONG_LONG;
    }
};

template<>
struct mpi_type_trait<uint64_t> {
    static MPI_Datatype type() {
        return MPI_UNSIGNED_LONG_LONG;
    }
};

template<>
struct mpi_type_trait<long long> {
    static MPI_Datatype type() {
        return MPI_LONG_LONG;
    }
};

template<>
struct mpi_type_trait<unsigned long long> {
    static MPI_Datatype type() {
        return MPI_UNSIGNED_LONG_LONG;
    }
};

struct MioMpiInfo {
    int rank = 0;
    int size = 1;
    std::vector<std::string> ip_table;
};

inline MioMpiInfo& mpi_info_internal() {
    static MioMpiInfo x;
    return x;
}

inline int mpi_rank() {
    return mpi_info_internal().rank;
}

inline int mpi_size() {
    return mpi_info_internal().size;
}

inline MPI_Comm mpi_comm() {
    return MPI_COMM_WORLD;
}

inline const std::string& mpi_ip(int rank = mpi_rank()) {
    return mpi_info_internal().ip_table[rank];
}

inline void mpi_barrier() {
    CHECK(0 == MPI_Barrier(mpi_comm()));
}

template<class T>
T mpi_allreduce(T x, MPI_Op op) {
    T tot;
    CHECK(0 == MPI_Allreduce(&x, &tot, 1, mpi_type_trait<T>::type(), op, mpi_comm()));
    return tot;
}

template<class T>
void mpi_bcast(T* p, int count, int root) {
    BinaryArchive ar;
    int len = 0;
    if (mpi_rank() == root) {
        for (int i = 0; i < count; i++) {
            ar << p[i];
        }
        len = boost::lexical_cast<int>(ar.length());
    }
    CHECK(0 == MPI_Bcast(&len, 1, MPI_INT, root, mpi_comm()));
    ar.resize(len);
    ar.set_cursor(ar.buffer());
    CHECK(0 == MPI_Bcast(ar.buffer(), len, MPI_BYTE, root, mpi_comm()));
    for (int i = 0; i < count; i++) {
        ar >> p[i];
    }
}

template<class T>
void mpi_check_consistency(const T* p, int count) {
    BinaryArchive ar;
    for (int i = 0; i < count; i++) {
        ar << p[i];
    }
    size_t hash_code = boost::hash_range(ar.buffer(), ar.finish());
    size_t root_hash_code = hash_code;
    CHECK(0 == MPI_Bcast(&root_hash_code, 1, mpi_type_trait<size_t>::type(), 0, mpi_comm()));
    CHECK(root_hash_code == hash_code);
    mpi_barrier();
}

inline std::string mpi_get_local_ip_internal() {
    int sockfd;
    char buf[512];
    struct ifconf ifconf;
    struct ifreq* ifreq;

    ifconf.ifc_len = 512;
    ifconf.ifc_buf = buf;
    PCHECK((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0);
    PCHECK(ioctl(sockfd, SIOCGIFCONF, &ifconf) >= 0);
    PCHECK(0 == close(sockfd));

    ifreq = (struct ifreq*)buf;
    for (int i = 0; i < int(ifconf.ifc_len / sizeof(struct ifreq)); i++) {
        std::string ip;
        ip = inet_ntoa(((struct sockaddr_in*)&ifreq->ifr_addr)->sin_addr);
        if (ip != "127.0.0.1") {
            return ip;
        }
        ifreq++;
    }
    LOG(FATAL) << "IP not found";
    return "";
}

inline void mpi_init_internal() {
    MioMpiInfo& info = mpi_info_internal();
    CHECK(0 == MPI_Comm_rank(MPI_COMM_WORLD, &info.rank));
    CHECK(0 == MPI_Comm_size(MPI_COMM_WORLD, &info.size));

    std::vector<std::string>& ip_table = info.ip_table;
    ip_table.assign(mpi_size(), "");
    ip_table[mpi_rank()] = mpi_get_local_ip_internal();
    for (int i = 0; i < mpi_size(); i++) {
        mpi_bcast(&ip_table[i], 1, i);
    }
}

}

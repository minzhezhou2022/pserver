namespace mio {

inline void fs_add_read_converter_internal(std::string& path, bool& is_pipe, const std::string& converter) {
    if (converter == "") {
        return;
    }
    if (!is_pipe) {
        path = format_string("( %s ) < \"%s\"", converter.c_str(), path.c_str());
        is_pipe = true;
    } else {
        path = format_string("%s | %s", path.c_str(), converter.c_str());
    }
}

inline void fs_add_write_converter_internal(std::string& path, bool& is_pipe, const std::string& converter) {
    if (converter == "") {
        return;
    }
    if (!is_pipe) {
        path = format_string("( %s ) > \"%s\"", converter.c_str(), path.c_str());
        is_pipe = true;
    } else {
        path = format_string("%s | %s", converter.c_str(), path.c_str());
    }
}

inline shared_ptr<FILE> fs_open_internal(const std::string& path, bool is_pipe, const std::string& mode, size_t buffer_size) {
    if (!is_pipe) {
        return shell_open_file(path, mode, buffer_size);
    } else {
        return shell_open_pipe(path, mode, buffer_size);
    }
}

inline bool fs_begin_with_internal(const std::string& path, const std::string& str) {
    return strncmp(path.c_str(), str.c_str(), str.length()) == 0;
}

inline bool fs_end_with_internal(const std::string& path, const std::string& str) {
    return path.length() >= str.length() && strncmp(&path[path.length() - str.length()], str.c_str(), str.length()) == 0;
}

inline size_t& localfs_buffer_size_internal() {
    static size_t x = 0;
    return x;
}

inline size_t localfs_buffer_size() {
    return localfs_buffer_size_internal();
}

inline void localfs_set_buffer_size(size_t x) {
    localfs_buffer_size_internal() = x;
}

inline shared_ptr<FILE> localfs_open_read(std::string path, const std::string& converter) {
    bool is_pipe = false;
    if (fs_end_with_internal(path, ".gz")) {
        fs_add_read_converter_internal(path, is_pipe, "zcat");
    }
    fs_add_read_converter_internal(path, is_pipe, converter);
    return fs_open_internal(path, is_pipe, "r", localfs_buffer_size());
}

inline shared_ptr<FILE> localfs_open_write(std::string path, const std::string& converter) {
    shell_execute(format_string("mkdir -p $(dirname \"%s\")", path.c_str()));

    bool is_pipe = false;
    if (fs_end_with_internal(path, ".gz")) {
        fs_add_write_converter_internal(path, is_pipe, "gzip");
    }
    fs_add_write_converter_internal(path, is_pipe, converter);
    return fs_open_internal(path, is_pipe, "w", localfs_buffer_size());
}

inline int64_t localfs_file_size(const std::string& path) {
    struct stat buf;
    CHECK(0 == stat(path.c_str(), &buf));
    return (int64_t)buf.st_size;
}

inline void localfs_remove(const std::string& path) {
    if (path == "") {
        return;
    }
    shell_execute(format_string("rm -rf %s", path.c_str()));
}

inline std::vector<std::string> localfs_list(const std::string& path) {
    if (path == "") {
        return {};
    }
    shared_ptr<FILE> pipe;
    pipe = shell_open_pipe(format_string("find %s -type f -maxdepth 1", path.c_str()), "r");
    LineFileReader reader;
    std::vector<std::string> list;
    while (reader.getline(&*pipe)) {
        list.push_back(reader.get());
    }
    return list;
}

inline std::string localfs_tail(const std::string& path) {
    if (path == "") {
        return "";
    }
    return shell_get_command_output(format_string("tail -1 %s ", path.c_str()));
}

inline bool localfs_exists(const std::string& path) {
    std::string test_f = shell_get_command_output(format_string("[ -f %s ] ; echo $?", path.c_str()));
    if (trim_spaces(test_f) == "0") {
        return true;
    }
    std::string test_d = shell_get_command_output(format_string("[ -d %s ] ; echo $?", path.c_str()));
    if (trim_spaces(test_d) == "0") {
        return true;
    }
    return false;
}

inline size_t& s3_buffer_size_internal() {
    static size_t x = 0;
    return x;
}

inline size_t s3_buffer_size() {
    return s3_buffer_size_internal();
}

inline void s3_set_buffer_size(size_t x) {
    s3_buffer_size_internal() = x;
}

inline std::string& s3_command_internal() {
    static std::string x = "";
    return x;
}

inline const std::string& s3_command() {
    return s3_command_internal();
}

inline void s3_set_command(const std::string& x) {
    s3_command_internal() = x;
}

inline shared_ptr<FILE> s3_open_read(std::string path, const std::string& converter) {
    if (fs_end_with_internal(path, ".gz")) {
        path = format_string("%s cp --quiet \"%s\" - | gunzip -c", s3_command().c_str(), path.c_str());
    } else {
        path = format_string("%s cp --quiet \"%s\" - ", s3_command().c_str(), path.c_str());
    }
    bool is_pipe = true;
    fs_add_read_converter_internal(path, is_pipe, converter);
    return fs_open_internal(path, is_pipe, "r", s3_buffer_size());
}

inline shared_ptr<FILE> s3_open_write(std::string path, const std::string& converter) {
    path = format_string("%s cp - \"%s\"", s3_command().c_str(), path.c_str());
    bool is_pipe = true;
    if (fs_end_with_internal(path, ".gz\"")) {
        fs_add_write_converter_internal(path, is_pipe, "gzip");
    }
    fs_add_write_converter_internal(path, is_pipe, converter);
    return fs_open_internal(path, is_pipe, "w", s3_buffer_size());
}

inline std::vector<std::string> s3_list(const std::string& path) {
    if (path == "") {
        return {};
    }
    CHECK(path.back() == '/');
    shared_ptr<FILE> pipe;
    pipe = shell_open_pipe(format_string("%s ls %s |  grep ^20 | (awk -F\" \" '{print $NF}'; [ $? != 2 ] )", s3_command().c_str(), path.c_str()), "r");
    LineFileReader reader;
    std::vector<std::string> list;
    while (reader.getline(&*pipe)) {
        std::string fname = reader.get();
        CHECK(fname.size() > 0);
        list.push_back(path + fname);
    }
    return list;
}

inline std::string s3_tail(const std::string& path, int ln = 1) {
    if (path == "") {
        return "";
    }
    return shell_get_command_output(format_string("%s cp --quiet %s - | tail -%d ", s3_command().c_str(), path.c_str(), ln));
}

inline bool local_isdir(const std::string& path) {
    std::string result = shell_get_command_output(
        format_string("ls -a %s", path.c_str()));
    result.pop_back();
    return result != path;
}

inline bool s3_isdir(const std::string& path) {
    if (path == "") {
        return false;
    }
    CHECK(path.back() != '/');
    std::string result = shell_get_command_output(format_string("%s ls %s | awk -F\" \" '{print $1}'", s3_command().c_str(), path.c_str()));
    return result == "PRE";
}

inline void s3_remove(const std::string& path) {
    if (path == "") {
        return;
    }
    std::string mode = "--recursive";
    if (path.back() != '/' && !s3_isdir(path)) {
        mode = "";
    }
    shell_execute(format_string("%s rm --quiet %s  %s; true", s3_command().c_str(), mode.c_str(), path.c_str()));
}

inline bool s3_exists(const std::string& path) {
    std::string fname = split_string(path, '/').back();
    std::string test = shell_get_command_output(format_string("%s ls %s | grep %s$ ; echo $?",
    s3_command().c_str() , path.c_str(), fname.c_str()));
    test.pop_back();
    return test != "1";
}

inline int fs_select_internal(const std::string& path) {
    if (fs_begin_with_internal(path, "s3:")) {
        return 1;
    }
    return 0;
}

inline shared_ptr<FILE> fs_open_read(const std::string& path, const std::string& converter) {
    switch (fs_select_internal(path)) {
        case 0: 
            return localfs_open_read(path, converter);
        case 1:
            return s3_open_read(path, converter);
        default:
            LOG(FATAL) << "Not supported";
    }
    return {};
}

inline shared_ptr<FILE> fs_open_read(const std::string& path) {
    return fs_open_read(path, "");
}

inline shared_ptr<FILE> fs_open_write(const std::string& path, const std::string& converter) {
    switch (fs_select_internal(path)) {
        case 0: 
            return localfs_open_write(path, converter);
        case 1:
            return s3_open_write(path, converter);
        default:
            LOG(FATAL) << "Not supported";
    }
    return {};
}

inline shared_ptr<FILE> fs_open_write(const std::string& path) {
    return fs_open_write(path, "");
}

inline shared_ptr<FILE> fs_open(const std::string& path, const std::string& mode, const std::string& converter = "") {
    if (mode == "r" || mode == "rb") {
        return fs_open_read(path, converter);
    }   
    if (mode == "w" || mode == "wb") {
        return fs_open_write(path, converter);
    }
    LOG(FATAL) << "Unknown mode: " << mode;
    return {};
}   

inline int64_t fs_file_size(const std::string& path) {
    switch (fs_select_internal(path)) {
        case 0: 
            return localfs_file_size(path);
        default:
            LOG(FATAL) << "Not supported";
    }
    return 0;
}

inline void fs_remove(const std::string& path) {
    switch (fs_select_internal(path)) {
        case 0: 
            return localfs_remove(path);
        case 1:
            return s3_remove(path);
        default:
            LOG(FATAL) << "Not supported";
    }
}

inline std::vector<std::string> fs_list(const std::string& path) {
    switch (fs_select_internal(path)) {
        case 0: 
            return localfs_list(path);
        case 1:
            return s3_list(path);
        default:
            LOG(FATAL) << "Not supported";
    }
    return {};
}

inline std::string fs_tail(const std::string& path) {
    switch (fs_select_internal(path)) {
        case 0: 
            return localfs_tail(path);
        case 1:
            return s3_tail(path);
        default:
            LOG(FATAL) << "Not supported";
    }
    return "";
}

inline bool fs_exists(const std::string& path) {
    switch (fs_select_internal(path)) {
        case 0: 
            return localfs_exists(path);
        case 1:
            return s3_exists(path);
        default:
            LOG(FATAL) << "Not supported";
    }
    return false;
}

}

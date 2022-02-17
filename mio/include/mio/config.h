namespace mio {

// Calling operator[] to non-const YAML::Node is not thread-safe, since it will create a temporary child node when the key doesn't exist
// Calling operator[] to const YAML::Node is thread-safe, but you cannot create child node in this way
// operator[] of Config always call operator[] to const YAML::Node, so it is thread-safe to call operator[] to Config, though you cannot create child node in this way
// I also wonder how yaml-cpp manages its resource since it allows loops
// Looking for better c++ yaml library
class Config {
public:
    Config() {
    }
    Config(const Config& other) : _node(other._node), _path(other._path) {
    }
    Config(const YAML::Node& node, const std::string& path) : _node(node), _path(path) {
    }
    Config& operator=(const Config& other) {
        _path = other._path;
        _node.reset(other._node); // don't use operator=, since it has different semantic
        return *this;
    }
    const YAML::Node& node() const {
        return _node;
    }
    YAML::Node& node() {
        return _node;
    }
    const std::string& path() const {
        return _path;
    }
    std::string& path() {
        return _path;
    }
    const YAML::Node& operator*() const {
        return _node;
    }
    YAML::Node& operator*() {
        return _node;
    }
    const YAML::Node* operator->() const {
        return &_node;
    }
    YAML::Node* operator->() {
        return &_node;
    }
    bool is_defined() const {
        return _node.IsDefined();
    }
    bool is_null() const {
        return _node.IsNull();
    }
    bool is_scalar() const {
        return _node.IsScalar();
    }
    bool is_sequence() const {
        return _node.IsSequence();
    }
    bool is_map() const {
        return _node.IsMap();
    }
    size_t size() const {
        return _node.size();
    }
    Config operator[](size_t i) const {
        return {_node[i], _path + "[" + boost::lexical_cast<std::string>(i) + "]"};
    }
    Config operator[](const std::string& key) const {
        return {_node[key], _path + "." + key};
    }
    template<class T>
    T as() const {
        try {
            return _node.as<T>();
        }
        catch (const std::exception& e) {
            LOG(FATAL) << "Error reading conf item " << _path << " : " << e.what();
            throw;
        }
    }
private:
    YAML::Node _node{YAML::NodeType::Undefined};
    std::string _path;
};

inline Config& global_config() {
    static Config conf;
    return conf;
}

}

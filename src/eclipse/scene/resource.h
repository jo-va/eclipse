#pragma once

#include <string>
#include <fstream>

namespace eclipse {

class Resource
{
public:
    explicit Resource(const std::string& file_path, const Resource* rel_to = nullptr);
    ~Resource();

    std::istream& get_stream();
    size_t length();

    const std::string& get_uri() const { return m_uri; }
    const std::string& get_path() const { return m_path; }

    bool is_remote() const { return m_is_remote; }

    static void set_remote_cache_root(const std::string& root) { m_remote_root = root; }

private:
    std::string m_uri;
    std::string m_path;
    bool m_is_remote;

    std::ifstream m_stream;

    static std::string m_remote_root;
};

} // namespace eclipse

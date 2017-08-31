#pragma once

#include <string>

namespace eclipse {

class HTTPDownloader
{
public:
    HTTPDownloader();
    ~HTTPDownloader();

    std::string get(const std::string& url);

    bool save_to_file(const std::string& url, const std::string& filepath);

private:
    void* m_curl;
};

} // namespace eclipse

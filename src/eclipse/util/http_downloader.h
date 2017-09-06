#pragma once

#include "eclipse/util/except.h"

#include <string>

namespace eclipse {

class HTTPError : public Error
{
public:
    HTTPError(const std::string& m) : Error(m) { }
};

class HTTPDownloader
{
public:
    HTTPDownloader();
    ~HTTPDownloader();

    std::string get(const std::string& url);

    void save_to_file(const std::string& url, const std::string& filepath);

private:
    void* m_curl;
};

} // namespace eclipse

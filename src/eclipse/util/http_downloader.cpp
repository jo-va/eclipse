#include "eclipse/util/http_downloader.h"
#include "eclipse/util/logger.h"

#include <string>
#include <fstream>
#include <curl/curl.h>

namespace eclipse {

HTTPDownloader::HTTPDownloader()
{
    m_curl = (void*)curl_easy_init();
    if (!m_curl)
        LOG_ERROR("Can't initialize CURL");
}

HTTPDownloader::~HTTPDownloader()
{
    if (m_curl)
        curl_easy_cleanup((CURL*)m_curl);
    m_curl = nullptr;
}

static size_t write_callback_string(void *contents, size_t size, size_t nmemb, void *userp)
{
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

static size_t write_callback_file(void *contents, size_t size, size_t nmemb, void *userp)
{
    std::string data((const char*)contents, size * nmemb);
    *((std::ofstream*)userp) << data;
    return size * nmemb;
}

std::string HTTPDownloader::get(const std::string& url)
{
    std::string read_buffer;
    if (!m_curl)
    {
        LOG_ERROR("Curl not initialized");
        return read_buffer;
    }

    curl_easy_setopt((CURL*)m_curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt((CURL*)m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt((CURL*)m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt((CURL*)m_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt((CURL*)m_curl, CURLOPT_ACCEPT_ENCODING, "deflate");
    curl_easy_setopt((CURL*)m_curl, CURLOPT_WRITEFUNCTION, write_callback_string);
    curl_easy_setopt((CURL*)m_curl, CURLOPT_WRITEDATA, &read_buffer);

    LOG_INFO("Downloading ", url);

    CURLcode res = curl_easy_perform((CURL*)m_curl);
    if (res != CURLE_OK)
        LOG_ERROR("Failed to download [", url, "]: ", curl_easy_strerror(res));

    return read_buffer;
}

bool HTTPDownloader::save_to_file(const std::string& url, const std::string& filepath)
{
    std::ofstream file_stream;
    file_stream.open(filepath, std::ios::trunc | std::ios::out);

    if (!m_curl)
    {
        LOG_ERROR("Curl not initialized");
        return false;
    }

    curl_easy_setopt((CURL*)m_curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt((CURL*)m_curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt((CURL*)m_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt((CURL*)m_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt((CURL*)m_curl, CURLOPT_ACCEPT_ENCODING, "deflate");
    curl_easy_setopt((CURL*)m_curl, CURLOPT_WRITEFUNCTION, write_callback_file);
    curl_easy_setopt((CURL*)m_curl, CURLOPT_WRITEDATA, &file_stream);

    LOG_INFO("Downloading ", url);

    CURLcode res = curl_easy_perform((CURL*)m_curl);
    if (res != CURLE_OK)
        LOG_ERROR("Failed to download [", url, "]: ", curl_easy_strerror(res));

    file_stream.close();

    return true;
}

} // namespace eclipse

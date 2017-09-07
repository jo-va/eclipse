#include "eclipse/util/resource.h"
#include "eclipse/util/http_downloader.h"
#include "eclipse/util/file_util.h"

#include <string>
#include <fstream>
#include <memory>

namespace eclipse {

static HTTPDownloader downloader;

std::string Resource::m_remote_root = "remote_files";

Resource::Resource(const std::string& uri, std::shared_ptr<const Resource> rel_to)
{
    m_is_remote = (uri.find("http://")  != std::string::npos) ||
                  (uri.find("https://") != std::string::npos) ||
                  (rel_to && rel_to->is_remote());

    if (m_is_remote)
    {
        m_path = rel_to ? concat_paths(rel_to->get_path(), uri) : m_remote_root + "/" + get_filename(uri);
        m_uri  = rel_to ? concat_paths(rel_to->get_uri(), uri)  : uri;

        create_dir(remove_filename(m_path));
        try
        {
            downloader.save_to_file(m_uri, m_path);
        }
        catch (HTTPError& e)
        {
            throw ResourceError(e.what());
        }
    }
    else
    {
        m_path = m_uri = rel_to ? concat_paths(rel_to->get_path(), uri) : uri;
    }

    if (!file_exists(m_path))
        throw ResourceError(m_path + " does not exist");
}

Resource::~Resource()
{
    if (m_stream.is_open())
        m_stream.close();
}

size_t Resource::length()
{
    std::istream& s = get_stream();
    std::streampos begin = s.tellg();
    s.seekg(0, std::ios::end);
    std::streampos end = s.tellg();

    return (size_t)(end - begin);
}

std::istream& Resource::get_stream()
{
    if (!m_stream.is_open())
        m_stream.open(get_path());

    return m_stream;
}

} // namespace eclipse

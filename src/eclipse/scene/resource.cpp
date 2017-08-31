#include "eclipse/scene/resource.h"
#include "eclipse/util/http_downloader.h"
#include "eclipse/util/logger.h"

#include <string>
#include <cstring>
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>

namespace eclipse {

static HTTPDownloader downloader;

std::string Resource::m_remote_root = "remote_files";

std::string remove_filename(const std::string& uri)
{
    std::string base_path;
    size_t pos = uri.find_last_of('/');
    if (pos != std::string::npos)
        base_path = uri.substr(0, pos);
    return base_path;
}

std::string get_filename(const std::string& uri)
{
    std::string name = uri;
    size_t pos = uri.find_last_of('/');
    if (pos != std::string::npos)
        name = uri.substr(pos + 1, uri.length() - pos - 1);
    return name;
}

std::string assemble(const std::string& base_uri, const std::string& rel_uri)
{
    // base_uri = http://www.google.ca/dir1/dir2/dir3/dir4/blabla.txt
    // rel_uri  = ../.././dir/data.txt
    // resutl   = http://www.google.ca/dir1/dir2/dir/data.txt

    // First remove filename from base_uri
    // Then count number of folders to remove by counting .. and . from rel_uri
    // Remove the .. and . from rel_uri
    // Remove the corresponding number of folders from base_uri
    // Append base_uri and rel_uri

    std::string parent_uri = remove_filename(base_uri);
    std::string child_uri = rel_uri;
    while (1)
    {
        if (child_uri.substr(0, 3) == "../")
        {
            child_uri = child_uri.substr(3, child_uri.length() - 3);

            size_t pos;
            if ((pos = parent_uri.find_last_of('/')) != std::string::npos)
            {
                parent_uri = parent_uri.substr(0, pos);
            }
            else
            {
                LOG_ERROR("Can't concatenate ", base_uri, " with ", rel_uri);
                return "";
            }
        }
        else if (child_uri.substr(0, 2) == "./")
        {
            child_uri = child_uri.substr(2, child_uri.length() - 2);
        }
        else
        {
            break;
        }
    }

    return parent_uri + "/" + child_uri;
}

bool create_dir(const std::string& dir)
{
    struct stat st;
    std::memset(&st, 0, sizeof(st));
    if (stat(dir.c_str(), &st) == -1)
    {
        mkdir(dir.c_str(), 0777);
        return true;
    }
    return false;
}

Resource::Resource(const std::string& uri, const Resource* rel_to)
{
    m_is_remote = (uri.find("http://")  != std::string::npos) ||
                  (uri.find("https://") != std::string::npos) ||
                  (rel_to && rel_to->is_remote());

    if (m_is_remote)
    {
        m_path = rel_to ? assemble(rel_to->get_path(), uri) : m_remote_root + "/" + get_filename(uri);
        m_uri  = rel_to ? assemble(rel_to->get_uri(), uri)  : uri;

        create_dir(remove_filename(m_path));
        downloader.save_to_file(m_uri, m_path);
    }
    else
    {
        m_path = m_uri = rel_to ? assemble(rel_to->get_path(), uri) : uri;
    }

    LOG_INFO("Created new resource ", m_path);
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

#include "eclipse/util/file_util.h"

#include <string>
#include <vector>
#include <fstream>
#include <cstring>
#include <iterator>
#include <utility>
#include <sys/stat.h>
#include <sys/types.h>

namespace eclipse {

bool has_extension(const std::string& file, const std::string& ext)
{
    if (file.substr(file.length() - ext.length(), ext.length()) == ext)
        return true;
    return false;
}

std::string remove_filename(const std::string& uri)
{
    std::string base_path = uri;
    size_t pos = uri.find_last_of('/');
    if (pos != std::string::npos)
        base_path = uri.substr(0, pos);
    return base_path;
}

std::string remove_extension(const std::string& uri)
{
    std::string base_path = uri;
    size_t pos = uri.find_last_of('.');
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

std::string concat_paths(const std::string& base_uri, const std::string& rel_uri)
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
                parent_uri += "/..";
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

bool file_exists(const std::string& name)
{
    std::ifstream file(name.c_str());
    return file.good();
}

bool create_dir(const std::string& dir)
{
    struct stat st;
    std::memset(&st, 0, sizeof(st));
    if (stat(dir.c_str(), &st) == -1)
    {
        if (mkdir(dir.c_str(), 0777) == -1)
        {
            throw IOError("cant' create directory `" + dir + "`: " + std::strerror(errno));
        }
        return true;
    }
    return false;
}

std::vector<char> read_file(const std::string& file)
{
    std::ifstream file_stream(file, std::ios::binary);
    std::vector<char> data((std::istreambuf_iterator<char>(file_stream)),
                            std::istreambuf_iterator<char>());

    return std::move(data);
}

} // namespace eclipse

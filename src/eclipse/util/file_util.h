#pragma once

#include "eclipse/util/except.h"

#include <string>
#include <vector>

namespace eclipse {

class IOError : public Error
{
public:
    IOError(const std::string& msg) : Error(msg) { }
};

bool has_extension(const std::string& file, const std::string& ext);
std::string remove_filename(const std::string& uri);
std::string get_filename(const std::string& uri);
std::string concat_paths(const std::string& base_uri, const std::string& rel_uri);

bool file_exists(const std::string& name);
bool create_dir(const std::string& dir);
std::vector<char> read_file(const std::string& file);

} // namespace eclipse

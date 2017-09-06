#pragma once

#include <stdexcept>
#include <string>

namespace eclipse {

class Error : public std::runtime_error
{
public:
    Error(const std::string& msg) : std::runtime_error(msg) { }
};

} // namespace eclipse

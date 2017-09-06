#pragma once

#include <stdexcept>
#include <string>

namespace eclipse { namespace material {

class Error : public std::runtime_error
{
public:
    Error(const std::string& m) : std::runtime_error(m) { }
};

class ParseError : public Error
{
public:
    ParseError(const std::string& m) : Error(m) { }
};

class ValidationError : public Error
{
public:
    ValidationError(const std::string& msg) : Error(msg) { }
};

} } // namespace eclipse::material

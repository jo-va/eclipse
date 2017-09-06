#pragma once

#include "eclipse/util/except.h"

#include <string>

namespace eclipse { namespace material {

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

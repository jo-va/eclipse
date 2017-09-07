#include "eclipse/util/input_parser.h"

#include <string>
#include <vector>
#include <algorithm>

namespace eclipse {

InputParser::InputParser(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i)
        m_tokens.push_back(std::string(argv[i]));
}

const std::string& InputParser::get_option(const std::string& option) const
{
    auto it = std::find(m_tokens.begin(), m_tokens.end(), option);
    if (it != m_tokens.end() && ++it != m_tokens.end())
        return *it;

    static const std::string empty_str("");
    return empty_str;
}

bool InputParser::option_exists(const std::string& option) const
{
    return std::find(m_tokens.begin(), m_tokens.end(), option) != m_tokens.end();
}

} // namespace eclipse

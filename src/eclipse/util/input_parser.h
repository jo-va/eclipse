#pragma once

#include <string>
#include <vector>

namespace eclipse {

class InputParser
{
public:
    InputParser(int argc, char** argv);
    const std::string& get_option(const std::string& option) const;
    bool option_exists(const std::string& option) const;

private:
    std::vector<std::string> m_tokens;
};

} // namespace eclipse

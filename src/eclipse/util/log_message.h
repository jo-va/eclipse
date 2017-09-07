#pragma once

#include <string>
#include <sstream>
#include <chrono>

namespace eclipse {

enum LogLevel
{
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

struct LogMessage
{
    template <LogLevel msg_level, typename... Args>
    static LogMessage make(const std::string& name, Args const&... args)
    {
        LogMessage msg;
        msg.name = name;
        msg.level = msg_level;
        msg.timepoint  = std::chrono::high_resolution_clock::now();

        std::ostringstream oss;
        using List = int[];
        (void)List{ 0, ((void)(oss << args), 0) ... };
        msg.message = oss.str();

        msg.num = ++g_num;

        return msg;
    }

    std::string name;
    std::string message;
    std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;
    LogLevel level;
    unsigned int num;

    static unsigned int g_num;
};

} // namespace eclipse

#pragma once

#include <string>
#include <sstream>
#include <chrono>

namespace eclipse { namespace logging {

enum Level
{
    Debug,
    Information,
    Warning,
    Error
};

struct LogMessage
{
    template <Level msg_level, typename... Args>
    static LogMessage make(Args const&... args)
    {
        LogMessage msg;
        msg.level = msg_level;
        msg.timepoint  = std::chrono::high_resolution_clock::now();

        std::ostringstream oss;
        using List = int[];
        (void)List{ 0, ((void)(oss << args), 0) ... };
        msg.message = oss.str();

        return msg;
    }

    std::string message;
    std::chrono::time_point<std::chrono::high_resolution_clock> timepoint;
    Level level;
};

} } // namespace eclipse::logging

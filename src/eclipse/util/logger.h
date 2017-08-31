#pragma once

#include "eclipse/prerequisites.h"
#include "eclipse/util/log_message.h"

#include <string>
#include <cstdio>

namespace eclipse { namespace logging {

namespace details
{
    std::string format_message(const LogMessage& msg);
}

struct ConsolePolicy
{
    void open(const std::string& name)
    {
        (void)name;
    }

    void write(const LogMessage& message)
    {
        std::string fmt_msg = details::format_message(message);
        fprintf(stdout, "%s\n", fmt_msg.c_str());
        fflush(stdout);
    }

    void close() { }
};

template <typename LogPolicy>
class Logger
{
    public:
        Logger(const std::string& name = "")
        {
            _policy.open(name);
        }

        ~Logger()
        {
            _policy.close();
        }

        template <Level level, typename... Args>
        void post(Args const&... args)
        {
            LogMessage msg = LogMessage::make<level>(args...);
            _policy.write(msg);
        }

    private:
        LogPolicy _policy;
};

#ifdef ENABLE_LOG_DEBUG
    #define LOG_DEBUG eclipse::logging::instance.post<eclipse::logging::Debug>
#else
    #define LOG_DEBUG(...)
#endif

#ifdef ENABLE_LOG_INFO
    #define LOG_INFO eclipse::logging::instance.post<eclipse::logging::Information>
#else
    #define LOG_INFO(...)
#endif

#ifdef ENABLE_LOG_WARNING
    #define LOG_WARNING eclipse::logging::instance.post<eclipse::logging::Warning>
#else
    #define LOG_WARNING(...)
#endif

#ifdef ENABLE_LOG_ERROR
    #define LOG_ERROR eclipse::logging::instance.post<eclipse::logging::Error>
#else
    #define LOG_ERROR(...)
#endif

extern Logger<ConsolePolicy> instance;

} } // namespace eclipse::logging

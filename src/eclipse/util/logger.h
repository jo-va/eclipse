#pragma once

#include "eclipse/prerequisites.h"
#include "eclipse/util/log_message.h"

#include <string>
#include <memory>
#include <iostream>

namespace eclipse {

namespace details
{
    std::string format_message(const LogMessage& msg);
}

class LogPolicy
{
public:
    virtual void open() = 0;
    virtual void close() = 0;
    virtual void write(const std::string& msg) = 0;
};

struct ConsoleLogPolicy : public LogPolicy
{
    void open() override { }
    void close() override { }

    void write(const std::string& msg) override
    {
        std::cout << msg << "\n" << std::flush;
    }
};

class Logger
{
public:
    ~Logger()
    {
        m_policy->close();
    }

    template <LogLevel level, typename... Args>
    void log(Args const&... args)
    {
        if (level >= m_level)
        {
            LogMessage msg = LogMessage::make<level>(m_name, args...);
            std::string fmt_msg = details::format_message(msg);
            m_policy->write(fmt_msg);
        }
    }

    static Logger create(const std::string& name, std::shared_ptr<LogPolicy> policy = nullptr)
    {
        if (policy == nullptr)
            policy = std::make_shared<ConsoleLogPolicy>();
        return Logger(name, policy);
    }

private:
    Logger(const std::string& name, std::shared_ptr<LogPolicy> policy)
        : m_name(name), m_policy(policy)
    {
        m_policy->open();
    }

private:
    LogLevel m_level;
    std::string m_name;
    std::shared_ptr<LogPolicy> m_policy;
};

} // namespace eclipse

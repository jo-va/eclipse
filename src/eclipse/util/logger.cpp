#include "eclipse/util/logger.h"
#include "eclipse/util/log_message.h"
#include "eclipse/prerequisites.h"

#include <string>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>

namespace eclipse {

namespace details {

std::string format_message(const LogMessage& msg)
{
    std::ostringstream oss;

#ifdef LOG_COLOR
    oss << "\033[";
    switch (msg.level) {
    case DEBUG:
        oss << "1;32m"; break;
    case INFO:
        oss << "1;34m"; break;
    case WARNING:
        oss << "1;33m"; break;
    case ERROR:
        oss << "1;31m"; break;
    }
#endif

#ifdef LOG_TIMESTAMP
    using namespace std::chrono;
    std::time_t t = system_clock::to_time_t(msg.timepoint);
    auto millis = duration_cast<milliseconds>(msg.timepoint.time_since_epoch());
    uint64_t millis_remainder = millis.count() % 1000;

    oss << std::put_time(std::localtime(&t), "[%H:%M:%S.")
        << std::setfill('0') << std::setw(3) << millis_remainder << "] ";
#endif

    const char* levels[] = { "DEBUG", "INFO", "WARN", "ERROR" };
    oss << '[' << levels[(int)msg.level % 4] << "] " << std::setw(3) << msg.num << ". ";

#ifdef LOG_COLOR
    oss << "\033[0m";
#endif

    if (!msg.name.empty())
        oss << "[" << msg.name << "] ";

    oss << msg.message;

    return oss.str();
}

} // namespace details

} // namespace eclipse

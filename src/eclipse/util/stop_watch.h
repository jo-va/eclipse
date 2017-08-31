#pragma once

#include <time.h>

namespace eclipse {

class StopWatch
{
public:
    StopWatch();
    ~StopWatch();

    void start();
    void stop();

    double get_elapsed_time_s();
    double get_elapsed_time_ms();
    double get_elapsed_time_us();
    double get_elapsed_time_ns();

private:
    struct timespec m_start_count;
    struct timespec m_end_count;
    bool m_stopped;
};

} // namespace eclipse

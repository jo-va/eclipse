#include "eclipse/util/timer.h"

namespace eclipse {

Timer::Timer()
{
    m_stopped = false;
    m_start_count.tv_sec = m_start_count.tv_nsec = 0;
    m_end_count.tv_sec = m_end_count.tv_nsec = 0;
}

Timer::~Timer()
{
}

void Timer::start()
{
    m_stopped = false;
    clock_gettime(CLOCK_MONOTONIC, &m_start_count);
}

void Timer::stop()
{
    m_stopped = true;
    clock_gettime(CLOCK_MONOTONIC, &m_end_count);
}

double Timer::get_elapsed_time_s()
{
    return get_elapsed_time_ns() * 0.000000001;
}

double Timer::get_elapsed_time_ms()
{
    return get_elapsed_time_ns() * 0.000001;
}

double Timer::get_elapsed_time_us()
{
    return get_elapsed_time_ns() * 0.001;
}

double Timer::get_elapsed_time_ns()
{
    if (!m_stopped)
        clock_gettime(CLOCK_MONOTONIC, &m_end_count);

    double start_time_ns = m_start_count.tv_sec * 1000000000.0 + m_start_count.tv_nsec;
    double end_time_ns = m_end_count.tv_sec * 1000000000.0 + m_end_count.tv_nsec;

    return end_time_ns - start_time_ns;
}

} // namespace eclipse


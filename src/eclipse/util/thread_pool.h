#pragma once

#include "eclipse/util/mpmc_queue.h"

#include <vector>
#include <chrono>
#include <atomic>
#include <thread>
#include <future>
#include <functional>

namespace eclipse {

template <typename RetType>
class ThreadPool
{
public:
    explicit ThreadPool(int sleep_time = 50)
        : m_done(false), m_sleep_time(sleep_time)
    {
        int num_threads = std::thread::hardware_concurrency();
        num_threads = (num_threads == 0) ? 2 : num_threads;

        for (int i = 0; i < num_threads; ++i)
            m_threads.push_back(std::thread(&ThreadPool::thread_loop, this));
    }

    ~ThreadPool()
    {
        m_done = true;
        for (auto& t : m_threads)
            t.join();
    }

    std::future<RetType> submit(std::function<RetType()>&& f)
    {
        std::packaged_task<RetType()> task(std::move(f));
        auto future = task.get_future();
        m_work_queue.push(std::move(task));
        return future;
    }

    size_t size() const
    {
        return m_work_queue.size();
    }

    void set_sleep_time(int sleep_time)
    {
        m_sleep_time = sleep_time;
    }

private:
    void thread_loop()
    {
        std::packaged_task<RetType()> task;
        while (!m_done)
        {
            if (m_work_queue.try_pop(task))
                task();
            else
                std::this_thread::sleep_for(std::chrono::milliseconds(m_sleep_time));
        }
    }

private:
    MPMCQueue<std::packaged_task<RetType()>> m_work_queue;
    std::atomic_bool m_done;
    std::vector<std::thread> m_threads;
    int m_sleep_time;
};

} // namespace eclipse

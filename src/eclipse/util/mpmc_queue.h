#pragma once

#include <cstddef>
#include <queue>
#include <algorithm>
#include <utility>
#include <mutex>
#include <condition_variable>

namespace eclipse {

template <typename T>
class MPMCQueue
{
public:
    MPMCQueue() { }
    ~MPMCQueue() { }

    void push(const T& value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(value);
        m_cond.notify_one();
    }

    void push(T&& value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(std::move(value));
        m_cond.notify_one();
    }

    void wait_and_pop(T& value)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_cond.wait(lock, [this]() { return !m_queue.empty(); });
        value = m_queue.front();
        m_queue.pop();
    }

    bool try_pop(T& value)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_queue.empty())
            return false;
        value = std::move(m_queue.front());
        m_queue.pop();
        return true;
    }

    void clear()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        std::queue<T> empty;
        std::swap(m_queue, empty);
    }

    size_t size() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_queue.size();
    }

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
    std::queue<T> m_queue;
};

} // namespace eclipse

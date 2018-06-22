/******************************************************************************
Copyright (c) 2016. All Rights Reserved.

FileName: message_queue.h
Version: 1.0
Date: 2018.5.4

History:
eric        2018.5.4   1.0     Create
******************************************************************************/

#ifndef MESSAGE_QUEUE_H_
#define MESSAGE_QUEUE_H_

#include <deque>
#include <mutex>
#include <condition_variable>
#include "state.h"

namespace eco{;

template<typename Message>
class MessageQueue
{
public:
    enum {default_capacity = 5000 };

    inline MessageQueue(IN const uint32_t capacity = default_capacity)
    {
        set_capacity(capacity);
        open();
    }

    inline void set_capacity(IN const uint32_t capacity)
    {
        m_capacity = (capacity > 0) ? capacity : default_capacity;
    }

    inline void open()
    {
        m_state.ok();
    }

    inline const bool is_open() const
    {
        return m_state.is_ok();
    }

    inline void close()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        raw_close();
    }

    inline const bool is_close() const
    {
        return m_state.is_none();
    }

    inline void release()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_deque.clear();
        raw_close();
    }

    void post(IN Message& msg)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (is_full()) {
            m_full_cond_var.wait(lock);
        }
        raw_post(msg);
    }

    template<typename UniqueChecker>
    inline void post_unique(IN Message& msg, IN UniqueChecker& unique_check)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        auto it = std::find_if(m_deque.begin(), m_deque.end(), unique_check);
        if (it != m_deque.end())
        {
            *it = std::move(msg);
            return;
        }
        while (is_full()) {
            m_full_cond_var.wait(lock);
        }
        raw_post(msg);
    }

    inline const bool pop(OUT Message& msg)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (is_empty())
        {
            if (is_close())
            {
                return false;
            }
            m_empty_cond_var.wait(lock);
        }
        
        msg = std::move(m_deque.front());
        m_deque.pop_front();
        m_full_cond_var.notify_one();
        return true;
    }

    inline const bool empty() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_deque.empty();
    }

    inline const uint32_t size() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_deque.size();
    }


////////////////////////////////////////////////////////////////////////////////
private:
    inline void raw_close()
    {
        if (m_state.is_none()) {
            return;
        }
        m_state.none();
        m_empty_cond_var.notify_all();
        m_full_cond_var.notify_all();
    }

    inline void raw_post(IN Message& msg)
    {
        //m_deque.push_back(Message());
        //m_deque.back().swap(msg);
        //m_deque.push_back(std::move(msg));
        m_deque.push_back(msg);
        m_empty_cond_var.notify_one();
    }

    inline bool is_empty()
    {
        return m_deque.empty();
    }

    inline bool is_full()
    {
        return (m_deque.size() == m_capacity);
    }

    uint32_t m_capacity;
    std::deque<Message> m_deque;
    eco::atomic::State m_state;

    // when message queue is full and empty, synchronous notify.
    std::mutex m_mutex;
    std::condition_variable m_full_cond_var;
    std::condition_variable m_empty_cond_var;
};

}// ns.eco
#endif

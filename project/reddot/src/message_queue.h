#ifndef MESSAGE_QUEUE_H_
#define MESSAGE_QUEUE_H_

#include <deque>
#include <mutex>
#include <condition_variable>

#include "state.h"

namespace eco{;


////////////////////////////////////////////////////////////////////////////////
template<typename Message>
class MessageQueue
{
    ECO_OBJECT(MessageQueue)
////////////////////////////////////////////////////////////////////////////////
public:
    // max size.
    enum {default_capacity = 5000 };

    /*@ the message queue max message size.*/
    inline MessageQueue(IN const uint32_t capacity = default_capacity)
    {
        set_capacity(capacity);
        open();
    }

    /*@ set message queue capacity.*/
    inline void set_capacity(IN const uint32_t capacity)
    {
        m_capacity = (capacity > 0) ? capacity : default_capacity;
    }

    /*@ open message queue so that it can recv message.*/
    inline void open()
    {
        m_state.ok();
    }
    inline const bool is_open() const
    {
        return m_state.is_ok();
    }

    /*@ close message queue so that it stop to recv message. but it will wait 
    all message be handled.
    */
    inline void close()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        raw_close();
    }
    inline const bool is_close() const
    {
        return m_state.is_none();
    }

    /*@ release message queue so that it stop to recv message and clear message.*/
    inline void release()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_deque.clear();
        raw_close();
    }

    /*@ post message to message queue.
    * @ para.msg: message type is like "std::function", "std::shared_ptr",
    and some can be operated by "std::move()".
    */
    void post(IN Message& msg)
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        while (is_full()) {
            m_full_cond_var.wait(lock);
        }
        raw_post(msg);
    }

    /*@ post message to message queue and ensure that this message is unique.
    if this message is exist, replace it.
    */
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

    /*@ pop message from this message queue.*/
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

    // is message queue empty.
    inline const bool empty() const
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        return m_deque.empty();
    }

    // is message queue empty.
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
        // notify all thread to exit message queue.
        m_state.none();
        m_empty_cond_var.notify_all();
        m_full_cond_var.notify_all();
    }

    inline void raw_post(IN Message& msg)
    {
        //m_deque.push_back(message());
        //m_deque.back().swap(msg);
        m_deque.push_back(std::move(msg));
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

    //  message queue max size.
    uint32_t m_capacity;

    // message queue data.
    std::deque<Message> m_deque;

    // message queue state.
    eco::atomic::State m_state;

    // when message queue is full and empty, synchronous notify.
    std::mutex m_mutex;
    std::condition_variable m_full_cond_var;
    std::condition_variable m_empty_cond_var;
};


////////////////////////////////////////////////////////////////////////////////
}// ns.eco
#endif
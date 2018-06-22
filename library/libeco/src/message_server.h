/******************************************************************************
Copyright (c) 2016. All Rights Reserved.

FileName: message_server.h
Version: 1.0
Date: 2018.5.4

History:
eric        2018.5.4   1.0     Create
******************************************************************************/

#ifndef MESSAGE_SERVER_H_
#define MESSAGE_SERVER_H_

#include "message_queue.h"
#include "thread_pool.h"

namespace eco{;

template<
    typename Message,
    typename Handler = std::function<void (Message&)> >
class MessageServer
{
public:
    typedef MessageServer<Message, Handler> MessageServerT;

public:
    MessageServer()
    {}

    ~MessageServer()
    {
        stop();
    }

    template<typename T>
    void set_message_handler(IN T h)
    {
        m_message_handler = h;
    }

    inline Handler& message_handler()
    {
        return m_message_handler;
    }

    inline void set_capacity(IN const uint32_t capacity)
    {
        m_message_queue.set_capacity(capacity);
    }

    void run(
        IN uint32_t thread_size = 1,
        IN const char* name = nullptr)
    {
        if (thread_size == 0) {
            thread_size = 1;
        }
        m_message_queue.open();
        m_thread_pool.run(std::bind(&MessageServerT::work, this),
            thread_size);
    }

    inline void join()
    {
        m_thread_pool.join();
    }

    void stop()
    {
        m_message_queue.close();
        join();
    }

    void release()
    {
        m_message_queue.release();
        join();
    }

    inline const MessageQueue<Message>& get_queue() const
    {
        return m_message_queue;
    }

    inline void post(IN Message& msg)
    {
        m_message_queue.post(msg);
    }
    template<typename UniqueChecker>
    inline void post_unique(IN Message& msg, IN UniqueChecker& unique_check)
    {
        m_message_queue.post_unique(msg, unique_check);
    }

    inline void post_value(IN Message msg)
    {
        m_message_queue.post(msg);
    }

    template<typename UniqueChecker>
    inline void post_unique_value(
        IN Message msg, IN UniqueChecker& unique_check)
    {
        m_message_queue.post_unique(msg, unique_check);
    }

protected:
    void work()
    {
        while (true) {
            Message msg;
            if (!m_message_queue.pop(msg)) {
                break;  // message queue has been closed.
            }
            try {
                m_message_handler(msg);
            }
            catch (std::exception& e) {
                std::cout << "message server: " << e.what() << std::endl;
            }
        }// end while
    }

    MessageQueue<Message> m_message_queue;
    Handler m_message_handler;
    ThreadPool m_thread_pool;
};

}
#endif

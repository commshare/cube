/******************************************************************************
Copyright (c) 2016. All Rights Reserved.

FileName: dispatch_server.h
Version: 1.0
Date: 2018.5.4

History:
eric        2018.5.4   1.0     Create
******************************************************************************/


#ifndef DISPATCH_SERVER_H_
#define DISPATCH_SERVER_H_

#include "message_server.h"
#include <unordered_map>


namespace eco{;

template<typename MessageType, typename Message>
class DispatchHandler
{
public:
    typedef std::function<void(IN Message&)> HandlerFunc;
    typedef std::unordered_map<MessageType, HandlerFunc> HandlerMap;

    HandlerMap  m_handler_map;
    HandlerFunc m_default_handler;

public:
    DispatchHandler() : m_default_handler(nullptr)
    {}

    inline void operator()(IN Message& msg) const
    {
        dispatch((MessageType)msg["type"].AnyCast<int>(), msg);
    }

    inline void dispatch(IN const MessageType& type, IN Message& msg) const
    {
        auto it = m_handler_map.find(type);
        if (it != m_handler_map.end()) {
            it->second(msg);
        }
        else if (m_default_handler) {
            m_default_handler(msg);
        }
        else {
            std::cout << "dispatch unknown message type: " << type << std::endl;
        }
    }

    void set_dispatch(IN const MessageType& type, IN HandlerFunc& handler)
    {
        m_handler_map[type] = handler;
    }

    void set_default(IN HandlerFunc& handler)
    {
        m_default_handler = handler;
    }
};


////////////////////////////////////////////////////////////////////////////////
template<typename MessageType, typename Message>
class DispatchServer :
    public MessageServer<Message, DispatchHandler<MessageType, Message> >
{
public:
    typedef DispatchHandler<MessageType, Message> ThisType;
    typedef typename ThisType::HandlerFunc HandlerFunc;

    void set_dispatch(IN const MessageType& type, IN HandlerFunc func)
    {
        this->message_handler().set_dispatch(type, func);
    }

    void set_default(IN HandlerFunc func)
    {
        this->message_handler().set_default(func);
    }
};

}
#endif

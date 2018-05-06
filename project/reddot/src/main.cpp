/******************************************************************************
Copyright (c) 2016. All Rights Reserved.

FileName: main.cpp
Version: 1.0
Date: 2016.1.13

History:
eric     2018.4.27   1.0     Create
******************************************************************************/
#include <iostream>
#include <sstream>
#include <string>
#include <set>
#include <functional>
#include <chrono>
#include <thread>

#include "project.h"
#include "dispatch_server.h"
#include "protocol.h"
#include "monitor.h"
#include "event.h"
#include "memory_pool.h"
#include "any.h"

using namespace std;

typedef std::map<std::string, eco::Any> MessagePro;
class Test
{
    ECO_VALUE_API(Test)
public:
    Test& set(IN const char* str);
    const char* get_str() const;
};

class Test::Impl
{
    ECO_IMPL_INIT(Test)
public:
    std::string data_;
public:
    void set_data(IN const char* str)
    {
        data_ = str;
    }
};

ECO_VALUE_IMPL(Test);
Test& Test::set(IN const char* str)
{
    m_impl->set_data(str);
    return *this;
}

const char* Test::get_str() const 
{
    cout << __FUNCTION__ << endl;
    return m_impl->data_.c_str();
}

void fun_default(IN eco::MessageMeta& msg)
{
    std::cout << __FUNCTION__  
        <<" session_id:" << msg.get_session_id()
        << " message_type:" << msg.get_type()
        << " data:" << msg.get_request_data()
        << std::endl;
}

void fun_eco(IN eco::MessageMeta& msg)
{
    std::cout << __FUNCTION__ 
        << " session_id:" << msg.get_session_id()
        << " message_type:" << msg.get_type()
        << " data:" << msg.get_request_data()
        << std::endl;
}

class MyClass
{
    ECO_SINGLETON_UNINIT(MyClass)
    MyClass() {}
public:
    ~MyClass() {};

    void test()
    {
        EcoCout << "hello world";
    }
private:

};
ECO_SINGLETON_GET(MyClass)


template <typename T>
class Templ
{
public:
    void  test(T val)
    {
        cout << __FUNCTION__ << val << endl;
    }

    template <typename Object>
    void  test(Object val)
    {
        cout << __FUNCTION__ << val << endl;
    }
};

void fun_any_default(IN MessagePro& msg)
{
    std::cout << __FUNCTION__ << std::endl;
    cout << msg["type"].AnyCast<int>() << endl;
    cout << msg["two"].AnyCast<double>() << endl;
    cout << msg["three"].AnyCast<const char*>() << endl;
    cout << msg["four"].AnyCast<string>() << endl;
}

void fun_any_eco(IN MessagePro& msg)
{
    std::cout << __FUNCTION__ << std::endl;
    cout << msg["type"].AnyCast<int>() << endl;
    cout << msg["two"].AnyCast<double>() << endl;
    cout << msg["three"].AnyCast<const char*>() << endl;
    cout << msg["four"].AnyCast<string>() << endl;

    vector<int> tmp = msg["five"].AnyCast< vector<int> >();
    for (auto it = tmp.begin(); it != tmp.end(); ++it) {
        cout << *it << endl;
    }
}

int main(int argc, char **argv)
{
    cout << "hello reddot" << endl;


    eco::DispatchServer < uint32_t, std::map<std::string, eco::Any> > dispatch;
    dispatch.set_default(fun_any_default);
    dispatch.set_dispatch(10001, fun_any_eco);
    dispatch.run(1);

    for (int i = 0; i < 10; ++i)
    {
        MessagePro msg;
        msg["type"] = 10001+i;
        msg["two"] = 1.2;
        msg["three"] = "hello";
        msg["four"] = string("word");
        vector<int> two = { 1, 2, 3, 4, 5 };
        msg["five"] = two;
        dispatch.post(msg);
    }

    //eco::DispatchServer<uint32_t, eco::MessageMeta> dispatch;
    //dispatch.set_default(fun_default);
    //dispatch.set_dispatch(10001, fun_eco);
    //dispatch.run(1);

    //for (int i = 0; i < 10; ++i)
    //{
    //    eco::MessageMeta msg(i, 10001+i, "hello world");
    //    dispatch.post(msg);
    //}


    getchar();
    return 0;
}

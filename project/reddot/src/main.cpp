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
#include "file.h"
#include "lexical_cast.h"

using namespace std;

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
        <<" session_id:" << msg.get_session()
        << " message_type:" << msg.get_type()
        << " data:" << msg.get_data()
        << std::endl;
}

void fun_eco(IN eco::MessageMeta& msg)
{
    std::cout << __FUNCTION__
        << " session_id:" << msg.get_session()
        << " message_type:" << msg.get_type()
        << " data:" << msg.get_data()
        << std::endl;
}

void test()
{
    cout << eco::lexical_cast<int>(1) << endl;
    cout << eco::lexical_cast<int>("1") << endl;
    cout << eco::lexical_cast<long>("1") << endl;
    cout << eco::lexical_cast<string>(1) << endl;
    cout << eco::lexical_cast<bool>(2) << endl;
    cout << eco::lexical_cast<double>("1.2") << endl;
    cout << eco::lexical_cast<float>("1.2") << endl;
    string s = "true";
    cout << eco::lexical_cast<bool>(s) << endl;
    char* p = "false";
    cout << eco::lexical_cast<bool>(p) << endl;
    const char* q = "false";
    cout << eco::lexical_cast<bool>(q) << endl;
    cout << eco::lexical_cast<bool>("false") << endl;
    cout << eco::lexical_cast<bool>("test") << endl;
}

int main(int argc, char **argv)
{
    cout << "hello reddot" << endl;

    try
    {
        test();
    }
    catch (const std::exception& e)
    {
        cout << e.what() << endl;
    }

    //eco::DispatchServer<uint32_t, eco::MessageMeta> dispatch;
    //dispatch.set_default(fun_default);
    //dispatch.set_dispatch(10001, fun_eco);
    //dispatch.run(4);
    //std::this_thread::sleep_for(std::chrono::seconds(3));
    //for (int i = 0; i < 5; ++i)
    //{
    //    eco::MessageMeta msg(i, 10001+i, "hello world");
    //    dispatch.post(msg);
    //}

    getchar();
    return 0;
}

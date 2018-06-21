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

#include "libeco/project.h"
#include "libeco/dispatch_server.h"
#include "libeco/protocol.h"
#include "libeco/monitor.h"
#include "libeco/event.h"
#include "libeco/memory_pool.h"
#include "libeco/file.h"
#include "libeco/lexical_cast.h"
#include "libeco/any.h"

using namespace std;

using protocal_struct = std::map<std::string, eco::Any>;

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

void fun_default(IN protocal_struct& msg)
{
    std::cout << __FUNCTION__
        << " message_type:" << msg["type"].AnyCast<int>()
        << " sessionid:" << msg["sessionid"].AnyCast<int>()
        << " price:" << msg["price"].AnyCast<double>()
        << " data:" << msg["data"].AnyCast<const char*>()
        << std::endl;
}

void fun_eco(IN protocal_struct& msg)
{
    std::cout << __FUNCTION__
        << " message_type:" << msg["type"].AnyCast<int>()
        << " sessionid:" << msg["sessionid"].AnyCast<int>()
        << " price:" << msg["price"].AnyCast<double>()
        << " data:" << msg["data"].AnyCast<const char*>()
        << std::endl;
}

template <typename T, typename... argv>
int print(T first, argv... rest)
{
    return first + print(std::forward<argv>(rest)...);
}

template <typename T>
int print(T value)
{
    return value;
}

int main(int argc, char **argv)
{
    cout << "hello reddot" << endl;
	std::string str = "hello'world''hehe'";
	cout << str << endl;
	eco::replace_all(str, "'", "''");
	cout << str << endl;

    
    eco::DispatchServer<uint32_t, protocal_struct> dispatch;
    dispatch.set_default(fun_default);
    dispatch.set_dispatch(10001, fun_eco);
    dispatch.run(1);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    for (int i = 0; i < 5; ++i)
    {
        std::map<std::string, eco::Any> msg;
        msg["type"] = 10000 + i;
        msg["sessionid"] = 100;
        msg["price"] = 99.999;
        msg["data"] = "hello world";
        dispatch.post(msg);
    }

    getchar();
    return 0;
}

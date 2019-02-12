#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <string.h>

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <memory>
#include <vector>
#include <set>
#include <queue>
#include <stack>
#include <deque>
#include <map>
#include <unordered_map>
#include <functional>
#include <numeric>
#include <thread>
#include <chrono>
#include <mutex>
#include <exception>
#include <iomanip>
#include <algorithm>
#include <utility>
#include <tuple>
#include <chrono>
#include <random>
#include <time.h>
#include <type_traits>
#include <utility>
#include <stdexcept>
#include <typeindex>
#include <exception>

class MyClass
{
public:
    MyClass(int i, double j);

    ~MyClass() 
    {
        std::cout << __FUNCTION__ << std::endl;
    };

    void test() const
    {
        std::cout << "hello world" << std::endl;
    }
private:
    int num;
};

MyClass::MyClass(int i, double j)
{
    std::cout << __FUNCTION__ << std::endl;
    num = i;
}

template <typename T1, typename T2, 
    template<typename T, typename = std::allocator<T> > class Container>
class Relation
{
private:
    Container<T1> dom1;
    Container<T2> dom2;
};

int main(int argc, char *argv[])
{
    Relation<int, double, std::list> relation;

    std::vector<char> sql;
    sql.resize(1024);
    snprintf(sql.data(), 1024, "hello world");
    std::cout << sql.data() << std::endl;

    std::chrono::time_point<std::chrono::system_clock> begin_clock = std::chrono::system_clock::now();

    std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
    std::time_t time = std::chrono::system_clock::to_time_t(now +
        std::chrono::hours(24 * 5));
    struct tm ti = *std::localtime(&time);

    char date_time_buf[32];
    sprintf(date_time_buf, "%04d-%02d-%02d %02d:%02d:%02d", 1900 + ti.tm_year,
        ti.tm_mon + 1, ti.tm_mday, ti.tm_hour, ti.tm_min, ti.tm_sec);

    std::cout << date_time_buf << std::endl;

    auto end_clock = std::chrono::system_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::microseconds>
        (end_clock - begin_clock).count() << std::endl;

    getchar();
    return 0;
}
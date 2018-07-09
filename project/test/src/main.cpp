#include <stdio.h>
#include <assert.h>
#include <signal.h>
#include <string.h>

#include <iostream>
#include <string>
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
using namespace std;

class MyClass
{
public:
    MyClass(int i = 100);

    ~MyClass() 
    {
        cout << __FUNCTION__ << endl;
    };

    void test()
    {
        cout << "hello world" << endl;
    }
private:

};

MyClass::MyClass(int i)
{
    cout << __FUNCTION__ << endl;
    cout << i << endl;
}

template <typename Derived>
class Strategy
{
public:
    Strategy(void)
    {
        cout << "Strategy....." << endl;
    }
    virtual ~Strategy()
    {
        cout << "~Strategy....." << endl;
    }

    //由于虚函数不能是模板函数，故使用c++ template多态的CRTP 模式
    template<typename ...Args>
    void VirAlgrithmInterface(Args...args)
    {
        static_cast<Derived&>(*this).AlgrithmInterface(args...);
    }

    template<typename ...Args>
    void AlgrithmInterface(Args...args)
    {
        std::cout << "Strategy::AlgrithmInterface\n";
    }

protected:
private:
};

template <typename T>
class Test
{
public:
    Test(const Strategy<T>& t)
    : inst(t)
    {}

    void test(int a, int b)
    {
        inst.VirAlgrithmInterface(a, b);
    }

private:
    Strategy<T> inst;
};

class ConcreteStrategyA : public Strategy<ConcreteStrategyA>
{
public:
    ConcreteStrategyA()
    {
        cout << "ConcreteStrategyA....." << endl;
    }
    virtual ~ConcreteStrategyA()
    {
        cout << "~ConcreteStrategyA....." << endl;
    }

    void AlgrithmInterface(int a, int b)
    {
        cout << "testConcreteStrategyA....." << a << ", " << b << endl;
    }

protected:
private:
};

int main(int argc, char *argv[])
{
    std::string date = "SHFE,CFFEX,DCE,CZCE,INE";
    if (date.find("INE") == std::string::npos) {
        cout << "hello" << endl;
    }


    ConcreteStrategyA inst;
    Test<ConcreteStrategyA> one(inst);
    one.test(1, 2);

    getchar();
    return 0;
}
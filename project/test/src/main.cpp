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

int main(int argc, char *argv[])
{
    MyClass inst;

    getchar();
    return 0;
}
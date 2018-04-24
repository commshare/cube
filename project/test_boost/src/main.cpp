#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <map>
#include <list>
#include "test_strand.h"
#include "test_deadline_timer.h"
#include "test_log.h"
#include "test_base.h"
#include "alarm.h"
#include "timer_manager.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp> 
void test_strand1(int argc, char* argv[])
{
    test_strand tt;

    //tt.test_strands();
    tt.test_service();
    getchar(); //换行符
    getchar(); 
}

void test_deadline_timer1(int argc, char* argv[])
{
    test_deadline_timer tt;

    //tt.test_timer_syn();
    tt.test_timer_asyn();
    //tt.test_timer_asyn_loop();
    getchar(); //换行符
    getchar();
}

void test_log1(int argc, char* argv[])
{
    test_log tt;
    tt.test();
    getchar(); //换行符
    getchar();
}

void TestBase1(int argc, char* argv[])
{
    TestBase tt;
    tt.Test();
    getchar(); //换行符
    getchar();
}

void test()
{
    cout << "hello world" << endl;
}

void TestTimer(int argc, char* argv[])
{
    alarm inst(9, 14, 0);
    //TimerManager::get_timer_manager().Register(5000, test);
    TimerManager::get_timer_manager().Register("11:11:00", test);
    TimerManager::get_timer_manager().start();
    getchar();
    getchar();
}

int main(int argc, char* argv[])
{
    //simple test
    cout << "现在时间：" 
        << boost::posix_time::to_iso_string(boost::posix_time::microsec_clock::local_time()) << endl;

    int ch = '1';
    do{
        printf("the list: \n");
        printf("0: exit \n");
        printf("1: test strand \n");
        printf("2: test deadline_timer \n");
        printf("3: test log \n");
        printf("4: test base \n");
        printf("5: test timer \n");
        printf("please select your decide: ");
        ch = getchar();
        switch (ch) {
        case '0':
            printf("exit OK ~~\n");
            break;
        case '1':
            test_strand1(argc, argv);
            break;
        case '2':
            test_deadline_timer1(argc, argv);
            break;
        case '3':
            test_log1(argc, argv);
            break;
        case '4':
            TestBase1(argc, argv);
            break;
        case '5':
            TestTimer(argc, argv);
            break;
        default:
            printf("please input right decide~~\n");
            break;
        }
    } while (ch != '0');

    getchar();
    getchar();
    return 0;
}

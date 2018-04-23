/******************************************************************************
Copyright (c) 2016. All Rights Reserved.

FileName: event.h
Version: 1.0
Date: 2016.1.13

History:
eric        2018.4.9   1.0     Create
******************************************************************************/

#ifndef EVENT_H_
#define EVENT_H_

#ifdef _MSC_VER
#include <WinSock2.h>
#include <Windows.h>
#else
#include <pthread.h>
#endif

namespace test_proto
{
class Event
{

public:
    Event();
    ~Event();

    void SignalEvent();
    void WaitEvent();
    void Reset();
    void Pulse();

private:

#ifdef _MSC_VER
    HANDLE handle_;
#else
    pthread_cond_t cond;
    pthread_mutex_t mutex;
    bool bIsSignal;
#endif

};
}
#endif

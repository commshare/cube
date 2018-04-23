/******************************************************************************
Copyright (c) 2016. All Rights Reserved.

FileName: client.h
Version: 1.0
Date: 2016.1.13

History:
eric        2018.4.9   1.0     Create
******************************************************************************/

#ifndef CLIENT_H_
#define CLIENT_H_

#include <string>
#include "liblight/transaction.h"
#include "liblight/clientreceivehandle.h"
#include "liblight/clientapi.h"
#include "event.h"

namespace test_proto
{

class Client
    : public light::clientreceivehandle
{
public:
    virtual void receive(const light::transaction_head& head, const char* data);
    virtual void onfrontconnected();
    virtual void onfrontdisconnected(int reason);

public:
    Client();
    virtual ~Client();
    void Start();
    void Send(int transactionid, const std::string& msg);

private:
    Event event_;
    std::shared_ptr<light::clientapi> clientptr_;

};

}
#endif

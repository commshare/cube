/******************************************************************************
Copyright (c) 2016. All Rights Reserved.

FileName: client.cpp
Version: 1.0
Date: 2016.1.13

History:
eric        2018.4.9   1.0     Create
******************************************************************************/

#include "client.h"
#include <iostream>
#include "msgtype_def.h"
#include "../proto/manager_protocal.pb.h"
#include "../proto/riskapi_data_protocal.pb.h"

using namespace std;

namespace test_proto
{

void Client::receive(const light::transaction_head& head, const char* data)
{
    std::cout << "receive\n";
    switch (head.transaction_type_)
    {
    case common::TYPE_MANAGER_LOGIN_RSP:
    {
        manager::rsp_login message;
        message.ParseFromString(data);
        
        cout << "errorid:" << message.header().info().errorid()
            << " errormsg:" << message.header().info().errormsg() << endl;

        for (int i = 0; i < message.auth_size(); ++i) {
            for (int j = 0; j < message.auth(i).auth_size(); ++j) {
                cout << "roleid:" << message.auth(i).roleid()
                    << " authid:" << message.auth(i).auth(j).authid()
                    << " name:" << message.auth(i).auth(j).name()
                    << endl;
            }
        }
    }
    break;
    case common::TYPE_MANAGER_QUERY_USERINFO_RSP:
    {
        manager::rsp_sys_subs message;
        message.ParseFromString(data);

        cout << "errorid:" << message.header().info().errorid()
            << " errormsg:" << message.header().info().errormsg() << endl;

        for (int i = 0; i < message.info_size(); ++i) {
            cout << "id:" << message.info(i).id()
                << " userid:" << message.info(i).userid()
                << " username:" << message.info(i).username()
                << " father:" << message.info(i).father()
                << " roleid:" << message.info(i).roleid()
                << " pwd:" << message.info(i).pwd()
                << endl;
        }
    }
    break;
    case common::TYPE_MANAGER_ADD_USER_RSP:
    {
        manager::rsp_action_info message;
        message.ParseFromString(data);

        cout << "errorid:" << message.info().errorid()
            << " errormsg:" << message.info().errormsg() << endl;
    }
    break;
    case common::TYPE_MANAGER_QUERY_STATE_INVESTOR_RSP:
    {
        manager::rsp_investors message;
        message.ParseFromString(data);

        for (int i = 0; i < message.investors_size(); ++i)
        {
            cout << "investor:" << message.investors(i).investor() << endl;
        }
    }
    break;
    case common::TYPE_MANAGER_QUERY_RISKER_INVESTOR_RSP:
    {
        manager::rsp_risk_investors message;
        message.ParseFromString(data);

        for (int i = 0; i < message.investors_size(); ++i)
        {
            cout << "userid:" << message.userid() 
                << "investor:" << message.investors(i).investor() << endl;
        }
    }
    break;
    case common::TYPE_MANAGER_ADD_INVESTOR_RSP:
    {
        manager::rsp_action_info message;
        message.ParseFromString(data);

        cout << "errorid:" << message.info().errorid()
            << " errormsg:" << message.info().errormsg() << endl;
    }
    break;
    case common::TYPE_MANAGER_DEL_INVESTOR_RSP:
    {
        manager::rsp_action_info message;
        message.ParseFromString(data);

        cout << "errorid:" << message.info().errorid()
            << " errormsg:" << message.info().errormsg() << endl;
    }
    break;
    case common::TYPE_MANAGER_QUERY_INSMRGNRATE_RSP:
    {
        manager::rsp_insmrgnrate message;
        message.ParseFromString(data);

        for (int i = 0; i < message.infos_size(); ++i)
        {
            cout << "contractcode:" << message.infos(i).contractcode()
                << "marginrate:" << message.infos(i).marginrate() << endl;
        }
    }
    break;
    case common::TYPE_MANAGER_EDIT_INSMRGNRATE_RSP:
    {
        manager::rsp_action_info message;
        message.ParseFromString(data);

        cout << "errorid:" << message.info().errorid()
            << " errormsg:" << message.info().errormsg() << endl;
    }
    break;
    case common::TYPE_RISKAPI_ADD_RISKGROUP_RSP:
    {
        manager::rsp_action_info message;
        message.ParseFromString(data);

        cout << "errorid:" << message.info().errorid()
            << " errormsg:" << message.info().errormsg() << endl;
    }
    break;
    case common::TYPE_RISKAPI_DEL_RISKGROUP_RSP:
    {
        manager::rsp_action_info message;
        message.ParseFromString(data);

        cout << "errorid:" << message.info().errorid()
            << " errormsg:" << message.info().errormsg() << endl;
    }
    case common::TYPE_RISKAPI_QUERY_RISKGROUP_RSP:
    {
        risk::rsp_groups message;
        message.ParseFromString(data);

        for (int i = 0; i < message.infos_size(); ++i)
        {
            cout << "riskgroupid:" << message.infos(i).riskgroupid()
                << "name:" << message.infos(i).name() << endl;
        }
    }
    break;
    default:
        break;
    }
}

void Client::onfrontconnected()
{
    std::cout << "onfrontconnected\n";
    event_.SignalEvent();
}

void Client::onfrontdisconnected(int reason)
{
    std::cout << "onfrontdisconnected\n";
}

Client::Client()
{
}

Client::~Client()
{
}

void Client::Start()
{
    //connect trialserver
    boost::asio::ip::tcp::endpoint ed(
            boost::asio::ip::address::from_string("120.136.160.83"),
        16270);
    clientptr_ = std::make_shared<light::clientapi>(ed);
    clientptr_->set_handle(this);
    clientptr_->run();
    event_.WaitEvent();

    cout << "client start successfully!" << endl;
}

void Client::Send(int transactionid, const std::string& msg)
{
    light::transaction_head head;
    head.transaction_type_ = transactionid;
    head.body_length_ = msg.size();

    clientptr_->send(head, msg.c_str());
}

}

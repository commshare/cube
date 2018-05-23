
#include <string.h>
#include <iostream>
#include <string>
#include "../proto/manager_protocal.pb.h"
#include "../proto/riskapi_action_protocal.pb.h"
#include "client.h"
#include "msgtype_def.h"

using namespace std;
using namespace test_proto;

Client client;

void test_manager_login()
{
    manager::req_login req_message;
    req_message.set_requestid(10000);
    req_message.set_userid("9999");
    req_message.set_pwd("111111");

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_MANAGER_LOGIN_REQ, reply);
}

void test_manager_subs()
{
    manager::req_risk_investors req_message;
    req_message.set_requestid(10000);
    req_message.set_userid("9999");

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_MANAGER_QUERY_USERINFO_REQ, reply);
}

void test_manager_adduser()
{
    manager::req_add_sys_user req_message;
    req_message.set_requestid(10000);
    req_message.mutable_user()->set_userid("9999");
    req_message.mutable_user()->set_father("9999");
    req_message.mutable_user()->set_roleid(2);

    for (size_t i = 0; i < 4; i++)
    {
        manager::sys_auth *auth = req_message.mutable_user()->add_auths();
        auth->set_authid(i+4);
        auth->set_name("add");
    }

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_MANAGER_ADD_USER_REQ, reply);
}

void test_manager_deluser()
{
    std::string userid;

    cout << "input deluser userid£º ";
    cin >> userid;
    manager::req_del_sys_user req_message;
    req_message.set_requestid(10000);
    req_message.set_userid(userid);

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_MANAGER_DEL_USER_REQ, reply);
}

void test_manager_req_investor()
{
    int userid = -1;

    manager::req_investors req_message;
    req_message.set_requestid(10000);

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_MANAGER_QUERY_STATE_INVESTOR_REQ, reply);
}

void test_manager_req_riskinvestor()
{
    std::string userid;

    cout << "input userid£º ";
    cin >> userid;
    manager::req_risk_investors req_message;
    req_message.set_requestid(10000);
    req_message.set_userid(userid);

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_MANAGER_QUERY_RISKER_INVESTOR_REQ, reply);
}

void test_manager_addinvestor()
{
    std::string userid;
    cout << "input addinvestor userid£º ";
    cin >> userid;

    manager::req_add_investors req_message;
    req_message.set_requestid(10000);
    req_message.set_userid(userid);
    req_message.add_investor("028601270413");
    req_message.add_investor("028601322592");

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_MANAGER_ADD_INVESTOR_REQ, reply);
}

void test_manager_delinvestor()
{
    std::string userid;
    cout << "input delinvestor userid£º ";
    cin >> userid;

    manager::req_del_investors req_message;
    req_message.set_requestid(10000);
    req_message.set_userid(userid);
    req_message.add_investor("028601270413");
    req_message.add_investor("028601322592");

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_MANAGER_DEL_INVESTOR_REQ, reply);
}

void test_manager_queryrate()
{
    std::string userid;

    manager::req_insmrgnrate req_message;
    req_message.set_requestid(10000);

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_MANAGER_QUERY_INSMRGNRATE_REQ, reply);
}

void test_manager_editrate()
{
    std::string userid;

    manager::req_edit_insmrgnrate req_message;
    req_message.set_requestid(10000);
    manager::insmrgnrate_info *info = req_message.add_infos();
    info->set_tradingday("20180420");
    info->set_contractcode("BU1806");
    info->set_direction("1");
    info->set_hedgeflag("*");
    info->set_marginrate(0.08);

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_MANAGER_EDIT_INSMRGNRATE_REQ, reply);
}

void test_riskapi_addgroup()
{
    std::string userid;

    cout << "input addgroup userid£º ";
    cin >> userid;
    risk::req_add_group req_message;
    req_message.set_requestid(10000);
    req_message.set_userid(userid);
    req_message.mutable_group()->set_riskgroupid("1234");

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_RISKAPI_ADD_RISKGROUP_REQ, reply);
}

void test_riskapi_delgroup()
{
    std::string userid;

    cout << "input delgroup userid£º ";
    cin >> userid;
    risk::req_del_group req_message;
    req_message.set_requestid(10000);
    req_message.set_userid(userid);
    req_message.set_riskgroupid("1234");

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_RISKAPI_DEL_RISKGROUP_REQ, reply);
}

void test_riskapi_querygroup()
{
    std::string userid;

    cout << "input querygroup userid£º ";
    cin >> userid;
    risk::req_groups req_message;
    req_message.set_requestid(10000);
    req_message.set_userid(userid);

    std::string reply;
    req_message.AppendToString(&reply);
    client.Send(common::TYPE_RISKAPI_QUERY_RISKGROUP_REQ, reply);
}

int main(int argc, char *argv[])
{
    client.Start();

    int ch = '1';
    do {
        printf("the list: \n");
        printf("0: exit \n");
        printf("1: test manager login \n");
        printf("2: test manager subs \n");
        printf("3: test manager adduser \n");
        printf("4: test manager deluser \n");
        printf("5: test manager reqinvestor \n");
        printf("6: test manager reqriskinvestor \n");
        printf("7: test manager addinvestor \n");
        printf("8: test manager delinvestor \n");
        printf("9: test manager queryrate \n");
        printf("a: test manager editrate \n");
        printf("z: test riskapi addgroup \n");
        printf("y: test riskapi delgroup \n");
        printf("x: test riskapi querygroup \n");
        printf("please select your decide: ");
        ch = getchar();
        switch (ch) {
        case '0':
            printf("exit OK ~~\n");
            break;
        case '1':
            test_manager_login();
            break;
        case '2':
            test_manager_subs();
            break;
        case '3':
            test_manager_adduser();
            break;
        case '4':
            test_manager_deluser();
            break;
        case '5':
            test_manager_req_investor();
            break;
        case '6':
            test_manager_req_riskinvestor();
            break;
        case '7':
            test_manager_addinvestor();
            break;
        case '8':
            test_manager_delinvestor();
            break;
        case '9':
            test_manager_queryrate();
            break;
        case 'a':
            test_manager_editrate();
            break;
        case 'z':
            test_riskapi_addgroup();
            break;
        case 'y':
            test_riskapi_delgroup();
            break;
        case 'x':
            test_riskapi_querygroup();
            break;
        default:
            printf("please input right decide!");
            break;
        }
    //} while (0);
    } while (ch != '0');

    getchar();
    getchar();

    return 0;
}
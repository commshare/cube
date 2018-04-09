/*****************************************************************************
Nautilus Module ctp_trade Copyright (c) 2016. All Rights Reserved.

FileName: trade_unit.h
Version: 1.0
Date: 2017.08.08

History:
eric     2017.08.08   1.0     Create
******************************************************************************/

#include "trade_unit.h"
#include "common.h"
#include "trade_struct.h"
#include "trade_server.h"
#include "base/trace.h"
#include "base/util.h"
#include "base/base.h"
#include "base/file.h"
#include "database/unidbpool.h"

namespace ctp
{

trade_unit::trade_unit() 
    : mdpt_(NULL)
    , tserver_(NULL)
    , trader_(NULL)
    , connected_(false)
    , logined_(false)
    , m_last_respond(base::util::clock())
    , m_conn_state(cs_init)
    , msg_event_( NULL)
    , state_event_(NULL)
    , started_(false)
    , m_RequestId_(0)
    , m_FrontID_(0)
    , m_SessionID_(0)
    , m_stockhold_old_buy_(0)
    , m_stockhold_old_sell_(0)
    , m_last_qryacc_finish(true)
{
}

trade_unit::~trade_unit()
{
    stop();
}

int trade_unit::start(const trade_unit_params& params, message_dispatcher* mdpt,
        trade_server* tserver)
{
    assert(mdpt != NULL);

    if (started_) {
        return NAUT_AT_S_OK;
    }

    params_ = params;
    mdpt_ = mdpt;
    tserver_ = tserver;

    userid_ = params_.userid;
    account_ = params_.account;
    broker_ = params_.broker;
    trade_pwd_ = params_.password;

    int ret = start_internal();
    if (BSUCCEEDED(ret)) {
        started_ = true;
    } else {
        stop();
    }
    return ret;
}

void trade_unit::stop()
{
    started_ = false;
    stop_internal();
}

void trade_unit::post(atp_message& msg)
{
    /* ignore query if current server is disconnected */
    if (connected_ || msg.type == ATP_MESSAGE_TYPE_SERVER_TRADE_REQ) {
        processor_base::post(msg);
    }

    if (started_) {
        assert(msg_event_ != NULL);
        msg_event_->set();
    }
}

int trade_unit::start_internal()
{
    int ret = NAUT_AT_S_OK;

    LABEL_SCOPE_START;

    msg_event_ = new base::event();
    state_event_ = new base::event();

    if (!params_.use_simulation_flag) {
        trade_reconnect();
    }

    /* start processor thread */
    processor_base::start();

    LABEL_SCOPE_END;

end:
    return ret;
}

int trade_unit::stop_internal()
{
    /* stop processor thread */
    processor_base::stop();

    /* release trade messages left in the queue */
    release_messages();

    if (msg_event_ != NULL) {
        delete msg_event_;
        msg_event_ = NULL;
    }

    if (state_event_ != NULL) {
        delete state_event_;
        state_event_ = NULL;
    }

    map_int_req_fun_.clear();

    if (trader_ != NULL) {
        trader_->Release();
        trader_ = NULL;
    }
    return NAUT_AT_S_OK;
}


void trade_unit::release_messages()
{
    atp_message msg;
    while (get(msg) == 0) {
        ref_dictionary* rd = (ref_dictionary*)msg.param1;
        if (rd != NULL) {
            rd->release();
        }
    }
}

void trade_unit::run()
{
    atp_message msg;
    std::string force_start_time;
    while (is_running_) {
        if (!params_.use_simulation_flag) {
            force_start_time = base::util::local_time_string();
            std::string trade_day = tserver_->get_comm_holiday().get_trade_day();
            if (tserver_->get_comm_holiday().isholiday(base::util::string_to_datestamp((char*)trade_day.c_str())) == true){
                base::util::sleep(10000);
                continue;
            }

            if (((force_start_time >= "08:30:00" && force_start_time <= "16:00:00")
                || (force_start_time >= "20:30:00" || force_start_time <= "03:00:00"))) {
                //û��λ����ĵ���ʱ�����������󣬺������е��Ӷ��Ѿ��ر�
                if (MakeActive() == false){
                    if (base::util::clock() - m_last_respond > 10 * 1000) {   //20��
                        TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_CTPTRADE_SEND_TRADE_INFO_FAILED, 
                            "error ʵ���߳� ����̫��(%lld��)û��ͨѶ�ˣ�ɾ���ؽ� \n", (base::util::clock() - m_last_respond) / 1000);
                        trade_reconnect();
                        base::util::sleep(1000);
                    }

                    if (!connected_ || !logined_){
                        TRACE_WARNING(AT_TRACE_TAG, "reconnect failed will retray now~~~");
                        base::util::sleep(1000);
                        continue;
                    }
                }
            }
        }

        if (!connected_ || !logined_ || !started_ || !tserver_->get_server_start_flag()) {
            state_event_->wait(50);
            continue;
        }

        int ret = get(msg);
        if (ret != 0) {
            msg_event_->reset();
            msg_event_->wait(20);
            continue;
        }

        ref_dictionary* rd = (ref_dictionary*)msg.param1;
        assert(rd != NULL);

        base::dictionary* dict = rd->get();
        std::string cmd = (*dict)["cmd"].string_value();

        TRACE_SYSTEM(AT_TRACE_TAG, "trade unit prepare to send order, %s",
            dict->to_string().c_str());

        ret = NAUT_AT_S_OK;

        if (cmd == ATPM_CMD_ENTRUST) {
            ret = prepare_entrust(*dict);
        }
        else if (cmd == ATPM_CMD_WITHDRAW) {
            ret = prepare_withdraw(*dict);
        }
        else if (cmd == ATPM_CMD_QRY_ACCOUNT) {
            ret = prepare_qryaccount(*dict);
        }
        else if (cmd == ATPM_CMD_QRY_DEAL
            || cmd == ATPM_CMD_QRY_CHECKER_DEAL) {
            ret = prepare_qrydeal(*dict);
        }
        else if (cmd == ATPM_CMD_QRY_OPENINTER) {
            ret = prepare_qryopeninter(*dict);
        }
        else if (cmd == ATPM_CMD_QRY_ENTRUST
            || cmd == ATPM_CMD_QRY_CHECKER_ENTRUST) {
            ret = prepare_qryentrust(*dict);
        }
        else {
            ret = NAUT_AT_E_UNKNOWN_TRADE_CMD;
        }

        /* notify if the order is failed */
        if (BFAILED(ret)) {
            TRACE_SYSTEM(AT_TRACE_TAG,
                "trade unit prepare order failed, ret: %d, dict: %s", ret,
                dict->to_string().c_str());

            base::dictionary* pdict = new base::dictionary(*dict);
            ref_dictionary* rd1 = new ref_dictionary(pdict);

            (*pdict)["real_cmd"] = (*pdict)["cmd"].string_value();
            (*pdict)["cmd"] = ATPM_CMD_ERROR;
            (*pdict)["error_code"] = get_response_error_code(ret);
            (*pdict)["error_msg"] = get_response_error_msg((*pdict)["error_code"].integer_value());

            atp_message msg1;
            if (msg.type == ATP_MESSAGE_TYPE_SERVER_TRADE_REQ) {
                msg1.type = ATP_MESSAGE_TYPE_SERVER_TRADE_RSP;
            }
            else {
                msg1.type = ATP_MESSAGE_TYPE_SERVER_QUERY_RSP;
            }
            msg1.param1 = (void*)rd1;
            mdpt_->dispatch_message(msg1);
        }

        rd->release();
    }
}

int trade_unit::login()
{
    if (!connected_) {
        return NAUT_AT_E_CTPTRADE_NOT_CONNECTED;
    }

    int ret = NAUT_AT_S_OK;

    CThostFtdcReqUserLoginField loginReq;
    memset(&loginReq, 0, sizeof(loginReq));

    strcpy(loginReq.BrokerID, params_.broker.c_str());
    strcpy(loginReq.UserID, params_.userid.c_str());
    strcpy(loginReq.Password, params_.password.c_str());

    int result = trader_->ReqUserLogin(&loginReq, GetRequestId());

    if (result != 0) {
        TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_CTPTRADE_SEND_TRADE_INFO_FAILED,
            "tu(%s): send login data to trade server failed, ret: %d, errormsg: %s",
            account_.c_str(), result, GetRetErrorMsg(result).c_str());
        ASSIGN_AND_CHECK_LABEL(ret, NAUT_AT_E_CTPTRADE_SEND_TRADE_INFO_FAILED,
            end);
    }

end: return ret;
}

int trade_unit::logout()
{
    if (!connected_) {
        return NAUT_AT_E_CTPTRADE_NOT_CONNECTED;
    }

    int ret = NAUT_AT_S_OK;

    CThostFtdcUserLogoutField logoutReq;
    memset(&logoutReq, 0, sizeof(logoutReq));

    strcpy(logoutReq.BrokerID, params_.broker.c_str());
    strcpy(logoutReq.UserID, params_.userid.c_str());

    int result = trader_->ReqUserLogout(&logoutReq, GetRequestId());

    if (result != 0) {
        TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_CTPTRADE_SEND_TRADE_INFO_FAILED,
            "tu(%s): send login data to trade server failed, ret: %d, errormsg: %s",
            account_.c_str(), result, GetRetErrorMsg(result).c_str());
        ASSIGN_AND_CHECK_LABEL(ret, NAUT_AT_E_CTPTRADE_SEND_TRADE_INFO_FAILED,
            end);
    }

end: 
    return ret;
}

int trade_unit::trade_reconnect()
{
    int ret = NAUT_AT_S_OK;

    m_last_respond = base::util::clock() + 3 * 1000;
    m_conn_state = cs_connecting;
    if (trader_ != NULL) {
        //base::util::sleep(10000);
        trader_->Release();
        trader_ = NULL;
    }
    connected_ = false;
    logined_ = false;
    char addr[256];
    std::string spath;
    sprintf(addr, "tcp://%s:%d", params_.trade_host.c_str(),
        params_.trade_port);

    spath = "./ctp_log/" + params_.account;
    base::file::make_sure_directory_exist(spath.c_str());
    spath += "/";
    trader_ = CThostFtdcTraderApi::CreateFtdcTraderApi(spath.c_str());
    trader_->RegisterFront(addr);
    trader_->SubscribePublicTopic(THOST_TERT_RESUME);
    trader_->SubscribePrivateTopic(THOST_TERT_RESUME);
    trader_->RegisterSpi(this);
    trader_->Init();
    return ret;
}

int trade_unit::prepare_entrust(const base::dictionary& dict)
{
    int ret = NAUT_AT_S_OK;

end: 
     return ret;
}
int trade_unit::prepare_withdraw(const base::dictionary& dict)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}
int trade_unit::prepare_qryaccount(const base::dictionary& dict)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}
int trade_unit::prepare_qrydeal(const base::dictionary& dict)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}
int trade_unit::prepare_qryopeninter(const base::dictionary& dict)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}
int trade_unit::prepare_qryentrust(const base::dictionary& dict)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}

int trade_unit::process_entrust_result(CThostFtdcOrderField *pOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}
int trade_unit::process_entrust_deal_result(CThostFtdcTradeField *pTrade)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}
int trade_unit::process_withdraw_result(CThostFtdcOrderField *pOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}
int trade_unit::process_qryaccount_result(CThostFtdcTradingAccountField *pTradingAccount,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}
int trade_unit::process_qrydeal_result(CThostFtdcTradeField *pTrade,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}
int trade_unit::process_qryopeniner_result(CThostFtdcInvestorPositionField *pInvestorPosition,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}
int trade_unit::process_qryentrust_result(CThostFtdcOrderField *pOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    int ret = NAUT_AT_S_OK;

end:
    return ret;
}

std::string trade_unit::GetRetErrorMsg(int errorcode)
{
    std::string errmsg("");
    switch (errorcode) {
    case -1:
        errmsg = "������ԭ����ʧ��";
        break;
    case -2:
        errmsg = "δ���������������������";
        break;
    case -3:
        errmsg = "ÿ�뷢��������������";
        break;
    default:
        errmsg = "����δ����";
        break;

    }
    return errmsg;
}

char* trade_unit::ChangeDateFormat(char* outdate, char* inputdate)
{
    if (outdate == NULL || inputdate == NULL) {
        return NULL;
    }
    for (int i = 0, j = 0; i < 11; i++) {
        if (i == 4 || i == 7) {
            outdate[i] = '-';
            continue;
        }
        outdate[i] = inputdate[j];
        j++;
    }
    return outdate;
}

bool trade_unit::IsErrorRspInfo(CThostFtdcRspInfoField *pRspInfo)
{
    return ((pRspInfo) && (pRspInfo->ErrorID != AT_ERROR_CTP_NONE));
}

bool trade_unit::MakeActive() {
    long cur_millsec = base::util::clock();

    if (m_conn_state == cs_connecting && cur_millsec - m_last_respond < 5 * 1000)
    {
        TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_CTPTRADE_SEND_TRADE_INFO_FAILED, "���Ӽ��(%s) >> ����������..... \n", account_.c_str());
        return false;
    }
    if (connected_ == false){
        m_last_respond = 0;
        TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_CTPTRADE_SEND_TRADE_INFO_FAILED, "���Ӽ��(%s) >> û������ \n", account_.c_str());
        return false;
    }
    //?��Ҫ
    if (cur_millsec - m_last_respond > 10 * 1000){
        connected_ = false;
        TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_CTPTRADE_SEND_TRADE_INFO_FAILED, "���Ӽ��(%s) >> ��ʱ(%lld),�������� \n", account_.c_str(), cur_millsec - m_last_respond);
        m_last_respond = 0;
        return false;
    }

    return true;
}


///���ͻ����뽻�׺�̨������ͨ������ʱ����δ��¼ǰ�����÷��������á�
void trade_unit::OnFrontConnected()
{
    m_last_respond = base::util::clock();
    TRACE_SYSTEM(AT_TRACE_TAG,
        "broker_account:%s_%s connection to server sucessfully",
        params_.broker.c_str(), params_.account.c_str());
    connected_ = true;
    m_conn_state = cs_connected;

    if (started_) {
        state_event_->set();
    }
    if (!logined_) {
        login();
    }
}

///���ͻ����뽻�׺�̨ͨ�����ӶϿ�ʱ���÷��������á���������������API���Զ��������ӣ��ͻ��˿ɲ�������
///@param nReason ����ԭ��
///        0x1001 �����ʧ��
///        0x1002 ����дʧ��
///        0x2001 ����������ʱ
///        0x2002 ��������ʧ��
///        0x2003 �յ�������
void trade_unit::OnFrontDisconnected(int nReason)
{
    m_last_respond = base::util::clock();
    connected_ = false;
    m_conn_state = cs_disconnected;
    std::string errmsg("");
    switch (nReason) {
    case 0x1001:
        errmsg = "�����ʧ��";
        break;
    case 0x1002:
        errmsg = "����дʧ��";
        break;
    case 0x2001:
        errmsg = "����������ʱ";
        break;
    case 0x2002:
        errmsg = "��������ʧ��";
        break;
    case 0x2003:
        errmsg = "�յ�������";
        break;
    }
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_CTPTRADE_NOT_CONNECTED,
        "connection to server failed '%d', will retry later, '%s'", nReason,
        errmsg.c_str());
}

///������ʱ���档����ʱ��δ�յ�����ʱ���÷��������á�
///@param nTimeLapse �����ϴν��ձ��ĵ�ʱ��
void trade_unit::OnHeartBeatWarning(int nTimeLapse)
{
    connected_ = false;
    TRACE_WARNING(AT_TRACE_TAG, "tu(%s)  the heartbeat is over time (%ds) from last one", userid_.c_str(), nTimeLapse);
}

///�ͻ�����֤��Ӧ
void trade_unit::OnRspAuthenticate(
    CThostFtdcRspAuthenticateField *pRspAuthenticateField,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s)  pRspAuthenticateField:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d",
        userid_.c_str(), pRspAuthenticateField, pRspInfo, nRequestID, bIsLast);
    if (!pRspAuthenticateField) {
        return;
    }
}

///��¼������Ӧ
void trade_unit::OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    m_last_respond = base::util::clock();
    if (!pRspUserLogin) {
        return;
    }
    /* if receive login response, indicate that it is connected */
    m_last_respond = base::util::clock();
    if (!IsErrorRspInfo(pRspInfo)) {
        logined_ = true;

        assert(state_event_ != NULL);
        state_event_->set();
        m_FrontID_ = pRspUserLogin->FrontID;
        m_SessionID_ = pRspUserLogin->SessionID;
        m_MaxOrderRef_ = pRspUserLogin->MaxOrderRef;

        TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): login successfully~~~ m_FrontID_:%d, m_SessionID_:%d, m_MaxOrderRef_:%s",
            userid_.c_str(), m_FrontID_, m_SessionID_, m_MaxOrderRef_.c_str());

        CThostFtdcSettlementInfoConfirmField req;
        memset(&req, 0, sizeof(req));
        strcpy(req.BrokerID, broker_.c_str());
        strcpy(req.InvestorID, account_.c_str());
        while (true) {
            m_last_respond = base::util::clock();
            int iResult = trader_->ReqSettlementInfoConfirm(&req, GetRequestId());
            if (!iResult) {
                TRACE_SYSTEM(AT_TRACE_TAG, "ReqSettlementInfoConfirm result: %d \n", iResult);
                break;
            }
            else {
                TRACE_SYSTEM(AT_TRACE_TAG, "ReqSettlementInfoConfirm result: %d \n", iResult);
                base::util::sleep(1000);
            }
        } // while
    }
    else {
        TRACE_SYSTEM(AT_TRACE_TAG,
            "tu(%s): receive login response, userid: %s, BrokerID:%s ,errormsg: %s",
            userid_.c_str(), pRspUserLogin->UserID, pRspUserLogin->BrokerID,
            get_response_error_msg(pRspInfo->ErrorID).c_str());
    }

}

///�ǳ�������Ӧ
void trade_unit::OnRspUserLogout(CThostFtdcUserLogoutField *pUserLogout,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    m_last_respond = base::util::clock();
    connected_ = false;
    m_conn_state = cs_disconnected;

    if (!pUserLogout) {
        return;
    }
    else {
        TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): logout successfully~~~",
            userid_.c_str());
    }

    if (!IsErrorRspInfo(pRspInfo)) {
        logined_ = false;

        assert(state_event_ != NULL);
        state_event_->reset();

        TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): logout successfully~~~",
            userid_.c_str());
    }
    else {
        TRACE_SYSTEM(AT_TRACE_TAG,
            "tu(%s): receive logout response, userid: %s, BrokerID:%s ,errormsg: %s",
            userid_.c_str(), pUserLogout->UserID, pUserLogout->BrokerID,
            get_response_error_msg(pRspInfo->ErrorID).c_str());
    }
}

///�û��������������Ӧ
void trade_unit::OnRspUserPasswordUpdate(
    CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pUserPasswordUpdate:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pUserPasswordUpdate, pRspInfo, nRequestID, bIsLast);
    if (!pUserPasswordUpdate) {
        return;
    }
}

///�ʽ��˻��������������Ӧ
void trade_unit::OnRspTradingAccountPasswordUpdate(
    CThostFtdcTradingAccountPasswordUpdateField *pTradingAccountPasswordUpdate,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pUserPasswordUpdate:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pTradingAccountPasswordUpdate, pRspInfo, nRequestID, bIsLast);
    if (!pTradingAccountPasswordUpdate) {
        return;
    }
}

///����¼��������Ӧ
void trade_unit::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    m_last_respond = base::util::clock();
    TRACE_SYSTEM(AT_TRACE_TAG, "pInputOrder:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d", pInputOrder, pRspInfo, nRequestID, bIsLast);
    if (!pInputOrder) {
        return;
    }

    CThostFtdcOrderField rspOrder;
    memset(&rspOrder, 0, sizeof(rspOrder));
    strcpy(rspOrder.BrokerID, pInputOrder->BrokerID);
    strcpy(rspOrder.BusinessUnit, pInputOrder->BusinessUnit);
    strcpy(rspOrder.CombHedgeFlag, pInputOrder->CombHedgeFlag);
    strcpy(rspOrder.CombOffsetFlag, pInputOrder->CombOffsetFlag);
    strcpy(rspOrder.GTDDate, pInputOrder->GTDDate);
    strcpy(rspOrder.InstrumentID, pInputOrder->InstrumentID);
    strcpy(rspOrder.InvestorID, pInputOrder->InvestorID);
    strcpy(rspOrder.OrderRef, pInputOrder->OrderRef);
    strcpy(rspOrder.UserID, pInputOrder->UserID);

    rspOrder.ContingentCondition = pInputOrder->ContingentCondition;
    rspOrder.Direction = pInputOrder->Direction;
    rspOrder.ForceCloseReason = pInputOrder->ForceCloseReason;
    rspOrder.IsAutoSuspend = pInputOrder->IsAutoSuspend;
    rspOrder.IsSwapOrder = pInputOrder->IsSwapOrder;
    rspOrder.LimitPrice = pInputOrder->LimitPrice;
    rspOrder.MinVolume = pInputOrder->MinVolume;
    rspOrder.OrderPriceType = pInputOrder->OrderPriceType;
    rspOrder.RequestID = pInputOrder->RequestID;
    rspOrder.StopPrice = pInputOrder->StopPrice;
    rspOrder.TimeCondition = pInputOrder->TimeCondition;
    rspOrder.UserForceClose = pInputOrder->UserForceClose;
    rspOrder.VolumeCondition = pInputOrder->VolumeCondition;
    rspOrder.VolumeTotalOriginal = pInputOrder->VolumeTotalOriginal;

    process_entrust_result(&rspOrder, pRspInfo, nRequestID, bIsLast);
}

///Ԥ��¼��������Ӧ
void trade_unit::OnRspParkedOrderInsert(
    CThostFtdcParkedOrderField *pParkedOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    if (!pParkedOrder) {
        return;
    }
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pParkedOrder:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pParkedOrder, pRspInfo, nRequestID, bIsLast);
}

///Ԥ�񳷵�¼��������Ӧ
void trade_unit::OnRspParkedOrderAction(
    CThostFtdcParkedOrderActionField *pParkedOrderAction,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pParkedOrderAction:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pParkedOrderAction, pRspInfo, nRequestID, bIsLast);
    if (!pParkedOrderAction) {
        return;
    }
}

///��������������Ӧ
void trade_unit::OnRspOrderAction(
    CThostFtdcInputOrderActionField *pInputOrderAction,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    m_last_respond = base::util::clock();
    if (!pInputOrderAction) {
        return;
    }
    CThostFtdcOrderField rspOrder;
    memset(&rspOrder, 0, sizeof(rspOrder));
    strcpy(rspOrder.BrokerID, pInputOrderAction->BrokerID);
    strcpy(rspOrder.InvestorID, pInputOrderAction->InvestorID);
    strcpy(rspOrder.OrderRef, pInputOrderAction->OrderRef);
    rspOrder.RequestID = pInputOrderAction->RequestID;
    rspOrder.FrontID = pInputOrderAction->FrontID;
    rspOrder.SessionID = pInputOrderAction->SessionID;
    strcpy(rspOrder.ExchangeID, pInputOrderAction->ExchangeID);
    strcpy(rspOrder.OrderSysID, pInputOrderAction->OrderSysID);
    rspOrder.LimitPrice = pInputOrderAction->LimitPrice;
    rspOrder.VolumeTotal = pInputOrderAction->VolumeChange;
    strcpy(rspOrder.UserID, pInputOrderAction->UserID);
    strcpy(rspOrder.InstrumentID, pInputOrderAction->InstrumentID);

    process_withdraw_result(&rspOrder, pRspInfo, nRequestID, bIsLast);
}

///��ѯ��󱨵�������Ӧ
void trade_unit::OnRspQueryMaxOrderVolume(
    CThostFtdcQueryMaxOrderVolumeField *pQueryMaxOrderVolume,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pQueryMaxOrderVolume:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pQueryMaxOrderVolume, pRspInfo, nRequestID, bIsLast);
    if (!pQueryMaxOrderVolume) {
        return;
    }
}

///Ͷ���߽�����ȷ����Ӧ
void trade_unit::OnRspSettlementInfoConfirm(
    CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pSettlementInfoConfirm:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pSettlementInfoConfirm, pRspInfo, nRequestID, bIsLast);
    if (!pSettlementInfoConfirm) {
        return;
    }

    if (bIsLast && !IsErrorRspInfo(pRspInfo))
    {
        m_stockhold_old_buy_ = 0;
        m_stockhold_old_sell_ = 0;

        CThostFtdcQryInvestorPositionField QryInvestorPosition;
        memset(&QryInvestorPosition, 0, sizeof(QryInvestorPosition));
        strcpy(QryInvestorPosition.InvestorID, userid_.c_str());
        strcpy(QryInvestorPosition.BrokerID, broker_.c_str());

        int iResult = 0;
        while (true)  {
            iResult = trader_->ReqQryInvestorPosition(&QryInvestorPosition, GetRequestId());
            if (!iResult) {
                TRACE_SYSTEM(AT_TRACE_TAG, "ReqQryInvestorPosition result: %d \n", iResult);
                break;
            }
            else {
                TRACE_SYSTEM(AT_TRACE_TAG, "ReqQryInvestorPosition result: %d \n", iResult);
                base::util::sleep(1000);
            }

        } // while
    }
}

///ɾ��Ԥ����Ӧ
void trade_unit::OnRspRemoveParkedOrder(
    CThostFtdcRemoveParkedOrderField *pRemoveParkedOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRemoveParkedOrder:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pRemoveParkedOrder, pRspInfo, nRequestID, bIsLast);
    if (!pRemoveParkedOrder) {
        return;
    }
}

///ɾ��Ԥ�񳷵���Ӧ
void trade_unit::OnRspRemoveParkedOrderAction(
    CThostFtdcRemoveParkedOrderActionField *pRemoveParkedOrderAction,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRemoveParkedOrderAction:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pRemoveParkedOrderAction, pRspInfo, nRequestID, bIsLast);
    if (!pRemoveParkedOrderAction) {
        return;
    }
}

///ִ������¼��������Ӧ
void trade_unit::OnRspExecOrderInsert(
    CThostFtdcInputExecOrderField *pInputExecOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInputExecOrder:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInputExecOrder, pRspInfo, nRequestID, bIsLast);
    if (!pInputExecOrder) {
        return;
    }
}

///ִ���������������Ӧ
void trade_unit::OnRspExecOrderAction(
    CThostFtdcInputExecOrderActionField *pInputExecOrderAction,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInputExecOrderAction:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInputExecOrderAction, pRspInfo, nRequestID, bIsLast);
    if (!pInputExecOrderAction) {
        return;
    }
}

///ѯ��¼��������Ӧ
void trade_unit::OnRspForQuoteInsert(
    CThostFtdcInputForQuoteField *pInputForQuote,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInputForQuote:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInputForQuote, pRspInfo, nRequestID, bIsLast);
    if (!pInputForQuote) {
        return;
    }
}

///����¼��������Ӧ
void trade_unit::OnRspQuoteInsert(CThostFtdcInputQuoteField *pInputQuote,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInputQuote:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInputQuote, pRspInfo, nRequestID, bIsLast);
    if (!pInputQuote) {
        return;
    }
}

///���۲���������Ӧ
void trade_unit::OnRspQuoteAction(
    CThostFtdcInputQuoteActionField *pInputQuoteAction,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInputQuoteAction:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInputQuoteAction, pRspInfo, nRequestID, bIsLast);
    if (!pInputQuoteAction) {
        return;
    }
}

///�������¼��������Ӧ
void trade_unit::OnRspCombActionInsert(
    CThostFtdcInputCombActionField *pInputCombAction,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInputCombAction:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInputCombAction, pRspInfo, nRequestID, bIsLast);
    if (!pInputCombAction) {
        return;
    }
}

///�����ѯ������Ӧ
void trade_unit::OnRspQryOrder(CThostFtdcOrderField *pOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    m_last_respond = base::util::clock();
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pOrder:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pOrder, pRspInfo, nRequestID, bIsLast);
    if (!pOrder) {
        return;
    }
    process_qryentrust_result(pOrder, pRspInfo, nRequestID, bIsLast);
}

///�����ѯ�ɽ���Ӧ
void trade_unit::OnRspQryTrade(CThostFtdcTradeField *pTrade,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pTrade:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pTrade, pRspInfo, nRequestID, bIsLast);
    if (!pTrade) {
        return;
    }
    process_qrydeal_result(pTrade, pRspInfo, nRequestID, bIsLast);
}

///�����ѯͶ���ֲ߳���Ӧ
void trade_unit::OnRspQryInvestorPosition(
    CThostFtdcInvestorPositionField *pInvestorPosition,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    m_last_respond = base::util::clock();
    if (!pInvestorPosition) {
        return;
    }
    if (pInvestorPosition->PosiDirection == '2'){
        if (pInvestorPosition->YdPosition > 0 && pInvestorPosition->Position > 0)
            m_stockhold_old_buy_ += pInvestorPosition->Position;
    }
    else if (pInvestorPosition->PosiDirection == '3'){
        if (pInvestorPosition->YdPosition > 0 && pInvestorPosition->Position > 0)
            m_stockhold_old_sell_ += pInvestorPosition->Position;
    }

    process_qryopeniner_result(pInvestorPosition, pRspInfo, nRequestID,
        bIsLast);
}

///�����ѯ�ʽ��˻���Ӧ
void trade_unit::OnRspQryTradingAccount(
    CThostFtdcTradingAccountField *pTradingAccount,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): recv rsp_account nRequestID = %d",
        userid_.c_str(), nRequestID);
    m_last_respond = base::util::clock();
    if (!pTradingAccount) {
        return;
    }
    process_qryaccount_result(pTradingAccount, pRspInfo, nRequestID, bIsLast);
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): finish rsp_account nRequestID = %d",
        userid_.c_str(), nRequestID);
    m_last_qryacc_finish = true;
}

///�����ѯͶ������Ӧ
void trade_unit::OnRspQryInvestor(CThostFtdcInvestorField *pInvestor,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInvestor:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInvestor, pRspInfo, nRequestID, bIsLast);
    if (!pInvestor) {
        return;
    }
}

///�����ѯ���ױ�����Ӧ
void trade_unit::OnRspQryTradingCode(CThostFtdcTradingCodeField *pTradingCode,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pTradingCode:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pTradingCode, pRspInfo, nRequestID, bIsLast);
    if (!pTradingCode) {
        return;
    }
}

///�����ѯ��Լ��֤������Ӧ
void trade_unit::OnRspQryInstrumentMarginRate(
    CThostFtdcInstrumentMarginRateField *pInstrumentMarginRate,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInstrumentMarginRate:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInstrumentMarginRate, pRspInfo, nRequestID, bIsLast);
    if (!pInstrumentMarginRate) {
        return;
    }
}

///�����ѯ��Լ����������Ӧ
void trade_unit::OnRspQryInstrumentCommissionRate(
    CThostFtdcInstrumentCommissionRateField *pInstrumentCommissionRate,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInstrumentCommissionRate:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInstrumentCommissionRate, pRspInfo, nRequestID, bIsLast);
    if (!pInstrumentCommissionRate) {
        return;
    }
}

///�����ѯ��������Ӧ
void trade_unit::OnRspQryExchange(CThostFtdcExchangeField *pExchange,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pExchange:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pExchange, pRspInfo, nRequestID, bIsLast);
    if (!pExchange) {
        return;
    }
}

///�����ѯ��Ʒ��Ӧ
void trade_unit::OnRspQryProduct(CThostFtdcProductField *pProduct,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pProduct:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pProduct, pRspInfo, nRequestID, bIsLast);
    if (!pProduct) {
        return;
    }
}

///�����ѯ��Լ��Ӧ
void trade_unit::OnRspQryInstrument(CThostFtdcInstrumentField *pInstrument,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInstrument:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInstrument, pRspInfo, nRequestID, bIsLast);
    if (!pInstrument) {
        return;
    }
}

///�����ѯ������Ӧ
void trade_unit::OnRspQryDepthMarketData(
    CThostFtdcDepthMarketDataField *pDepthMarketData,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    m_last_respond = base::util::clock();
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pDepthMarketData:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pDepthMarketData, pRspInfo, nRequestID, bIsLast);
    if (!pDepthMarketData) {
        return;
    }
}

///�����ѯͶ���߽�������Ӧ
void trade_unit::OnRspQrySettlementInfo(
    CThostFtdcSettlementInfoField *pSettlementInfo,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pSettlementInfo:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pSettlementInfo, pRspInfo, nRequestID, bIsLast);
    if (!pSettlementInfo) {
        return;
    }
}

///�����ѯת��������Ӧ
void trade_unit::OnRspQryTransferBank(
    CThostFtdcTransferBankField *pTransferBank,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pTransferBank:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pTransferBank, pRspInfo, nRequestID, bIsLast);
    if (!pTransferBank) {
        return;
    }
}

///�����ѯͶ���ֲ߳���ϸ��Ӧ
void trade_unit::OnRspQryInvestorPositionDetail(
    CThostFtdcInvestorPositionDetailField *pInvestorPositionDetail,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInvestorPositionDetail:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInvestorPositionDetail, pRspInfo, nRequestID, bIsLast);
    if (!pInvestorPositionDetail) {
        return;
    }
}

///�����ѯ�ͻ�֪ͨ��Ӧ
void trade_unit::OnRspQryNotice(CThostFtdcNoticeField *pNotice,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pNotice:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pNotice, pRspInfo, nRequestID, bIsLast);
    if (!pNotice) {
        return;
    }
}

///�����ѯ������Ϣȷ����Ӧ
void trade_unit::OnRspQrySettlementInfoConfirm(
    CThostFtdcSettlementInfoConfirmField *pSettlementInfoConfirm,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pSettlementInfoConfirm:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pSettlementInfoConfirm, pRspInfo, nRequestID, bIsLast);
    if (!pSettlementInfoConfirm) {
        return;
    }
}

///�����ѯͶ���ֲ߳���ϸ��Ӧ
void trade_unit::OnRspQryInvestorPositionCombineDetail(
    CThostFtdcInvestorPositionCombineDetailField *pInvestorPositionCombineDetail,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInvestorPositionCombineDetail:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInvestorPositionCombineDetail, pRspInfo, nRequestID, bIsLast);
    if (!pInvestorPositionCombineDetail) {
        return;
    }
}

///��ѯ��֤����ϵͳ���͹�˾�ʽ��˻���Կ��Ӧ
void trade_unit::OnRspQryCFMMCTradingAccountKey(
    CThostFtdcCFMMCTradingAccountKeyField *pCFMMCTradingAccountKey,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pCFMMCTradingAccountKey:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pCFMMCTradingAccountKey, pRspInfo, nRequestID, bIsLast);
    if (!pCFMMCTradingAccountKey) {
        return;
    }
}

///�����ѯ�ֵ��۵���Ϣ��Ӧ
void trade_unit::OnRspQryEWarrantOffset(
    CThostFtdcEWarrantOffsetField *pEWarrantOffset,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pEWarrantOffset:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pEWarrantOffset, pRspInfo, nRequestID, bIsLast);
    if (!pEWarrantOffset) {
        return;
    }
}

///�����ѯͶ����Ʒ��/��Ʒ�ֱ�֤����Ӧ
void trade_unit::OnRspQryInvestorProductGroupMargin(
    CThostFtdcInvestorProductGroupMarginField *pInvestorProductGroupMargin,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInvestorProductGroupMargin:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInvestorProductGroupMargin, pRspInfo, nRequestID, bIsLast);
    if (!pInvestorProductGroupMargin) {
        return;
    }
}

///�����ѯ��������֤������Ӧ
void trade_unit::OnRspQryExchangeMarginRate(
    CThostFtdcExchangeMarginRateField *pExchangeMarginRate,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pExchangeMarginRate:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pExchangeMarginRate, pRspInfo, nRequestID, bIsLast);
    if (!pExchangeMarginRate) {
        return;
    }
}

///�����ѯ������������֤������Ӧ
void trade_unit::OnRspQryExchangeMarginRateAdjust(
    CThostFtdcExchangeMarginRateAdjustField *pExchangeMarginRateAdjust,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pExchangeMarginRateAdjust:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pExchangeMarginRateAdjust, pRspInfo, nRequestID, bIsLast);
    if (!pExchangeMarginRateAdjust) {
        return;
    }
}

///�����ѯ������Ӧ
void trade_unit::OnRspQryExchangeRate(
    CThostFtdcExchangeRateField *pExchangeRate,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pExchangeRate:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pExchangeRate, pRspInfo, nRequestID, bIsLast);
    if (!pExchangeRate) {
        return;
    }
}

///�����ѯ�����������Ա����Ȩ����Ӧ
void trade_unit::OnRspQrySecAgentACIDMap(
    CThostFtdcSecAgentACIDMapField *pSecAgentACIDMap,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pSecAgentACIDMap:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pSecAgentACIDMap, pRspInfo, nRequestID, bIsLast);
    if (!pSecAgentACIDMap) {
        return;
    }
}

///�����ѯ��Ʒ��
void trade_unit::OnRspQryProductGroup(
    CThostFtdcProductGroupField *pProductGroup,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pProductGroup:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pProductGroup, pRspInfo, nRequestID, bIsLast);
    if (!pProductGroup) {
        return;
    }
}

///�����ѯ������������Ӧ
void trade_unit::OnRspQryInstrumentOrderCommRate(
    CThostFtdcInstrumentOrderCommRateField *pInstrumentOrderCommRate,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInstrumentOrderCommRate:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pInstrumentOrderCommRate, pRspInfo, nRequestID, bIsLast);
    if (!pInstrumentOrderCommRate) {
        return;
    }
}

///�����ѯ��Ȩ���׳ɱ���Ӧ
void trade_unit::OnRspQryOptionInstrTradeCost(
    CThostFtdcOptionInstrTradeCostField *pOptionInstrTradeCost,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pOptionInstrTradeCost:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pOptionInstrTradeCost, pRspInfo, nRequestID, bIsLast);
    if (!pOptionInstrTradeCost) {
        return;
    }
}

///�����ѯ��Ȩ��Լ��������Ӧ
void trade_unit::OnRspQryOptionInstrCommRate(
    CThostFtdcOptionInstrCommRateField *pOptionInstrCommRate,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pOptionInstrCommRate:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pOptionInstrCommRate, pRspInfo, nRequestID, bIsLast);
    if (!pOptionInstrCommRate) {
        return;
    }
}

///�����ѯִ��������Ӧ
void trade_unit::OnRspQryExecOrder(CThostFtdcExecOrderField *pExecOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pExecOrder:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pExecOrder, pRspInfo, nRequestID, bIsLast);
    if (!pExecOrder) {
        return;
    }
}

///�����ѯѯ����Ӧ
void trade_unit::OnRspQryForQuote(CThostFtdcForQuoteField *pForQuote,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pForQuote:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pForQuote, pRspInfo, nRequestID, bIsLast);
    if (!pForQuote) {
        return;
    }
}

///�����ѯ������Ӧ
void trade_unit::OnRspQryQuote(CThostFtdcQuoteField *pQuote,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pQuote:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pQuote, pRspInfo, nRequestID, bIsLast);
    if (!pQuote) {
        return;
    }
}

///�����ѯ��Ϻ�Լ��ȫϵ����Ӧ
void trade_unit::OnRspQryCombInstrumentGuard(
    CThostFtdcCombInstrumentGuardField *pCombInstrumentGuard,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pCombInstrumentGuard:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pCombInstrumentGuard, pRspInfo, nRequestID, bIsLast);
    if (!pCombInstrumentGuard) {
        return;
    }
}

///�����ѯ���������Ӧ
void trade_unit::OnRspQryCombAction(CThostFtdcCombActionField *pCombAction,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pCombAction:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pCombAction, pRspInfo, nRequestID, bIsLast);
    if (!pCombAction) {
        return;
    }
}

///�����ѯת����ˮ��Ӧ
void trade_unit::OnRspQryTransferSerial(
    CThostFtdcTransferSerialField *pTransferSerial,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pTransferSerial:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pTransferSerial, pRspInfo, nRequestID, bIsLast);
    if (!pTransferSerial) {
        return;
    }
}

///�����ѯ����ǩԼ��ϵ��Ӧ
void trade_unit::OnRspQryAccountregister(
    CThostFtdcAccountregisterField *pAccountregister,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pAccountregister:%p, pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pAccountregister, pRspInfo, nRequestID, bIsLast);
    if (!pAccountregister) {
        return;
    }
}

///����Ӧ��
void trade_unit::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID,
    bool bIsLast)
{
    connected_ = false;
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pRspInfo:%p, nRequestID: %d, bIsLast:%d",
        userid_.c_str(), pRspInfo, nRequestID, bIsLast);
}

///����֪ͨ
void trade_unit::OnRtnOrder(CThostFtdcOrderField *pOrder)
{
    m_last_respond = base::util::clock();
    if (!pOrder) {
        return;
    }

    switch (map_int_req_fun_[pOrder->RequestID]) {
    case PROCESS_ENTRUST_RESULT_REQUESTID:
        TRACE_DEBUG(AT_TRACE_TAG, "pOrder:%p, nRequestID:%d", pOrder, pOrder->RequestID);
        process_entrust_result(pOrder, NULL, pOrder->RequestID, true);
        break;
    case PROCESS_WITHDRAW_RESULT_REQUESTID:
        process_withdraw_result(pOrder, NULL, pOrder->RequestID, true);
        break;
    case PROCESS_QRYENTRUST_RESULT_REQUESTID:
        process_qryentrust_result(pOrder, NULL, pOrder->RequestID, true);
        break;
    default:
        TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_ORDER_NOT_EXIST,
            "unknown userid:'%s' systemno : '%s' ", userid_.c_str(),
            pOrder->RequestID);
        break;
    }

}

///�ɽ�֪ͨ
void trade_unit::OnRtnTrade(CThostFtdcTradeField *pTrade)
{
    m_last_respond = base::util::clock();
    if (!pTrade) {
        return;
    }
    TRACE_DEBUG(AT_TRACE_TAG, "pTrade:%p", pTrade);
    process_entrust_deal_result(pTrade);
}

///����¼�����ر�
void trade_unit::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder,
    CThostFtdcRspInfoField *pRspInfo)
{
    m_last_respond = base::util::clock();
    TRACE_SYSTEM(AT_TRACE_TAG, "pInputOrder:%p, pRspInfo:%p", pInputOrder, pRspInfo);
    if (!pInputOrder) {
        return;
    }
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_ORDER_NOT_EXIST,
        "received error: '%d', errormsg, '%s'", pRspInfo->ErrorID,
        get_response_error_msg(pRspInfo->ErrorID).c_str());

    CThostFtdcOrderField rspOrder;
    memset(&rspOrder, 0, sizeof(rspOrder));
    strcpy(rspOrder.BrokerID, pInputOrder->BrokerID);
    strcpy(rspOrder.BusinessUnit, pInputOrder->BusinessUnit);
    strcpy(rspOrder.CombHedgeFlag, pInputOrder->CombHedgeFlag);
    strcpy(rspOrder.CombOffsetFlag, pInputOrder->CombOffsetFlag);
    strcpy(rspOrder.GTDDate, pInputOrder->GTDDate);
    strcpy(rspOrder.InstrumentID, pInputOrder->InstrumentID);
    strcpy(rspOrder.InvestorID, pInputOrder->InvestorID);
    strcpy(rspOrder.OrderRef, pInputOrder->OrderRef);
    strcpy(rspOrder.UserID, pInputOrder->UserID);

    rspOrder.ContingentCondition = pInputOrder->ContingentCondition;
    rspOrder.Direction = pInputOrder->Direction;
    rspOrder.ForceCloseReason = pInputOrder->ForceCloseReason;
    rspOrder.IsAutoSuspend = pInputOrder->IsAutoSuspend;
    rspOrder.IsSwapOrder = pInputOrder->IsSwapOrder;
    rspOrder.LimitPrice = pInputOrder->LimitPrice;
    rspOrder.MinVolume = pInputOrder->MinVolume;
    rspOrder.OrderPriceType = pInputOrder->OrderPriceType;
    rspOrder.RequestID = pInputOrder->RequestID;
    rspOrder.StopPrice = pInputOrder->StopPrice;
    rspOrder.TimeCondition = pInputOrder->TimeCondition;
    rspOrder.UserForceClose = pInputOrder->UserForceClose;
    rspOrder.VolumeCondition = pInputOrder->VolumeCondition;
    rspOrder.VolumeTotalOriginal = pInputOrder->VolumeTotalOriginal;

    process_entrust_result(&rspOrder, pRspInfo, 0, true);
}

///������������ر�
void trade_unit::OnErrRtnOrderAction(CThostFtdcOrderActionField *pOrderAction,
    CThostFtdcRspInfoField *pRspInfo)
{
    if (!pOrderAction) {
        return;
    }
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_ORDER_NOT_EXIST,
        "received error: '%d', errormsg, '%s'", pRspInfo->ErrorID,
        get_response_error_msg(pRspInfo->ErrorID).c_str());
}

///��Լ����״̬֪ͨ
void trade_unit::OnRtnInstrumentStatus(
    CThostFtdcInstrumentStatusField *pInstrumentStatus)
{
    m_last_respond = base::util::clock();
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pInstrumentStatus:%p",
        userid_.c_str(), pInstrumentStatus);

    m_last_respond = base::util::clock();
}

///����֪ͨ
void trade_unit::OnRtnTradingNotice(
    CThostFtdcTradingNoticeInfoField *pTradingNoticeInfo)
{
    m_last_respond = base::util::clock();
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pTradingNoticeInfo:%p",
        userid_.c_str(), pTradingNoticeInfo);
    m_last_respond = base::util::clock();
}

///��ʾ������У�����
void trade_unit::OnRtnErrorConditionalOrder(
    CThostFtdcErrorConditionalOrderField *pErrorConditionalOrder)
{
    m_last_respond = base::util::clock();
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pErrorConditionalOrder:%p ",
        userid_.c_str(), pErrorConditionalOrder);
}

///ִ������֪ͨ
void trade_unit::OnRtnExecOrder(CThostFtdcExecOrderField *pExecOrder)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pExecOrder:%p",
        userid_.c_str(), pExecOrder);
}

///ִ������¼�����ر�
void trade_unit::OnErrRtnExecOrderInsert(
    CThostFtdcInputExecOrderField *pInputExecOrder,
    CThostFtdcRspInfoField *pRspInfo)
{
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pInputExecOrder:%p , pRspInfo:%p",
        userid_.c_str(), pInputExecOrder, pRspInfo);
}

///ִ�������������ر�
void trade_unit::OnErrRtnExecOrderAction(
    CThostFtdcExecOrderActionField *pExecOrderAction,
    CThostFtdcRspInfoField *pRspInfo)
{
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pExecOrderAction:%p , pRspInfo:%p",
        userid_.c_str(), pExecOrderAction, pRspInfo);
}

///ѯ��¼�����ر�
void trade_unit::OnErrRtnForQuoteInsert(
    CThostFtdcInputForQuoteField *pInputForQuote,
    CThostFtdcRspInfoField *pRspInfo)
{
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pInputForQuote:%p , pRspInfo:%p",
        userid_.c_str(), pInputForQuote, pRspInfo);
}

///����֪ͨ
void trade_unit::OnRtnQuote(CThostFtdcQuoteField *pQuote)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pQuote:%p",
        userid_.c_str(), pQuote);
}

///����¼�����ر�
void trade_unit::OnErrRtnQuoteInsert(CThostFtdcInputQuoteField *pInputQuote,
    CThostFtdcRspInfoField *pRspInfo)
{
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pInputQuote:%p , pRspInfo:%p",
        userid_.c_str(), pInputQuote, pRspInfo);
}

///���۲�������ر�
void trade_unit::OnErrRtnQuoteAction(CThostFtdcQuoteActionField *pQuoteAction,
    CThostFtdcRspInfoField *pRspInfo)
{
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pQuoteAction:%p , pRspInfo:%p",
        userid_.c_str(), pQuoteAction, pRspInfo);
}

///ѯ��֪ͨ
void trade_unit::OnRtnForQuoteRsp(CThostFtdcForQuoteRspField *pForQuoteRsp)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pForQuoteRsp:%p",
        userid_.c_str(), pForQuoteRsp);
}

///��֤���������û�����
void trade_unit::OnRtnCFMMCTradingAccountToken(
    CThostFtdcCFMMCTradingAccountTokenField *pCFMMCTradingAccountToken)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pCFMMCTradingAccountToken:%p",
        userid_.c_str(), pCFMMCTradingAccountToken);
}

///�������֪ͨ
void trade_unit::OnRtnCombAction(CThostFtdcCombActionField *pCombAction)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pCombAction:%p",
        userid_.c_str(), pCombAction);
}

///�������¼�����ر�
void trade_unit::OnErrRtnCombActionInsert(
    CThostFtdcInputCombActionField *pInputCombAction,
    CThostFtdcRspInfoField *pRspInfo)
{
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pInputCombAction:%p , pRspInfo:%p",
        userid_.c_str(), pInputCombAction, pRspInfo);
}

///�����ѯǩԼ������Ӧ
void trade_unit::OnRspQryContractBank(
    CThostFtdcContractBankField *pContractBank,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pContractBank:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d",
        userid_.c_str(), pContractBank, pRspInfo, nRequestID, bIsLast);
}

///�����ѯԤ����Ӧ
void trade_unit::OnRspQryParkedOrder(CThostFtdcParkedOrderField *pParkedOrder,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pParkedOrder:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d",
        userid_.c_str(), pParkedOrder, pRspInfo, nRequestID, bIsLast);
}

///�����ѯԤ�񳷵���Ӧ
void trade_unit::OnRspQryParkedOrderAction(
    CThostFtdcParkedOrderActionField *pParkedOrderAction,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pParkedOrderAction:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d",
        userid_.c_str(), pParkedOrderAction, pRspInfo, nRequestID, bIsLast);
}

///�����ѯ����֪ͨ��Ӧ
void trade_unit::OnRspQryTradingNotice(
    CThostFtdcTradingNoticeField *pTradingNotice,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pTradingNotice:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d",
        userid_.c_str(), pTradingNotice, pRspInfo, nRequestID, bIsLast);
}

///�����ѯ���͹�˾���ײ�����Ӧ
void trade_unit::OnRspQryBrokerTradingParams(
    CThostFtdcBrokerTradingParamsField *pBrokerTradingParams,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pBrokerTradingParams:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d",
        userid_.c_str(), pBrokerTradingParams, pRspInfo, nRequestID, bIsLast);
}

///�����ѯ���͹�˾�����㷨��Ӧ
void trade_unit::OnRspQryBrokerTradingAlgos(
    CThostFtdcBrokerTradingAlgosField *pBrokerTradingAlgos,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pBrokerTradingAlgos:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d",
        userid_.c_str(), pBrokerTradingAlgos, pRspInfo, nRequestID, bIsLast);
}

///�����ѯ��������û�����
void trade_unit::OnRspQueryCFMMCTradingAccountToken(
    CThostFtdcQueryCFMMCTradingAccountTokenField *pQueryCFMMCTradingAccountToken,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pQueryCFMMCTradingAccountToken:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d",
        userid_.c_str(), pQueryCFMMCTradingAccountToken, pRspInfo, nRequestID, bIsLast);
}

///���з��������ʽ�ת�ڻ�֪ͨ
void trade_unit::OnRtnFromBankToFutureByBank(
    CThostFtdcRspTransferField *pRspTransfer)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRspTransfer:%p",
        userid_.c_str(), pRspTransfer);
}

///���з����ڻ��ʽ�ת����֪ͨ
void trade_unit::OnRtnFromFutureToBankByBank(
    CThostFtdcRspTransferField *pRspTransfer)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRspTransfer:%p",
        userid_.c_str(), pRspTransfer);
}

///���з����������ת�ڻ�֪ͨ
void trade_unit::OnRtnRepealFromBankToFutureByBank(
    CThostFtdcRspRepealField *pRspRepeal)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRspRepeal:%p",
        userid_.c_str(), pRspRepeal);
}

///���з�������ڻ�ת����֪ͨ
void trade_unit::OnRtnRepealFromFutureToBankByBank(
    CThostFtdcRspRepealField *pRspRepeal)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRspRepeal:%p",
        userid_.c_str(), pRspRepeal);
}

///�ڻ����������ʽ�ת�ڻ�֪ͨ
void trade_unit::OnRtnFromBankToFutureByFuture(
    CThostFtdcRspTransferField *pRspTransfer)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRspTransfer:%p",
        userid_.c_str(), pRspTransfer);
}

///�ڻ������ڻ��ʽ�ת����֪ͨ
void trade_unit::OnRtnFromFutureToBankByFuture(
    CThostFtdcRspTransferField *pRspTransfer)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRspTransfer:%p",
        userid_.c_str(), pRspTransfer);
}

///ϵͳ����ʱ�ڻ����ֹ������������ת�ڻ��������д�����Ϻ��̷��ص�֪ͨ
void trade_unit::OnRtnRepealFromBankToFutureByFutureManual(
    CThostFtdcRspRepealField *pRspRepeal)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRspRepeal:%p",
        userid_.c_str(), pRspRepeal);
}

///ϵͳ����ʱ�ڻ����ֹ���������ڻ�ת�����������д�����Ϻ��̷��ص�֪ͨ
void trade_unit::OnRtnRepealFromFutureToBankByFutureManual(
    CThostFtdcRspRepealField *pRspRepeal)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRspRepeal:%p",
        userid_.c_str(), pRspRepeal);
}

///�ڻ������ѯ�������֪ͨ
void trade_unit::OnRtnQueryBankBalanceByFuture(
    CThostFtdcNotifyQueryAccountField *pNotifyQueryAccount)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pNotifyQueryAccount:%p",
        userid_.c_str(), pNotifyQueryAccount);
}

///�ڻ����������ʽ�ת�ڻ�����ر�
void trade_unit::OnErrRtnBankToFutureByFuture(
    CThostFtdcReqTransferField *pReqTransfer,
    CThostFtdcRspInfoField *pRspInfo)
{
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pReqTransfer:%p , pRspInfo:%p",
        userid_.c_str(), pReqTransfer, pRspInfo);
}

///�ڻ������ڻ��ʽ�ת���д���ر�
void trade_unit::OnErrRtnFutureToBankByFuture(
    CThostFtdcReqTransferField *pReqTransfer,
    CThostFtdcRspInfoField *pRspInfo)
{
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pReqTransfer:%p , pRspInfo:%p",
        userid_.c_str(), pReqTransfer, pRspInfo);
}

///ϵͳ����ʱ�ڻ����ֹ������������ת�ڻ�����ر�
void trade_unit::OnErrRtnRepealBankToFutureByFutureManual(
    CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo)
{
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pReqRepeal:%p , pRspInfo:%p",
        userid_.c_str(), pReqRepeal, pRspInfo);
}

///ϵͳ����ʱ�ڻ����ֹ���������ڻ�ת���д���ر�
void trade_unit::OnErrRtnRepealFutureToBankByFutureManual(
    CThostFtdcReqRepealField *pReqRepeal, CThostFtdcRspInfoField *pRspInfo)
{
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pReqRepeal:%p , pRspInfo:%p",
        userid_.c_str(), pReqRepeal, pRspInfo);
}

///�ڻ������ѯ����������ر�
void trade_unit::OnErrRtnQueryBankBalanceByFuture(
    CThostFtdcReqQueryAccountField *pReqQueryAccount,
    CThostFtdcRspInfoField *pRspInfo)
{
    TRACE_ERROR(AT_TRACE_TAG, NAUT_AT_E_UNKNOWN_TRADE_CMD, "tu(%s): pReqQueryAccount:%p , pRspInfo:%p",
        userid_.c_str(), pReqQueryAccount, pRspInfo);
}

///�ڻ������������ת�ڻ��������д�����Ϻ��̷��ص�֪ͨ
void trade_unit::OnRtnRepealFromBankToFutureByFuture(
    CThostFtdcRspRepealField *pRspRepeal)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRspRepeal:%p",
        userid_.c_str(), pRspRepeal);
}

///�ڻ���������ڻ�ת�����������д�����Ϻ��̷��ص�֪ͨ
void trade_unit::OnRtnRepealFromFutureToBankByFuture(
    CThostFtdcRspRepealField *pRspRepeal)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pRspRepeal:%p",
        userid_.c_str(), pRspRepeal);
}

///�ڻ����������ʽ�ת�ڻ�Ӧ��
void trade_unit::OnRspFromBankToFutureByFuture(
    CThostFtdcReqTransferField *pReqTransfer,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pReqTransfer:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d",
        userid_.c_str(), pReqTransfer, pRspInfo, nRequestID, bIsLast);
}

///�ڻ������ڻ��ʽ�ת����Ӧ��
void trade_unit::OnRspFromFutureToBankByFuture(
    CThostFtdcReqTransferField *pReqTransfer,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pReqTransfer:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d",
        userid_.c_str(), pReqTransfer, pRspInfo, nRequestID, bIsLast);
}

///�ڻ������ѯ�������Ӧ��
void trade_unit::OnRspQueryBankAccountMoneyByFuture(
    CThostFtdcReqQueryAccountField *pReqQueryAccount,
    CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pReqQueryAccount:%p, pRspInfo:%p, nRequestID:%d, bIsLast:%d",
        userid_.c_str(), pReqQueryAccount, pRspInfo, nRequestID, bIsLast);
}

///���з������ڿ���֪ͨ
void trade_unit::OnRtnOpenAccountByBank(
    CThostFtdcOpenAccountField *pOpenAccount)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pOpenAccount:%p ",
        userid_.c_str(), pOpenAccount);
}

///���з�����������֪ͨ
void trade_unit::OnRtnCancelAccountByBank(
    CThostFtdcCancelAccountField *pCancelAccount)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pCancelAccount:%p ",
        userid_.c_str(), pCancelAccount);
}

///���з����������˺�֪ͨ
void trade_unit::OnRtnChangeAccountByBank(
    CThostFtdcChangeAccountField *pChangeAccount)
{
    TRACE_SYSTEM(AT_TRACE_TAG, "tu(%s): pChangeAccount:%p ",
        userid_.c_str(), pChangeAccount);
}

}

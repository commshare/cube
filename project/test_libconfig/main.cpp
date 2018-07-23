/******************************************************************************
Copyright (c) 2016. All Rights Reserved.

FileName: config.h
Version: 1.0
Date: 2016.1.13

History:
eric         2017.9.26   1.0     Create
******************************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_

#include <string>
#include <iostream>
#include "libmysql/unidb.h"
#include "libconfig/config_meta.h"
#include "libconfig/config_section_def.h"

struct tcp_info
{
    std::string ip;
    int port;
    void BindStruct(utility::TiXmlTieList &tl, bool)
    {
        tl.AddItem(ip, "ip");
        tl.AddItem(port, "port");
    }
};

struct DbInfo
{
    std::string dbtype; //dbtype eg:mysql oracle
    database::unidb_param dbparam;

    DbInfo()
        : dbtype("mysql")
    {
    }

    void BindStruct(utility::TiXmlTieList &tl, bool)
    {
        tl.AddItem(dbtype, "type");
        tl.AddItem(dbparam.host, "host");
        tl.AddItem(dbparam.port, "port");
        tl.AddItem(dbparam.user, "user");
        tl.AddItem(dbparam.password, "pwd");
        tl.AddItem(dbparam.database_name, "dbname");
        tl.AddItem(dbparam.charset, "charset");
    }
};

struct MysqlTableInfo
{
    std::string calendar_table;
    std::string instrument_table;
    std::string insmrgnrate_table;
    std::string dcepair_inscom_table;
    std::string bcalendar_table;
    std::string fucontract_table;
    std::string product_table;
    std::string hsholdsinfo_table;
    std::string stlinvestorposition_table;
    std::string investordelivery_table;
    std::string exchmarginrate_table;
    std::string mrgnratetmpl_table;
    std::string mrgnratetmplhldadj_table;
    std::string mrgnratetmplpriceadj_table;
    std::string productcomtmpl_table;
    std::string investor_table;
    MysqlTableInfo()
            : calendar_table(""),
              instrument_table(""),
              insmrgnrate_table(""),
              dcepair_inscom_table(""),
              bcalendar_table(""),
              fucontract_table(""),
              product_table(""),
              hsholdsinfo_table(""),
              stlinvestorposition_table(""),
              investordelivery_table(""),
              exchmarginrate_table(""),
              mrgnratetmpl_table(""),
              mrgnratetmplhldadj_table(""),
              mrgnratetmplpriceadj_table(""),
              productcomtmpl_table(""),
              investor_table("")
    {
    }
    void BindStruct(utility::TiXmlTieList &tl, bool)
    {
        tl.AddItem(calendar_table, "calendar");
        tl.AddItem(instrument_table, "instrument");
        tl.AddItem(insmrgnrate_table, "insmrgnrate");
        tl.AddItem(dcepair_inscom_table, "dcepair_inscom");
        tl.AddItem(bcalendar_table, "bcalendar");
        tl.AddItem(fucontract_table, "fucontract");
        tl.AddItem(product_table, "product");
        tl.AddItem(hsholdsinfo_table, "hsholdsinfo");
        tl.AddItem(stlinvestorposition_table, "stlinvestorposition");
        tl.AddItem(investordelivery_table, "investordelivery");
        tl.AddItem(exchmarginrate_table, "exchmarginrate");
        tl.AddItem(mrgnratetmpl_table, "mrgnratetmpl");
        tl.AddItem(mrgnratetmplhldadj_table, "mrgnratetmplhldadj");
        tl.AddItem(mrgnratetmplpriceadj_table, "mrgnratetmplpriceadj");
        tl.AddItem(productcomtmpl_table, "productcomtmpl");
        tl.AddItem(investor_table, "investor");
    }
};

struct OracleTableInfo
{
    std::string calendar_table;
    std::string instrument_table;
    std::string insmrgnrate_table;
    std::string dcepair_inscom_table;
    std::string bcalendar_table;
    std::string operation_instrument_table;
    std::string settlement_instrument_table;
    std::string settlement_cominstrument_table;
    std::string settlement_product_table;
    std::string hsholdsinfo_table;
    std::string investordelivery_table;
    std::string exchmarginrate_table;
    std::string mrgnratetmpl_table;
    std::string mrgnratetmplhldadj_table;
    std::string mrgnratetmplpriceadj_table;
    std::string productcomtmpl_table;
    std::string settlement_investor_table;
    OracleTableInfo()
            : calendar_table("") ,
              instrument_table(""),
              insmrgnrate_table(""),
              dcepair_inscom_table(""),
              bcalendar_table(""),
              operation_instrument_table(""),
              settlement_instrument_table(""),
              settlement_cominstrument_table(""),
              settlement_product_table(""),
              hsholdsinfo_table(""),
              investordelivery_table(""),
              exchmarginrate_table(""),
              mrgnratetmpl_table(""),
              mrgnratetmplhldadj_table(""),
              mrgnratetmplpriceadj_table(""),
              productcomtmpl_table(""),
              settlement_investor_table("")
    {
    }
    void BindStruct(utility::TiXmlTieList &tl, bool)
    {
        tl.AddItem(calendar_table, "calendar");
        tl.AddItem(instrument_table, "instrument");
        tl.AddItem(insmrgnrate_table, "insmrgnrate");
        tl.AddItem(dcepair_inscom_table, "dcepair_inscom");
        tl.AddItem(bcalendar_table, "bcalendar");
        tl.AddItem(operation_instrument_table, "operation_instrument");
        tl.AddItem(settlement_instrument_table, "settlement_instrument");
        tl.AddItem(settlement_cominstrument_table, "settlement_cominstrument");
        tl.AddItem(settlement_product_table, "settlement_product");
        tl.AddItem(hsholdsinfo_table, "hs_futures_hsholdsinfo");
        tl.AddItem(investordelivery_table, "settlement_investordelivery");
        tl.AddItem(exchmarginrate_table, "settlement_exchmarginrate");
        tl.AddItem(mrgnratetmpl_table, "mrgnratetmpl");
        tl.AddItem(mrgnratetmplhldadj_table, "mrgnratetmplhldadj");
        tl.AddItem(mrgnratetmplpriceadj_table, "mrgnratetmplpriceadj");
        tl.AddItem(productcomtmpl_table, "productcomtmpl");
        tl.AddItem(settlement_investor_table, "settlement_investor");
    }
};

struct AlarmInfo
{
    int hour;
    int minute;
    int second;

    AlarmInfo()
        : hour(0)
        , minute(0)
        , second(0)
    {}
    void BindStruct(utility::TiXmlTieList &tl, bool)
    {
        tl.AddItem(hour, "hour");
        tl.AddItem(minute, "minute");
        tl.AddItem(second, "second");
    }
};

struct time_duration
{
    std::string start_time;
    std::string stop_time;
    void BindStruct(utility::TiXmlTieList &tl, bool)
    {
        tl.AddItem(start_time, "start_time");
        tl.AddItem(stop_time, "stop_time");

    }
};

class Config : public CConfigMeta
{
public:
    Config(){}
    virtual ~Config(){}

    int log_level;
    bool load_once;
    bool load_bcalendar;
    bool load_insmrgnrate;
    struct tcp_info trial_server;
    DbInfo db_mysql;
    DbInfo db_oracle;
    MysqlTableInfo table_config;
    OracleTableInfo oracle_table_config;
    AlarmInfo alarm_info;
    std::string open_work_time;
    std::string close_work_time;
    time_duration hsholds_time;
    int hsholds_interval;
    virtual void GetItems(long SectionIDs, utility::TiXmlTieList &tl)
    {
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, log_level);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, load_once);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, load_bcalendar);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, load_insmrgnrate);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, trial_server);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, db_mysql);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, db_oracle);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, table_config);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, oracle_table_config);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, alarm_info);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, open_work_time);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, close_work_time);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, hsholds_time);
        DEFINECONFIGITEM(CONFIG_SECTION_ALL, hsholds_interval);
    }
};

int main()
{
    std::cout << "hello world" << std::endl;
    Config conf;
    conf.LoadFile("./db_server_config.xml", CONFIG_SECTION_ALL);

    std::cout << conf.db_mysql.dbparam.database_name << std::endl;
    std::cout << conf.table_config.calendar_table << std::endl;

    getchar();

    return 0;
}

#endif


/*****************************************************************************
 libmysql Copyright (c) 2020. All Rights Reserved.

 FileName: dbmysql.h
 Version: 1.0
 Date: 2018.7.2

 History:
 eric     2018.7.2   1.0     Create
 ******************************************************************************/

#ifndef DBMYSQL_H_
#define DBMYSQL_H_

#include "base.h"
#ifdef _WIN32   
#include "winsock.h"
#endif 

#include "unidb.h"
#include "mysql.h"
#include <map>
#include <unordered_map>

namespace database
{
class string_hash {
public:
    size_t operator()(const char* str) const {
        size_t hash = 1315423911;
        while (*str){
            hash ^= ((hash << 5) + (*str++) + (hash >> 2));
        }
        return (hash & 0x7FFFFFFF);
    }
};

class string_compare {
public:
    bool operator()(const char* a, const char* b) const {
        return (strcmp(a, b) == 0);
    }
};

    /*********************************************************************/

    class db_mysql_conn;
    class db_mysql
        : public unidb_base
    {
        friend class db_mysql_conn;

    public:
        db_mysql();
        virtual ~db_mysql();

    public:
        virtual int open(const unidb_param& params);
        virtual int close();
        virtual bool is_open();
        virtual bool connected();

        virtual std::string host_info();
        virtual std::string server_info();
        virtual std::string charset();
        virtual std::string engine_type() const;

        virtual const unidb_param& params() const;

    protected:
        int create_database_if_not_exists(const char* database_name);
        int recreate_database_if_exists(const char* database_name);

    protected:
        void add_connection(db_mysql_conn* conn);
        void release_connection(db_mysql_conn* conn);

    private:
        unidb_param params_;
        MYSQL mysql_;
        bool is_open_;
        volatile long conn_count_;
        int last_error_;
    };

    /*********************************************************************/

    class db_mysql_conn
        : public unidb_conn_base
    {
    public:
        db_mysql_conn(unidb_base* db);
        virtual ~db_mysql_conn();

    public:
        virtual bool init_conn();
        virtual bool release_conn();

        virtual bool connected();
        virtual bool begin_transaction();
        virtual bool commit();
        virtual bool rollback();
        virtual bool set_auto_commit(bool auto_commit);
        virtual bool is_transaction();

        virtual std::string get_error();
        virtual int get_errno();
        virtual std::string get_error_inner();
        virtual int get_errno_inner();

    public:
        virtual bool execute(const char* sql);
        virtual bool query(const char* sql);
        virtual long get_count() const;
        virtual long fields_count() const;
        virtual void free_result();
        virtual std::vector<std::string> get_fields();

        virtual bool first();
        virtual bool fetch_row();
        virtual int affected_rows();

        virtual bool is_null(int id);
        virtual bool is_null(const std::string& field_name);
        virtual bool is_null(const char* field_name);
        virtual const char* get_string(int id);
        virtual const char* get_string(const std::string& field_name);
        virtual const char* get_string(const char* field_name);
        virtual double get_double(int id);
        virtual double get_double(const std::string& field_name);
        virtual double get_double(const char* field_name);
        virtual long get_long(int id);
        virtual long get_long(const std::string& field_name);
        virtual long get_long(const char* field_name);
        virtual int64_t get_int64(int id);
        virtual int64_t get_int64(const std::string& field_name);
        virtual int64_t get_int64(const char* field_name);

    protected:
        bool reconnect();
        int real_connect();

    private:
        unidb_base* db_;
        MYSQL mysql_;
        MYSQL_RES* result_;
        MYSQL_ROW row_;
        std::unordered_map<const char*, int, string_hash, string_compare> map_fields_;
        bool is_transaction_;
        int num_cols_;
        bool initialized_;
        int last_error_;
        int last_error_inner_;
        std::string slast_error_inner_;
    };

}

#endif
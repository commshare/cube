#pragma once
#include <list>
#include "light_ptr.h"
#include "Buffer.h"

namespace light
{
class clientreceivehandle;
struct transaction_head;
struct transaction_additional;
class clientapi
{
public:
    clientapi(boost::asio::ip::tcp::endpoint ed);
    ~clientapi();
    void set_handle(clientreceivehandle* phandle);
    void run();
    void send(const transaction_head& head, const char* body);

private:
    void run_io(boost::asio::io_service* ps);
    void close(const std::string& message);
    void deliver(transaction_ptr trans);
    void pushtrans(transaction_ptr trans);
    void do_connect(boost::asio::ip::tcp::endpoint ed);
    void do_send(const transaction_head& head, const char* body);
    void do_receive(const transaction_head& head, 
        const transaction_additional& additional, const char* body);
    void do_read();
    void do_write();

    void handle_connect(boost::system::error_code ec);
    void handle_read(boost::system::error_code ec, std::size_t length);
    void handle_write(boost::system::error_code ec, std::size_t length);
    void handler_write_timer(const boost::system::error_code& error);
    void handler_read_timer(const boost::system::error_code& error);
    void handler_reconnect_timer(const boost::system::error_code& error);
    void handler_close();

private:
    static const int HeartbeatTime = 10;
    static const int ReconnectTime = 10;
    
    boost::asio::io_service io_service_;
    work_ptr work_;
    thread_ptr thread_;
    socket_ptr socket_;
    boost::asio::ip::tcp::endpoint endpoint_;
    clientreceivehandle* receive_handle_;
    Buffer buffer_read_;
    Buffer buffer_write_;
    std::list<transaction_ptr> cache_write_;
    std::string receive_buffer_;
    bool is_connected_;

    std::shared_ptr<boost::asio::deadline_timer> heartbeat_send_timer_;
    std::shared_ptr<boost::asio::deadline_timer> heartbeat_timer_;
    std::shared_ptr<boost::asio::deadline_timer> reconnect_timer_;
    transaction_ptr heartbeat_transaction_;
};
};

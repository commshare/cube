#pragma once

#include <algorithm>
#include <deque>
#include <memory>
#include <set>
#include "boost/thread.hpp"
#include "boost/asio.hpp"
#include "light_ptr.h"

namespace light
{
class LightService;
class Server
{
public:
    Server(const boost::asio::ip::tcp::endpoint& endpoint, int rwthread = 1);
    ~Server();
    void start();
    void stop();

private:
    void do_accept();
    void handle_accept(light::socket_ptr sock, std::shared_ptr<LightService>& service, 
        const boost::system::error_code& err);
    std::shared_ptr<LightService>& get_service();

private:
    boost::asio::io_service accept_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<LightService> > rw_services_;
    int rwthreadnum_;
};
};
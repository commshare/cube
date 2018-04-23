#include <iostream>
#include "server.h"
#include "session.h"
#include "business_unit_group.h"
#include "light_service.h"
#include "dispatcher.h"
#include "liblog/log.h"

light::Server::Server(const boost::asio::ip::tcp::endpoint& endpoint, int rwthread)
	: acceptor_(accept_service_, endpoint)
{
	rwthreadnum_ = rwthread > 0 ? rwthread : 1;
	for (int rwt = 0; rwt < rwthreadnum_; ++rwt)
	{
		rw_services_.push_back(std::make_shared<LightService>(rwt));
		Dispatcher::_Instance().RegisterLightService(*(rw_services_.rbegin()));
	}
	do_accept();
}
light::Server::~Server()
{
    STLOG_WARN << "server destroyed...";
}
void light::Server::start()
{
	for (auto it = rw_services_.begin(); it!= rw_services_.end(); ++it)
	{
		(*it)->start();
	}
}
void light::Server::stop() {
    accept_service_.stop();
    while(accept_service_.stopped()) {
        break;
    }
    for(auto it = rw_services_.begin(); it != rw_services_.end(); ++it) {
        if(*it) {
            (*it)->stop();
        }
    }
    rw_services_.clear();
}
void light::Server::run()
{
	start();
	accept_service_.run();
}

void light::Server::do_accept()
{
	//todo multi rw_io_service
	std::shared_ptr<LightService>& service = get_service();
	service->pre_add_session();
	light::socket_ptr sock = std::make_shared<boost::asio::ip::tcp::socket>(service->get_io_service());
	acceptor_.async_accept(*sock, std::bind(&light::Server::handle_accept, this, sock, service, std::placeholders::_1));
}

void light::Server::handle_accept( light::socket_ptr sock, std::shared_ptr<LightService>& service, const boost::system::error_code& err)
{
	if (!err)
	{
		sock->get_io_service().post(std::bind(&Session::start, std::make_shared<Session>(sock, service)));
	}else
	{
	    STLOG_FATAL << "accept error: " << err.message() << std::endl;
	}
	do_accept();
}

std::shared_ptr<light::LightService>& light::Server::get_service()
{
	auto it = std::min_element(rw_services_.begin(), rw_services_.end(), [](const std::shared_ptr<LightService>& ptr1, const std::shared_ptr<LightService>& ptr2) {
		return ptr1->get_sessions() < ptr2->get_sessions();
	});
	return *it;
}

 

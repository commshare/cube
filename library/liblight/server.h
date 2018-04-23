#pragma once
//
// serverlight.h
// ~~~~~~~~~~~~~~~
//
// Copyright (c) 2016-2016 shaotang.shi
//
//
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
		void run();
		void stop();
	private:
		void	do_accept();
		void	start();
		void	handle_accept(light::socket_ptr sock, std::shared_ptr<LightService>& service, const boost::system::error_code&	err);
		boost::asio::io_service accept_service_;
		boost::asio::ip::tcp::acceptor acceptor_;
		std::vector<std::shared_ptr<LightService> > rw_services_;
		std::shared_ptr<LightService>& get_service();
		int rwthreadnum_;
	};
};


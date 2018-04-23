#pragma once
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <memory>
#include <deque>

namespace light
{
	class transaction;
	class participant;
	typedef std::shared_ptr<participant> participant_ptr;
	typedef std::shared_ptr<boost::asio::io_service::work> work_ptr;
	typedef std::shared_ptr<boost::thread> thread_ptr;
	typedef std::shared_ptr<transaction> transaction_ptr;
	typedef	std::shared_ptr<boost::asio::ip::tcp::socket>	socket_ptr;
}
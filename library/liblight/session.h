#pragma once
#include <algorithm>
#include <list>
#include "light_ptr.h"
#include "transaction.h"
#include "Buffer.h"
#include "participant.h"

namespace light
{
	class BusinessUnitGroup;
	class LightService;
	class Session
		: public participant,
		public std::enable_shared_from_this<Session>
	{
	public:
		static const size_t Min_Left_Size = 1024;
		static const int HeartbeatTime = 60;
		Session(socket_ptr socket, std::shared_ptr<LightService>& ls);
		void	start();
		void	send(const light::transaction_head& head, const char* body);
		void	set_context(void* context);
		void*	get_context();
		void	close(const std::string& message);
		~Session();
	private:
		void	deliver(transaction_ptr trans);
		void	do_send(const light::transaction_head& head, const char* body);
		void	pushtrans(transaction_ptr trans);
		void	do_receive(const light::transaction_head& head, const light::transaction_additional& additional, const char* body);
		void	do_read();
		void	do_write();
		void	handler_close();
		void	handle_read(boost::system::error_code ec, std::size_t length);
		void	handle_write(boost::system::error_code ec, std::size_t length);
		void	handler_write_timer(const boost::system::error_code& error);
		void	handler_read_timer(const boost::system::error_code& error);
		light::socket_ptr socket_;
		Buffer buffer_read_;
		Buffer buffer_write_;
		std::list<transaction_ptr> cache_write_;
		std::shared_ptr<boost::asio::deadline_timer> heartbeat_send_timer_; 
		std::shared_ptr<boost::asio::deadline_timer> heartbeat_timer_; 
		transaction_ptr heartbeat_transaction_;
		bool is_connected_;
		void* context_;
		std::shared_ptr<LightService> light_service_;
		std::string receive_buffer_;
	};
};

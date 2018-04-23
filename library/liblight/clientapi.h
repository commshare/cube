#pragma once
#include <algorithm>
#include <list>
#include <boost/asio.hpp>
#include "light_ptr.h"
#include "Buffer.h"

namespace light
{
	class clientreceivehandle;
	class LightService;
	struct transaction_head;
	struct transaction_additional;
	class clientapi
	{
	public:
		clientapi(boost::asio::ip::tcp::endpoint ed);
		void send(const light::transaction_head& head, const char* body);
		void set_handle(clientreceivehandle* phandle);
		void run();
		~clientapi();
	private:
		void	do_send(const light::transaction_head& head, const char* body);
		void	do_receive(const light::transaction_head& head, const light::transaction_additional& additional, const char* body);
		void	close(const std::string& message);
		void	do_connect(boost::asio::ip::tcp::endpoint ed);
		void	do_read();
		void	do_write();
		void	deliver( transaction_ptr trans );
		void	pushtrans(transaction_ptr trans);
		void	handle_read(boost::system::error_code ec, std::size_t length);
		void	handle_write(boost::system::error_code ec, std::size_t length);
		void	handle_connect(boost::system::error_code ec);
		void	run_io(boost::asio::io_service* ps);
		void	handler_write_timer( const boost::system::error_code& error );
		void	handler_read_timer( const boost::system::error_code& error );
		void	handler_reconnect_timer( const boost::system::error_code& error );
		void	handler_close();
		static const int HeartbeatTime = 10;
		static const int ReconnectTime = 10;
	private:
		boost::asio::io_service io_service_;
		light::socket_ptr socket_;
		Buffer buffer_write_;
		Buffer buffer_read_;
		std::list<transaction_ptr> cache_write_;
		work_ptr work_;
		thread_ptr thread_;
		clientreceivehandle* receive_handle_;
		int write_pack_count_;
		int write_pack_length_;
		std::shared_ptr<boost::asio::deadline_timer> heartbeat_send_timer_; 
		std::shared_ptr<boost::asio::deadline_timer> heartbeat_timer_; 
		std::shared_ptr<boost::asio::deadline_timer> reconnect_timer_; 
		transaction_ptr heartbeat_transaction_;
		boost::asio::ip::tcp::endpoint endpoint_;
		bool is_connected_;
	private:
		std::shared_ptr<LightService> service_;
		std::string receive_buffer_;
	};
};

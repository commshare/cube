#pragma once
#include <memory>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include "light_ptr.h"

namespace light
{
	class Session;
	class LightService
		: public std::enable_shared_from_this<LightService>
	{
	public:
		LightService(int id);
		~LightService();
		void start();
		void stop();
		void add_session(std::shared_ptr<Session> s);
		void del_session(std::shared_ptr<Session> s);
		void send(int sessionid, transaction_ptr trans);
		inline void pre_add_session() { ++pre_add_session_; };
		inline int get_sessions() const { return sessions_.size() + pre_add_session_; };
		inline int serviceid() const { return serviceid_; };
		boost::asio::io_service& get_io_service();
	private:
		typedef std::shared_ptr<boost::asio::io_service::work> work_ptr;
		typedef std::shared_ptr<boost::thread> thread_ptr;
		void run();
		inline int GenerateSessionSerial() { return ++session_serial_; };
		boost::asio::io_service io_service_;
		work_ptr work_;
		thread_ptr thread_;
		std::map<int, std::shared_ptr<Session> > sessions_;
		int serviceid_;
		int session_serial_;
		std::atomic_int pre_add_session_;
	};
};

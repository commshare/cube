#pragma once
#include <algorithm>
#include <set>
#include <functional>
#include <boost/thread/mutex.hpp>
#include "light_ptr.h"

namespace light
{
	class transaction;
	class BusinessUnit;
	class BusinessUnitGroup
		: public std::enable_shared_from_this<BusinessUnitGroup>
	{
	public:
		BusinessUnitGroup(const std::string& name);
		~BusinessUnitGroup();
		inline const std::string& room_name() const { return group_name_; };
		void deliver(BusinessUnit* unit, participant_ptr participant, transaction_ptr trans);
		void deliver(BusinessUnit* unit, participant_ptr participant, int eventid);
		void RegisterTimer(int timeinterval, BusinessUnit* bu);
		void start();
		void stop();
		bool is_stopped() { return processor_service_.stopped();}
		boost::asio::io_service& get_io_service() { return processor_service_; };
	private:
		class Timer
			: public std::enable_shared_from_this<Timer>
		{
		public:
			Timer(boost::asio::io_service* service, int interval);
			void Register(std::function<void(int, int)> tf);
			void start();
			void stop();
			void handler_timer(const boost::system::error_code& error);
		private:
			std::shared_ptr<boost::asio::deadline_timer> timer_;
			std::vector<std::function<void(int, int)> > processors_;
			boost::asio::io_service* service_;
			int interval_;
		};
		void DoProcessTransaction(BusinessUnit* unit, participant_ptr parti, transaction_ptr trans);
		void DoProcessEvent(BusinessUnit* unit, participant_ptr parti, int eventid);
		void run( boost::asio::io_service* io_service );
		boost::asio::io_service processor_service_;
		light::thread_ptr iothread_;
		light::work_ptr work_ ;
		std::string		group_name_;
		std::map<int, std::shared_ptr<Timer> > clocks_;
	};
};


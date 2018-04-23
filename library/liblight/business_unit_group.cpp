#include <iostream>
#include "business_unit_group.h"
#include "business_unit.h"
#include "transaction.h"
#include "dispatcher.h"
#include "event_def.h"
#include "participant.h"
#include "liblog/log.h"

light::BusinessUnitGroup::BusinessUnitGroup(const std::string& name) : group_name_(name)
{

}

light::BusinessUnitGroup::~BusinessUnitGroup()
{
    STLOG_WARN << "destroy BusinessUnitGroup " << group_name_;
}

void light::BusinessUnitGroup::start()
{
	work_= std::make_shared<boost::asio::io_service::work>(processor_service_);
	iothread_ = light::thread_ptr(new boost::thread(std::bind(&BusinessUnitGroup::run, shared_from_this(), &processor_service_)));
	for (auto it = clocks_.begin(); it != clocks_.end(); ++it)
	{
		if (it->second)
		{
			it->second->start();
		}
	}
	STLOG_INFO << "threadid: " << iothread_->get_id() << ", group name: " << group_name_;
}

void light::BusinessUnitGroup::stop()
{
    processor_service_.stop();
    while(processor_service_.stopped()) {
        iothread_->join();
        break;
     }
    for(auto it = clocks_.begin(); it != clocks_.end(); ++it) {
        it->second->stop();
    }
    clocks_.clear();
    if(work_) work_.reset();
}

void light::BusinessUnitGroup::DoProcessTransaction(BusinessUnit* unit, participant_ptr parti, transaction_ptr trans )
{
	if (unit)
	{
		unit->OnReceive(parti? parti->sessionid_ : 0, trans->get_head_ptr()->transaction_type_, trans->get_body_string());
	}
}

void light::BusinessUnitGroup::DoProcessEvent(BusinessUnit* unit, participant_ptr parti, int eventid)
{
	if (unit)
	{
		unit->OnEvent(parti->sessionid_, eventid);
	}
}

void light::BusinessUnitGroup::run( boost::asio::io_service* io_service )
{
	io_service->run();
	STLOG_WARN << group_name_ << " io service loop end";
}

void light::BusinessUnitGroup::deliver(BusinessUnit* unit, participant_ptr participant, transaction_ptr trans)
{
	processor_service_.post(std::bind(&BusinessUnitGroup::DoProcessTransaction, shared_from_this(), unit, participant, trans));
}

void light::BusinessUnitGroup::RegisterTimer(int timeinterval, BusinessUnit* bu)
{
	auto it = clocks_.find(timeinterval);
	if (it != clocks_.end())
	{
		it->second->Register(std::bind(&BusinessUnit::OnEvent, bu, std::placeholders::_1, std::placeholders::_2));
	}
	else
	{
		std::shared_ptr<Timer> timer = std::make_shared<Timer>(&processor_service_, timeinterval);
		timer->Register(std::bind(&BusinessUnit::OnEvent, bu, std::placeholders::_1, std::placeholders::_2));
		clocks_[timeinterval] = timer;
	}
}

void light::BusinessUnitGroup::deliver(BusinessUnit* unit, participant_ptr participant, int eventid)
{
	processor_service_.post(std::bind(&BusinessUnitGroup::DoProcessEvent, shared_from_this(), unit, participant, eventid));
}

light::BusinessUnitGroup::Timer::Timer(boost::asio::io_service* service, int interval)
	: service_(service)
	, interval_(interval)
{
	
}

void light::BusinessUnitGroup::Timer::Register(std::function<void(int, int)> tf)
{
	processors_.push_back(tf);
}

void light::BusinessUnitGroup::Timer::start()
{
	timer_ = std::make_shared<boost::asio::deadline_timer>(*service_, boost::posix_time::seconds(interval_));
	timer_->expires_from_now(boost::posix_time::seconds(interval_));
	timer_->async_wait(std::bind(&light::BusinessUnitGroup::Timer::handler_timer, shared_from_this(), std::placeholders::_1));
}

void light::BusinessUnitGroup::Timer::stop()
{
    timer_->cancel();
}

void light::BusinessUnitGroup::Timer::handler_timer(const boost::system::error_code& error)
{
	if (error != boost::asio::error::operation_aborted)
	{
		// Timer was not cancelled, take necessary action.
		for (auto it = processors_.begin(); it != processors_.end(); ++it)
		{
			if (*it)
			{
				(*it)(0, light::LightEvent::OnTimer);
			}
		}
		timer_->expires_from_now(boost::posix_time::seconds(interval_));
		timer_->async_wait(std::bind(&light::BusinessUnitGroup::Timer::handler_timer, shared_from_this(), std::placeholders::_1));
	}
}

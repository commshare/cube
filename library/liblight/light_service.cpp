#include "light_service.h"
#include "session.h"
#include "liblog/log.h"
#include <iostream>
#include <thread>

light::LightService::LightService(int id)
	: serviceid_(id)
	, session_serial_(0)
	, pre_add_session_(0)
{
}

light::LightService::~LightService()
{
    STLOG_WARN << "LightService destroyed..." << serviceid_;
}

void light::LightService::start()
{
	work_ = std::make_shared<boost::asio::io_service::work>(io_service_);
	thread_ = std::make_shared<boost::thread>(std::bind(&light::LightService::run, shared_from_this()));
	STLOG_ERROR << "start light service thread...thread_id: " << thread_->get_id() << std::endl;
}

void light::LightService::stop()
{
    for(auto it = sessions_.begin(); it != sessions_.end(); ++it) {
        it->second->close("closed for stop");
    }
    while(!sessions_.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    };
    sessions_.clear();
    io_service_.stop();
    while(io_service_.stopped()) {
        thread_->join();
        break;
    }
    if(work_) work_.reset();
}

void light::LightService::add_session(std::shared_ptr<Session> s)
{
	--pre_add_session_;
	int childid = GenerateSessionSerial();
	s->sessionid_ = (serviceid_ << 16) + childid;
	sessions_[s->sessionid_] = s;
	STLOG_INFO << "light service "
		<< "0x" << std::setbase(16) << serviceid_ << " add a new session, sessionid " << "0x" << s->sessionid_ << ", session amount now is " << sessions_.size() << std::endl;
}

boost::asio::io_service& light::LightService::get_io_service()
{
	return io_service_;
}

void light::LightService::del_session(std::shared_ptr<Session> s)
{
	auto it = sessions_.find(s->sessionid_);
	if (it != sessions_.end())
	{
		sessions_.erase(it->first);
		STLOG_INFO << "light service " << thread_->get_id() << " delete a session, session amount now is " << sessions_.size() << std::endl;
	}
	else
	{
		STLOG_ERROR << "light service " << s->sessionid_ << " is invalid...";
	}
}

void light::LightService::send(int sessionid, transaction_ptr trans)
{
	auto it = sessions_.find(sessionid);
	if (sessions_.end() != it && it->second)
	{
		it->second->send(*trans->get_head_ptr(), trans->get_body_ptr());
	}
}

void light::LightService::run()
{
	io_service_.run();
}

#include "light_service.h"
#include <iostream>
#include <thread>
#include "liblog/log.h"
#include "session.h"

namespace light
{
LightService::LightService(int id)
    : serviceid_(id)
    , session_serial_(0)
    , pre_add_session_(0)
{
}

LightService::~LightService()
{
    STLOG_INFO << "LightService destructor..." << serviceid_;
}

void LightService::start()
{
    work_ = std::make_shared<boost::asio::io_service::work>(io_service_);
    thread_ = std::make_shared<boost::thread>(std::bind(&LightService::run, shared_from_this()));
    STLOG_INFO << "start LightService thread, thread_id: " << thread_->get_id();
}

void LightService::stop()
{
    for (auto val : sessions_) {
        val.second->close("closed for stop");
    }
    while (!sessions_.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(0));
    };
    sessions_.clear();
    io_service_.stop();
    while (io_service_.stopped()) {
        thread_->join();
        break;
    }
    if (work_) work_.reset();
}

void LightService::add_session(std::shared_ptr<Session> s)
{
    --pre_add_session_;
    int childid = GenerateSessionSerial();
    s->sessionid_ = (serviceid_ << 16) + childid;
    sessions_[s->sessionid_] = s;
    STLOG_INFO << "light service: " << "0x" << std::setbase(16) << serviceid_ << " add a new session, sessionid: " 
        << "0x" << s->sessionid_ << ", session amount now is: " << sessions_.size();
}

void LightService::del_session(std::shared_ptr<Session> s)
{
    auto it = sessions_.find(s->sessionid_);
    if (it != sessions_.end())
    {
        sessions_.erase(it->first);
        STLOG_INFO << "light service: " << "0x" << std::setbase(16) << serviceid_ << " del a new session, sessionid: "
            << "0x" << s->sessionid_ << ", session amount now is: " << sessions_.size();
    }
    else
    {
        STLOG_FATAL << "sessionid: " << s->sessionid_ << " is invalid...";
    }
}

void LightService::send(int sessionid, transaction_ptr trans)
{
    auto it = sessions_.find(sessionid);
    if (sessions_.end() != it && it->second)
    {
        it->second->send(*trans->get_head_ptr(), trans->get_body_ptr());
    }
}

void LightService::run()
{
    io_service_.run();
}
}
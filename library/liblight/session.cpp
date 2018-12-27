#include <iostream>
#include "session.h"
#include "dispatcher.h"
#include "light_service.h"
#include "event_def.h"
#include "liblog/log.h"

namespace light
{
Session::Session(socket_ptr socket, std::shared_ptr<LightService>& ls)
    : is_connected_(false)
    , socket_(socket)
    , light_service_(ls)
{
    transaction_head head;
    head.body_length_ = 0;
    head.transaction_type_ = kHeartBeatServer;
    heartbeat_transaction_ = std::make_shared<transaction>(&head);
    heartbeat_send_timer_ = std::make_shared<boost::asio::deadline_timer>
        (socket_->get_io_service(), boost::posix_time::seconds(HeartbeatTime));
    heartbeat_timer_ = std::make_shared<boost::asio::deadline_timer>
        (socket_->get_io_service(), boost::posix_time::seconds(HeartbeatTime));
}

Session::~Session()
{
    STLOG_INFO << "session " << "0x" << std::setbase(16) << sessionid_ << " destructor";
}

void Session::start()
{
    light_service_.lock()->add_session(shared_from_this());
    STLOG_INFO << "session " << "0x" << std::setbase(16) << sessionid_ << " start";
    is_connected_ = true;
    Dispatcher::_Instance().DispatchEvent(shared_from_this(), LightEvent::OnSessionConnected);
    deliver(heartbeat_transaction_);
    heartbeat_timer_->expires_from_now(boost::posix_time::seconds(HeartbeatTime));
    heartbeat_timer_->async_wait(
        std::bind(&Session::handler_read_timer, shared_from_this(), std::placeholders::_1));
    do_read();
}

void Session::stop(const std::string& message)
{
    if (is_connected_)
    {
        STLOG_INFO << "session " << "0x" << std::setbase(16) << sessionid_  
            << " stop, msg: " << message;
        is_connected_ = false;
        Dispatcher::_Instance().DispatchEvent(shared_from_this(), LightEvent::OnSessionDisconnected);
        socket_->get_io_service().post(std::bind(&Session::handler_close, shared_from_this()));
    }
}

void Session::send(const transaction_head& head, const char* body)
{
    //do_send(head, body);
    socket_->get_io_service().post(std::bind(&Session::deliver, shared_from_this(), 
        std::make_shared<transaction>(&head, body)));
}

bool Session::connected() const
{
    return is_connected_;
}

void Session::deliver(transaction_ptr trans)
{
    do_send(*trans->get_head_ptr(), trans->get_body_ptr());
}

void Session::do_send(const transaction_head& head, const char* body)
{
    int capacity = Buffer::kInitialSize - transaction::get_additionallength();
    int curpos = 0;
    transaction_head h(head.transaction_type_, 0);
    transaction_additional addition;
    addition.totallength_ = head.body_length_;
    do {
        h.body_length_ = (head.body_length_ - curpos) > capacity ? 
            capacity : (head.body_length_ - curpos);
        addition.hasnext_ = head.body_length_ - curpos - h.body_length_;
        //socket_->get_io_service().post(std::bind(&Session::deliver, shared_from_this(), std::make_shared<transaction>(&h, &addition, body + curpos)));
        pushtrans(std::make_shared<transaction>(&h, &addition, body + curpos));
        curpos += h.body_length_;
        ++addition.serialnumber_;
    } while (addition.hasnext_);
}

void Session::pushtrans(transaction_ptr trans)
{
    if (is_connected_) {
        bool write_in_progress = buffer_write_.readableBytes();
        if (!cache_write_.empty()) {
            cache_write_.push_back(trans);
        }
        else {
            if (buffer_write_.writableBytes() < (size_t)trans->trasaction_length()) {
                cache_write_.push_back(trans);
            }
            else {
                buffer_write_.append(trans->get_buffer(), trans->trasaction_length());
            }
        }
        if (!write_in_progress) {
            do_write();
        }
    }
}

void Session::do_receive(const transaction_head& head, const transaction_additional& additional, 
    const char* body)
{
    if (!additional.hasnext_ && !additional.serialnumber_) {
        Dispatcher::_Instance().DispatchTransaction(shared_from_this(), 
            std::make_shared<transaction>(&head, &additional, body));
    }
    else {
        //multi pack
        receive_buffer_.append(body, body + head.body_length_);
        if (!additional.hasnext_) {
            if (receive_buffer_.length() != (size_t)additional.totallength_) {
                STLOG_ERROR << "some transactions lost...";
            }
            transaction_head header(head.transaction_type_, receive_buffer_.length());
            Dispatcher::_Instance().DispatchTransaction(shared_from_this(), 
                std::make_shared<transaction>(&header, &additional, &receive_buffer_[0]));
            receive_buffer_.clear();
        }
    }
}

void Session::do_read()
{
    if (is_connected_) {
        socket_->async_read_some(
            boost::asio::buffer(buffer_read_.beginWrite(), buffer_read_.writableBytes()),
            std::bind(&Session::handle_read, shared_from_this(), 
                std::placeholders::_1, std::placeholders::_2));
    }
}

void Session::do_write()
{
    if (is_connected_) {
        socket_->async_write_some(
            boost::asio::buffer(buffer_write_.peek(), buffer_write_.readableBytes()),
            std::bind(&Session::handle_write, shared_from_this(), 
                std::placeholders::_1, std::placeholders::_2));
    }
}

void Session::handle_read(boost::system::error_code ec, std::size_t length)
{
    if (!ec) {
        buffer_read_.hasWritten(length);
        heartbeat_timer_->cancel();
        while (buffer_read_.readableBytes() >= transaction::get_additionallength()) {
            transaction_head* head_ptr = (transaction_head*)buffer_read_.peek();
            if (buffer_read_.readableBytes() < 
                head_ptr->body_length_ + transaction::get_additionallength()) {
                //only head read complete
                if ((size_t)head_ptr->body_length_ > Buffer::kInitialSize - transaction::get_additionallength()) {
                    STLOG_ERROR << "body lenth error, read error session 0x"
                        << std::setbase(16) << sessionid_
                        << ", errorid = " << ec.value()
                        << ", length = " << length
                        << ", bodylenth = " << head_ptr->body_length_
                        << ", transactionid = " << head_ptr->transaction_type_;
                    stop(std::string("handle read error: ") + "bodylenth error");
                    return;
                }
                buffer_read_.ensureWritableBytes( head_ptr->body_length_ + 
                    transaction::get_additionallength() - buffer_read_.readableBytes());
                break;
            }
            else {
                buffer_read_.retrieve(Trans_Head_Length_);
                transaction_additional* addition_ptr = (transaction_additional*)buffer_read_.peek();
                buffer_read_.retrieve(Trans_Head_Additional_Length_);
                if (head_ptr->transaction_type_ != kHeartBeatServer && 
                    head_ptr->transaction_type_ != kHeartBeatClient) {
                    do_receive(*head_ptr, *addition_ptr, buffer_read_.peek());
                }
                else {
                    if (head_ptr->transaction_type_ == kHeartBeatClient) {
                        deliver(std::make_shared<transaction>(head_ptr));
                    }
                    else if (head_ptr->transaction_type_ == kHeartBeatServer) {
                        heartbeat_timer_->cancel();
                    }
                }
                buffer_read_.retrieve(head_ptr->body_length_);
            }
        }
        if (!buffer_read_.readableBytes()) {
            buffer_read_.retrieveAll();
        }
        if (buffer_read_.readableBytes() < transaction::get_additionallength()) {
            buffer_read_.ensureWritableBytes(transaction::get_additionallength() - 
                buffer_read_.readableBytes());
        }
        do_read();
    }
    else {
        STLOG_ERROR << "read error session 0x" << std::setbase(16) << sessionid_ 
            << ", errorid = " << ec.value() << ", length = " << length
            << ", message = " << ec.message();
        stop(std::string("handle read error: ") + ec.message());
    }
}

void Session::handle_write(boost::system::error_code ec, std::size_t length)
{
    if (!ec) {
        buffer_write_.retrieve(length);
        while (!cache_write_.empty()) {
            size_t trans_size = cache_write_.front()->trasaction_length();
            if (trans_size > buffer_write_.writableBytes()) {
                if (!buffer_write_.has_capacity(trans_size)) break;
                buffer_write_.remove();
            }
            buffer_write_.append(cache_write_.front()->get_buffer(), trans_size);
            cache_write_.pop_front();
        }
        if (buffer_write_.readableBytes()) {
            do_write();
        }
        else {
            //hased check, so remove
            //buffer_write_.retrieveAll();
            if (cache_write_.empty()) {
                heartbeat_send_timer_->expires_from_now(boost::posix_time::seconds(HeartbeatTime));
                heartbeat_send_timer_->async_wait(std::bind(&Session::handler_write_timer, shared_from_this(), std::placeholders::_1));
            }
        }
    } 
    else {
        stop(std::string("handle write error: ") + ec.message());
    }
}

void Session::handler_write_timer(const boost::system::error_code& error)
{
    if (error != boost::asio::error::operation_aborted)
    {
        // Timer was not cancelled, take necessary action.
        deliver(heartbeat_transaction_);
        heartbeat_timer_->expires_from_now(boost::posix_time::seconds(HeartbeatTime));
        heartbeat_timer_->async_wait(std::bind(&Session::handler_read_timer, shared_from_this(), std::placeholders::_1));
    }
}

void Session::handler_read_timer(const boost::system::error_code& error)
{
    if (error != boost::asio::error::operation_aborted) {
        // Timer was not cancelled, take necessary action.
        stop("heartbeat out of time");
    }
}

void Session::handler_close()
{
    boost::system::error_code ec;
    socket_->cancel();
    socket_->close(ec);
    buffer_read_.retrieveAll();
    buffer_write_.retrieveAll();
    cache_write_.clear();
    receive_buffer_.clear();
    heartbeat_send_timer_->cancel();
    heartbeat_timer_->cancel();
    light_service_.lock()->del_session(shared_from_this());
    
    STLOG_INFO << "session 0x" << std::setbase(16) << sessionid_ 
        << " closed, msg: " << ec.message();
}

}
#include <iostream>
#include "clientapi.h"
#include "transaction.h"
#include "clientreceivehandle.h"

namespace light
{
clientapi::clientapi(boost::asio::ip::tcp::endpoint ed)
    : endpoint_(ed)
    , receive_handle_(NULL)
    , is_connected_(false)
{
    work_ = std::make_shared<boost::asio::io_service::work>(io_service_);
    transaction_head head;
    head.body_length_ = 0;
    head.transaction_type_ = kHeartBeatClient;
    heartbeat_transaction_ = std::make_shared<transaction>(&head);
    heartbeat_send_timer_ = std::make_shared<boost::asio::deadline_timer>(
        io_service_, boost::posix_time::seconds(HeartbeatTime));
    heartbeat_timer_ = std::make_shared<boost::asio::deadline_timer>(
        io_service_, boost::posix_time::seconds(HeartbeatTime));
    reconnect_timer_ = std::make_shared<boost::asio::deadline_timer>(
        io_service_, boost::posix_time::seconds(ReconnectTime));
    do_connect(ed);
}

clientapi::~clientapi()
{
    io_service_.stop();
    if (thread_) {
        if (thread_->joinable()) {
            thread_->join();
        }
        else {
            thread_->detach();
        }
    }
}

void clientapi::set_handle(clientreceivehandle* phandle)
{
    receive_handle_ = phandle;
}

void clientapi::run()
{
    thread_ = std::make_shared<boost::thread>(std::bind(&clientapi::run_io, this, &io_service_));
    std::cout << "io thread id " << thread_->get_id() << std::endl;;
}

void clientapi::send(const transaction_head& head, const char* body)
{
    //do_send(head, body);
    io_service_.post(std::bind(&clientapi::deliver, this, std::make_shared<transaction>(&head, body)));
}

void clientapi::run_io(boost::asio::io_service* ps)
{
    ps->run();
}

void clientapi::close(const std::string& message)
{
    std::cout << message << std::endl;
    if (is_connected_) {
        is_connected_ = false;
        io_service_.post(std::bind(&clientapi::handler_close, this));
    }
}

void clientapi::deliver(transaction_ptr trans)
{
    do_send(*trans->get_head_ptr(), trans->get_body_ptr());
}

void clientapi::pushtrans(transaction_ptr trans)
{
    if (is_connected_) {
        std::size_t write_in_progress = buffer_write_.readableBytes();
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
        if (write_in_progress == 0) {
            do_write();
        }
    }
}

void clientapi::do_connect(boost::asio::ip::tcp::endpoint ed)
{
    socket_ = std::make_shared<boost::asio::ip::tcp::socket>(io_service_);
    socket_->async_connect(ed, std::bind(&clientapi::handle_connect, this, std::placeholders::_1));
}

void clientapi::do_send(const transaction_head& head, const char* body)
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
        //io_service_.post(std::bind(&clientapi::deliver, this, std::make_shared<transaction>(&h, &addition, body + curpos)));
        pushtrans(std::make_shared<transaction>(&h, &addition, body + curpos));
        curpos += h.body_length_;
        ++addition.serialnumber_;
    } while (addition.hasnext_);
}

void clientapi::do_receive(const transaction_head& head, const transaction_additional& additional, const char* body)
{
    if (!additional.hasnext_ && !additional.serialnumber_) {
        //single pack
        receive_handle_->receive(head, body);
    }
    else {
        //multi pack
        receive_buffer_.append(body, body + head.body_length_);
        if (!additional.hasnext_) {
            receive_handle_->receive(transaction_head(head.transaction_type_, receive_buffer_.length()), &receive_buffer_[0]);
            receive_buffer_.clear();
        }
    }
}

void clientapi::do_read()
{
    if (is_connected_) {
        socket_->async_read_some(boost::asio::buffer(buffer_read_.beginWrite(), buffer_read_.writableBytes()),
            std::bind(&clientapi::handle_read, this, std::placeholders::_1, std::placeholders::_2));
    }
}

void clientapi::do_write()
{
    if (is_connected_) {
        socket_->async_write_some(boost::asio::buffer(buffer_write_.peek(), buffer_write_.readableBytes()),
            std::bind(&clientapi::handle_write, this, std::placeholders::_1, std::placeholders::_2));
    }
}

void clientapi::handle_connect(boost::system::error_code ec)
{
    if (!ec) {
        is_connected_ = true;
        reconnect_timer_->cancel();
        if (receive_handle_) {
            receive_handle_->onfrontconnected();
        }
        deliver(heartbeat_transaction_);
        heartbeat_timer_->expires_from_now(boost::posix_time::seconds(HeartbeatTime));
        heartbeat_timer_->async_wait(std::bind(&clientapi::handler_read_timer, this, std::placeholders::_1));
        do_read();
    }
    else {
        close(std::string("handle connect error: ") + ec.message());
        reconnect_timer_->expires_from_now(boost::posix_time::seconds(ReconnectTime));
        reconnect_timer_->async_wait(std::bind(&clientapi::handler_reconnect_timer, this, std::placeholders::_1));
        //do_connect(endpoint_);
    }
}

void clientapi::handle_read(boost::system::error_code ec, std::size_t length)
{
    if (!ec) {
        if (!receive_handle_) {
            close(std::string("handle read error: ") + "receive handle is NULL...");
            return;
        }
        buffer_read_.hasWritten(length);
        //读到数据即关掉心跳包计时
        heartbeat_timer_->cancel();
        while (buffer_read_.readableBytes() >= transaction::get_additionallength()) {
            transaction_head* head_ptr = (transaction_head*)buffer_read_.peek();
            if (buffer_read_.readableBytes() < 
                head_ptr->body_length_ + transaction::get_additionallength()) {
                //only head read complete
                buffer_read_.ensureWritableBytes(head_ptr->body_length_ + 
                    transaction::get_additionallength() - buffer_read_.readableBytes());
                break;
            }
            else {
                buffer_read_.retrieve(Trans_Head_Length_);
                transaction_additional* addition_ptr = (transaction_additional*)buffer_read_.peek();
                buffer_read_.retrieve(Trans_Head_Additional_Length_);
                if (head_ptr->body_length_) {
                    do_receive(*head_ptr, *addition_ptr, buffer_read_.peek());
                }
                else {
                    if (head_ptr->transaction_type_ == kHeartBeatClient) {
                        heartbeat_timer_->cancel();
                    }
                    else if (head_ptr->transaction_type_ == kHeartBeatServer) {
                        deliver(std::make_shared<transaction>(head_ptr));
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
        close(std::string("handle read error: ") + ec.message());
    }
}

void clientapi::handle_write(boost::system::error_code ec, std::size_t length)
{
    if (!ec) {
        buffer_write_.retrieve(length);
        while (!cache_write_.empty())
        {
            size_t trans_size = cache_write_.front()->trasaction_length();
            if (trans_size > buffer_write_.writableBytes())
            {
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
            buffer_write_.retrieveAll();
            if (cache_write_.empty()) {
                heartbeat_send_timer_->expires_from_now(boost::posix_time::seconds(HeartbeatTime));
                heartbeat_send_timer_->async_wait(std::bind(&clientapi::handler_write_timer, this, std::placeholders::_1));
            }
        }
    }
    else {
        close(std::string("handle write error: ") + ec.message());
    }
}

void clientapi::handler_write_timer(const boost::system::error_code& error)
{
    if (error != boost::asio::error::operation_aborted)
    {
        // Timer was not cancelled, take necessary action.
        deliver(heartbeat_transaction_);
        heartbeat_timer_->expires_from_now(boost::posix_time::seconds(HeartbeatTime));
        heartbeat_timer_->async_wait(std::bind(&clientapi::handler_read_timer, this, std::placeholders::_1));
    }
}

void clientapi::handler_read_timer(const boost::system::error_code& error)
{
    if (error != boost::asio::error::operation_aborted)
    {
        // Timer was not cancelled, take necessary action.
        close("session heartbeat out of time...");
    }
}

void clientapi::handler_reconnect_timer(const boost::system::error_code& error)
{
    if (error != boost::asio::error::operation_aborted)
    {
        do_connect(endpoint_);
    }
}

void clientapi::handler_close()
{
    boost::system::error_code ec;
    socket_->cancel();
    socket_->close(ec);
    buffer_write_.retrieveAll();
    buffer_read_.retrieveAll();
    cache_write_.clear();
    receive_buffer_.clear();
    heartbeat_send_timer_->cancel();
    heartbeat_timer_->cancel();
    reconnect_timer_->cancel();
    if (receive_handle_) {
        receive_handle_->onfrontdisconnected(0);
    }
    std::cout << "socket " << std::ios_base::hex << socket_ 
        << " closed..., close message " << ec.message() << std::endl;
    do_connect(endpoint_);
}
}
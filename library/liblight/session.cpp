#include <iostream>
#include "session.h"
#include "transaction.h"
#include "dispatcher.h"
#include "light_service.h"
#include "event_def.h"
#include "liblog/log.h"

void light::Session::start()
{
	light_service_->add_session(shared_from_this());
	STLOG_INFO << "session " << "0x" << std::setbase(16)<< sessionid_ << " start";
	is_connected_ = true;
	Dispatcher::_Instance().DispatchEvent(shared_from_this(), light::LightEvent::OnSessionConnected);
	deliver(heartbeat_transaction_);
	heartbeat_timer_->expires_from_now(boost::posix_time::seconds(HeartbeatTime));
	heartbeat_timer_->async_wait(std::bind(&Session::handler_read_timer, shared_from_this(), std::placeholders::_1));
	do_read();
}

light::Session::Session( socket_ptr socket, std::shared_ptr<LightService>& ls)
	: socket_(socket)
	, is_connected_(false)
	, context_(NULL)
	, light_service_(ls)
{
	//sessionid_ = (int)(shared_from_this().get());
	transaction_head head;
	head.body_length_ = 0;
	head.transaction_type_ = kHeartBeatServer;
	heartbeat_transaction_ = std::make_shared<transaction>(&head);
	heartbeat_send_timer_ = std::make_shared<boost::asio::deadline_timer>(socket_->get_io_service(), boost::posix_time::seconds(HeartbeatTime));
	heartbeat_timer_ = std::make_shared<boost::asio::deadline_timer>(socket_->get_io_service(), boost::posix_time::seconds(HeartbeatTime));
}

void light::Session::deliver( transaction_ptr trans )
{
	do_send(*trans->get_head_ptr(), trans->get_body_ptr());
}

void light::Session::do_write()
{
	if (is_connected_)
	{
		socket_->async_write_some(boost::asio::buffer(buffer_write_.peek(), buffer_write_.readableBytes()),
			std::bind(&Session::handle_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
}

void light::Session::do_send(const light::transaction_head& head, const char* body)
{
	int capacity = Buffer::kInitialSize - transaction::get_additionallength();
	int curpos = 0;
	light::transaction_head h(head.transaction_type_, 0);
	light::transaction_additional addition;
	addition.totallength_ = head.body_length_;
	do
	{
		h.body_length_ = (head.body_length_ - curpos) > capacity ? capacity : (head.body_length_ - curpos);
		addition.hasnext_ = head.body_length_ - curpos - h.body_length_;
		//socket_->get_io_service().post(std::bind(&Session::deliver, shared_from_this(), std::make_shared<transaction>(&h, &addition, body + curpos)));
		pushtrans(std::make_shared<transaction>(&h, &addition, body + curpos));
		curpos += h.body_length_;
		++addition.serialnumber_;
	} while (addition.hasnext_);
}

void light::Session::pushtrans(transaction_ptr trans)
{
	if (is_connected_)
	{
		bool write_in_progress = buffer_write_.readableBytes();
		if (!cache_write_.empty())
		{
			cache_write_.push_back(trans);
		}
		else
		{
			if ((buffer_write_.writableBytes() < trans->get_head_ptr()->body_length_ + Trans_Head_Length_))
			{
				cache_write_.push_back(trans);
			}
			else
			{
				buffer_write_.append(trans->get_buffer(), trans->trasaction_length());
			}
		}
		if (!write_in_progress)
		{
			do_write();
		}
	}
}

void light::Session::do_receive(const light::transaction_head& head, const light::transaction_additional& additional, const char* body)
{
	if (!additional.hasnext_ && !additional.serialnumber_)
	{
		Dispatcher::_Instance().DispatchTransaction(shared_from_this(), std::make_shared<light::transaction>(&head, body));
	}
	else
	{
		//multi pack
		receive_buffer_.append(body, body + head.body_length_);
		if (!additional.hasnext_)
		{
			if (receive_buffer_.length() != (size_t)additional.totallength_)
			{
				STLOG_ERROR << "some transactions lost...";
			}
			light::transaction_head header(head.transaction_type_, receive_buffer_.length());
			Dispatcher::_Instance().DispatchTransaction(shared_from_this(), std::make_shared<light::transaction>(&header, &receive_buffer_[0]));
			receive_buffer_.clear();
		}
	}
}

void light::Session::do_read()
{
	if (is_connected_)
	{
		socket_->async_read_some(boost::asio::buffer(buffer_read_.beginWrite(), buffer_read_.writableBytes()),
			std::bind(&Session::handle_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
	}
}

void light::Session::handle_read( boost::system::error_code ec, std::size_t length )
{
	if (!ec)
	{
		buffer_read_.hasWritten(length);
		heartbeat_timer_->cancel();
		while (buffer_read_.readableBytes() >= light::transaction::get_additionallength())
		{
			light::transaction_head* head_ptr = (light::transaction_head*)buffer_read_.peek();
			if(buffer_read_.readableBytes() < head_ptr->body_length_ + light::Trans_Head_Length_)
			{
				//only head read complete
				buffer_read_.ensureWritableBytes(head_ptr->body_length_ + Trans_Head_Length_ - buffer_read_.readableBytes());
				break;
			}else
			{
				buffer_read_.retrieve(light::Trans_Head_Length_);
				light::transaction_additional* addition_ptr = (light::transaction_additional*)buffer_read_.peek();
				buffer_read_.retrieve(light::Trans_Head_Additional_Length_);
				if (head_ptr->body_length_)
				{
					do_receive(*head_ptr, *addition_ptr, buffer_read_.peek());
				}else
				{
					if (head_ptr->transaction_type_ == kHeartBeatClient)
					{
						deliver(std::make_shared<transaction>(head_ptr));
					}else if(head_ptr->transaction_type_ == kHeartBeatServer)
					{
						heartbeat_timer_->cancel();
					}
				}
				buffer_read_.retrieve(head_ptr->body_length_);
			}
		}
		if (!buffer_read_.readableBytes())
		{
			buffer_read_.retrieveAll();
		}
		if (buffer_read_.readableBytes() < Trans_Head_Length_)
			buffer_read_.ensureWritableBytes(Trans_Head_Length_ - buffer_read_.readableBytes());
		do_read();
	}else
	{
		STLOG_INFO << "read error session 0x" 
			<< std::setbase(16) 
			<< sessionid_ 
			<< ", errorid = " << ec.value()
			<< ", length = " << length
			<< ", message = " << ec.message();
		close(std::string("handle read error: ") + ec.message());
	}
}

void light::Session::send(const light::transaction_head& head, const char* body)
{
	//do_send(head, body);
	socket_->get_io_service().post(std::bind(&Session::deliver, shared_from_this(), std::make_shared<transaction>(&head, body)));
}

void light::Session::handle_write( boost::system::error_code ec, std::size_t length )
{
	if (!ec)
	{
		buffer_write_.retrieve(length);
		while (!cache_write_.empty())
		{
			size_t trans_size = cache_write_.front()->trasaction_length();
			if (trans_size > buffer_write_.writableBytes())
			{
				if(!buffer_write_.has_capacity(trans_size)) break;
				buffer_write_.remove();
			}
			buffer_write_.append(cache_write_.front()->get_buffer(), trans_size);
			cache_write_.pop_front();
		}
		if (buffer_write_.readableBytes())
		{
			do_write();
		}else
		{
		//	buffer_write_.retrieveAll();
			if (cache_write_.empty())
			{
				heartbeat_send_timer_->expires_from_now(boost::posix_time::seconds(HeartbeatTime));
				heartbeat_send_timer_->async_wait(std::bind(&Session::handler_write_timer, shared_from_this(), std::placeholders::_1));
			}
		}
	}else
	{
		close(std::string("handle write error: ") + ec.message());
	}
}

light::Session::~Session()
{
	STLOG_INFO << "session 0x" << std::setbase(16) << sessionid_ << " has been destroyed...";
}

void light::Session::handler_write_timer( const boost::system::error_code& error )
{
	if (error != boost::asio::error::operation_aborted)
	{
		// Timer was not cancelled, take necessary action.
		deliver(heartbeat_transaction_);
		heartbeat_timer_->expires_from_now(boost::posix_time::seconds(HeartbeatTime));
		heartbeat_timer_->async_wait(std::bind(&Session::handler_read_timer, shared_from_this(), std::placeholders::_1));
	}
}

void light::Session::handler_read_timer( const boost::system::error_code& error )
{
	if (error != boost::asio::error::operation_aborted)
	{
		// Timer was not cancelled, take necessary action.
		close("heartbeat out of time");
	}
}

void light::Session::close( const std::string& message )
{
	if (is_connected_)
	{
		STLOG_INFO << "session 0x" << std::setbase(16) << sessionid_ << " " << message;
		is_connected_ = false;
		Dispatcher::_Instance().DispatchEvent(shared_from_this(), light::LightEvent::OnSessionDisconnected);
		socket_->get_io_service().post(std::bind(&Session::handler_close, shared_from_this()));
	}
}

void light::Session::handler_close()
{
	boost::system::error_code ec;
	socket_->cancel();
	socket_->close(ec);
	heartbeat_send_timer_->cancel();
	heartbeat_timer_->cancel();
	buffer_write_.retrieveAll();
	buffer_read_.retrieveAll();
	cache_write_.clear();
	light_service_->del_session(shared_from_this());
	light_service_.reset();
	receive_buffer_.clear();
	STLOG_INFO << "session 0x"<< std::setbase(16) << sessionid_ << " closed, close message " << ec.message();
}

void light::Session::set_context( void* context )
{
	context_ = context;
}

void* light::Session::get_context()
{
	return context_;
}

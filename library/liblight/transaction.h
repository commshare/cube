#pragma once
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <string>

namespace light
{
#pragma pack(1)
	struct transaction_head
	{
		transaction_head()
			: body_length_(0)
			, transaction_type_(0)
		{
		}
		transaction_head(int tt,int bl)
			: body_length_(bl)
			, transaction_type_(tt)
		{
		}
		int body_length_;
		int transaction_type_;
	};

	struct transaction_additional
	{
		transaction_additional()
			: serialnumber_(0)
			, hasnext_(0)
			, totallength_(0)
		{
			memset(userdata_, 0x0, sizeof(userdata_));
		}
		short serialnumber_;
		short hasnext_;
		short totallength_;
		char  userdata_[2];
	};
#pragma pack()
	static const size_t Trans_Head_Length_ = sizeof(transaction_head);
	static const size_t Trans_Head_Additional_Length_ = sizeof(transaction_additional);
	static const int kHeartBeatClient = 1;
	static const int kHeartBeatServer = 2;

	class transaction
	{
	public:
		transaction()
			: head_ptr_(NULL)
		{

		}
		static size_t get_additionallength() { return Trans_Head_Length_ + Trans_Head_Additional_Length_; };
		transaction(const transaction_head* phead, const char* pbody)
		{
			int totallength = get_additionallength() + phead->body_length_;
			head_ptr_ = new char[totallength];
			memset(head_ptr_, 0x0, totallength);
			memcpy(head_ptr_, phead, Trans_Head_Length_);
			if (pbody && phead->body_length_)
			{
				memcpy(head_ptr_ + Trans_Head_Length_ + Trans_Head_Additional_Length_, pbody, phead->body_length_);
			}
		}
		transaction(const transaction_head* phead, const transaction_additional* padditional, const char* pbody)
		{
			int totallength = get_additionallength() + phead->body_length_;
			head_ptr_ = new char[totallength];
			memset(head_ptr_, 0x0, totallength);
			memcpy(head_ptr_, phead, Trans_Head_Length_);
			memcpy(head_ptr_ + Trans_Head_Length_, padditional, Trans_Head_Additional_Length_);
			if (pbody && phead->body_length_)
			{
				memcpy(head_ptr_ + Trans_Head_Length_ + Trans_Head_Additional_Length_, pbody, phead->body_length_);
			}
		}

		transaction(const transaction_head* phead)
		{
			head_ptr_ = new char[get_additionallength() + phead->body_length_];
			memset(head_ptr_, 0x0, get_additionallength() + phead->body_length_);
			memcpy(head_ptr_, phead, Trans_Head_Length_);
			if (phead->body_length_)
			{
				memset(head_ptr_ + Trans_Head_Length_, 0x0, phead->body_length_);
			}
		}
		const char* get_body_ptr() const
		{
			return head_ptr_ + get_additionallength();
		}
		std::string get_body_string() const
		{
			return std::string(get_body_ptr(), get_body_ptr() + get_head_ptr()->body_length_);
		}
		const transaction_head* get_head_ptr() const
		{
			return (transaction_head*)head_ptr_;
		}
		const char* get_buffer() const
		{
			return head_ptr_;
		}
		inline int trasaction_length() const
		{
			return get_additionallength() + get_head_ptr()->body_length_;
		}
		~transaction()
		{
			if (head_ptr_)
			{
				delete [] head_ptr_;
			}
		}
	private:
		char* head_ptr_;
	};
}

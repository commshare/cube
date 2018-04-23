#include "business_unit.h"
#include "transaction.h"
#include "dispatcher.h"
#include <memory>
#include "liblog/log.h"
light::BusinessUnit::BusinessUnit(const std::string& name)
	: bu_name_(name)
	, business_unit_group_(nullptr)
{

}

light::BusinessUnit::~BusinessUnit()
{
}
std::shared_ptr<light::BusinessUnitGroup>& light::BusinessUnit::business_unit_group()
{
	return business_unit_group_;
}

void light::BusinessUnit::join(std::shared_ptr<BusinessUnitGroup> group)
{
	business_unit_group_ = group;
}

void light::BusinessUnit::send(int sessionid, int transactionid, const std::string& msg)
{
	if (sessionid)
	{
		transaction_head head;
		head.body_length_ = msg.length();
		head.transaction_type_ = transactionid;
		Dispatcher::_Instance().SendTransaction(sessionid, std::make_shared<transaction>(&head, &msg[0]));
	}
}

void light::BusinessUnit::publish(int trasactionid, const std::string& msg)
{
	transaction_head head;
	head.body_length_ = msg.size();
	head.transaction_type_ = trasactionid;
	Dispatcher::_Instance().DispatchTransaction(nullptr, std::make_shared<transaction>(&head, &msg[0]));
}


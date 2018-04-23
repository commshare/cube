#include <iostream>
#include "dispatcher.h"
#include "transaction.h"
#include "business_unit.h"
#include "business_unit_group.h"
#include "light_service.h"
#include "liblog/log.h"

light::Dispatcher& light::Dispatcher::_Instance()
{
	static Dispatcher dispatcher;
	return dispatcher;
}

void light::Dispatcher::RegisterBusinessUnitTransaction(int transactionid, light::BusinessUnit* bu)
{
	transactionid2bu_[transactionid].insert(bu);
}

void light::Dispatcher::RegisterBusinessUnitEvent(int eventid, BusinessUnit* bu)
{
	eventid2bu_[eventid].insert(bu);
}

void light::Dispatcher::RegisterBusinessUnitTimer(int interval, BusinessUnit* bu)
{
	if (bu)
	{
		bu->business_unit_group()->RegisterTimer(interval, bu);
	}
}

void light::Dispatcher::RegisterLightService(std::shared_ptr<LightService>& service)
{
	lightservices_[service->serviceid()] = service;
}

void light::Dispatcher::DispatchTransaction(participant_ptr participant, transaction_ptr trans)
{
	auto it = transactionid2bu_.find(trans->get_head_ptr()->transaction_type_);
	if (it != transactionid2bu_.end())
	{
		if (it->second.empty())
		{
			STLOG_ERROR << "no bu to process transaction " << trans->get_head_ptr()->transaction_type_ << std::endl;
			return;
		}
		for (auto it_bu = it->second.begin(); it_bu != it->second.end(); ++it_bu)
		{
			if ((*it_bu)->business_unit_group())
			{
				(*it_bu)->business_unit_group()->deliver(*it_bu, participant, trans);
			}else
			{
				STLOG_ERROR << "no group to process bu " << (*it_bu)->bu_name() << std::endl;
			}
		}
	}
	else
	{
		STLOG_ERROR << "no bu registered to process transaction " << trans->get_head_ptr()->transaction_type_ << std::endl;
	}
}

void light::Dispatcher::SendTransaction(int sessionid, transaction_ptr trans)
{
	if (sessionid)
	{
		int serviceid = (sessionid & 0xffff0000) >> 16;
		auto it = lightservices_.find(serviceid);
		if (it != lightservices_.end() && it->second)
		{
			it->second->get_io_service().post(std::bind(&LightService::send, it->second, sessionid, trans));
		}
		else
		{
			STLOG_ERROR << "invalid sessionid " << std::ios_base::hex << sessionid << std::endl;
		}
	}
}

void light::Dispatcher::DispatchEvent(participant_ptr participant, int eventid)
{
	auto it = eventid2bu_.find(eventid);
	if (it != eventid2bu_.end())
	{
		if (it->second.empty())
		{
			STLOG_ERROR << "no bu defined to process event " << eventid << std::endl;
			return;
		}
		for (auto it_bu = it->second.begin(); it_bu != it->second.end(); ++it_bu)
		{
			if ((*it_bu)->business_unit_group())
			{
				(*it_bu)->business_unit_group()->deliver(*it_bu, participant, eventid);
			}
			else
			{
				STLOG_ERROR << "no group defined to process event " << (*it_bu)->bu_name() << std::endl;
			}
		}
	}
	else
	{
		STLOG_ERROR << "no bu registered to process event " << eventid << std::endl;
	}
}

void light::Dispatcher::Clear()
{
    transactionid2bu_.clear();
    eventid2bu_.clear();
    lightservices_.clear();
}

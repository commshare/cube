#pragma once
#include "light_ptr.h"
namespace light
{
	class transaction;
	class itransaction_processor
	{
	public:
		virtual bool do_process(participant_ptr parti, transaction_ptr trans) = 0;
	};
};

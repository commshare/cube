#pragma once
#include "light_ptr.h"
namespace light
{
	struct transaction_head;
	class participant
	{
	public:
		virtual ~participant() {}
		int sessionid_;
	};
}

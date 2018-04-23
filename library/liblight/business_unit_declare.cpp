#include "business_unit_declare.h"

light::BusinessUnit* make_business_unit(const std::string& butype, const std::string& buname)
{
	const std::map<std::string, std::function<light::BusinessUnit*(const std::string&)> >&  factorys = BusinessDeclareWrapper::_Instance().get_factorys();
	auto it = factorys.find(butype);
	if (it != factorys.end())
	{
		return it->second(buname);
	}
	return nullptr;
}

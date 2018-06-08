#pragma once
#include <string>
#include <map>
#include <functional>

namespace light
{
    class BusinessUnit;
};

class BusinessDeclareWrapper
{
public:
    static BusinessDeclareWrapper& _Instance()
    {
        static BusinessDeclareWrapper wrapper;
        return wrapper;
    };
    void InsertFactory(std::string butype, std::function<light::BusinessUnit*(const std::string&) > function)
    {
        butype2factory_[butype] = function;
    };
    const std::map<std::string, std::function<light::BusinessUnit*(const std::string&)> >& get_factorys()
    {
        return butype2factory_;
    };
private:
    BusinessDeclareWrapper() { };
    std::map<std::string, std::function<light::BusinessUnit*(const std::string&)> > butype2factory_;
};

#define DECLARE_BUSINESS_UNIT(buclassname, butype) \
    class Factory##butype { \
    public: \
    light::BusinessUnit* CreateBusinessUnit(const std::string& buname) \
{ \
    return new buclassname(buname); \
}; \
    public: \
    Factory##butype(std::string buytpe) { \
    BusinessDeclareWrapper::_Instance().InsertFactory(#butype, std::bind(&Factory##butype::CreateBusinessUnit, this, std::placeholders::_1)); \
}; \
}; \
static const Factory##butype butype = Factory##butype(#butype);

light::BusinessUnit* make_business_unit(const std::string& butype, const std::string& buname);
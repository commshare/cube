#pragma once
#include <string>
#include <memory>

namespace light
{
class BusinessUnitGroup;
class BusinessUnit
{
public:
    BusinessUnit(const std::string& name);
    virtual ~BusinessUnit();
    inline const std::string& bu_name() const { return bu_name_; };
    void join(std::shared_ptr<BusinessUnitGroup> group);
    std::shared_ptr<BusinessUnitGroup>& business_unit_group();
    void send(int sessionid, int transactionid, const std::string& msg);
    void publish(int transactionid, const std::string& msg);
    virtual void init() {};
    virtual void OnReceive(int sessionid, int transactionid, const std::string& msg) = 0;
    virtual void OnEvent(int sessionid, int eventid) = 0;

private:
    std::string bu_name_;
    std::shared_ptr<BusinessUnitGroup> business_unit_group_;
};
};

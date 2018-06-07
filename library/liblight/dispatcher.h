#pragma once
#include <map>
#include <set>
#include <memory>
#include "light_ptr.h"
namespace light
{
    class BusinessUnit;
    class LightService;
    class Dispatcher
    {
    public:
        static Dispatcher& _Instance();
        void RegisterBusinessUnitTransaction(int transactionid, BusinessUnit* bu);
        void RegisterBusinessUnitEvent(int eventid, BusinessUnit* bu);
        void RegisterBusinessUnitTimer(int interval, BusinessUnit* bu);
        void RegisterLightService(std::shared_ptr<LightService>& service);
        void DispatchTransaction(participant_ptr participant, transaction_ptr trans);
        void SendTransaction(int sessionid, transaction_ptr trans);
        void DispatchEvent(participant_ptr participant, int eventid);
        void Clear();
    private:
        Dispatcher() {};
        std::map<int, std::set<BusinessUnit*> > transactionid2bu_;
        std::map<int, std::set<BusinessUnit*> > eventid2bu_;
        std::map<int, std::shared_ptr<LightService> > lightservices_;
    };
}
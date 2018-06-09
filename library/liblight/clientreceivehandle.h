#pragma once

namespace light
{
struct transaction_head;
class clientreceivehandle
{
public:
    virtual void receive(const transaction_head& head, const char* data) = 0;
    virtual void onfrontconnected() = 0;
    virtual void onfrontdisconnected(int reason) = 0;
};
};

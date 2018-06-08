#pragma once

namespace light
{
class LightEvent
{
public:
    enum {
        OnSessionConnected = 1,
        OnSessionDisconnected,
        OnTimer
    };
};
}
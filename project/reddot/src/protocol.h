#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "project.h"

namespace eco {
class MessageMeta
{
public:
    uint32_t session_;
    uint32_t type_;
    std::string data_;

public:
    inline MessageMeta()
    {
    }

    inline MessageMeta(
        IN const uint32_t session,
        IN const uint32_t type,
        IN const std::string& data)
        : session_(session),
        type_(type),
        data_(data)
    {
    }

    inline void set_session(IN const uint32_t session)
    {
        session_ = session;
    }

    inline void set_type(IN const uint32_t type)
    {
        type_ = type;
    }

    inline void set_data(IN const std::string& data)
    {
        data_ = data;
    }

    inline const uint32_t get_session() const
    {
        return session_;
    }

    inline const uint32_t get_type() const
    {
        return type_;
    }

    inline const std::string& get_data() const
    {
        return data_;
    }
};
}
#endif
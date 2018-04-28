#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#include "project.h"

namespace eco{;

////////////////////////////////////////////////////////////////////////////////
class MessageMeta
{
public:
	uint32_t		m_session_id;
	uint32_t		m_message_type;
	std::string		m_request_data;
	
public:
	inline MessageMeta()
	{
		memset(this, 0, sizeof(*this));
	}

	inline MessageMeta(
		IN const uint32_t session_id,
		IN const uint32_t type,
		IN const std::string& data)
	{
		memset(this, 0, sizeof(*this));
		set_session_id(session_id);
		set_message_type(type);
        set_request_data(data);
	}

	inline void set_session_id(IN const uint32_t id)
	{
		m_session_id = id;
	}

	inline void set_message_type(IN const uint32_t type)
	{
        m_message_type = type;
	}

	inline void set_request_data(IN const std::string& req_data)
	{
		m_request_data = req_data;
	}

    inline const uint32_t get_session_id() const
    {
        return m_session_id;
    }

    inline const uint32_t get_type() const
    {
        return m_message_type;
    }

    inline const std::string& get_request_data() const
    {
        return m_request_data;
    }
};

////////////////////////////////////////////////////////////////////////////////
}
#endif
#ifndef ECO_NET_ECODE_H
#define ECO_NET_ECODE_H
/*******************************************************************************
@ name


@ function


@ exception


@ remark


--------------------------------------------------------------------------------
@ history ver 1.0 @
@ records: ujoy modifyed on 2016-11-12.
1.create and init this class.


--------------------------------------------------------------------------------
* copyright(c) 2016 - 2019, ujoy, reserved all right.

*******************************************************************************/





namespace eco{;
namespace net{;


////////////////////////////////////////////////////////////////////////////////
enum Ecode
{
	e_ok							=  0,
	e_failed						= -1,
	e_timeout						= -2,

	// [tcp server]：[-1000, -2000)
	// ------------------------------------------------------------------------
	e_message_empty					= -1000,
	e_message_overszie				= -1001,
	e_message_unknown				= -1002,
	e_message_category				= -1003,
	e_message_checksum				= -1004,
	e_message_decode				= -1005,
	e_message_encode				= -1006,

	e_protocol_parameter			= -1100,
	e_protocol_subscription			= -1101,
	e_protocol_error				= -1102,
	e_protobuf_parse_error			= -1103,

	e_peer_expired					= -1201,
	e_peer_client_close				= -1202,
	e_peer_protocol_error			= -1203,
	e_session_expired				= -1204,
	e_session_over_max_size			= -1205,
	e_server_no_port				= -1206,
	e_server_no_session_data		= -1207,
	e_server_no_protocol_head		= -1208,
	e_server_no_protocol			= -1209,
	e_server_no_support_session		= -1210,

	e_websocket_payload_len			= -1500,


	// [tcp client]：(-2000, -3000]
	// ------------------------------------------------------------------------
	e_client_request_timeout		= -2000,
	e_client_connected_fail			= -2001,
	e_client_protocol_version		= -2002,
	e_client_no_protocol_head		= -2003,
	e_client_no_protocol			= -2004,
	e_client_message_empty			= -2005,
	e_client_pop_sync_req_fail		= -2006,
	e_client_request_fail			= -2007,
	e_client_no_address				= -2008,
	e_client_no_session_data		= -2009,
	e_client_session_not_opened		= -2010,

	// [tcp session]：(-3000, -4000]
	// ------------------------------------------------------------------------
	e_session_auth_host_client		= -3001,
};


////////////////////////////////////////////////////////////////////////////////
}}
#endif
/// \file		WnConnection.h
/// \brief		BASE���Ӷ�����������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-19
/// \history	2011-07-19 �½�

#ifdef _WIN32
#pragma once
#endif

#include "WnConnBase.h"

#ifdef _DEBUG
#define new	new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

template<typename Protocol>
class WnConnection : public WnConnBase, public boost::enable_shared_from_this<WnConnection<Protocol> >
{
public:
	typedef typename Protocol::endpoint endpoint_type;
	typedef typename Protocol::socket socket_type;
    typedef typename Protocol::resolver resolver_type;

	/// \brief ���캯��
	/// \param apEndPoint �����ն�
	/// \param blRawData ԭʼ���ݱ�־
	/// \param oIoService IO�첽�������
	WnConnection(END_POINT_APTR apEndPoint, bool blRawData, io_service& oIoService)
		: WnConnBase(apEndPoint, blRawData)
		, m_oSocket(oIoService)
		, m_oRemoteEndpoint()
		, m_oResolver(oIoService)
		, m_apPacket()
	{}

	/// \brief ��������
	~WnConnection(void) {}

	/// \brief ���ö��ַ�
	/// \param strHostName �Է�������
	/// \param strPort �Է��˿ں�
	/// \return 0 ���óɹ�������ֵΪ�������
	int SetAddr(const std::string& strHostName, const std::string& strPort);

	/// \brief �ͻ��˿�ʼ���Ӻ���
	void BeginConnect();

	/// \brief ����˽��յ����ӵ���Ӧ����
	void OnAccept();

	/// \brief �첽���ͺ���
	/// \param oPacket ���ݰ�
	/// \param blHighPriority �Ƿ��и����ȼ�
	/// \return ���ͽ��ֵ
	ENU_ASYNSEND_RESULT AsyncSend(const PACKET_APTR& oPacket, bool blHighPriority);

	/// \brief ��ȡSOCKET
	/// \return SOCKET
	socket_type& GetSocket() { return m_oSocket; }

	/// \brief ��ȡ���ַ���ַ
	/// \return ���ַ���ַ
	endpoint_type& GetRemoteEndPoint() { return m_oRemoteEndpoint; }

protected:
    /// ����Զ�˵�ַ
	void SetRemoteEndpoint(const endpoint_type& oEndPoint) { m_oRemoteEndpoint = oEndPoint; }

	/// \brief �첽������
	void BeginRead();

	/// \brief �첽д����
	void BeginWrite();

	/// \brief ���������¼�
	/// \param oError ����
	void HandleConnect(const boost::system::error_code& oErrorCode);

	/// \brief �������ݽ����¼�
	/// \param oError ����
	/// \param unTransBytes ������ֽ���
	void HandleRead(const boost::system::error_code& oErrorCode, std::size_t unTransBytes);

	/// \brief �������ݷ����¼�
	/// \param oError ����
	void HandleWrite(const boost::system::error_code& oErrorCode);

	/// \brief �ر����Ӵ�����
	void OnClose();

private:
	// SOCKET����
	socket_type m_oSocket;
	
	// ���Ӷ��ַ���ַ
	endpoint_type m_oRemoteEndpoint;

	// �ֽ���
	resolver_type m_oResolver;

	// ���Ͱ�
	PACKET_APTR m_apPacket;
};

template<typename Protocol>
int WnConnection<Protocol>::SetAddr(const std::string& strHostName, const std::string& strPort)
{
	int nResult = 0;

	do
	{
		typename resolver_type::query oQuery(strHostName, strPort);

		boost::system::error_code oErrorCode = boost::asio::error::host_not_found;
		typename resolver_type::iterator itEndpoint = m_oResolver.resolve(oQuery, oErrorCode);

		if(oErrorCode)
		{
			nResult = oErrorCode.value();
			break;
		}

		SetRemoteEndpoint(*itEndpoint);

	} while(false);

    return nResult;
}

template<typename Protocol>
inline void WnConnection<Protocol>::BeginConnect()
{
    m_oSocket.async_connect(m_oRemoteEndpoint,
        boost::bind(&WnConnection<Protocol>::HandleConnect, this->shared_from_this(), boost::asio::placeholders::error));
}

template<typename Protocol>
void WnConnection<Protocol>::OnAccept()
{
	// ��շ��Ͷ���
	m_oSendPacketList.clear();

	// ����������״̬
	SetConnectedState(true);

	// ����Զ�˵�ַ
    SetRemoteEndpoint(m_oSocket.remote_endpoint());

	// ֪ͨ���ӳɹ�
    m_apLocalEndPoint->OnAccept(this->shared_from_this());

	// ��ʼ��
    BeginRead();
}

template<typename Protocol>
ENU_ASYNSEND_RESULT WnConnection<Protocol>::AsyncSend(const PACKET_APTR& oPacket, bool blHighPriority)
{
	ENU_ASYNSEND_RESULT eResult = E_AR_SUCCESS;

	do
	{
		if(!IsConnected())
		{
			eResult = E_AR_NOCONNECT;
			break;
		}

		if(m_blRawData)
		{
			unsigned int unNewListSize = m_oSendPacketList.size();

			// ��һ�η��� �� ���ϴθ���ʱ�������������ݷ���
			if((m_unCurrentListSize == 0) || (unNewListSize < m_unCurrentListSize))
			{
				// ���·���ʱ��
				m_oLastestSendTime = second_clock::local_time();
			}
			else
			{
				// ���Ͷ���һֱ����
				if(second_clock::local_time() > m_oLastestSendTime + milliseconds(HEARTBEAT_INTERVAL * NO_HEARTBEAT_COUNT))
				{
					eResult = E_AR_NOSEND;
					break;
				}
			}

			// ���¶��г���ֵ
			m_unCurrentListSize = unNewListSize + 1;

			// ���Ͷ��л�ѹ��������ֵ�����
			if(m_unCurrentListSize > MAX_RAWDATA_COUNT)
			{
				eResult = E_AR_OVERFLOW;
				break;
			}
		}

		m_oWritingMutex.lock();

		if(blHighPriority)
		{
			m_oSendPacketList.push_front(oPacket);
		}
		else
		{
			m_oSendPacketList.push_back(oPacket);
        }

		if(m_blWriting)
		{
			m_oWritingMutex.unlock();
			break;
		}

		m_blWriting = true;

		BeginWrite();

		m_oWritingMutex.unlock();

    } while(false);

    return eResult;
}

template<typename Protocol>
void WnConnection<Protocol>::BeginRead()
{
	boost::asio::async_read(m_oSocket, buffer(m_pbRecvBuffer, m_unRecvBufferSize), transfer_at_least(1), 
		boost::bind(&WnConnection<Protocol>::HandleRead, this->shared_from_this(),
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

template<typename Protocol>
void WnConnection<Protocol>::BeginWrite()
{
	// ȡ���װ�
	m_apPacket = m_oSendPacketList.front();
	m_oSendPacketList.pop_front();

	boost::asio::async_write(m_oSocket, buffer(m_apPacket->GetPacketData(), m_apPacket->GetPacketLength()),
        boost::bind(&WnConnection<Protocol>::HandleWrite, this->shared_from_this(), boost::asio::placeholders::error));
}

template<typename Protocol>
void WnConnection<Protocol>::HandleConnect(const boost::system::error_code& oErrorCode)
{
    if(!oErrorCode)
    {
		// ��շ��Ͷ���
		m_oSendPacketList.clear();

		// ����������״̬
		SetConnectedState(true);

		// ����Զ�˵�ַ
        SetRemoteEndpoint(m_oSocket.remote_endpoint());

		// ֪ͨ���ӳɹ�
        m_apLocalEndPoint->OnConnect(this->shared_from_this(), E_CONNECT_SUCCESS);

		// ��ʼ��
        BeginRead();
    }
    else
    {
		// ֪ͨ����ʧ��
        m_apLocalEndPoint->OnConnect(this->shared_from_this(), E_CONNECT_IOERROR);
    }
}

template<typename Protocol>
void WnConnection<Protocol>::HandleRead(const boost::system::error_code& oErrorCode, std::size_t unTransBytes)
{
	do
	{
		if(!IsConnected())
		{
			break;
		}

		if(!oErrorCode)
		{
			// ֪ͨ���յ�����
			if(m_blRawData)
			{
				m_apLocalEndPoint->OnReceiveData(this->shared_from_this(),
					PACKET_APTR(new WnPacket(m_pbRecvBuffer, static_cast<unsigned int>(unTransBytes))));
			}
			else
			{
				HandleInData(this->shared_from_this(), unTransBytes);
			}

			// ������
			BeginRead();
		}
		else
		{
			boost::shared_ptr<WnConnection<Protocol> > apConnection = this->shared_from_this();

			SetConnectedState(false);

			// �ر�SOCKET
			m_oSocket.close();

			// ֪ͨ���ӶϿ�
			m_apLocalEndPoint->OnDisconnect(apConnection, E_DISCONNECT_IOERROR);
		}
	} while(false);
}

template<typename Protocol>
void WnConnection<Protocol>::HandleWrite(const boost::system::error_code& oErrorCode)
{
	do
	{
		if(!IsConnected())
		{
			m_oWritingMutex.lock();
			m_oSendPacketList.clear();
			m_oWritingMutex.unlock();
			break;
		}
		
		if(!oErrorCode)
		{
			m_oWritingMutex.lock();

			if(m_oSendPacketList.empty())
			{
				m_blWriting = false;
				m_oWritingMutex.unlock();

				break;
			}
			BeginWrite();

			m_oWritingMutex.unlock();
		}
		else
		{
			boost::shared_ptr<WnConnection<Protocol> > apConnection = this->shared_from_this();

			SetConnectedState(false);

			m_oWritingMutex.lock();
			m_oSendPacketList.clear();
			m_oWritingMutex.unlock();

			// �ر�SOCKET
			m_oSocket.close();

			// ֪ͨ���ӶϿ�
			m_apLocalEndPoint->OnDisconnect(apConnection, E_DISCONNECT_IOERROR);
		}

	} while(false);
}

template<typename Protocol>
inline void WnConnection<Protocol>::OnClose()
{
	// �ر�SOCKET
    m_oSocket.close();
}

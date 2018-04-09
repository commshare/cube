/// \file		WnTCPServer.h
/// \brief		BASEͨ��(TCP)�������������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-19
/// \history	2011-07-19 �½�

#ifdef _WIN32
#pragma once
#endif

#include "ComHeader.h"
#include "ITCPServer.h"
#include "WnEndPoint.h"
#include "WnAcceptor.h"

#ifdef _DEBUG
#define new	new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

typedef boost::shared_ptr<deadline_timer> TIMER_APTR;

/// \brief �ͻ�����
struct TAG_CLIENT_ITEM
{
public:
	/// \brief ����
	CONNECTION_APTR apConnection;

	/// \brief ������ʱ��
	TIMER_APTR apHeartBeatTimer;

	/// \brief ��ʱ��ʱ��
	TIMER_APTR apTimeOutTimer;

private:
	/// \brief �������
	unsigned char bCheckIndex;

	/// \brief ���������
	boost::mutex oMutex;

public:
	unsigned char GetCheckIndex()
	{
		oMutex.lock();
		unsigned char bResult = bCheckIndex;
		bCheckIndex++;
		oMutex.unlock();
		return bResult;
	}

	void SetCheckIndex(unsigned char bIndex)
	{
		oMutex.lock();
		bCheckIndex = bIndex;
		oMutex.unlock();
	}
};

typedef boost::shared_ptr<TAG_CLIENT_ITEM> CLIENT_ITEM_APTR;
typedef map<unsigned __int64, CLIENT_ITEM_APTR> MAP_CLIENT_ITEM;

class WnTCPServer :	public IFndNetServer, public WnEndPoint
{
public:
	/// \brief ���캯��
	/// \param oIoService IO�첽�������
	/// \param m_poNotify ͨ������ָ��
	WnTCPServer(io_service& oIoService, IFndNetServerNotify *poNotify);
	/// \brief ��������
	~WnTCPServer(void);

	/// \brief ��ָ���˿�������������TCP����������
	/// \param usPort �����˿ں�
	/// \param blAuth �Ƿ���֤(�ݲ�֧��)
	bool StartListenWithHB(unsigned short usPort, bool blAuth = false);

	/// \brief �������ݸ�ָ���Ŀͻ���
	/// \param ullClientID �ͻ���ID
	/// \param pbData ����ָ��
	/// \param unDataSize ���ݴ�С
	/// \return �Ƿ��ͳɹ�
	bool SendDataWithHB(unsigned __int64 ullClientID, const unsigned char *pbData, unsigned int unDataSize);

	/// \brief ��ָ���˿�������������TCP����������(���������������ڼ������е�Relay�����)
	/// \param usPort �����˿ں�
	/// \param blAuth �Ƿ���֤(�ݲ�֧��)
	bool StartListenWithoutHB(unsigned short usPort, bool blAuth = false);

	/// \brief �������ݸ��������Ŀͻ���(��StartListen���ʹ�ã������ڼ������е�Relay�����)
	/// \param ullClientID �ͻ���ID
	/// \param pbData ����ָ��
	/// \param unDataSize ���ݴ�С
	/// \return �Ƿ��ͳɹ�
	bool SendDataWithoutHB(unsigned __int64 ullClientID, const unsigned char *pbData, unsigned int unDataSize);

	/// \brief �ر�TCP Server
	void CloseSocket();

	/// \brief ���յ����ӵ�ʵ�ʴ�����
	/// \param apConnection ����֪ͨ������
	void OnAccept(CONNECTION_APTR apConnection);

	/// \brief ���ӶϿ���ʵ�ʴ�����
	/// \param apConnection ����֪ͨ������
	/// \param nStatus ״̬��
	void OnDisconnect(CONNECTION_APTR apConnection, int nStatus);

	/// \brief ���յ����ݵĴ�����
	/// \param apConnection ����֪ͨ������
	/// \param apPacket �յ������ݰ�
	void OnReceiveData(CONNECTION_APTR apConnection, const PACKET_APTR& apPacket);

	/// \brief ���ݽ�������Ĵ�����
	/// \param apConnection ����֪ͨ������
	void OnDataError(CONNECTION_APTR apConnection);

	/// \brief ���ͨ������
	void ClearNotifyObj();

protected:
	/// \brief �ر��ն�
	void Close();

private:
	/// \brief ��ָ���˿�����TCP����������
	/// \param usPort �����˿ں�
	/// \param blAuth �Ƿ���֤(�ݲ�֧��)
	bool StartListen(unsigned short usPort, bool blAuth);

	/// \brief �������ݸ��ͻ���
	/// \param ullClientID �ͻ���ID
	/// \param pbData ����ָ��
	/// \param unDataSize ���ݴ�С
	/// \return �Ƿ��ͳɹ�
	bool SendData(unsigned __int64 ullClientID, const unsigned char *pbData, unsigned int unDataSize);

	/// \brief �������̺߳���
	void ListenThread();

	/// \brief ���յ�����֪ͨ���̺߳���
	/// \param apConnection ����֪ͨ������
	void OnAcceptThread(CONNECTION_APTR apConnection);

	/// \brief ���ӶϿ���ʵ���̺߳���
	/// \param apConnection ����֪ͨ������
	/// \param nStatus ״̬��
	void OnDisconnectThread(CONNECTION_APTR apConnection, int nStatus);

	/// \brief ���ݽ���������̺߳���
	/// \param apConnection ����֪ͨ������
	void OnDataErrorThread(CONNECTION_APTR apConnection);

	/// \brief ������ʱ������
	/// \param apClientItem �ͻ�����
	/// \param oErrorCode �������
	void OnHeartBeatTimer(CLIENT_ITEM_APTR apClientItem, const boost::system::error_code& oErrorCode);

	/// \brief �޽������ݶ�ʱ������
	/// \param apConnection ����֪ͨ������
	/// \param oErrorCode �������
	void OnNoRecvDataTimer(CONNECTION_APTR apConnection, const boost::system::error_code& oErrorCode);

private:
	/// \brief ������
	ACCEPTOR_APTR m_apAcceptor;

	/// \brief �����߳̾��
	boost::shared_ptr<boost::thread> m_apListenThread;

	/// \brief ����״̬
	int m_nRunState;

	/// \brief �������Ŀͻ�������MAP
	MAP_CLIENT_ITEM m_oClientsMap;

	/// \brief �������Ŀͻ���������
	boost::shared_mutex m_oSharedMutex;

	/// \brief ������
	boost::mutex m_oOperateMutex;

	/// \brief ͨ������ָ��
	IFndNetServerNotify *m_poNotify;

	/// \brief ͨ��������
	boost::shared_mutex m_oNotifyMutex;
};

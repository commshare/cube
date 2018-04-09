/// \file		WnTCPClient.h
/// \brief		BASEͨ��(TCP)�ͻ�����������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-19
/// \history	2011-07-19 �½�

#ifdef _WIN32
#pragma once
#endif

#include "ITCPClient.h"
#include "WnEndPoint.h"

/// \brief BASEͨ��(TCP)�ͻ�����
class WnTCPClient :	public IFndNetClient, public WnEndPoint
{
public:
	/// \brief ���캯��
	/// \param oIoService IO�첽�������
	/// \param poNotify ͨ������ָ��
	WnTCPClient(io_service& oIoService, IFndNetClientNotify *poNotify);
	/// \brief ��������
	~WnTCPClient(void);

	/// \brief ������������TCP������
	/// \param szIP ������IP
	/// \param usPort �������˿�
	/// \param blAuth �Ƿ���֤(�ݲ�֧��)
	/// \param unTimeOut ��ʱʱ��(��λ����)
	void ConnectServerWithHB(const char *szIP, unsigned short usPort, bool blAuth = false, unsigned int unTimeOut = 0);

	/// \brief �������ݸ��������ķ�����(��ConnectServerWithHB���ʹ��)
	/// \param pbData ����ָ��
	/// \param unDataSize ���ݴ�С
	/// \return �Ƿ��ͳɹ�
	bool SendDataToServerWithHB(const unsigned char *pbData, unsigned int unDataSize);

	/// \brief ������������TCP������(�������еķ����������)
	/// \param szIP ������IP
	/// \param usPort �������˿�
	/// \param blAuth �Ƿ���֤(�ݲ�֧��)
	/// \param unTimeOut ��ʱʱ��(��λ����)
	void ConnectServerWithoutHB(const char *szIP, unsigned short usPort, bool blAuth = false, unsigned int unTimeOut = 0);

	/// \brief �������ݸ��������ķ�����(��ConnectServerWithoutHB���ʹ�ã��������еķ����������)
	/// \param pbData ����ָ��
	/// \param unDataSize ���ݴ�С
	/// \return �Ƿ��ͳɹ�
	bool SendDataToServerWithoutHB(const unsigned char *pbData, unsigned int unDataSize);

	/// \brief ��������Ͽ���������
	void CloseSocket();

	/// \brief ���ӷ�����֪ͨ����
	/// \param apConnection ����֪ͨ������
	/// \param nStatus ����״̬
	void OnConnect(CONNECTION_APTR apConnection, int nStatus);

	/// \brief ���ӶϿ���֪ͨ����
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
	/// \brief ����TCP������
	/// \param szIP ������IP
	/// \param usPort �������˿�
	/// \param blAuth �Ƿ���֤(�ݲ�֧��)
	/// \param unTimeOut ��ʱʱ��(��λ����)
	void Connect(const char *szIP, unsigned short usPort, bool blAuth, unsigned int unTimeOut);

	/// \brief �������ݸ�������
	/// \param pbData ����ָ��
	/// \param unDataSize ���ݴ�С
	/// \return �Ƿ��ͳɹ�
	bool SendData(const unsigned char *pbData, unsigned int unDataSize);

	/// \brief ���ӷ�������ʵ���̺߳���
	/// \param nStatus ����״̬
	void OnConnectThread(int nStatus);

	/// \brief ���ӶϿ���ʵ���̺߳���
	/// \param nStatus ״̬��
	void OnDisconnectThread(int nStatus);

	/// \brief ���ݽ���������̺߳���
	void OnDataErrorThread();

	/// \brief ���ӳ�ʱ����ʱ������
	/// \param oErrorCode �������
	void OnConnectTimeOutTimer(const boost::system::error_code& oErrorCode);

	/// \brief �޽������ݶ�ʱ������
	/// \param oErrorCode �������
	void OnNoRecvDataTimer(const boost::system::error_code& oErrorCode);

private:
	/// \brief ������������Ӷ���
	CONNECTION_APTR m_apConnection;

	/// \brief ���ӳ�ʱ��ʱ��
	deadline_timer m_oTimer;

	/// \brief ���ճ�ʱ��ʱ��
	deadline_timer m_oTimeOutTimer;

	/// \brief ������
	boost::mutex m_oOperateMutex;

	/// \brief ͨ������ָ��
	IFndNetClientNotify *m_poNotify;

	/// \brief ͨ��������
	boost::shared_mutex m_oNotifyMutex;
};


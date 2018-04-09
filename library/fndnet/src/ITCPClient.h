/// \file		ITCPClient.h
/// \brief		ͨ��(TCP)�ͻ�����������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-19
/// \history	2011-07-19 �½�

#ifdef _WIN32
#pragma once
#endif

#ifndef _BASECCOM_ENUM_DEF_
#define _BASECCOM_ENUM_DEF_

/// \brief ����״̬ö��
enum ENU_CONNECT_STATUS
{
	E_CONNECT_SUCCESS,
	E_CONNECT_ADDRERROR,
	E_CONNECT_IOERROR,
	E_CONNECT_TIMEOUT,
};

/// \brief �Ͽ�����״̬ö��
enum ENU_DISCONNECT_STATUS
{
	E_DISCONNECT_USERASK,
	E_DISCONNECT_IOERROR,
	E_DISCONNECT_RCVTIMEOUT,
};

#endif //_BASECCOM_ENUM_DEF_

/// \brief ͨ��(TCP)�ͻ���ͨ����
class IFndNetClientNotify
{
public:
	/*
		ע�⣺�����߳�ʱ���ԭ���п��ܳ���OnRawData������OnConnected�����õ������
		����ͬ���������ڻص����������д���
	*/
	/// \brief ���ӷ������ص���������nStatusָ�������Ƿ�ɹ���
	virtual void OnConnected(int nStatus) = 0;

	/// \brief �ӷ��������յ����ݻص�(�ص�����Ӧ�������ݺ��������أ�����Ӱ�����Ч�ʣ���������ʽ����ʱ�䲻�ó���15s���������)
	/// \param pbData ����ָ��(�������ú��ͷţ��𳬷�Χʹ�ã����ں�����ɾ��)
	/// \param unDataSize ���ݴ�С
	virtual void OnRawData(const unsigned char* pbData, unsigned int unDataSize) = 0;

	/// \brief �ӷ��������������쳣�ص�
	virtual void OnServerDataError() = 0;

	/// \brief Socket���ӶϿ��ص�
	virtual void OnDisconnected(int nStatus) = 0;
};

/// \brief ͨ��(TCP)�ͻ�����
class IFndNetClient
{
public:
	/// \brief ������������TCP������
	/// \param szIP ������IP
	/// \param usPort �������˿�
	/// \param blAuth �Ƿ���֤(�ݲ�֧��)
	/// \param unTimeOut ��ʱʱ��(��λ��)
	virtual void ConnectServerWithHB(const char *szIP, unsigned short usPort, bool blAuth = false, unsigned int unTimeOut = 0) = 0;

	/// \brief �������ݸ��������ķ�����(��ConnectServerWithHB���ʹ��)
	/// \param pbData ����ָ��
	/// \param unDataSize ���ݴ�С
	/// \return �Ƿ��ͳɹ�
	virtual bool SendDataToServerWithHB(const unsigned char *pbData, unsigned int unDataSize) = 0;

	/// \brief ������������TCP������
	/// \param szIP ������IP
	/// \param usPort �������˿�
	/// \param blAuth �Ƿ���֤(�ݲ�֧��)
	/// \param unTimeOut ��ʱʱ��(��λ��)
	virtual void ConnectServerWithoutHB(const char *szIP, unsigned short usPort, bool blAuth = false, unsigned int unTimeOut = 0) = 0;

	/// \brief �������ݸ��������ķ�����(��ConnectServerWithoutHB���ʹ��)
	/// \param pbData ����ָ��
	/// \param unDataSize ���ݴ�С
	/// \return �Ƿ��ͳɹ�
	virtual bool SendDataToServerWithoutHB(const unsigned char *pbData, unsigned int unDataSize) = 0;

	/// \brief ��������Ͽ���������
	virtual void CloseSocket() = 0;
};

extern "C"
{
	/// \brief �����ͻ��˶���
	/// \param poNotifyObj ͨ������ָ��
	/// \return �ͻ��˶���ָ��
	IFndNetClient* NetCreateClient( IFndNetClientNotify* poNotifyObj, unsigned short usThreadCount = 0 );

	/// \brief �ͷſͻ��˶���
	/// \param poClientInstance �ͻ��˶���ָ��
	void NetDeleteClient( IFndNetClient* poClientInstance );
}

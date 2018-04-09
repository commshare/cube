/// \file		ITCPServer.h
/// \brief		ͨ��(TCP)�ͻ�����������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-19
/// \history	2011-07-19 �½�

#ifdef _WIN32

#pragma once

#else

#include "ComHeader.h"

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

/// \brief ͨ��(TCP)�����ͨ����
class IFndNetServerNotify
{
public:
	/*
		ע�⣺�����߳�ʱ���ԭ���п��ܳ���OnRawData������OnAccept�����õ������
		����ͬ���������ڻص����������д���
	*/

	/// \brief ���ܿͻ������ӻص�
	/// \param ullClientID �ͻ���ID
	/// \param szClientIP �ͻ���IP
	/// \param usPort �ͻ����˿�
	virtual void OnAccept(unsigned __int64 ullClientID, const char *szClientIP, unsigned short usPort) = 0;

	/// \brief �ӿͻ��˽��յ����ݻص�(�ص�����Ӧ�������ݺ��������أ�����Ӱ�����Ч�ʣ���������ʽ����ʱ�䲻�ó���15s���������)
	/// \param ullClientID �ͻ���ID
	/// \param pbData ����ָ��(�������ú��ͷţ��𳬷�Χʹ�ã����ں�����ɾ��)
	/// \param unDataSize ���ݴ�С
	virtual void OnRawData(unsigned __int64 ullClientID, const unsigned char *pbData, unsigned int unDataSize) = 0;

	/// \brief �ӿͻ��˽��������쳣�ص�
	/// \param ullClientID �ͻ���ID
	virtual void OnClientDataError(unsigned __int64 ullClientID) = 0;

	/// \brief �Ͽ��ͻ����������ӻص�
	/// \param ullClientID �ͻ���ID
	virtual void OnClientDisconnect(unsigned __int64 ullClientID) = 0;
};

/// \brief ͨ��(TCP)�������
class IFndNetServer
{
public:
/*	/// \brief ��������
	virtual ~IFndNetServer() {}
*/
	/// \brief ��ָ���˿�����TCP����������
	/// \param usPort �����˿ں�
	/// \param blAuth �Ƿ���֤(�ݲ�֧��)
	virtual bool StartListenWithHB(unsigned short usPort, bool blAuth = false) = 0;

	/// \brief �������ݸ�ָ���Ŀͻ���
	/// \param ullClientID �ͻ���ID
	/// \param pbData ����ָ��
	/// \param unDataSize ���ݴ�С
	/// \return �Ƿ��ͳɹ�
	virtual bool SendDataWithHB(unsigned __int64 ullClientID, const unsigned char *pbData, unsigned int unDataSize) = 0;

	/// \brief ��ָ���˿�������������TCP����������(���������������ڼ������е�Relay�����)
	/// \param usPort �����˿ں�
	/// \param blAuth �Ƿ���֤(�ݲ�֧��)
	virtual bool StartListenWithoutHB(unsigned short usPort, bool blAuth = false) = 0;

	/// \brief �������ݸ��������Ŀͻ���(��StartListen���ʹ��)
	/// \param ullClientID �ͻ���ID
	/// \param pbData ����ָ��
	/// \param unDataSize ���ݴ�С
	/// \return �Ƿ��ͳɹ�
	virtual bool SendDataWithoutHB(unsigned __int64 ullClientID, const unsigned char *pbData, unsigned int unDataSize) = 0;

	/// \brief �ر�TCP Server
	virtual void CloseSocket() = 0;
};

extern "C"
{
	/// \brief ��������˶���
	/// \param poNotifyObj ͨ������ָ��
	/// \return ����˶���ָ��(���Զ�ɾ������ɾ��)
	IFndNetServer* NetCreateServer(IFndNetServerNotify* poNotifyObj, unsigned short usThreadCount = 0);

	/// \brief �ͷŷ���˶���
	/// \param poServerInstance ����˶���ָ��
	void NetDeleteServer( IFndNetServer *poServerInstance );
}

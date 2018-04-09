/// \file		WnConnBase.h
/// \brief		BASE���ӻ���������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-19
/// \history	2011-07-19 �½�

#ifdef _WIN32
#pragma once
#endif

#include "WnPacket.h"
#include "WnEndPoint.h"
#include "WnThreadSafeList.h"

typedef WnThreadSafeList<PACKET_APTR> SAFELIST_PACKET;

enum ENU_ASYNSEND_RESULT
{
	E_AR_SUCCESS,
	E_AR_NOCONNECT,
	E_AR_NOSEND,
	E_AR_OVERFLOW,
};

/// \brief BASE���ӻ���
class WnConnBase
{
public:
	/// \brief ���캯��
	/// \param apEndPoint �����ն�
	/// \param blRawData ԭʼ���ݱ�־
	WnConnBase(END_POINT_APTR apEndPoint, bool blRawData);
	/// \brief ��������
	virtual ~WnConnBase(void);

	/// \brief ���ö��ַ�
	/// \param strHostName �Է�������
	/// \param strPort �Է��˿ں�
	/// \return 0 ���óɹ�������ֵΪ�������
	virtual int SetAddr(const std::string& strHostName, const std::string& strPort) = 0;

	/// \brief �ͻ��˿�ʼ���Ӻ���
	virtual void BeginConnect() = 0;

	/// \brief ����˽��յ����ӵ���Ӧ����
	virtual void OnAccept() = 0;

	/// \brief �첽���ͺ���
	/// \param oPacket ���ݰ�
	/// \param blHighPriority �Ƿ��и����ȼ�
	/// \return ���ͽ��ֵ
	virtual ENU_ASYNSEND_RESULT AsyncSend(const PACKET_APTR& oPacket, bool blHighPriority) = 0;

	/// \brief �ر����Ӻ���
	void Close();

protected:
	/// \brief �ر����Ӵ�����
	virtual void OnClose() = 0;

	/// \brief ��������״̬
	/// \param blConnected �Ƿ�����
	void SetConnectedState(bool blConnected);

	/// \brief �Ƿ�������
	/// \return ���ӱ�־
	bool IsConnected();

	/// \brief ���յ����ݵĴ�����
	/// \param apConnection ����ָ��
	/// \param unLength ���ݳ���
	virtual void HandleInData(CONNECTION_APTR apConnection, size_t unLength);

protected:
	// �����ն�
	END_POINT_APTR m_apLocalEndPoint;

	// ԭʼ���ݱ�־(�޸�ʽ)
	bool m_blRawData;

	// �Ƿ������ӳɹ�
	bool m_blConnected;

	// ���ӳɹ�״̬��
	boost::shared_mutex m_oConnectedMutex;

	// ����д�ı�־
	bool m_blWriting;

	// д״̬��
	boost::mutex m_oWritingMutex;

	// ���Ͱ�����
	SAFELIST_PACKET m_oSendPacketList;

	// ��ǰ���Ͱ����г���
	unsigned int m_unCurrentListSize;

	// �����ʱ��
	ptime m_oLastestSendTime;

	// ��������С
	unsigned int m_unRecvBufferSize;

	// ���ջ�����
	unsigned char* m_pbRecvBuffer;

	/// \brief ��ͷ
	TAG_PACKET_HEADER m_tHeader;

	/// \brief �Ѵ����ֽ���
	unsigned int m_unReadBytes;

	/// \brief ��������ָ��
	unsigned char *m_pbBodyData;

	/// \brief �������ݴ�С
	unsigned int m_unBodySize;
};

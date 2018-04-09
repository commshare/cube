/// \file		WnEndPoint.h
/// \brief		BASEͨ���ն���������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-19
/// \history	2011-07-19 �½�
#ifndef WNENDPOINT_H_
#define WNENDPOINT_H_

#ifdef _WIN32
#pragma once
#endif

#include "WnPacket.h"

class WnConnBase;
typedef boost::shared_ptr<WnConnBase> CONNECTION_APTR;

class WnEndPoint : public boost::enable_shared_from_this<WnEndPoint>
{
public:
	/// \brief ���캯��
	WnEndPoint();
	/// \brief ��������
	virtual ~WnEndPoint(void);

	/// \brief �Ƿ�������
	bool IsEnableHB() { return m_blHB; }

	/// \brief ���յ����ӵĴ�����
	/// \param apConnection ����֪ͨ������
	virtual void OnAccept(CONNECTION_APTR apConnection) {}

	/// \brief ���ӷ��صĴ�����
	/// \param apConnection ����֪ͨ������
	/// \param nStatus ����״̬
	virtual void OnConnect(CONNECTION_APTR apConnection, int nStatus) {}

	/// \brief ���ӶϿ��Ĵ�����
	/// \param apConnection ����֪ͨ������
	/// \param nStatus ״̬��
	virtual void OnDisconnect(CONNECTION_APTR apConnection, int nStatus) = 0;

	/// \brief ���յ����ݵĴ�����
	/// \param apConnection ����֪ͨ������
	/// \param apPacket �յ������ݰ�
	virtual void OnReceiveData(CONNECTION_APTR apConnection, const PACKET_APTR& apPacket) = 0;

	/// \brief ���ݽ�������Ĵ�����
	/// \param apConnection ����֪ͨ������
	virtual void OnDataError(CONNECTION_APTR apConnection) = 0;

	/// \brief �ر��ն�
	virtual void Close() {}

	/// \brief �ж��Ƿ���Ҫ����
	/// \return ������־
	bool IsNeedReconnect() { return m_blReconnect; }

	/// \brief ����������־
	/// \param blReconnect ������־
	void SetReconnectFlag(bool blReconnect) { m_blReconnect = blReconnect; }

	/// \brief ��ȡ�������(��λ����)
	/// \return �������
	unsigned int GetReconnectInterval() { return m_unReconnectInterval; }

	/// \brief �����������(��λ����)
	/// \param unReconnectInterval �������
	void SetReconnectInterval(unsigned int unReconnectInterval) { m_unReconnectInterval = unReconnectInterval; }

protected:
	/// \brief �Ƿ���������ģʽ�ı�־
	bool m_blHB;

	/// \brief ������־
	bool m_blReconnect;

	/// \brief �������
	unsigned int m_unReconnectInterval;
};

typedef boost::shared_ptr<WnEndPoint> END_POINT_APTR;
typedef std::list<END_POINT_APTR> LIST_END_POINT;

#endif

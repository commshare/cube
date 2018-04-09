/// \file		WnAcceptorBase.h
/// \brief		BASE��������������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-19
/// \history	2011-07-19 �½�

#ifdef _WIN32
#pragma once
#endif

/// \brief BASE��������
class WnAcceptorBase
{
public:
	/// \brief ���캯��
	WnAcceptorBase(END_POINT_APTR apLocalEndPoint)
		: m_apLocalEndPoint(apLocalEndPoint)
		, m_blRunning(false)
		, m_oRunningMutex()
	{}

	/// \brief ��������
    virtual ~WnAcceptorBase() {}

	/// \brief �趨�ն˵�ַ
	/// \param strHostName ������
	/// \param strPort �˿ں�
    virtual int SetAddr(const string& strHostName, const string& strPort) = 0;

    /// \brief ��������
    virtual bool Start() = 0;

	/// \brief ֹͣ����
    virtual void Stop() = 0;

protected:
	// �����ն�
	END_POINT_APTR m_apLocalEndPoint;

	// �������еı�־
    bool m_blRunning;

	// ����״̬��
	boost::mutex m_oRunningMutex;
};

typedef boost::shared_ptr<WnAcceptorBase> ACCEPTOR_APTR;
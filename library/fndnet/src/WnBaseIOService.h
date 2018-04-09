/// \file		WnBaseIOService.h
/// \brief		BASE����IO������������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-20
/// \history	2011-07-20 �½�

#ifdef _WIN32
#pragma once
#endif

/// \brief BASE����IO������������ļ�
class WnBaseIOService
{
public:
	/// \brief ��ȡ����IO�������ָ��
	static WnBaseIOService* GetInstance();

	/// \brief ɾ������IO�������
	static void Delete();

	/// \brief ��������
	void StartService(unsigned short usThreadCount);

	/// \brief ֹͣ����
	void StopService();

	/// \brief ��ȡIO�첽�������
	io_service& GetIOService() { return m_oIoService; }

private:
	/// \brief ���캯��
	WnBaseIOService(void);

	/// \brief ��������
	~WnBaseIOService(void);

	/// \brief ��ȡ��CPU��ƥ����߳���
	unsigned short GetThreadCount();

	/// \brief IO�첽��������̺߳���
	void ThreadsFunc();

private:
	/// \brief �������ľ�ָ̬��
	static WnBaseIOService* s_poService;

	/// \brief IO�첽�������
	io_service m_oIoService;

	/// \brief IO����״̬
	boost::shared_ptr<io_service::work> m_apWork;

	/// \brief IO�첽��������߳���
	boost::thread_group m_oIoServiceThreads;

	/// \brief ֹͣ��־
	bool m_blStop;
};

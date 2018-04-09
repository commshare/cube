/// \file		WnBaseIOService.cpp
/// \brief		BASE����IO������Ķ����ļ�
/// \author
/// \version	1.0
/// \date		2011-07-20
/// \history	2011-07-20 �½�

#include "ComHeader.h"

#include "WnBaseIOService.h"

#ifdef _DEBUG
#define new	new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

WnBaseIOService* WnBaseIOService::s_poService = NULL;

WnBaseIOService::WnBaseIOService(void)
: m_oIoService()
, m_apWork()
, m_oIoServiceThreads()
, m_blStop(false)
{
}

WnBaseIOService::~WnBaseIOService(void)
{
}

WnBaseIOService* WnBaseIOService::GetInstance()
{
	if(s_poService == NULL)
	{
		// ����ʵ��
		s_poService = new WnBaseIOService;
	}

	return s_poService;
}

void WnBaseIOService::Delete()
{
	delete s_poService;
	s_poService = NULL;
}

void WnBaseIOService::StartService(unsigned short usThreadCount)
{
	if(!m_apWork)
	{
		m_blStop = false;

		// ʹIO���񱣳��ڹ���״̬����ֹrunֱ�ӷ���
		m_apWork.reset(new io_service::work(m_oIoService));
	}

	// ��ȡ���ʵ��߳���
	if(usThreadCount < DEFAULT_IOTHREAD_COUNT)
	{
		usThreadCount = GetThreadCount();
	}

	// �����߳���
	size_t usCount = 0;
	for(usCount = m_oIoServiceThreads.size(); usCount < usThreadCount; usCount++)
	{
		m_oIoServiceThreads.create_thread(boost::bind(&WnBaseIOService::ThreadsFunc, this));
	}
}

void WnBaseIOService::StopService()
{
	if(m_apWork)
	{
		m_blStop = true;

		// ʹIO�����˳�
		m_apWork.reset();

		// �ȴ������߳̽���
		m_oIoServiceThreads.join_all();
	}
}


unsigned short WnBaseIOService::GetThreadCount()
{
	// ��ȡcpu����
	unsigned short usThreadCount = boost::thread::hardware_concurrency();

	// ��ȡʧ��
	if(usThreadCount == 0)
	{
		// ʹ��Ĭ���߳���
		usThreadCount = DEFAULT_IOTHREAD_COUNT;
	}
	else
	{
		// ʹ���Ƽ��߳���
		usThreadCount = usThreadCount * 2 + 2;
	}

	return usThreadCount;
}

void WnBaseIOService::ThreadsFunc()
{
	// ���˳�״̬
	while(!m_blStop)
	{
		try
		{
			// IO�첽�������λ
			m_oIoService.reset();

			boost::system::error_code oErrorCode;
			m_oIoService.run(oErrorCode);
			if(oErrorCode)
			{
				//?? ���������Ϣ
			}
			oErrorCode.clear();
		}
		catch(boost::system::system_error const& ex)
		{
			//?? ���������Ϣ

			ex.what();
		}
		catch(...)
		{
			//?? ���������Ϣ
        }
    }
}

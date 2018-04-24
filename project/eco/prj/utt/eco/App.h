#ifndef ECO_CMD_COMMAND_APP_H
#define ECO_CMD_COMMAND_APP_H
/*******************************************************************************
@ ����

@ ����

@ �쳣

@ ��ע

--------------------------------------------------------------------------------
@ ��ʷ��¼ @
@ ujoy modifyed on 2016-10-29

--------------------------------------------------------------------------------
* ��Ȩ����(c) 2015 - 2017, ujoychou, ��������Ȩ����

*******************************************************************************/
#include <eco/App.h>
#include <eco/Timer.h>
#include <eco/thread/ThreadPool.h>

namespace eco{;
namespace cmd{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class App : public eco::App
{
public:
	App();

private:
	virtual void on_init() override;
	virtual void on_exit() override;

	void task_func(IN const uint32_t, IN bool repeated);
	eco::Timer m_timer;

	eco::ThreadPool m_thread_pool;
};

ECO_APP(App, GetApp);
}}}
#endif
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
#include <eco/thread/topic/TopicServer.h>
#include "Test.h"


namespace eco{;
namespace topic{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class App : public eco::App
{
public:
	App();
	virtual void on_init() override;
	virtual void on_exit() override;

private:
	Subscriber m_suber;
	eco::TopicServer m_topic_server;
};

ECO_APP(App, GetApp);
}}}
#endif
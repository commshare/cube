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


namespace eco{;
namespace log{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class App : public eco::App
{
public:
	App();

protected:
	// ������
	virtual void on_cmd() override;

	// �����µ��ļ�
	static void OnChangedLogFile(IN const char* file_path);
};

ECO_APP(App, GetApp);
}}}
#endif
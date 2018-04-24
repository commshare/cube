#ifndef ECO_CMD_TEST_H
#define ECO_CMD_TEST_H
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
#include <eco/thread/ThreadPool.h>


namespace eco{;
namespace log{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class MtCommand : public eco::cmd::Command
{
	ECO_COMMAND(MtCommand, "mt", "mt");
public:
	virtual void execute(
		IN const eco::cmd::Context& context) override;

private:
	void log_level(
		IN size_t loop_times);

	void log_domain(
		IN size_t loop_times,
		IN const char* domain);

private:
	eco::ThreadPool m_thread_pool;
};


////////////////////////////////////////////////////////////////////////////////
class FuncCommand : public eco::cmd::Command
{
	ECO_COMMAND(FuncCommand, "func", "fc");
public:
	virtual void execute(
		IN const eco::cmd::Context& context) override;

private:
	void log_test(
		IN const char* level,
		IN const char* domain);

private:
	eco::ThreadPool m_thread_pool;
};


}}}
#endif
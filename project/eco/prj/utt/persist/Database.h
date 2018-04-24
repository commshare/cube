#ifndef ECO_PERSIST_TEST_H
#define ECO_PERSIST_TEST_H
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
#include <eco/persist/Sqlite.h>
#include "Data.h"
#include "DataMapping.h"


namespace eco{;
namespace persist{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
class Database
{
public:
	Database(IN const std::string& file);

	// ��ȡ��Ʊ
	void ReadStock(
		OUT Stock& stock,
		IN  const std::string& stock_id);

	// ��ȡ��Ʊ
	void ReadSomeStock(
		OUT Stock& stock,
		IN  const std::string& stock_id);

	// ����Դ
	inline eco::Database& Source()
	{
		return *m_source;
	}

private:
	// �������
	void InitTable();

private:
	std::shared_ptr<eco::Sqlite> m_source;
};


}}}
#endif
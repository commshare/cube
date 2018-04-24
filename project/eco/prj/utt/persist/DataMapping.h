#ifndef ECO_PERSIST_DATA_MAPPING_H
#define ECO_PERSIST_DATA_MAPPING_H
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
#include <eco/persist/ObjectMapping.h>
#include <eco/meta/Timestamp.h>


namespace eco{;
namespace persist{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
// �־û�ӳ���ϵ
class DataMapping
{
	ECO_SINGLETON(DataMapping);
public:
	void Init();

	// ��Ʊ
	inline const eco::ObjectMapping& GetStock() const
	{
		return m_stock_orm;
	}

	// ��Ʊ��
	inline const eco::ObjectMapping& GetStockGroup() const
	{
		return m_stock_group_orm;
	}

	// ��Ʊ��
	inline const eco::ObjectMapping& GetStockPool() const
	{
		return m_stock_pool_orm;
	}

private:
	eco::ObjectMapping m_stock_orm;
	eco::ObjectMapping m_stock_group_orm;
	eco::ObjectMapping m_stock_pool_orm;
};
ECO_SINGLETON_GET(DataMapping);


}}}
#endif
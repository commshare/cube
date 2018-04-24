#include "PrecHeader.h"
#include "App.h"
////////////////////////////////////////////////////////////////////////////////
#include <eco/Project.h>
#include "Database.h"


namespace eco{;
namespace persist{;
namespace test{;


////////////////////////////////////////////////////////////////////////////////
App::App()
{
	// ��ʼ�����ݿ�.
	GetDataMapping().Init();
	m_database.reset(new Database("vision.db"));

	// 1.����������
	std::shared_ptr<Stock> stock(new Stock);
	stock->m_id = "600018";
	stock->m_name = "��������";
	stock->m_group_id = 10001;
	m_database->Source().save(*stock, StockMeta(), GetDataMapping().GetStock());
	// 1.���������ݣ���ʽ1��
	stock.reset(new Stock);
	stock->m_id = "600058";
	stock->m_name = "��������";
	stock->m_group_id = 100001;
	m_database->Source().save(*stock, StockMeta(), GetDataMapping().GetStock());
	// 1.���������ݣ���ʽ2��
	stock->m_id = "600048";
	stock->m_name = "����ę́";
	stock->m_group_id = 10002;
	stock->timestamp().insert();	// ָ���������
	m_database->Source().save(*stock, StockMeta(), GetDataMapping().GetStock());

	// ��ȡָ������
	stock.reset(new Stock);
	stock->m_id = "600018";
	m_database->Source().read(*stock, StockMeta(), GetDataMapping().GetStock());

	//--------------------------------------------------------------------------
	// 2.��������
	stock->SetValue("name", "����Һ");
	m_database->Source().save(*stock, StockMeta(), GetDataMapping().GetStock());

	// ��ȡ���º������
	stock.reset(new Stock);
	stock->m_id = "600018";
	m_database->Source().read(*stock, StockMeta(), GetDataMapping().GetStock());

	// ��ѯ��������
	std::list<Stock::value> stock_list;
	m_database->Source().select(
		stock_list, StockMeta(), GetDataMapping().GetStock());

	// ��ѯָ������
	stock_list.clear();
	m_database->Source().select(
		stock_list, StockMeta(), GetDataMapping().GetStock(),
		"id like '600%'");

	//--------------------------------------------------------------------------
	// 3.ɾ��ָ������
	stock.reset(new Stock);
	stock->m_id = "600018";
	stock->timestamp().remove();
	m_database->Source().save(*stock, StockMeta(), GetDataMapping().GetStock());

	// ��ѯ��������
	stock_list.clear();
	m_database->Source().select(
		stock_list, StockMeta(), GetDataMapping().GetStock());

	// ɾ����������
	m_database->Source().remove(
		GetDataMapping().GetStock().get_table());

	// ��ѯ��������
	stock_list.clear();
	m_database->Source().select(
		stock_list, StockMeta(), GetDataMapping().GetStock());
}


////////////////////////////////////////////////////////////////////////////////
}}}
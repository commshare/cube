/// \file		WnUtility.cpp
/// \brief		BASE������Ķ����ļ�
/// \author
/// \version	1.0
/// \date		2011-07-20
/// \history	2011-07-20 �½�

#include "ComHeader.h"

#include "WnUtility.h"

#ifdef _DEBUG
#define new	new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

WnUtility* WnUtility::s_poUtility = NULL;

WnUtility::WnUtility(void)
{
}

WnUtility::~WnUtility(void)
{
}

WnUtility* WnUtility::GetInstance()
{
	if(s_poUtility == NULL)
	{
		// ����ʵ��
		s_poUtility = new WnUtility;
	}

	return s_poUtility;
}

void WnUtility::Delete()
{
	delete s_poUtility;
	s_poUtility = NULL;
}

unsigned __int64 WnUtility::GetTimeValue()
{
	// ȡ��ǰʱ���ַ���(YYYYMMDDThhmmss)
	string strTime(to_iso_string(second_clock::local_time()));
	// ɾ��T�ַ�
	strTime.erase(strTime.find('T'), 1);
	// ɾ����ݵ�ǰ��λ
	strTime.erase(0, 2);

	// תΪ����
	return boost::lexical_cast<unsigned __int64>(strTime);
}

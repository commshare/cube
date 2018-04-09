/// \file		WnCommGlobal.h
/// \brief		BASEͨ��ģ���ȫ�ֱ�����������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-20
/// \history	2011-07-20 �½�

#ifdef _WIN32
#pragma once
#endif

#include "WnEndPoint.h"

class WnBaseIOService;
class WnUtility;

/// \brief BASEͨ��ģ���ȫ�ֱ�����
class WnCommGlobal
{
public:
	/// \brief ��ȡȫ�ֶ�����
	static boost::mutex& GetGlobalMutex() { return s_oGlobalMutex; }

	/// \brief ��ȡȫ�ֱ�����������ָ��
	static WnCommGlobal* GetInstance();
	/// \brief ɾ��ȫ�ֱ�����������
	static void Delete();

	/// \brief ������������
	/// \param usThreadCount �����߳���
	void StartService(unsigned short usThreadCount);

	/// \brief ���ͨ���ն�
	/// \param poEndPoint ͨ���ն�ָ��
	void AddEndPoint(WnEndPoint *poEndPoint) { m_oEndPointList.push_back(END_POINT_APTR(poEndPoint)); }

	/// \brief ɾ��ͨ���ն�
	/// \param poEndPoint ͨ���ն�ָ��
	void RemoveEndPoint(WnEndPoint *poEndPoint);

	/// \brief ͨ���ն˱��Ƿ�Ϊ��
	/// \return trueΪ�գ�false����
	bool IsEndPointListEmpty() { return m_oEndPointList.empty(); }

private:
	/// \brief ���캯��
	WnCommGlobal(void);
	/// \brief ��������
	~WnCommGlobal(void);

private:
	/// \brief ȫ�ֶ�����
	static boost::mutex s_oGlobalMutex;

	/// \brief ȫ�ֱ�����������ָ��
	static WnCommGlobal* s_poGlobalVar;

	/// \brief ����IO�����ָ��
	WnBaseIOService *m_poBaseIOService;

	/// \brief ���ܶ���
	WnUtility *m_poUtility;

	/// \brief ͨ���ն˱�
	LIST_END_POINT m_oEndPointList;
};

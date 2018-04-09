/// \file		WnThreadSafeList.h
/// \brief		�̰߳�ȫ��listģ����������ļ�
/// \author
/// \version	1.0
/// \date		2011-04-20
/// \history	2011-04-20 �½�

#ifdef _WIN32
#pragma once
#endif

#include <list>
#include <boost/thread/mutex.hpp>

#ifdef _DEBUG
#define new	new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

/// \brief �̰߳�ȫ��listģ����
template <typename T>
class WnThreadSafeList
{
public:
	/// \brief ���캯��
	WnThreadSafeList() : m_oList(), m_oSharedMutex() {}

	/// \brief ��������
	~WnThreadSafeList()
	{
		clear();
	}

	/// \brief ȡ����Ԫ��(����ǰ���������Ƿ�Ϊ��)
	/// \return ����Ԫ��(����Ϊ��ʱ������ֵδ����) 
	T front()
	{
		boost::shared_lock<boost::shared_mutex> oLock(m_oSharedMutex);
		return m_oList.front();
	}

	/// \brief ȡ��βԪ��(����ǰ���������Ƿ�Ϊ��)
	/// \return ��βԪ��(����Ϊ��ʱ������ֵδ����) 
	T back()
	{
		boost::shared_lock<boost::shared_mutex> oLock(m_oSharedMutex);
		return m_oList.back();
	}

	/// \brief ��������Ԫ��(����ǰ���������Ƿ�Ϊ��)
	void pop_front()
	{
		boost::unique_lock<boost::shared_mutex> oLock(m_oSharedMutex);
		m_oList.pop_front();
	}

	/// \brief ������βԪ��(����ǰ���������Ƿ�Ϊ��)
	void pop_back()
	{
		boost::unique_lock<boost::shared_mutex> oLock(m_oSharedMutex);
		m_oList.pop_back();
	}

	/// \brief ��Ԫ�ؼ������ͷ��
	/// \param t ������е�Ԫ�� 
	void push_front(T t)
	{
		boost::unique_lock<boost::shared_mutex> oLock(m_oSharedMutex);
		m_oList.push_front(t);
	}

	/// \brief ��Ԫ�ؼ������β��
	/// \param t ������е�Ԫ�� 
	void push_back(T t)
	{
		boost::unique_lock<boost::shared_mutex> oLock(m_oSharedMutex);
		m_oList.push_back(t);
	}

	/// \brief �ж϶����Ƿ�Ϊ��
	/// \return false ���в��գ�true ����Ϊ��
	bool empty()
	{
		boost::shared_lock<boost::shared_mutex> oLock(m_oSharedMutex);
		return m_oList.empty();
	}

	/// \brief ��ȡ������Ԫ�ظ���
	/// \return ������Ԫ�ظ���
	unsigned int size()
	{
		boost::shared_lock<boost::shared_mutex> oLock(m_oSharedMutex);
		return static_cast<unsigned int>(m_oList.size());
	}

	/// \brief ��ն���
	void clear()
	{
		boost::unique_lock<boost::shared_mutex> oLock(m_oSharedMutex);
		m_oList.clear();
	}
private:
	// �ڲ�ʵ������List
	std::list<T> m_oList;

	// ���м���������
	boost::shared_mutex m_oSharedMutex;
};
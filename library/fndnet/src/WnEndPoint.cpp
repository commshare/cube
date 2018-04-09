/// \file		WnEndPoint.cpp
/// \brief		BASEͨ���ն���Ķ����ļ�
/// \author
/// \version	1.0
/// \date		2011-07-19
/// \history	2011-07-19 �½�

#include "ComHeader.h"

#include "WnEndPoint.h"
#include "WnConnBase.h"

#ifdef _DEBUG
#define new	new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

WnEndPoint::WnEndPoint()
: m_blHB(true)
, m_blReconnect(false)
, m_unReconnectInterval(0)
{
}

WnEndPoint::~WnEndPoint(void)
{
}

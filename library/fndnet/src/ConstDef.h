/// \file		ConstDef.h
/// \brief		ʹ�ó����������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-19
/// \history	2011-07-19 �½�
#ifndef CONSTDEF_H_
#define CONSTDEF_H_

#ifdef _WIN32
#pragma once
#endif

// Ĭ��IO�����߳���
const unsigned short DEFAULT_IOTHREAD_COUNT = 6;

// �������ʱ��(����)
const unsigned int HEARTBEAT_INTERVAL = 15000;

// ����������
const unsigned int NO_HEARTBEAT_COUNT = 3;

// һ��˯��ʱ��(����)
const unsigned int NORMAL_SLEEP_TIME = 50;

// ��ʼ�ֽ�
const unsigned char STX = 0x02;

// ��ֹ�ֽ�
const unsigned char ETX = 0x03;

// ���ݰ���ʼ��־�ֽ�
const unsigned char PACKET_START_FLAG = 0x78;

// ������־
const unsigned char HEARTBEAT_BITFLAG = 0x01;

// ���ջ�������С
const unsigned int RECV_BUFFER_SIZE = 1024 * 1024;

// ԭʼ���ݷ��Ͷ���������󳤶�
const unsigned int MAX_RAWDATA_COUNT = 10000;

#endif

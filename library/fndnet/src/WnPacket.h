/// \file		WnPacket.h
/// \brief		BASE���ݰ���������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-19
/// \history	2011-07-19 �½�
#ifndef WNPACKET_H_
#define WNPACKET_H_

#ifdef _WIN32
#pragma once
#endif

/// \brief BASE���ݰ�ͷ�ṹ
struct TAG_PACKET_HEADER
{
	/// \brief ��ʼ��־
	unsigned char bStartFlag;
	/// \brief ��ͷ����
	unsigned char bHeaderLen;
	/// \brief �������
	unsigned char bCheckIndex;
	/// \brief ��־�ֽ�
	unsigned char bFlag;
	/// \brief ���峤��(�ֽ���)
	unsigned char bPacketLen[4];
};

/// \brief BASE���ݰ���
class WnPacket
{
public:
	/// \brief ���캯��(�����շ�ʹ�ã�������)
	/// \param tHeader ��ͷ
	/// \param pbData ��������ָ��
	/// \param unSize �������ݴ�С
	WnPacket(const TAG_PACKET_HEADER& tHeader, const unsigned char* pbData, unsigned int unSize);

	/// \brief ���캯��(�����ͷ�ʹ�ã�������)
	/// \param pbData ��������ָ��
	/// \param unSize �������ݴ�С
	/// \param bCheckIndex �������
	/// \param bFlag ��־�ֽ�
	WnPacket(const unsigned char* pbData, unsigned int unSize, unsigned char bCheckIndex, unsigned char bFlag);

	/// \brief ���캯��(��������ʽ)
	/// \param pbData ��������ָ��
	/// \param unSize �������ݴ�С
	WnPacket(const unsigned char* pbData, unsigned int unSize);

	/// \brief ��������
	~WnPacket(void);

	/// \brief ��ȡ����ָ��
	const unsigned char* GetPacketData() { return m_pbPacketData; }

	/// \brief ��ȡ���ݴ�С
	unsigned int GetPacketLength() { return m_unPacketLength; }

	/// \brief ��ȡ����ָ��
	const unsigned char* GetBodyData();

	/// \brief ��ȡ�����С
	unsigned int GetBodyLength();

	/// \brief �Ƿ�������
	bool IsHeartBeatPacket();

private:
	// ����ָ��
	unsigned char* m_pbPacketData;

	// ���ݴ�С
	unsigned int m_unPacketLength;
};

typedef boost::shared_ptr<WnPacket> PACKET_APTR;

#endif

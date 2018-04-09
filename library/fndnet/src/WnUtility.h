/// \file		WnUtility.h
/// \brief		BASE������������ļ�
/// \author
/// \version	1.0
/// \date		2011-07-20
/// \history	2011-07-20 �½�

#ifdef _WIN32
#pragma once
#endif

/// \brief BASE������
class WnUtility
{
public:
	/// \brief ��ȡ����IO�������ָ��
	static WnUtility* GetInstance();

	/// \brief ɾ������IO�������
	static void Delete();

	/// \brief ��ȡʱ��ֵ
	/// \return ʱ��ֵ
	unsigned __int64 GetTimeValue();

	/// \brief ����ת��Ϊ�����ֽ���
	/// \param Int ����ֵ
	/// \param pbStream ��ָ��
	template<typename Integer>
	void IntegerToStream(Integer Int, unsigned char* pbStream)
	{
		unsigned int unStreamSize = sizeof(Int);

		unsigned int unCount = 0;
		for(unCount = 0; unCount < unStreamSize; unCount++)
		{
			pbStream[unStreamSize - 1 - unCount] = Int & 0xFF;
			Int >>= 8;
		}
	}

	/// \brief �����ֽ���ת��Ϊ����
	/// \param pbStream ��ָ��
	/// \param Int ����ֵ
	template<typename Integer>
	void StreamToInteger(unsigned char* pbStream, Integer& Int)
	{
		unsigned int unStreamSize = sizeof(Int);

		unsigned int unCount = 0;

		Int = 0;
		for(unCount = 0; unCount < unStreamSize; unCount++)
		{
			Int = (Int << 8) | pbStream[unCount];
		}
	}

private:
	/// \brief ���캯��
	WnUtility(void);
	/// \brief ��������
	~WnUtility(void);

private:
	/// \brief ���ܶ���ľ�ָ̬��
	static WnUtility* s_poUtility;
};

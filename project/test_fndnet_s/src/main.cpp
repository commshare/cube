#include <iostream>
#include <string>
#include "fndnet/ITCPServer.h"

using namespace std;


IFndNetServer* server;

class client_notify : public IFndNetServerNotify
{
public:
    client_notify(){};
    ~client_notify(){};

    /*
    ע�⣺�����߳�ʱ���ԭ���п��ܳ���OnRawData������OnAccept�����õ������
    ����ͬ���������ڻص����������д���
    */

    /// \brief ���ܿͻ������ӻص�
    /// \param ullClientID �ͻ���ID
    /// \param szClientIP �ͻ���IP
    /// \param usPort �ͻ����˿�
    virtual void OnAccept(unsigned __int64 ullClientID, const char *szClientIP, unsigned short usPort)
    {
        cout << __FUNCTION__ << endl;
        cout << "ullClientID:" << ullClientID << endl;
    }

    /// \brief �ӿͻ��˽��յ����ݻص�(�ص�����Ӧ�������ݺ��������أ�����Ӱ�����Ч�ʣ���������ʽ����ʱ�䲻�ó���15s���������)
    /// \param ullClientID �ͻ���ID
    /// \param pbData ����ָ��(�������ú��ͷţ��𳬷�Χʹ�ã����ں�����ɾ��)
    /// \param unDataSize ���ݴ�С
    virtual void OnRawData(unsigned __int64 ullClientID, const unsigned char *pbData, unsigned int unDataSize)
    {
        cout << __FUNCTION__ << endl;
        cout << "pbData:" << pbData << endl;

        string msg = "hello world";
        server->SendDataWithHB(ullClientID, (const unsigned char*)msg.c_str(), msg.size());
    }

    /// \brief �ӿͻ��˽��������쳣�ص�
    /// \param ullClientID �ͻ���ID
    virtual void OnClientDataError(unsigned __int64 ullClientID)
    {
        cout << __FUNCTION__ << endl;
    }

    /// \brief �Ͽ��ͻ����������ӻص�
    /// \param ullClientID �ͻ���ID
    virtual void OnClientDisconnect(unsigned __int64 ullClientID)
    {
        cout << __FUNCTION__ << endl;
    }
};



int main(int argc,char* argv[])
{
    client_notify notify;
    server = NetCreateServer(&notify, 2);
    server->StartListenWithHB(9025);

    getchar();
	return 0;
}
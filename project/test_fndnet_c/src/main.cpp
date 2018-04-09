#include <iostream>
#include <string>
#include "fndnet/ITCPClient.h"

using namespace std;

class client_notify : public IFndNetClientNotify
{
public:
    client_notify(){};
    ~client_notify(){};

    /*
    ע�⣺�����߳�ʱ���ԭ���п��ܳ���OnRawData������OnConnected�����õ������
    ����ͬ���������ڻص����������д���
    */
    /// \brief ���ӷ������ص���������nStatusָ�������Ƿ�ɹ���
    virtual void OnConnected(int nStatus)
    {
        cout << __FUNCTION__ << endl;
        cout << "status:" << nStatus << endl;
    }

    /// \brief �ӷ��������յ����ݻص�(�ص�����Ӧ�������ݺ��������أ�����Ӱ�����Ч�ʣ���������ʽ����ʱ�䲻�ó���15s���������)
    /// \param pbData ����ָ��(�������ú��ͷţ��𳬷�Χʹ�ã����ں�����ɾ��)
    /// \param unDataSize ���ݴ�С
    virtual void OnRawData(const unsigned char* pbData, unsigned int unDataSize)
    {
        cout << __FUNCTION__ << endl;
        string result;
        result.append((char*)(pbData), unDataSize);
        cout << result << endl;
    }

    /// \brief �ӷ��������������쳣�ص�
    virtual void OnServerDataError()
    {
        cout << __FUNCTION__ << endl;
    }

    /// \brief Socket���ӶϿ��ص�
    virtual void OnDisconnected(int nStatus)
    {
        cout << __FUNCTION__ << endl;
    }
};

int main(int argc,char* argv[])
{
    client_notify notify;
    IFndNetClient* client = NetCreateClient(&notify, 2);
    client->ConnectServerWithHB("127.0.0.1", 9025);

    string msg = "hello world";
    client->SendDataToServerWithHB((const unsigned char*)msg.c_str(), msg.size());

    getchar();
	return 0;
}
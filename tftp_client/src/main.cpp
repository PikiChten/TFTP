#include"tftp_client.h"
#include<myhead.h>

int main(int argc, char const *argv[])
{
    //对输入的服务器的ip地址进行判断
    if(argc<2)
    {
        cout<<"输入IP地址"<<endl;
        return -1;
    }
    //实例化一个客户端
    try
    {
        TFTPClient client(argv[1]);
        client.run();   
    }
    catch(const exception&e)
    {
        cerr<<e.what()<<'\n';
        return -1;
    }

    return 0;
}

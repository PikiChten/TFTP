#ifndef TFTP_CLIENT_H
#define TFTP_CLIENT_H
#include<myhead.h>

//封装一个客户端类
class TFTPClient
{
private:
    static const int PORT=69;           //服务器端口号
    static const int BUFFER_SIZE=516;   //协议包大小

    int sfd;                            //客户端套接字
    struct sockaddr_in server_addr;     //服务器地址信息结构体

    int doDownload();                   //下载函数
    int doUpload();                     //上传函数
    void clearScream();                 //清屏函数
    void waitForInput();                //等待输入函数
    void showMenu();                    //展示菜单

public:
    TFTPClient(const string & serverIP);                       //构造函数
    ~TFTPClient();                      //析构函数
    void run();                         //执行
};


#endif
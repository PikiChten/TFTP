#ifndef TFTP_SERVER_H
#define TFTP_SERVER_H
#include<myhead.h>

class TFTPServer
{
private:
    static const int PORT=69;   //服务器端口号
    static const int BUFFER_SIZE=516;
    int sfd;    //服务器套接字文件描述符
    struct sockaddr_in server_addr;
    string root_dir;    //文件服务根目录

    void handleReadRequest(const char *filename,struct sockaddr_in &client,socklen_t addr_len);
    //参数：发送给客户端的文件名，客户端地址信息结构体，地址信息结构体的长度
    void handleWriteRequest(const char *filename,struct sockaddr_in &client,socklen_t addr_len);
    //参数:将客户端发来的数据存储到的文件，客户端地址信息结构体，地址信息结构体的大小
    void sendError(const char *msg,struct sockaddr_in &client_addr);
public:
    TFTPServer(const string &root=".");
    ~TFTPServer();
    void run();
};

#endif
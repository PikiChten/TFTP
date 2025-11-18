#include"tftp_server.h"

#define ERR_LOG(msg)do{\
    perror(msg);\
    cout<<__LINE__<<" "<<__func__<<" "<<__FILE__<<endl;\
}while (0)

TFTPServer::TFTPServer(const string &root) :root_dir(root)
{
    //创建UDP套接字
    this->sfd=socket(AF_INET,SOCK_DGRAM,0);
    if (sfd<0)
    {
        ERR_LOG("socket error");
        return;
    }
    //填充地址信息结构体
    this->server_addr.sin_family=AF_INET;
    this->server_addr.sin_port=htons(PORT);
    this->server_addr.sin_addr.s_addr=htonl(INADDR_ANY);    //表示任意一个主机都可以给我发消息

    int reuse=1;
    if(setsockopt(sfd,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0)
    {
        ERR_LOG("setsockopt error");
        return;
    }

    if(bind(sfd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
        ERR_LOG("bind error");
        return;
    }
    
}

TFTPServer::~TFTPServer()
{
    if(sfd>=0)
    {
        close(sfd);
    }
}

void TFTPServer::sendError(const char *msg,sockaddr_in &client_addr)
{
    //封装错误包  操作码(2B)  错误码(2B)   错误信息(nB)
    char buf[BUFFER_SIZE]=""; 
    buf[0]=0;
    buf[1]=5;
    buf[2]=0;
    buf[3]=1;
    strcpy(buf+4,msg);
    if(sendto(sfd,buf,strlen(msg)+5,0,(struct sockaddr*)&client_addr,sizeof(client_addr))<0)  //算'\0'
    {
        ERR_LOG("sendto error");
        return;
    }
}

void TFTPServer::handleReadRequest(const char *filename,sockaddr_in &client_addr,socklen_t addr_len)
{
    //找要下载的文件路径
    string full_path=root_dir+"/"+filename;
    int fd=open(full_path.c_str(),O_RDONLY);
    if(fd<0)
    {
        sendError("file not found",client_addr);
        return ;
    }
    char buf[BUFFER_SIZE]="";
    unsigned short block_num=1; //块编号

    //开始循环给客户端发送数据包
    while (1)
    {
        buf[0]=0;
        buf[1]=3;
        *(unsigned short*)(buf+2)=htons(block_num);
        int n=read(fd,buf+4,BUFFER_SIZE-4);
        if(n<0)
        {
            sendError("read error",client_addr);
            close(fd);
            return ;
        }
        if(sendto(sfd,buf,n+4,0,(struct sockaddr*)&client_addr,addr_len)<0)
        {
            ERR_LOG("sendto error");
            close(fd);
            return;
        }
        //等待客户端返回ACK包
        do
        {
            if(recvfrom(sfd,buf,BUFFER_SIZE,0,(struct sockaddr*)&client_addr,&addr_len)<0)
            {
                ERR_LOG("recvfrom error");
                close(fd);
                return;
            }

        } while (buf[1]!=4||ntohs(*(unsigned short*)(buf+2))!=block_num);
        if(n<BUFFER_SIZE-4) //判断文件传输是否结束
        {
            break;
        } 
    
        block_num++;
    }
    close(fd);

}

void TFTPServer::handleWriteRequest(const char *filename,sockaddr_in &client_addr,socklen_t addr_len)
{
    string full_path=root_dir+"/"+filename;
    int fd=open(full_path.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0664);
    if(fd<0)
    {
        sendError("can not create file",client_addr);
        return;
    }
    //开始发送ack包，接受数据

    char buf[BUFFER_SIZE]="";
    unsigned short block_num=0;
    buf[0]=0;
    buf[1]=4;
    *(unsigned short*)(buf+2)=htons(block_num);
    if(sendto(sfd,buf,4,0,(struct sockaddr*)&client_addr,addr_len)<0)
    {
        ERR_LOG("sendto error");
        close(fd);
        return;
    }
    //循环接受数据包
    while (1)
    {
        int n=recvfrom(sfd,buf,BUFFER_SIZE,0,(struct sockaddr*)&client_addr,&addr_len);
        if(n<0)
        {
            ERR_LOG("recvfrom error");
            close(fd);
            return;
        }
        //判断收到的包是否为数据包
        if(buf[1]==3&&ntohs(*(unsigned short*)(buf+2))==block_num+1)
        {
            if(write(fd,buf+4,n-4)<0)
            {
                sendError("write error",client_addr);
                close(fd);
                return ;
            }
        //回复给客户端一个ACK
        block_num++;
        buf[0]=0;
        buf[1]=4;
        *(unsigned short*)(buf+2)=htons(block_num);
        if(sendto(sfd,buf,4,0,(struct sockaddr*)&client_addr,addr_len)<0)
        {
            ERR_LOG("sendto error");
            close(fd);
            return;
        }

        //判断读取长度和数据包长度看看是不是已经结束
        if(n<BUFFER_SIZE)
        {
            break;
        }
        }
    }
    close(fd);
    
}

void TFTPServer::run()
{
    //给出提示
    cout<<"TFTP Server started on port:"<<this->PORT<<endl;
    cout<<"Serving file from: "<<root_dir<<endl;
    
    char buf[BUFFER_SIZE]="";   //用来存储数据包
    struct sockaddr_in client_addr; //客户端地址信息结构体
    socklen_t addr_len=sizeof(client_addr); //存储客户端地址信息结构体大小

    //循环运行服务器
    while (1)
    {
        int n=recvfrom(sfd,buf,BUFFER_SIZE,0,(struct sockaddr*)&client_addr,&addr_len);
        if(n<0)
        {
            ERR_LOG("recvfrom error");
            return;
        }
        //解析请求包
        char *filename=buf+2;     //操作码（2）  文件名（n)  0  通信模式(n) 0
        const char *mode=filename+strlen(filename)+1;   //协议模式

        if (strcasecmp(mode,"octet")!=0)
        {
            sendError("Only binary mode supported",client_addr);
            continue;
        }

        if(buf[0]!=0)   //协议错误
        {
            continue;
        }
        switch (buf[1]) //检查操作码
        {
        case 1:
            cout<<"read request for:"<<filename<<endl;
            handleReadRequest(filename,client_addr,addr_len);
            break;
        case 2:
            cout<<"write request for:"<<filename<<endl;
            handleWriteRequest(filename,client_addr,addr_len);
            break;
        default:
            sendError("Unknow request",client_addr);
            break;
        }

    }
    
}
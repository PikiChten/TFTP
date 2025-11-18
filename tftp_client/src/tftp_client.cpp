#include"tftp_client.h"
//打印错误信息
#define ERR_LOG(msg) do{\
    perror(msg);\
    cout<<__LINE__<<" "<<__func__<<" "<<__FILE__<<endl;\
}while(0)


TFTPClient::TFTPClient(const string & serverIP)
{
    //创建UDP套接字
    this->sfd=socket(AF_INET,SOCK_DGRAM,0);
    if(sfd<0)
    {
        ERR_LOG("socket error");
        return;
    }
    //设置服务器地址信息结构体
    server_addr.sin_family=AF_INET;     //通信域
    server_addr.sin_port=htons(PORT);   //服务器端口号
    server_addr.sin_addr.s_addr=inet_addr(serverIP.c_str());    //服务器IP地址

}

TFTPClient::~TFTPClient()
{
    if(sfd>0)
    {
        close(sfd);
    }
}

//定义运行函数
void TFTPClient::run()
{
    while (true)
    {
        /* code */
        showMenu();
        char choice;
        cin>>choice;
        waitForInput();//吸收回城
        switch (choice)
        {
        case '1':
            /* code */
            doDownload();
            break;
        case '2':
            doUpload();
            break;
        case '3':
            return;
        default:
            cout<<"输入有误，重新输入"<<endl;
            break;
        }
        clearScream();
    }
    
}

int TFTPClient::doDownload()
{
    string filename;    //存储要下载的文件名
    cout<<"输入需要下载的文件名称: ";
    getline(cin,filename);  //cin不能处理有空格的字符串

    //封装下载请求
    char buf[BUFFER_SIZE]="";
    int size=sprintf(buf,"%c%c%s%c%s%c",0,1,filename.c_str(),0,"octet",0);
    if(sendto(sfd,buf,size,0,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
        ERR_LOG("sendto error");
        return -1; 
    }

    //循环接受服务器发送来的数据包
    size_t recv_len;
    unsigned short num=1;
    socklen_t addrlen=sizeof(server_addr);  //接收服务器地址信息结构体大小

    //定义文件操作相关操作
    int flag=0; //文件是否被打开
    int fd;

    while (true)
    {
        //清空消息容器
        bzero(buf,BUFFER_SIZE); 
        recv_len=recvfrom(sfd,buf,BUFFER_SIZE,0,(struct sockaddr*)&server_addr,&addrlen);
        if(recv_len<0)
        {
            ERR_LOG("recvfrom error");
            return -1;
        }
        //对取下来的数据进行解析
        if(buf[1]==3)   //数据包
        {
            //读取操作
            if (flag==0)
            {
                fd=open(filename.c_str(),O_WRONLY|O_CREAT|O_TRUNC,0664);
                if (fd<0)
                {
                    ERR_LOG("open error");
                    return -1;
                }
                flag=1;
            }

            //判断数据
            if(htons(num)==*(unsigned short*)(buf+2))    //传来第一个数据包
            {
                if(write(fd,buf+4,recv_len-4)<0)
                {
                    cout<<"fd = "<<fd<<"  recv_len = "<<recv_len<<endl;
                    ERR_LOG("write error");
                    close(fd);
                    break;
                }
            }
            //组装ACK发送给服务器
            buf[1]=4;
            if(sendto(sfd,buf,4,0,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
            {
                ERR_LOG("sendto error");
                return -1;
            }
            
            //判断收取的数据长度是否小于容器长度
            if(recv_len<BUFFER_SIZE)
            {
                cout<<"文件下载完毕"<<endl;
                close(fd);
                break;
            }
            num++;  //  块号自增
        }
        else if (buf[1]==5)     //错误包
        {
           cout<<"______error: "<<buf+4<<"_______"<<endl;
           if(flag==1)
           {
                close(fd);
           }
           break;
        }
        

    }
    return 0;
}

int TFTPClient::doUpload()
{
    string filename;    //要上传的文件名
    cout<<"输入需要上传的文件名称：";
    getline(cin,filename);

    //检查文件是否存在
    int fd=open(filename.c_str(),O_RDONLY);
    if(fd<0)  
    {
        if(errno==ENOENT)
        {
            cout<<"文件不存在"<<endl;
            return -2;
        }
        ERR_LOG("open error");
        return -1;
    }
    //构建上传请求
    char buf[BUFFER_SIZE]="";
    int size=sprintf(buf,"%c%c%s%c%s%c",0,2,filename.c_str(),0,"octet",0);
    if(sendto(sfd,buf,size,0,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
    {
        ERR_LOG("sendto error");
        close(fd);
        return -1;
    }
    //等待服务器接受传递的信息
    int recv_len;   //接受的长度
    unsigned short num=0;   //消息块
    socklen_t addrlen=sizeof(server_addr);
    while (1)
    {
        bzero(buf,BUFFER_SIZE);
        //读取服务器发的消息
        recv_len=recvfrom(sfd,buf,BUFFER_SIZE,0,(struct sockaddr*)&server_addr,&addrlen);
        if(recv_len<0)
        {
            ERR_LOG("recvfrom error");
            close(fd);
            return -1;
        }
        //判断收到消息类型
        if(buf[1]==4)   //ack包
        {
            if(num==ntohs(*(unsigned short*)(buf+2)))
            {
                buf[1]=3;
                num++;
                *(unsigned short*)(buf+2)=htons(num);
            }
            int res=read(fd,buf+4,BUFFER_SIZE-4);
            if(res<0)
            {
                ERR_LOG("read error");
                close(fd);
                break;
            }else if(res==0)
            {
                cout<<"上传完毕"<<endl;
                break;
            }

            //将要上传的数据发送到服务器
            if(sendto(sfd,buf,res+4,0,(struct sockaddr*)&server_addr,sizeof(server_addr))<0)
            {
                ERR_LOG("sendto error");
                return -1;
            }

        }
        else if(buf[1]==5)
        {
            cout<<"文件发送失败，请检查网络"<<endl;
            break;
        }
    }
    
    return 0;
}

void TFTPClient::clearScream()
{
    cout<<"输入任意字符清屏"<<endl;
    while (getchar()!='\n');
}

void TFTPClient::waitForInput()
{
    while (getchar()!='\n');
}

void TFTPClient::showMenu()
{
    //清屏
    system("clear");
    cout<<"*************基于UDP的TFTP文件传输*************"<<endl;
    cout<<"*************1、下载*************"<<endl;
    cout<<"*************2、上传*************"<<endl;
    cout<<"*************3、退出*************"<<endl;
    cout<<"**********************************************"<<endl;
}
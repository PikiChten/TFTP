#include"tftp_server.h"
#include<myhead.h>

int main(int argc, char const *argv[])
{
    try
    {
        string root_dir;
        if(argc>1){
             root_dir=argv[1];    //如果外部传入操作的路径使用改路径否则使用当前可执行程序所在路径
        }else
        {
             root_dir=".";
        }
        TFTPServer server(root_dir);   //构造服务器
        server.run();

    }
    catch(const std::exception& e)
    {
        cerr << e.what() << '\n';
    }
    
    return 0;
}

#include "hot_backup_client.h"

#include <sys/param.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "log.h"
#include "common_function.h"
#include "hot_backup_server.h"

using namespace std;
const char* log_config_file = "./log4cplus.properties";
#define LOCAL_PORT 8101

#include <signal.h>
#include <sys/stat.h>
int init_daemon(void)  //创建守护进程
{
    int pid;
    string cur_path = GetCurrentPath();

    //1)屏蔽一些阻断信号
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGHUP, SIG_IGN);
 
    //8)处理SIGCHLD信号
    //signal(SIGCHLD, SIG_IGN);

    //2)后台运行
    pid = fork();
    if(pid > 0)
    {
        exit(0);
    }
    else if(pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    //3)脱离控制终端、登录会话和进程组
    setsid();

    //4)禁止进程重新打开控制终端
    /*pid = fork();
    if(pid > 0)
    {
        exit(0);
    }
    else if(pid < 0)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }*/

    //5)关闭打开的文件描述符
    for(int i = 0; i < NOFILE; i++)
    {
        close(i);
    }

    //6)改变当前工作目录
    chdir(cur_path.c_str());

    //7)重新设置文件创建掩码
    umask(0);

    //9)重定向标准输出
    int fd = creat("/tmp/hot_backup_server.stdout", 0644);
    if(fd < 0)
    {
        perror("creat");
    }
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);

    return 0;
}

string GetServerIp()
{
    string ip_file("/tmp/server_ip");
    FILE* fp = fopen(ip_file.c_str(), "r");
    if(fp == NULL)
    {
        LOG_ERROR("StartHotBackupClient: open file "<<ip_file.c_str()<<" error("<<strerror(errno)<<").");
        return "";
    }
    char ip[20] = {0};
    fgets(ip, sizeof(ip) - 1, fp);
    fclose(fp);
    return string(ip);
}

int main(int args, char* argv[])
{
    init_daemon();
    INIT_LOG(log_config_file);
    if(args < 2)
    {
        LOG_ERROR("Usage: ./hot_backup client/server");
        return -1;
    }

    string local_ip = GetCurrentIp();
    int port = LOCAL_PORT;
    string server_ip;
    int server_port;
    int flag;
    if(strcmp(argv[1], "client") == 0)
    {
        if(args != 4)
        {
            LOG_ERROR("Usage: ./hot_backup client server_ip server_port");
            return -1;
        }
        server_ip = argv[2];
        server_port = atoi(argv[3]);
        LOG_INFO("main: local_ip="<<local_ip.c_str()<<", local_port="<<port<<", server_ip="<<server_ip.c_str()<<", server_port="<<server_port);
        printf("main: local_ip=%s, local_port=%d, server_ip=%s, server_port=%d\n", local_ip.c_str(), port, server_ip.c_str(), server_port);
        fflush(NULL);
        flag = 0;
    }

    if(strcmp(argv[1], "server") == 0)
    {
        server_ip = local_ip;

        if(server_ip.empty() || port <= 0)
        {
            LOG_ERROR("main: ip/port is error.");
            return -1;
        }
        LOG_INFO("main: server_ip="<<server_ip.c_str()<<", server_port="<<port);
        printf("main: server_ip=%s, server_port=%d\n", server_ip.c_str(), port);
        fflush(NULL);

        flag = 1;
    }

    while(1)
    {
        if(flag == 1)
        {
            HotBackupServer server(server_ip, port);
            server.Start();
            //server_ip = GetServerIp();
            local_ip = GetCurrentIp();
            LOG_INFO("main: server stoped. local_ip="<<local_ip.c_str()<<", server_ip="<<server_ip.c_str());
            if(server_ip == local_ip)
            {
                flag = 1;
            }
            else
            {
                flag = 0;
            }
        }
        else
        {
            HotBackupClient client(local_ip, port, server_ip, port);
            client.Start();
            //server_ip = GetServerIp();
            local_ip = GetCurrentIp();
            LOG_INFO("main: client stoped. local_ip="<<local_ip.c_str()<<", server_ip="<<server_ip.c_str());
            if(server_ip.compare(local_ip) == 0)
            {
                flag = 1;
            }
            else
            {
                flag = 0;
            }
        }
    }
    return 0;
}

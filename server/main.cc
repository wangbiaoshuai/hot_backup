#include "hot_backup_server.h"

#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>// NOFILE头文件
#include <stdio.h>

#include "log.h"
#include "common_function.h"

using namespace std;
const char* log_config_file = "./log4cplus.properties";
#define SERVER_PORT 8101

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
    signal(SIGCHLD, SIG_IGN);

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

int main(int args, char* argv[])
{
    init_daemon();
    INIT_LOG(log_config_file);
    /*if(args != 3)
    {
        LOG_WARN("Usage: ./server ip port");
        return -1;
    }

    string ip = argv[1];
    int port = atoi(argv[2]);*/

    string ip = GetCurrentIp();
    int port = SERVER_PORT;

    if(ip.empty() || port <= 0)
    {
        LOG_ERROR("main: ip/port is error.");
        return -1;
    }
    LOG_INFO("main: server_ip="<<ip.c_str()<<", server_port="<<port);
    printf("main: server_ip=%s, server_port=%d\n", ip.c_str(), port);
    fflush(NULL);

    HotBackupServer server(ip, port);
    server.Start();
    return 0;
}

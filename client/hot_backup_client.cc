#include "hot_backup_client.h"

#include "log.h"
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "common_function.h"
using namespace std;

#define CHECK_TIME 5

HotBackupClient::HotBackupClient(const std::string& local_ip, int local_port, const std::string& server_ip, int server_port):
local_ip_(local_ip),
local_port_(local_port),
server_ip_(server_ip),
server_port_(server_port),
timeout_count_(0)
{
}

HotBackupClient::~HotBackupClient()
{
}

int HotBackupClient::Start()
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
    {
        LOG_ERROR("Start: create socket error("<<strerror(errno)<<").");
        return -1;
    }

    struct sockaddr_in my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(local_port_);
    my_addr.sin_addr.s_addr = inet_addr(local_ip_.c_str());

    int bind_flag = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &bind_flag, sizeof(bind_flag)) < 0)
    {
        //设置地址可重用
        LOG_ERROR("Start: setsockopt SO_REUSEADDR error("<<strerror(errno)<<").");
        close(sock);
        return -1;
    }

    int res = bind(sock, (struct sockaddr*)&my_addr, sizeof(my_addr));
    if(res < 0)
    {
        LOG_ERROR("Start: bind error("<<strerror(errno)<<").");
        close(sock);
        return -1;
    }

    struct timeval tv;
    tv.tv_sec = CHECK_TIME;
    tv.tv_usec = 0;
    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        //设置消息接收超时
        LOG_ERROR("Start: setsockopt error("<<strerror(errno)<<").");
        close(sock);
        return -1;
    }

    const char* data = "heart-beat";
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port_);
    server_addr.sin_addr.s_addr = inet_addr(server_ip_.c_str());

    while(1)
    {
        res = sendto(sock, data, strlen(data), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(res < 0)
        {
            LOG_ERROR("Start: heart beat send error("<<strerror(errno)<<"). Sleep 1s...");
            sleep(1);
            continue;
        }

        char buf[512] = {0};
        int len = sizeof(server_addr);
        res = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr*)&server_addr, (socklen_t*)&len);
        if(res < 0)
        {
            timeout_count_++;
            if(timeout_count_ <= 6)
            {
                LOG_ERROR("Start: heart beat recv error("<<strerror(errno)<<"). Sleep "<<CHECK_TIME<<"s...");
                sleep(CHECK_TIME);
                continue;
            }
            LOG_WARN("Start: heart beat failed, start change server...");
            ChangeServer();
            break;
        }

        timeout_count_ = 0;
        res = ParsePackage(buf);
        if(res == 0) //正常心跳 
        {
            LOG_DEBUG("heart beat...");
            sleep(CHECK_TIME);
            continue; 
        }
        else if(res == 1) //切换服务器
        {
            LOG_WARN("Start: recv "<<buf<<", start change server...");
            ChangeServer();
            break;
        }
        else
        {
            LOG_WARN("Start: recv unknow message("<<buf<<"). Sleep "<<CHECK_TIME<<"s...");
            sleep(CHECK_TIME);
            continue;
        }
    }
    close(sock);
    LOG_INFO("Hot backup client stoped!");
    return 0;
}

int HotBackupClient::ParsePackage(const char* msg)
{
    if(strcmp(msg, "heart-beat") == 0)
    {
        return 0;
    }
    else if(strcmp(msg, "switch-server") == 0)
    {
        return 1;
    }
    else
        return -1;
}

int HotBackupClient::ChangeServer()
{
    LOG_INFO("ChangeServer: begin.");
    string path = GetCurrentPath();
    string cmd = path + "/switch_server.sh " + server_ip_ + " >>/tmp/hot_backup_client.stdout 2>&1";
/*    FILE* fp = popen(cmd.c_str(), "r");
    if(fp == NULL)
    {
        LOG_ERROR("ChangeServer: popen error("<<strerror(errno)<<").");
        return -1;
    }

    int status = pclose(fp);*/
    int status = system(cmd.c_str());
    if(WIFEXITED(status) != 0)
    {
        if(WEXITSTATUS(status) != 0)
        {
            LOG_ERROR("ChangeServer: command("<<cmd.c_str()<<") exec failed, exit("<<WEXITSTATUS(status)<<").");
            return -1;
        }
        else
        {
            LOG_INFO("ChangeServer: command("<<cmd.c_str()<<") exec success!");
            return 0;
        }
    }
    else
    {
        LOG_ERROR("ChangeServer: command("<<cmd.c_str()<<") exit error.");
        return -1;
    }
}

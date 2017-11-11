#include "hot_backup_server.h"
#include "log.h"

#include <pthread.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mysql/mysql.h>
#include <stdio.h>
#include <stdlib.h>

#include "common_function.h"
#include "hot_backup_client.h"

#define MAX_COUNT 6
#define CHECK_TIME 30

using namespace std;

HotBackupServer::HotBackupServer(const string& ip, int port):
server_ip_(""),
server_port_(-1),
abnormal_count_(0),
flag_switch_(false),
stop_heart_beat_(false)
{
    ip.empty()? "" : server_ip_ = ip;
    (port <= 0)? 0 : server_port_ = port;
}

HotBackupServer::~HotBackupServer()
{
}

void HotBackupServer::Start()
{
    LOG_INFO("Hot backup server Start!");
    while(1)
    {
        if(StartHeartBeat() == 0)
        {
            break;
        }
        else
        {
            LOG_WARN("Start: start hear beat failed. Sleep 3s...");
            sleep(3);
        }
    }

    while(1)
    {
        int res1 = CheckDatabaseStatus();
        int res2 = CheckMonitorStatus();
        if(res1 == 0 && res2 == 0)
        {
            LOG_INFO("All is well. Sleep "<<CHECK_TIME<<"s...");
            abnormal_count_ = 0;
            sleep(CHECK_TIME);
        }
        else
        {
            abnormal_count_++;
            if(abnormal_count_ <= MAX_COUNT)
            {
                if(res1 < 0)
                {
                    LOG_WARN("Start: database is abnormal. Restart database...");
                    RestartDatabase();
                }
                if(res2 < 0)
                {
                    LOG_WARN("Start: monitor is abnormal. Restart monitor...");
                    RestartMonitor();
                }
                sleep(CHECK_TIME);
            }
            else
            {
                flag_switch_ = true;
                break;
            }
        }
    }
    StopHeartBeat();

    //StartClient();
    LOG_INFO("Hot backup server stoped!");
    return;
}

void* thread_client(void* context)
{
    HotBackupServer* ctx = (HotBackupServer*)context;
    ctx->StartHotBackupClient();
    pthread_exit(NULL);
}

int HotBackupServer::StartClient()
{
    pthread_t thread;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int res = pthread_create(&thread, &attr, thread_client, this);
    if(res < 0)
    {
        LOG_ERROR("StartClient: create thread error("<<strerror(errno)<<").");
        return -1;
    }

    pthread_attr_destroy(&attr);
    return 0;
}

int HotBackupServer::StartHotBackupClient()
{
    LOG_INFO("StartHotBackupClient begin.");
    string ip_file("/tmp/server_ip");
    FILE* fp = fopen(ip_file.c_str(), "r");
    if(fp == NULL)
    {
        LOG_ERROR("StartHotBackupClient: open file "<<ip_file.c_str()<<" error("<<strerror(errno)<<").");
        return -1;
    }
    char ip[20] = {0};
    fgets(ip, sizeof(ip) - 1, fp);
    fclose(fp);

    string local_ip = GetCurrentIp();
    int port = server_port_;
    
    LOG_INFO("local_ip:"<<local_ip.c_str()<<", server_ip:"<<ip);

    if(strcmp(ip, local_ip.c_str()) == 0)
    {
        Start();
    }
    else
    {
        sleep(5 * 60 * 2);
        HotBackupClient client(ip, port, server_ip_, server_port_);
        client.Start();
    }
    LOG_INFO("StartHotBackupClient end.");
    return 0;
}

void* thread_function(void* context)
{
    HotBackupServer* ctx = static_cast<HotBackupServer*>(context);
    ctx->HeartBeat();
    LOG_INFO("thread_function: heart beat stoped!");
    pthread_exit(NULL);
}

int HotBackupServer::StartHeartBeat()
{
    stop_heart_beat_ = false;
    int res = pthread_create(&heart_beat_thread_, NULL, thread_function, this);
    if(res < 0)
    {
        LOG_ERROR("StartHeartBeat: create thread error("<<strerror(errno)<<").");
        return -1;
    }
    return 0;
}

int HotBackupServer::StopHeartBeat()
{
    if(stop_heart_beat_ == false)
    {
        stop_heart_beat_ = true;
        int res = pthread_join(heart_beat_thread_, NULL);
        if(res < 0)
        {
            LOG_ERROR("StopHeartBeat: stop heart beat error("<<strerror(errno)<<").");
            return -1;
        }
    }
    return 0;
}

void HotBackupServer::HeartBeat()
{
    LOG_INFO("HeartBeat: start.");
    int sock;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
 
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip_.c_str());
    server_addr.sin_port = htons(server_port_);

    //设置地址可重用
    int flag = 1;
    if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
    {
        LOG_ERROR("HeartBeat: setsockopt SO_REUSEADDR error("<<strerror(errno)<<").");
        close(sock);
        return;
    }

    int res = bind(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if(res < 0)
    {
        LOG_ERROR("HeartBeat: bind error("<<strerror(errno)<<").");
        close(sock);
        return;
    }

    struct timeval tv;
    tv.tv_sec = 4 * CHECK_TIME;
    tv.tv_usec = 0;

    //设置数据接收超时
    if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        LOG_ERROR("HeartBeat: setsockopt SO_RCVTIMEO error("<<strerror(errno)<<").");
        close(sock);
        return;
    }

    while(!stop_heart_beat_)
    {
        char data[512] = {0};
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        int len = sizeof(client_addr);
        
        res = recvfrom(sock, data, sizeof(data) - 1, 0, (struct sockaddr*)&client_addr, (socklen_t*)&len);
        if(res < 0)
        {
            LOG_ERROR("HeartBeat: recv message error("<<strerror(errno)<<").");
            continue;
        }
        if(strcmp(data, "heart-beat") != 0)
        {
            LOG_WARN("HeartBeat: recv message error, message="<<data);
            continue;
        }

        if(flag_switch_)
        {
            const char* message = "switch-server";
            res = sendto(sock, message, strlen(message), 0, (struct sockaddr*)&client_addr, len);
            LOG_DEBUG("HeartBeat: switch-server...");
            if(res < 0)
            {
                LOG_ERROR("HeartBeat: send message error("<<strerror(errno)<<").");
                continue;
            }

            if(SwitchIp() < 0)
            {
                LOG_ERROR("HeartBeat: switch IP error.");
            } 
            break;
        }
        else
        {
            res = sendto(sock, data, strlen(data), 0, (struct sockaddr*)&client_addr, len);
            if(res < 0)
            {
                LOG_ERROR("HeartBeat: send message error("<<strerror(errno)<<").");
            }
            
            LOG_DEBUG("HeartBeat: heart-beat...");
            continue;
        }
    }
    close(sock);
    return;
}

int HotBackupServer::CheckDatabaseStatus()
{
    int res = 0;
    MYSQL* con = mysql_init((MYSQL* )0);

    const char* host = "localhost";
    const char* user = "root";
    const char* passwd = "VRV!@#3xa";
    const char* database = "cems";
    unsigned int port = 3306;
    const char* unix_socket = "/tmp/mysql.sock";

    if(con != NULL && mysql_real_connect(con, host, user, passwd, database, port, unix_socket, 0))
    {
        res = 0;
        //LOG_DEBUG("CheckDatabaseStatus: database is normal.");
    }
    else
    {
        res = -1;
        LOG_ERROR("CheckDatabaseStatus: connect mysql error("<<mysql_error(con)<<").");
    }

    mysql_close(con);
    return res;
}

int HotBackupServer::CheckMonitorStatus()
{
    /*string cmd = "pgrep cemsmonitor";
    int status = system(cmd.c_str());
    if(WIFEXITED(status) != 0)
    {
        if(WEXITSTATUS(status) != 0)
        {
            LOG_ERROR("CheckMonitorStatus: cmd("<<cmd.c_str()<<") exec error. Monitor has down.");
            return -1;
        }
    }
    else
    {
        LOG_ERROR("CheckMonitorStatus: cmd("<<cmd.c_str()<<") exit error.");
        return -1;
    }
    return 0;*/
    return ProcIsExist("cemsmonitor");
}

int HotBackupServer::RestartDatabase()
{
    LOG_INFO("RestartDatabase: begin.");
    string cmd = "service mysqld start";
    FILE* fp = popen(cmd.c_str(), "w");
    if(fp == NULL)
    {
        LOG_ERROR("RestartDatabase: popen error("<<strerror(errno)<<").");
        return -1;
    }

    int status = pclose(fp);
    //int status = system(cmd.c_str());
    if(WIFEXITED(status) != 0)
    {
        if(WEXITSTATUS(status) != 0)
        {
            LOG_ERROR("RestartDatabase: command("<<cmd.c_str()<<") exec failed, exit("<<WEXITSTATUS(status)<<").");
            return -1;
        }
        else
        {
            LOG_INFO("RestartDatabase: command("<<cmd.c_str()<<") exec success!");
            return 0;
        }
    }
    else
    {
        LOG_ERROR("RestartDatabase: command("<<cmd.c_str()<<") exit error.");
        return -1;
    }
}

int HotBackupServer::RestartMonitor()
{
    LOG_INFO("RestartMonitor: begin.");
    string cmd = "service CEMS-SERVICE-MONITOR start";
    FILE* fp = popen(cmd.c_str(), "w");
    if(fp == NULL)
    {
        LOG_ERROR("RestartMonitor: popen error("<<strerror(errno)<<").");
        return -1;
    }

    int status = pclose(fp);
    //int status = system(cmd.c_str());
    if(WIFEXITED(status) != 0)
    {
        if(WEXITSTATUS(status) != 0)
        {
            LOG_ERROR("RestartMonitor: command("<<cmd.c_str()<<") exec failed, exit("<<WEXITSTATUS(status)<<").");
            return -1;
        }
        else
        {
            LOG_INFO("RestartMonitor: command("<<cmd.c_str()<<") exec success!");
            return 0;
        }
    }
    else
    {
        LOG_ERROR("RestartMonitor: command("<<cmd.c_str()<<") exit error.");
        return -1;
    }
}

int HotBackupServer::SwitchIp()
{
    LOG_INFO("SwitchIp: begin.");
    string path = GetCurrentPath();
    string cmd = path + "/switch_ip.sh";
    FILE* fp = popen(cmd.c_str(), "w");
    if(fp == NULL)
    {
        LOG_ERROR("SwitchIp: popen error("<<strerror(errno)<<").");
        return -1;
    }

    int status = pclose(fp);
    //int status = system(cmd.c_str());
    if(WIFEXITED(status) != 0)
    {
        if(WEXITSTATUS(status) != 0)
        {
            LOG_ERROR("SwitchIp: command("<<cmd.c_str()<<") exec failed, exit("<<WEXITSTATUS(status)<<").");
            return -1;
        }
        else
        {
            LOG_INFO("SwitchIp: command("<<cmd.c_str()<<") exec success!");
            return 0;
        }
    }
    else
    {
        LOG_ERROR("SwitchIp: command("<<cmd.c_str()<<") exit error.");
        return -1;
    }
}

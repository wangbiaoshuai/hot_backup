#include "common_function.h"
#include "log.h"

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

#define MAXINTERFACES 16

std::string GetCurrentIp()
{
    std::string szIp;
    int sock_fd;
    struct ifreq buf[MAXINTERFACES];
    struct ifconf ifc;
    int interface_num;

    if((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        LOG_ERROR("GetCurrentIp: create socket failed");
        return "";
    }

    ifc.ifc_len = sizeof(buf);
    ifc.ifc_req = buf;
    if(ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc) < 0)
    {
        LOG_ERROR("GetCurrentIp: get a list of interface addresses failed.");
        close(sock_fd);
        return "";
    }

    interface_num = ifc.ifc_len / sizeof(struct ifreq);
    LOG_DEBUG("GetCurrentIp: the number of interfaces is "<<interface_num);

    while(interface_num--)
    {
        LOG_DEBUG("GetCurrentIp: net device--"<<buf[interface_num].ifr_name);

        if(ioctl(sock_fd, SIOCGIFFLAGS, (char *)&buf[interface_num]) < 0)
        {
            LOG_ERROR("GetCurrentIp: get the active flag word of the device.");
            continue;;
        }

        if(buf[interface_num].ifr_flags & IFF_PROMISC)
        {
            LOG_DEBUG("GetCurrentIp: interface is in promiscuous mode");
        }

        if(buf[interface_num].ifr_flags & IFF_UP)
        {
            LOG_DEBUG("GetCurrentIp: interface is running.");
        }
        else
        {
            LOG_DEBUG("GetCurrentIp: interface is not running.");
            continue;
        }

        if(ioctl(sock_fd, SIOCGIFADDR, (char *)&buf[interface_num]) < 0)
        {
            LOG_ERROR("GetCurrentIp: get interface address failed.");
            continue;
        }

        szIp = inet_ntoa(((struct sockaddr_in*)(&buf[interface_num].ifr_addr))->sin_addr);

        printf("GetCurrentIp: %s\n", szIp.c_str());
        close(sock_fd);
        return szIp;
    }

    close(sock_fd);
    return szIp;
}

std::string GetCurrentPath()
{
    char buf[255] = {0};
    getcwd(buf, sizeof(buf));
    std::string path(buf);
    return path;
}

#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <string>
using namespace std;

const char* f_cmdline = "cmdline";
char* proc_name = NULL;
#define PROC_DIR "/proc"

int GrepFile(const char* file_name, const char* str)
{
    FILE* fp = fopen(file_name, "r");
    if(fp == NULL)
    {
        printf("open file error: %s\n", strerror(errno));
        return -1;
    }

    while(!feof(fp))
    {
        char line[1024] = {0};
        fgets(line, sizeof(line), fp);
        /*if(feof(fp))
        {
            break;
        }*/
        string tmp = line;
        if(tmp.find(str) != string::npos)
        {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    return -1;
}

int printdir(char *dir, int depth)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    if ((dp = opendir(dir)) == NULL) 
    {
        fprintf(stderr, "Can`t open directory %s\n", dir);
        return -1;
    }
    
    chdir(dir);
    while ((entry = readdir(dp)) != NULL) 
    {
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) 
        {
            if (strcmp(entry->d_name, ".") == 0 || 
                strcmp(entry->d_name, "..") == 0 )  
                continue;   
            //printf("%*s%s/\n", depth, "", entry->d_name);
            if(printdir(entry->d_name, depth+4) == 0)
            {
                chdir("..");
                closedir(dp);
                return 0;
            }
        } 
        else
        {
            //printf("d_name: %s\n", entry->d_name);
            if(strcmp(f_cmdline, entry->d_name) == 0)
            {
                if(GrepFile(f_cmdline, proc_name) == 0)
                {
                    chdir("..");
                    closedir(dp);
                    return 0;
                }
            }
            //printf("%*s%s\n", depth, "", entry->d_name);
        }
    }
    chdir("..");
    closedir(dp);
    return -1;
}

int ProcIsExist(const std::string& proc)
{
    proc_name = (char*)proc.c_str();
    if(printdir(PROC_DIR, 0) == 0)
    {
        return 0;
    }
    return -1;
}

/*
int main(int argc, char *argv[])
{
    char *topdir = ".";
    if (argc >= 2)
        topdir = argv[1];

    printf("Directory scan of %s\n", topdir);
    if(printdir(topdir, 0) == 0)
    {
        printf("proc %s is exit\n", proc_name);
    }
    else
    {
        printf("proc %s is not exit.\n", proc_name);
    }
    printf("done.\n");
    exit(0);
}*/

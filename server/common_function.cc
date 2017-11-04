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

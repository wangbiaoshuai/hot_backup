#!/bin/sh

#1.停止数据库
#2.修改本机IP
#3.修改cems所有服务的配置IP
#4.启动cems

server_ip="$1"
install_dir="/usr/local/service"
address_install_dir="$install_dir/CEMS-SERVICE-ADDRESS"
fastdfs_conf_file="/etc/fdfs/storage.conf"
elasticsearch_conf_file="/usr/local/elasticsearch-2.1.1/config/elasticsearch.yml"

function check_result()
{
    if [ "$1" != "0" ]
    then
        echo "result error"
        exit 1
    fi
}

function stop_mysql()
{
    echo "begin stop mysql..."
    service mysqld stop
    check_result $?
}

function switch_ip()
{
    echo "begin switch to $server_ip..."
    ping $server_ip -c2
    if [ "$?" = "0" ]
    then
        sleep 10
    fi

    local_ip=$(ifconfig eth0 | egrep -o "inet addr:[^ ]*" | grep -o "[0-9.]*")
    echo "switch $local_ip----->$server_ip"
    sed -i "s/^IPADDR.*/IPADDR=$server_ip/g" /etc/sysconfig/network-scripts/ifcfg-eth0
    service network restart
    check_result $?
}

function fix_services_ip()
{
    echo "begin fix services' ip...."
    conf_files=$(find $install_dir -name config.properties)
    for file in "$conf_files"
    do
        sed -i "s/^server\.ip.*/server\.ip=$server_ip/g" $file
        sed -i "s/^service\.ip.*/service\.ip=$server_ip/g" $file
    done
    check_result $?

    echo "begin fix fastdfs's ip..."
    sed -i "s/^tracker_server.*:/tracker_server=$server_ip:/g" $fastdfs_conf_file
    fdfs_files=$(find $install_dir -name fastdfs.properties)
    for file in "$fdfs_files"
    do
        sed -i "s/^tracker_server.*:/tracker_server=$server_ip:/g" $file
        sed -i "s/^es\.ip.*/es\.ip=$server_ip/g" $file
    done
    check_result $?

    echo "begin fix jdbc's ip..."
    jdbc_files=$(find $install_dir -name jdbc.properties)
    for file in "$jdbc_files"
    do
        sed -i "s/^url=jdbc:mysql:\/\/.*:/url=jdbc:mysql:\/\/$server_ip:/g" $file
    done
    check_result $?

    echo "begin fix ftp's ip..."
    ftp_files=$(find $install_dir -name ftp.properties)
    for file in "$ftp_files"
    do
        sed -i "s/^ftp\.ip.*/ftp\.ip=$server_ip/g" $file
    done
    check_result $?

    echo "begin fix polixy's ip..."
    policy_files=$(find $address_install_dir -name policy.xml)
    for file in "$policy_files"
    do
        \sed -i "s/<ip>.*.<\/ip>/<ip>$server_ip<\/ip>/g" $file
        \sed -i "s/loadbalance:\/\/.*.:3306/loadbalance:\/\/$server_ip:3306/g" $file
        \sed -i "s/mysql:\/\/.*.:3306/mysql:\/\/$server_ip:3306/g" $file
        \sed -i "s/<tracker_server>.*.:22122/<tracker_server>$server_ip:22122/g" $file
        \sed -i "s/<es.host>.*/<es.host>$server_ip<\/es.host>/g" $file
    done
    check_result $?

    echo "begin fix elasticsearch's ip..."
    sed -i "s/network.host:.*/network.host: $server_ip/g" $elasticsearch_conf_file
}

function main()
{
    if [ "$#" != "1" ]
    then
        echo "Usage: $0 server_ip"
        exit 1
    fi

    if [ -f "/etc/init.d/CEMS_AUTO_START" ]
    then
        pids=$(pgrep cems)
        if [ "$pids" != "" ]
        then
            service CEMS_AUTO_START stop
        fi
    fi
    stop_mysql
    switch_ip
    fix_services_ip
    echo "config success! begin start cems..."
    service CEMS_AUTO_START start
}

main $@
exit 0

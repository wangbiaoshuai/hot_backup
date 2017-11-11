#!/bin/sh

other_ip=$1

function check_result()
{
    if [ "$1" != "0" ]
    then
        echo "result error"
        exit 1
    fi
}

function auto_switch_ip()
{
    echo "begin auto switch ip."
    local_ip=$(ifconfig eth0 | egrep -o "inet addr:[^ ]*" | grep -o "[0-9.]*")
    sub_ip=${local_ip%.*}
    remaind_ip=${local_ip##*.}

    for tmp in {2..255}
    do
        ip=$sub_ip.$tmp
        ping $ip -c2
        if [ "$?" != 0 ]
        then
            remaind_ip=$tmp
            break;
        fi
    done

    if [ "$remaind_ip" = "255" ]
    then
        echo "error: no use ip, exit."
        exit 1
    fi

    other_ip="$sub_ip.$remaind_ip"
    echo "switch $local_ip -------> $other_ip"
    echo -n $local_ip > /tmp/server_ip
    sed -i "s/^IPADDR.*/IPADDR=$other_ip/g" /etc/sysconfig/network-scripts/ifcfg-eth0
    service network stop
    sleep 3
    service network start
    check_result $?
}

function switch_ip()
{
    local other_ip="$1"
    echo "begin switch to $other_ip..."
    ping $other_ip -c2
    if [ "$?" = "0" ]
    then
        sleep 10
    fi

    local_ip=$(ifconfig eth0 | egrep -o "inet addr:[^ ]*" | grep -o "[0-9.]*")
    echo "switch $local_ip----->$other_ip"
    echo -n $local_ip > /tmp/server_ip
    sed -i "s/^IPADDR.*/IPADDR=$other_ip/g" /etc/sysconfig/network-scripts/ifcfg-eth0
    service network stop
    sleep 3
    service network start
    check_result $?
}

function main()
{
    if [ "$#" = "0" ]
    then
        auto_switch_ip
    else
        switch_ip $1
    fi
}

main $@
echo "$0 exec success!"
exit 0

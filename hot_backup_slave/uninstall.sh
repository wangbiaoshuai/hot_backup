#!/bin/sh

install_dir="/usr/local"
DEAMON_DIR="${install_dir}/hot_backup_slave"
proc_name="hot_backup_slave"

ps axf | grep ${proc_name} | grep -v grep | awk '{print $1}' | xargs -t -I{} kill -9 {}

pids=`ps axf | grep ${proc_name} | grep -v grep | awk '{print $1}'`
if [ "$pids" != "" ]
then
    echo "stop ${proc_name} failed!"
    exit 1
fi

rm -rf ${DEAMON_DIR}
echo "uninstall ${proc_name} success!"
exit 0

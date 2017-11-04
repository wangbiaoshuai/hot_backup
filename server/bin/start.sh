#!/bin/sh

install_dir="/usr/local"
DEAMON_DIR="${install_dir}/hot_backup_server"
proc_name="hot_backup_server"
DEAMON="${DEAMON_DIR}/${proc_name}"

ps axf | grep ${proc_name} | grep -v grep | awk '{print $1}' | xargs -t -I{} kill -9 {}
cd ${DEAMON_DIR} && chmod +x switch_ip.sh && cd -
cd ${DEAMON_DIR} && ${DEAMON} && cd -

pid=`ps axf | grep ${proc_name} | grep -v grep | awk '{print $1}'`
if [ "${pid}" = "" ]
then
    echo "${proc_name} start failed."
    exit 1
fi

echo "${proc_name} start success."
exit 0

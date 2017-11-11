#!/bin/sh

if [ "$#" != "1" ]
then
    echo "Usage: install.sh server_ip"
    exit 1
fi

install_dir="/usr/local"
DEAMON_DIR="${install_dir}/hot_backup_client"
proc_name="hot_backup_client"
DEAMON="${DEAMON_DIR}/${proc_name}"
server_ip=$1
server_port=8101

sh uninstall.sh

mkdir -p ${install_dir}
cp -r ./bin ${DEAMON_DIR}

cd ${DEAMON_DIR} && chmod +x switch_server.sh && cd -

ps axf | grep ${proc_name} | grep -v grep | awk '{print $1}' | xargs -t -I{} kill -9 {}
cd ${DEAMON_DIR} && ${DEAMON} client ${server_ip} ${server_port} && cd -

sleep 1
pid=`ps axf | grep ${proc_name} | grep -v grep | awk '{print $1}'`
if [ "${pid}" = "" ]
then
    echo "${proc_name} start failed."
    exit 1
fi

echo "${proc_name} start success."
exit 0

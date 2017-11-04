#!/bin/sh

install_dir="/usr/local"
DEAMON_DIR="${install_dir}/hot_backup_client"
proc_name="hot_backup_client"
DEAMON="${DEAMON_DIR}/${proc_name}"
server_ip="$1"
server_port=8101

mkdir -p ${install_dir}
cp -r ./bin ${DEAMON_DIR}

cd ${DEAMON_DIR} && chmod +x switch_ip.sh && cd -

pgrep ${proc_name} | xargs -t -I{} kill -9 {}
${DEAMON} $server_ip $server_port

pid=`pgrep ${proc_name}`
if [ "${pid}" = "" ]
then
    echo "${proc_name} start failed."
    exit 1
fi

echo "${proc_name} start success."
exit 0

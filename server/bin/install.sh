#!/bin/sh

install_dir="/usr/local"
DEAMON_DIR="${install_dir}/hot_backup_server"
proc_name="hot_backup_server"
DEAMON="${DEAMON_DIR}/${proc_name}"

mkdir -p ${install_dir}
cp -r ./bin ${DEAMON_DIR}

cd ${DEAMON_DIR} && chmod +x switch_ip.sh && cd -

pgrep ${proc_name} | xargs -t -I{} kill -9 {}
${DEAMON}

pid=`pgrep ${proc_name}`
if [ "${pid}" = "" ]
then
    echo "${proc_name} start failed."
    exit 1
fi

echo "${proc_name} start success."
exit 0

#!/bin/sh

install_dir="/usr/local"
DEAMON_DIR="${install_dir}/hot_backup_master"
proc_name="hot_backup_master"
DEAMON="${DEAMON_DIR}/${proc_name}"

ps axf | grep ${proc_name} | grep -v grep | awk '{print $1}' | xargs -t -I{} kill -9 {}
mkdir -p ${install_dir}
cp -r ./bin ${DEAMON_DIR}

cd ${DEAMON_DIR} && chmod +x switch_ip.sh && cd -
cd ${DEAMON_DIR} && ${DEAMON} server && cd -

pid=`ps axf | grep ${proc_name} | grep -v grep | awk '{print $1}'`
if [ "${pid}" = "" ]
then
    echo "${proc_name} start failed."
    exit 1
fi

echo "${proc_name} start success."
exit 0

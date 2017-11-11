#!/bin/sh

proc_name="hot_backup_master"

ps axf | grep ${proc_name} | grep -v grep | awk '{print $1}' | xargs -t -I{} kill -9 {}

pids=`ps axf | grep ${proc_name} | grep -v grep | awk '{print $1}'`
if [ "$pids" != "" ]
then
    echo "stop ${proc_name} failed!"
    exit 1
fi

echo "stop ${proc_name} success!"
exit 0

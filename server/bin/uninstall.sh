#!/bin/sh

install_dir="/usr/local"
DEAMON_DIR="${install_dir}/hot_backup_server"
proc_name="hot_backup_server"

pgrep ${proc_name} | xargs -t -I{} kill -9 {}

rm -rf ${DEAMON_DIR}

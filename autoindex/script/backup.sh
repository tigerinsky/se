#!/bin/bash
. ./conf/conf.sh

#find . -maxdepth 1 -mindepth 1 -type d -ctime +2 
find ${INDEX_DIR} -maxdepth 1 -mindepth 1 -type d | sort -r | awk '{if(NR>5) print $0;}' | xargs rm -rf  
rm -rf ${WORK_BAK_DIR}
mv ${WORK_DIR} ${WORK_BAK_DIR}

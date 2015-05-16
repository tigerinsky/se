#!/bin/bash
. ./conf/conf.sh

if [ -e ${FLAG} ]
then
    date
    echo "cancel autoindex, flag exists"
    exit 1 
fi

date
echo "start autoindex"
touch ${FLAG}

index_path=${INDEX_DIR}/`date +%s`
sh ${SCRIPT_DIR}/get_data.sh || exit 1
sh ${SCRIPT_DIR}/build_index.sh ${index_path} || exit 1
sh ${SCRIPT_DIR}/trans_index.sh ${index_path} || exit 1
sh ${SCRIPT_DIR}/backup.sh

date
echo "end autoindex"
rm -rf ${FLAG}


#!/bin/bash
. ./conf/conf.sh

if [ $# -lt 1 ]
then
    date
    echo "transindex: missing output path"
    exit 1
fi

index_path=$1
date
echo "start trans index"
echo $index_path > $SE_INDEX_CONF
trans_flag=false
for ((i=0;i<10;++i))
do
    sleep 5
    current_path=`cat ${SE_VERSION_FILE}` 
    date
    echo "trans_index: current_path[$current_path] index_path[$index_path]"
    if [ $current_path == $index_path ]
    then
        trans_flag=true
        break
    fi
done
if $trans_flag
then
    date
    echo "transindex finish, se have already trans new index"
else
    date
    echo "transindex timeout"
    exit 1
fi

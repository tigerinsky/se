#!/bin/bash
. ./conf/conf.sh

function check_size() {
    local file=$1
    local check_size=$2 #单位k
    file_size=`ls -s ${file} | cut -d ' ' -f 1`
    if [ ${file_size} -lt ${check_size} ]
    then
        return 1
    fi
    return 0
}

function dump_data() {
    date
    echo "dump $2" 
    ${MYSQL} -h${DB_HOST} -P${DB_PORT} -u${DB_USER} -p${DB_PWD} ${DB_ARGS} < $1 > $2 
    if [ $? -ne 0 ]
    then
        echo "dump $2 error"
        return 1
    fi
    if [ $# -gt 2 ]
    then
        check_size $2 $3
        if [ $? -ne 0 ]
        then
            echo "$2 size too small"
            return 1
        fi
    fi
}

dump_data ${SQL_DIR}/get_all_tweet.sql ${TWEET} || exit 1
dump_data ${SQL_DIR}/get_all_zan.sql ${ZAN} || exit 1
dump_data ${SQL_DIR}/get_all_comment.sql ${COMMENT} || exit 1

#!/bin/bash
. ./conf/conf.sh

if [ ! -d ${WORK_DIR} ]
then
    rm -rf ${WORK_DIR}
    mkdir -p ${WORK_DIR}
fi

date
echo 'start dump data from mysql'
sh ${SCRIPT_DIR}/dump_data.sh
if [ $? -ne 0 ]
then
    date
    echo 'dump date error'
    exit 1
fi

date
echo 'start merge data'
${PYTHON} ${SCRIPT_DIR}/merge_data.py \
    ${RAW} \
    ${TWEET} \
    ${ZAN} \
    ${COMMENT} \
    ${RESOURCE}
if [ $? -ne 0 ]
then
    date
    echo 'merge date error'
    exit 1
fi

#!/bin/bash
. ./conf/conf.sh
. ./script/func.sh

output_file="${FINALLY}_`date +%F`"

if [ ! -d ${WORK_DIR} ]
then
    rm -rf ${WORK_DIR}
    mkdir -p ${WORK_DIR}
fi

if [ ! -d ${OUTPUT_DIR} ]
then
    rm -rf ${OUTPUT_DIR}
    mkdir -p ${OUTPUT_DIR}
fi

date
echo 'start dump data from mysql'
sh ${SCRIPT_DIR}/dump_data.sh
if [ $? -ne 0 ]
then
    date
    echo 'dump date error'
    #send_warning_message ${TRAGEDY_LIST_FILE} 't10: dump data error'
    exit 1
fi

date
echo 'start merge data'
${PYTHON} ${SCRIPT_DIR}/merge_data.py \
    ${output_file} \
    ${TWEET} \
    ${ZAN} \
    ${COMMENT} \
    ${CATALOG} \
    ${TAG}
if [ $? -ne 0 ]
then
    date
    echo 'merge date error'
    #send_warning_message ${TRAGEDY_LIST_FIEL} 't10: merge data error'
    exit 1
fi

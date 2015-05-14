#!/bin/bash
. ./conf/conf.sh

sh ${SCRIPT_DIR}/get_data.sh >> ./log/get_data.log 2>&1 || exit 1

#下面是灌数据
#sh ${SCRIPT_DIR}/import_data.sh >> ./log/brand_poi.log 2>&1 || exit 1

#!/bin/bash
. ./conf/conf.sh

if [ $# -lt 1 ]
then
    date
    echo "makeindex: missing output path"
    exit 1
fi

output=$1

if [ ! -d $output ]
then
    rm -rf $output
    mkdir -p $output
fi

date
echo "start makeindex, index[${output}]" 
cd ${MAKEINDEX_DIR}
${BIN_DIR}/makeindex \
    -input=${RAW} \
    -output=$output \
    -field_conf=./conf/field.conf \
    -segment_dict=./conf/seg/utf8/ \
    -catalog_dict=./conf/catalog.conf \
    -tag_dict=./conf/tag.conf \
    -log_dir=./log/ \
    -stderrthreshold=4 
if [ $? -ne 0 ]
then
    date
    echo 'makeindex error'
    exit 1
fi
cd -
date
echo "make index finish, new index[$output]"

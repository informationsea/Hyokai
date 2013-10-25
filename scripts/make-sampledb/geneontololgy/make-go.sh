#!/bin/sh

if [ ! -e go_daily-termdb-tables.tar.gz ];then
    curl -O http://archive.geneontology.org/latest-termdb/go_daily-termdb-tables.tar.gz
fi




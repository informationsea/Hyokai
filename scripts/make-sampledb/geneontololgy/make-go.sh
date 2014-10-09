#!/bin/sh

if [ ! -e go.obo ];then
    curl -L -O http://purl.obolibrary.org/obo/go.obo
fi

python import-go.py




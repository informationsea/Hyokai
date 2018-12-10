#!/bin/sh

source setenv.sh

if [ -z "${CODE_SIGN_IDENTITY}" ]; then
    echo "CODE_SIGN_IDENTITY is not found"
    exit
fi

if [ -z "${QTPATH}" ]; then
    echo "QTPATH is not found"
    exit
fi

APPNAME=Hyokai.app

#find ${APPNAME} -type f -perm 755 -exec codesign --force --verify --verbose -s "$CODE_SIGN_IDENTITY" '{}' ';'
#find ${APPNAME} -name '*.framework' -exec codesign --force --verify --verbose -s "$CODE_SIGN_IDENTITY" '{}' ';'
#codesign --force --verify --verbose -s "$CODE_SIGN_IDENTITY" ${APPNAME}

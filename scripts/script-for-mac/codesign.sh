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

for i in `find ${APPNAME}/Contents/Frameworks -name '*.framework'`; do
    BASENAME="${i##*/}"

    pushd $i
    rmdir Resources
    mkdir -p Versions/5/Resources
    ln -s Versions/5/Resources .
    pushd Versions
    ln -s 5 Current
    popd
    cp ${QTPATH}/lib/$BASENAME/Contents/Info.plist Versions/5/Resources
    popd
    
done

find ${APPNAME} -type f -perm 755 -exec codesign --force --verify --verbose -s "$CODE_SIGN_IDENTITY" '{}' ';'
find ${APPNAME} -name '*.framework' -exec codesign --force --verify --verbose -s "$CODE_SIGN_IDENTITY" '{}' ';'
codesign --force --verify --verbose -s "$CODE_SIGN_IDENTITY" ${APPNAME}

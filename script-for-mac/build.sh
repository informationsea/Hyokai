#!/bin/sh

if [ -f Makefile ];then
    make distclean
fi
qmake -spec macx-g++ -config release ..
make
macdeployqt TableView.app

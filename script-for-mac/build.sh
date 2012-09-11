#!/bin/sh

if [ -f Makefile ];then
    make distclean
fi
qmake -spec macx-g++ -config release ..
make
#macdeployqt TableView.app # for dynamic link library
strip TableView.app/Contents/MacOS/TableView # for static link library
cp -R qt_menu.nib TableView.app/Contents/Resources
cp Info.plist TableView.app/Contents/


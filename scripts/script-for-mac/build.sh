#!/bin/sh

if [ -f Makefile ];then
    make distclean
fi
qmake -spec macx-g++ -config release ../../src
make -j5
macdeployqt TableView.app # for dynamic link library
#strip TableView.app/Contents/MacOS/TableView # for static link library
#cp -R qt_menu.nib TableView.app/Contents/Resources
cp Info.plist TableView.app/Contents/
cp ../../src/images/fileicon/sqlite.icns TableView.app/Contents/Resources
cp ../../src/images/fileicon/table.icns TableView.app/Contents/Resources

if [ -f ./codesign.sh ];then
    sh ./codesign.sh
fi

rm TableView-osx.zip
7za a TableView-osx.zip TableView.app


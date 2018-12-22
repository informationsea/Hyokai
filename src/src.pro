#-------------------------------------------------
#
# Project created by QtCreator 2012-08-16T22:27:16
#
#-------------------------------------------------

QT       += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Hyokai
TEMPLATE = app
INCLUDEPATH += filereader/src zlib

ICON = images/icon.icns
RC_FILE = windowsicon.rc

include(QtSimplePlot/libsrc/SimplePlot.pri)

SOURCES += main.cpp\
        mainwindow.cpp \
    schemadialog.cpp \
    sheetmessagebox.cpp \
    schematablemodel.cpp \
    sqltablemodelalternativebackground.cpp \
    sqlquerymodelalternativebackground.cpp \
    attachdatabasedialog.cpp \
    jointabledialog.cpp \
    preferencewindow.cpp \
    fileeventhandler.cpp \
    imageview.cpp \
    sqltextedit.cpp \
    databaseconnectiondialog.cpp \
    sheettextinputdialog.cpp \
    sqlservice.cpp \
    sqlplotchart.cpp \
    customsqldialog.cpp \
    sqlfileimporter.cpp \
    sqlfileexporter.cpp \
    checkboxitemdelegate.cpp \
    sqldatatypeitemdelegate.cpp \
    filereader/src/tablereader.cpp \
    filereader/src/filereader.cpp \
    filereader/src/filereader_stdio.cpp \
    filereader/src/filereader_mmap.cpp \
    filereader/src/filereader_core.cpp \
    filereader/src/csvreader.cpp \
    tableviewstyleditemdelegate.cpp \
    filereader/src/filereader_gzip.cpp \
    filetype.cpp \
    zlib/adler32.c \
    zlib/compress.c \
    zlib/crc32.c \
    zlib/deflate.c \
    zlib/gzclose.c \
    zlib/gzlib.c \
    zlib/gzread.c \
    zlib/gzwrite.c \
    zlib/infback.c \
    zlib/inffast.c \
    zlib/inflate.c \
    zlib/inftrees.c \
    zlib/trees.c \
    zlib/uncompr.c \
    zlib/zutil.c \
    sqlhistoryhelper.cpp \
    sqlasynchronousexecutor.cpp \
    summarydialog2.cpp \
    sqlitemcounttablemodel.cpp \
    tableviewstatus.cpp \
    addfilterdialog.cpp

HEADERS  += mainwindow.h \
    main.h \
    schemadialog.h \
    sheetmessagebox.h \
    schematablemodel.h \
    sqltablemodelalternativebackground.h \
    sqlquerymodelalternativebackground.h \
    attachdatabasedialog.h \
    jointabledialog.h \
    preferencewindow.h \
    fileeventhandler.h \
    imageview.h \
    sqltextedit.h \
    databaseconnectiondialog.h \
    sheettextinputdialog.h \
    sqlservice.h \
    sqlplotchart.h \
    customsqldialog.h \
    sqlfileimporter.h \
    sqlfileexporter.h \
    checkboxitemdelegate.h \
    sqldatatypeitemdelegate.h \
    filereader/src/tablereader.hpp \
    filereader/src/filereader.hpp \
    filereader/src/filereader_stdio.hpp \
    filereader/src/filereader_mmap.hpp \
    filereader/src/filereader_core.hpp \
    filereader/src/csvreader.hpp \
    hyokaiconfig.h \
    tableviewstyleditemdelegate.h \
    filereader/src/filereader_gzip.hpp \
    filetype.h \
    zlib/crc32.h \
    zlib/deflate.h \
    zlib/gzguts.h \
    zlib/inffast.h \
    zlib/inffixed.h \
    zlib/inflate.h \
    zlib/inftrees.h \
    zlib/trees.h \
    zlib/zconf.h \
    zlib/zlib.h \
    zlib/zutil.h \
    filereader/src/growbuffer.hpp \
    sqlhistoryhelper.h \
    sqlasynchronousexecutor.h \
    summarydialog2.h \
    sqlitemcounttablemodel.h \
    tableviewstatus.h \
    addfilterdialog.h

FORMS    += mainwindow.ui \
    schemadialog.ui \
    attachdatabasedialog.ui \
    jointabledialog.ui \
    preferencewindow.ui \
    databaseconnectiondialog.ui \
    sheettextinputdialog.ui \
    sqlplotchart.ui \
    customsqldialog.ui \
    summarydialog2.ui \
    addfilterdialog.ui

RESOURCES += \
    resouces.qrc

OTHER_FILES += \
    windowsicon.rc \
    images/fileicon/table.icns \
    images/fileicon/sqlite.ico \
    images/fileicon/sqlite.icns \
    functionlist.txt \
    keywords/sqlite-keywords.txt \
    keywords/sqlite-functions.txt \
    keywords/postgresql-keywords.txt \
    keywords/postgresql-functions.txt \
    keywords/mysql-keywords.txt \
    keywords/mysql-functions.txt

QMAKE_INFO_PLIST = Info.plist

greaterThan(QT_MAJOR_VERSION, 4): cache()

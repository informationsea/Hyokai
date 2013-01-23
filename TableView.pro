#-------------------------------------------------
#
# Project created by QtCreator 2012-08-16T22:27:16
#
#-------------------------------------------------

QT       += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = TableView
TEMPLATE = app
VERSION = 0.2

ICON = images/icon.icns
RC_FILE = windowsicon.rc

SOURCES += main.cpp\
        mainwindow.cpp \
    schemadialog.cpp \
    sheetmessagebox.cpp \
    schematablemodel.cpp \
    custumsql.cpp \
    sqltablemodelalternativebackground.cpp \
    sqlquerymodelalternativebackground.cpp \
    attachdatabasedialog.cpp \
    jointabledialog.cpp \
    preferencewindow.cpp \
    fileeventhandler.cpp \
    summarydialog.cpp \
    imageview.cpp \
    sqltextedit.cpp \
    sqlite3-extension/extension-functions.c

HEADERS  += mainwindow.h \
    main.h \
    schemadialog.h \
    sheetmessagebox.h \
    schematablemodel.h \
    custumsql.h \
    sqltablemodelalternativebackground.h \
    sqlquerymodelalternativebackground.h \
    attachdatabasedialog.h \
    jointabledialog.h \
    preferencewindow.h \
    fileeventhandler.h \
    summarydialog.h \
    imageview.h \
    sqltextedit.h \
    sqlite3-extension/sqlite3ext.h \
    sqlite3-extension/sqlite3.h

win32 {
    SOURCES -= sqlite3-extension/extension-functions.c
    HEADERS -= sqlite3-extension/sqlite3ext.h \
        sqlite3-extension/sqlite3.h
}

FORMS    += mainwindow.ui \
    schemadialog.ui \
    custumsql.ui \
    attachdatabasedialog.ui \
    jointabledialog.ui \
    preferencewindow.ui \
    summarydialog.ui

RESOURCES += \
    resouces.qrc

OTHER_FILES += \
    windowsicon.rc \
    images/fileicon/table.icns \
    images/fileicon/sqlite.ico \
    images/fileicon/sqlite.icns \
    sqlkeywords.txt \
    functionlist.txt

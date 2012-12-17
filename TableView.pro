#-------------------------------------------------
#
# Project created by QtCreator 2012-08-16T22:27:16
#
#-------------------------------------------------

QT       += core gui sql

TARGET = TableView
TEMPLATE = app

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
    sqltextedit.cpp

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
    sqltextedit.h

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
    sqlkeywords.txt

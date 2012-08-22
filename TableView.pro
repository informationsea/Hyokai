#-------------------------------------------------
#
# Project created by QtCreator 2012-08-16T22:27:16
#
#-------------------------------------------------

QT       += core gui sql

TARGET = TableView
TEMPLATE = app

ICON = images/icon.icns

SOURCES += main.cpp\
        mainwindow.cpp \
    schemadialog.cpp \
    sheetmessagebox.cpp \
    schematablemodel.cpp \
    custumsql.cpp \
    sqltablemodelalternativebackground.cpp \
    sqlquerymodelalternativebackground.cpp

HEADERS  += mainwindow.h \
    main.h \
    schemadialog.h \
    sheetmessagebox.h \
    schematablemodel.h \
    custumsql.h \
    sqltablemodelalternativebackground.h \
    sqlquerymodelalternativebackground.h

FORMS    += mainwindow.ui \
    schemadialog.ui \
    custumsql.ui

RESOURCES += \
    resouces.qrc

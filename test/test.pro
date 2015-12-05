#-------------------------------------------------
#
# Project created by QtCreator 2015-12-06T01:56:13
#
#-------------------------------------------------

QT       += widgets testlib gui sql

TARGET = tst_csvwritertest
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(../src/qtxlsx/src/xlsx/qtxlsx.pri)

SOURCES += tst_csvwritertest.cpp \
    ../src/sqlfileexporter.cpp
DEFINES += SRCDIR=\\\"$$PWD/\\\"

HEADERS += \
    ../src/sqlfileexporter.h

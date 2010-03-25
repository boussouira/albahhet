#-------------------------------------------------
#
# Project created by QtCreator 2010-03-21T11:57:14
#
#-------------------------------------------------

QT       += core gui sql

TARGET = clucene_qt
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui
INCLUDEPATH += "/home/naruto/Bureau/clucene-2/src/ext"
INCLUDEPATH += "/home/naruto/Bureau/clucene-2/src/shared"
LIBS += -lclucene-core
DEFINES += _REENTRANT _UCS2 _UNICODE

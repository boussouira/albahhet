#-------------------------------------------------
#
# Project created by QtCreator 2010-03-21T11:57:14
#
#-------------------------------------------------

QT       += core gui sql
TARGET = clucene_qt
TEMPLATE = app

SOURCES += main.cpp mainwindow.cpp
HEADERS  += mainwindow.h
FORMS    += mainwindow.ui

CLUCENE_PATH = "/home/naruto/Bureau/clucene-2.3.2"

DEFINES += _REENTRANT _UCS2 _UNICODE
INCLUDEPATH += $$CLUCENE_PATH/src/core
INCLUDEPATH += $$CLUCENE_PATH/src/ext
INCLUDEPATH += $$CLUCENE_PATH/src/shared
LIBS += -lclucene-core -L$$CLUCENE_PATH/bin

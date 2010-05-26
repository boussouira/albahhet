
QT       += core gui sql
TARGET = clucene_qt
TEMPLATE = app

SOURCES += main.cpp mainwindow.cpp \
    arabicanalyzer.cpp
HEADERS  += mainwindow.h \
    arabicanalyzer.h
FORMS    += mainwindow.ui

unix:CLUCENE_PATH = "/home/naruto/Bureau/clucene-2.3.2"
win32:CLUCENE_PATH = "C:/clucene-2.3.2"

DEFINES += _REENTRANT _UCS2 _UNICODE
INCLUDEPATH += $$CLUCENE_PATH/src/core
INCLUDEPATH += $$CLUCENE_PATH/src/ext
INCLUDEPATH += $$CLUCENE_PATH/src/shared
LIBS += -lclucene-core -L$$CLUCENE_PATH/bin

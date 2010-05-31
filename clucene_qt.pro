
QT       += core gui sql
TARGET = clucene_qt
TEMPLATE = app

SOURCES += main.cpp mainwindow.cpp \
    arabicanalyzer.cpp
HEADERS  += mainwindow.h \
    arabicanalyzer.h
FORMS    += mainwindow.ui

win32{
    CLUCENE_PATH = "C:/clucene-2.3.2"
    CLUCENE_LIBS_PATH = $$CLUCENE_PATH/bin/release
    DESTDIR = bin
    RC_FILE = win_rc.rc
}
unix {
    CLUCENE_PATH = "/home/naruto/Bureau/clucene-2.3.2"
    CLUCENE_LIBS_PATH = $$CLUCENE_PATH/bin

    HEADERS += mdbconverter.h
    SOURCES += mdbconverter.cpp

    LIBS += -L/usr/local/lib -lmdb -lglib-2.0 -lsqlite3
    INCLUDEPATH += /usr/include/glib-2.0 /usr/lib/glib-2.0/include
}
 win32-msvc* {
     QMAKE_CXXFLAGS -= -Zc:wchar_t-
     QMAKE_CFLAGS -= -Zc:wchar_t-
 }

DEFINES += _REENTRANT _UCS2 _UNICODE
INCLUDEPATH += $$CLUCENE_PATH/src/core
INCLUDEPATH += $$CLUCENE_PATH/src/ext
INCLUDEPATH += $$CLUCENE_PATH/src/shared
LIBS += -lclucene-core -lclucene-shared -L$$CLUCENE_LIBS_PATH

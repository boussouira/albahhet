QT += core \
    gui \
    sql \
    webkit
TEMPLATE = app
TARGET = clucene_qt

MOC_DIR += .moc
OBJECTS_DIR += .obj
UI_DIR += .ui
RCC_DIR += .rcc


SOURCES += main.cpp \
    mainwindow.cpp \
    arabicanalyzer.cpp \
    indexingdialg.cpp \
    indexthread.cpp \
    booksdb.cpp \
    indexinfo.cpp \
    shamelasearcher.cpp \
    shamelaresult.cpp \
    shamelaresultwidget.cpp \
    arabicfilter.cpp \
    arabictokenizer.cpp \
    settingsdialog.cpp \
    indexesdialog.cpp
HEADERS += mainwindow.h \
    arabicanalyzer.h \
    indexingdialg.h \
    indexthread.h \
    common.h \
    booksdb.h \
    indexinfo.h \
    shamelasearcher.h \
    shamelaresult.h \
    shamelaresultwidget.h \
    cl_common.h \
    arabicfilter.h \
    arabictokenizer.h \
    settingsdialog.h \
    indexesdialog.h
FORMS += mainwindow.ui \
    indexingdialg.ui \
    shamelaresultwidget.ui \
    settingsdialog.ui \
    indexesdialog.ui
win32 {
    win32-msvc* {
        CLUCENE_PATH = "C:/clucene-2.3.2"
        CLUCENE_LIBS_PATH = $$CLUCENE_PATH/bin/release
    } else {
        CLUCENE_PATH = "C:/clucene-2.3.2_mingw"
        CLUCENE_LIBS_PATH = $$CLUCENE_PATH/bin
    }
    DESTDIR = bin
    RC_FILE = win_rc.rc
}
unix { 
    CLUCENE_PATH = "/home/naruto/Bureau/clucene-2.3.2"
    CLUCENE_LIBS_PATH = $$CLUCENE_PATH/bin
    HEADERS += mdbconverter.h
    SOURCES += mdbconverter.cpp
    LIBS += -L/usr/local/lib \
        -lmdb \
        -lglib-2.0 \
        -lsqlite3
    INCLUDEPATH += /usr/include/glib-2.0 \
        /usr/lib/glib-2.0/include
}
win32-msvc* { 
    QMAKE_CXXFLAGS -= -Zc:wchar_t-
    QMAKE_CFLAGS -= -Zc:wchar_t-
}
exists(.git/HEAD) { 
    GITVERSION = $$system(git log -n1 --pretty=format:%h)
    !isEmpty(GITVERSION) { 
        GITCHANGENUMBER = $$system(git log --pretty=format:%h | wc -l)
        DEFINES += GITVERSION=\"\\\"$$GITVERSION\\\"\"
        DEFINES += GITCHANGENUMBER=\"\\\"$$GITCHANGENUMBER\\\"\"
    }
}
DEFINES += _REENTRANT \
    _UCS2 \
    _UNICODE

INCLUDEPATH += $$CLUCENE_PATH/src/core
INCLUDEPATH += $$CLUCENE_PATH/src/ext
INCLUDEPATH += $$CLUCENE_PATH/src/shared
INCLUDEPATH += $$CLUCENE_PATH/src/contribs-lib

LIBS += -lclucene-core \
    -lclucene-shared \
    -lclucene-contribs-lib \
    -L$$CLUCENE_LIBS_PATH

RESOURCES += \
    resources.qrc

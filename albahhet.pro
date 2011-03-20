QT += core \
    gui \
    sql \
    webkit
TEMPLATE = app
TARGET = albahhet

MOC_DIR += .moc
OBJECTS_DIR += .obj
UI_DIR += .ui
RCC_DIR += .rcc

INCLUDEPATH += $$PWD

SOURCES += main.cpp \
    mainwindow.cpp \
    arabicanalyzer.cpp \
    indexingdialg.cpp \
    shamelaindexer.cpp \
    booksdb.cpp \
    indexinfo.cpp \
    shamelasearcher.cpp \
    shamelaresult.cpp \
    shamelaresultwidget.cpp \
    arabicfilter.cpp \
    arabictokenizer.cpp \
    settingsdialog.cpp \
    indexesdialog.cpp \
    shamelamodels.cpp \
    bookinfo.cpp \
    fancylineedit.cpp \
    common.cpp \
    searchfilterhandler.cpp \
    settingschecker.cpp \
    shamelaupdaterdialog.cpp \
    logdialog.cpp \
    loghighlighter.cpp \
    webview.cpp \
    tabwidget.cpp \
    shamelasearchwidget.cpp \
    shamelabooksreader.cpp
HEADERS += mainwindow.h \
    arabicanalyzer.h \
    indexingdialg.h \
    shamelaindexer.h \
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
    indexesdialog.h \
    shamelamodels.h \
    bookinfo.h \
    fancylineedit.h \
    searchfilterhandler.h \
    settingschecker.h \
    shamelaupdaterdialog.h \
    logdialog.h \
    loghighlighter.h \
    webview.h \
    tabwidget.h \
    shamelasearchwidget.h \
    shamelabooksreader.h
FORMS += mainwindow.ui \
    indexingdialg.ui \
    shamelaresultwidget.ui \
    settingsdialog.ui \
    indexesdialog.ui \
    shamelaupdaterdialog.ui \
    logdialog.ui \
    shamelasearchwidget.ui

CLUCENE_SOURCE_PATH = $$(CLUCENE_SOURCE_PATH)
CLUCENE_BUILD_PATH = $$(CLUCENE_BUILD_PATH)

win32 {
    win32-msvc* {
        CONFIG(debug, debug|release) {
            CLUCENE_LIBS_PATH = $$CLUCENE_BUILD_PATH/bin/debug
            CLUCENE_LIB_SUFFIX = "d"
        }

        CONFIG(release, debug|release) {
            CLUCENE_LIBS_PATH = $$CLUCENE_BUILD_PATH/bin/release
        }

        DEFINES +=  _CRT_SECURE_NO_DEPRECATE \
                    _CRT_NONSTDC_NO_DEPRECATE
    }

    win32-g++ {
        CLUCENE_LIBS_PATH = $$CLUCENE_BUILD_PATH/bin
    }

    LIBS += -luser32
    DESTDIR = bin
    RC_FILE = win_rc.rc
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

INCLUDEPATH += $$CLUCENE_SOURCE_PATH/src/core \
        $$CLUCENE_SOURCE_PATH/src/ext \
        $$CLUCENE_SOURCE_PATH/src/shared \
        $$CLUCENE_SOURCE_PATH/src/contribs-lib \
        $$CLUCENE_BUILD_PATH/src/shared

LIBS += -lclucene-core$$CLUCENE_LIB_SUFFIX \
        -lclucene-shared$$CLUCENE_LIB_SUFFIX \
        -lclucene-contribs-lib$$CLUCENE_LIB_SUFFIX \
        -L$$CLUCENE_LIBS_PATH

#message(The project will be linked with $$LIBS)

RESOURCES += resources.qrc

QT += core \
    gui \
    sql \
    webkit \
    xml

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
    shamelabooksreader.cpp \
    qthighlighter.cpp \
    bookprogresswidget.cpp \
    shamelafilterproxymodel.cpp \
    selectedfilterwidget.cpp \
    indexesmanager.cpp \
    shamelaindexerwidget.cpp \
    abstractindexingwidget.cpp \
    quranindexer.cpp \
    abstractsearchwidget.cpp \
    quransearchwidget.cpp \
    quransearcher.cpp \
    quranresult.cpp \
    quranindexerwidget.cpp \
    searchfield.cpp \
    searchfieldsdialog.cpp \
    customchartokenizer.cpp

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
    shamelabooksreader.h \
    qthighlighter.h \
    bookprogresswidget.h \
    shamelafilterproxymodel.h \
    selectedfilterwidget.h \
    indexesmanager.h \
    shamelaindexerwidget.h \
    abstractindexingwidget.h \
    quranindexer.h \
    abstractsearchwidget.h \
    quransearchwidget.h \
    quransearcher.h \
    quranresult.h \
    quranindexerwidget.h \
    searchfield.h \
    searchfieldsdialog.h \
    customchartokenizer.h

FORMS += mainwindow.ui \
    indexingdialg.ui \
    shamelaresultwidget.ui \
    settingsdialog.ui \
    indexesdialog.ui \
    shamelaupdaterdialog.ui \
    logdialog.ui \
    shamelasearchwidget.ui \
    selectedfilterwidget.ui \
    shamelaindexerwidget.ui \
    quransearchwidget.ui \
    quranindexerwidget.ui \
    searchfieldsdialog.ui

RESOURCES += resources.qrc

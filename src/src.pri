QT += core \
    gui \
    sql \
    network \
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
    shamelaindexer.cpp \
    booksdb.cpp \
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
    abstractsearchwidget.cpp \
    searchfield.cpp \
    searchfieldsdialog.cpp \
    shamelaupdater.cpp \
    customchartokenizer.cpp \
    shamelaselectbookdialog.cpp \
    aboutdialog.cpp \
    updatechecker.cpp \
    updatedialog.cpp \
    wordtypefilter.cpp \
    supportdialog.cpp \
    searchquerywidget.cpp \
    networkrequest.cpp \
    filterlineedit.cpp \
    createindexdialog.cpp \
    filechooserwidget.cpp \
    indexoptimizer.cpp \
    shamelaindexerpages.cpp \
    indexwidgetbase.cpp \
    shamelaindexinfo.cpp \
    indexinfobase.cpp

HEADERS += mainwindow.h \
    arabicanalyzer.h \
    shamelaindexer.h \
    common.h \
    booksdb.h \
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
    abstractsearchwidget.h \
    searchfield.h \
    searchfieldsdialog.h \
    shamelaupdater.h \
    customchartokenizer.h \
    shamelaselectbookdialog.h \
    aboutdialog.h \
    updatechecker.h \
    updatedialog.h \
    wordtypefilter.h \
    app_version.h \
    supportdialog.h \
    searchquerywidget.h \
    networkrequest.h \
    filterlineedit.h \
    createindexdialog.h \
    filechooserwidget.h \
    indexoptimizer.h \
    shamelaindexerpages.h \
    indexwidgetbase.h \
    shamelaindexinfo.h \
    indexinfobase.h

FORMS += mainwindow.ui \
    shamelaresultwidget.ui \
    settingsdialog.ui \
    indexesdialog.ui \
    shamelaupdaterdialog.ui \
    logdialog.ui \
    shamelasearchwidget.ui \
    selectedfilterwidget.ui \
    searchfieldsdialog.ui \
    shamelaselectbookdialog.ui \
    aboutdialog.ui \
    updatedialog.ui \
    supportdialog.ui \
    searchquerywidget.ui

RESOURCES += resources.qrc




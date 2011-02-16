project(al-bahhet)

cmake_minimum_required(VERSION 2.6)
set(CMAKE_BUILD_TYPE Release)
add_definitions(-Wall)
find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})

IF (WIN32)
    set(CLUCENE_PATH C:/clucene-2.3.2)
    set(CLUCENE_LIBRARYS_PATH ${CLUCENE_PATH}/bin/release)
    set(CLUCENE_LIBRARYS clucene-core clucene-shared clucene-contribs-lib)
ELSE(WIN32)
    set(CLUCENE_PATH /home/naruto/Bureau/clucene-2.3.2)
ENDIF(WIN32)
    set(CLUCENE_INLUDE_DIRS
        ${CLUCENE_PATH}/src/ext
        ${CLUCENE_PATH}/src/core
        ${CLUCENE_PATH}/src/shared
        ${CLUCENE_PATH}/src/contribs-lib)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${QT_QTCORE_INCLUDE_DIR}
    ${QT_QTGUI_INCLUDE_DIR}
    ${QT_QTWEBKIT_INCLUDE_DIR}
    ${QT_QTXMLPATTERNS_INCLUDE_DIR}
    ${QT_QTSQL_INCLUDE_DIR}
    ${CLUCENE_INLUDE_DIRS})

link_directories(${CLUCENE_PATH}/bin)

set(bahhet_SRCS
    arabicanalyzer.cpp
    arabicfilter.cpp
    arabictokenizer.cpp
    bookinfo.cpp
    booksdb.cpp
    common.cpp
    fancylineedit.cpp
    indexesdialog.cpp
    indexinfo.cpp
    indexingdialg.cpp
    indexthread.cpp
    main.cpp
    mainwindow.cpp
    searchfilterhandler.cpp
    settingsdialog.cpp
    shamelamodels.cpp
    shamelaresult.cpp
    shamelaresultwidget.cpp
    shamelasearcher.cpp)

set(bahhet_UIS
    indexesdialog.ui
    indexingdialg.ui
    mainwindow.ui
    settingsdialog.ui
    shamelaresultwidget.ui)

QT4_WRAP_UI(UIS ${bahhet_UIS})

set(bahhet_MOCS
    fancylineedit.h
    indexesdialog.h
    indexingdialg.h
    indexthread.h
    mainwindow.h
    searchfilterhandler.h
    settingsdialog.h
    shamelamodels.h
    shamelaresultwidget.h
    shamelasearcher.h)

QT4_WRAP_CPP(MOCS ${bahhet_MOCS})
QT4_ADD_RESOURCES(RSCS resources.qrc)

add_executable(al-bahhet
    ${bahhet_SRCS}
    ${UIS} ${RSCS} ${TRS} ${MOCS})

target_link_libraries(al-bahhet
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTSQL_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    ${QT_QTXMLPATTERNS_LIBRARY}
    ${CLUCENE_LIBRARYS})

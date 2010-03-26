project(clucene_qt)
cmake_minimum_required(VERSION 2.6.0)
find_package(Qt4 4.6 COMPONENTS QtCore QtGui QtSql REQUIRED)
include(${QT_USE_FILE})
include_directories(
	"/home/naruto/Bureau/clucene-2/src/ext"
	"/home/naruto/Bureau/clucene-2/src/shared"
)
set(clucene_qt_SRCS
    main.cpp
    mainwindow.cpp
)
set(clucene_qt_MOC_SRCS
    mainwindow.h
)
set(clucene_qt_UIS
    mainwindow.ui
)
#set(clucene_qt_RCCS
#    clucene_qt.qrc
#)
qt4_wrap_cpp(clucene_qt_MOCS ${clucene_qt_MOC_SRCS})
qt4_wrap_ui(clucene_qt_UIS_H ${clucene_qt_UIS})
#qt4_wrap_cpp(clucene_qt_MOC_UI ${clucene_qt_UIS_H})
#qt4_add_resources(clucene_qt_RCC_SRCS ${clucene_qt_RCCS})
add_definitions(-DQT_NO_DEBUG)
add_executable(clucene_qt
    ${clucene_qt_SRCS}
    ${clucene_qt_MOCS}
    ${clucene_qt_UIS_H}
#    ${clucene_qt_MOC_UI}
    ${clucene_qt_RCC_SRCS})
target_link_libraries(clucene_qt ${QT_LIBRARIES} pthread clucene-core)

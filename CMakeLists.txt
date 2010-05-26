project( clucene_qt )
cmake_minimum_required( VERSION 2.6 )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall )
find_package ( Qt4 REQUIRED )
include ( ${QT_USE_FILE} )
set ( CLUCENE_DIR C:/clucene-2.3.2)
include_directories (
	${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} 
	${QT_QTCORE_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} 
	${QT_QTSQL_INCLUDE_DIR}
	${CLUCENE_DIR}/src/ext
	${CLUCENE_DIR}/src/core
	${CLUCENE_DIR}/src/shared
)
link_directories (
	${CLUCENE_DIR}/bin
)
set ( clucene_qt_HDRS
	arabicanalyzer.h
	)

set ( clucene_qt_SRCS
	main.cpp
	mainwindow.cpp
	arabicanalyzer.cpp
	)

set ( clucene_qt_UIS
	mainwindow.ui
	)
QT4_WRAP_UI(UIS ${clucene_qt_UIS})

set ( clucene_qt_MOCS
	mainwindow.h
	)
QT4_WRAP_CPP(MOCS ${clucene_qt_MOCS})

add_executable ( clucene_qt ${clucene_qt_SRCS} ${clucene_qt_HDRS} ${UIS} ${RSCS} ${TRS} ${MOCS} )
target_link_libraries ( clucene_qt  ${QT_QTCORE_LIBRARY} 
						${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} 
						${QT_QTGUI_LIBRARY} ${QT_QTSQL_LIBRARY}
						 clucene-core)

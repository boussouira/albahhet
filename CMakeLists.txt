project( clucene_qt )
cmake_minimum_required( VERSION 2.6 )
set ( CMAKE_BUILD_TYPE Release )
add_definitions ( -Wall )
find_package ( Qt4 REQUIRED )
include ( ${QT_USE_FILE} )
IF (WIN32)
	set( CLUCENE_PATH C:/clucene-2.3.2)
	set( CLUCENE_LIBRARYS_PATH ${CLUCENE_PATH}/bin/release)
	set( CLUCENE_LIBRARYS clucene-core clucene-shared)
ELSE(WIN32)
	set ( CLUCENE_PATH /home/naruto/Bureau/clucene-2.3.2)
ENDIF(WIN32)
	set ( CLUCENE_INLUDE_DIRS 
			${CLUCENE_PATH}/src/ext
			${CLUCENE_PATH}/src/core
			${CLUCENE_PATH}/src/shared)
			
include_directories (
	${CMAKE_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
	${QT_QTCORE_INCLUDE_DIR}
	${QT_QTGUI_INCLUDE_DIR}
	${QT_QTWEBKIT_INCLUDE_DIR}
	${QT_QTXMLPATTERNS_INCLUDE_DIR}
	${QT_QTSQL_INCLUDE_DIR}
	${CLUCENE_INLUDE_DIRS})
link_directories (${CLUCENE_PATH}/bin)
set ( clucene_qt_HDRS
	arabicanalyzer.h
	mdbconverter.h)

set ( clucene_qt_SRCS
	main.cpp
	mainwindow.cpp
	arabicanalyzer.cpp
	mdbconverter.cpp
	indexingdialg.cpp
	indexthread.cpp)

set ( clucene_qt_UIS
	mainwindow.ui
	indexingdialg.ui)
	
QT4_WRAP_UI(UIS ${clucene_qt_UIS})

set ( clucene_qt_MOCS
	mainwindow.h
	indexingdialg.h
	indexthread.h)
	
QT4_WRAP_CPP(MOCS ${clucene_qt_MOCS})

add_executable ( clucene_qt 
				${clucene_qt_SRCS} 
				${clucene_qt_HDRS} ${UIS} ${RSCS} ${TRS} ${MOCS})
				
target_link_libraries ( clucene_qt  
						${QT_QTCORE_LIBRARY} 
						${QT_QTGUI_LIBRARY} 
						${QT_QTCORE_LIBRARY} 
						${QT_QTSQL_LIBRARY}
						${QT_QTWEBKIT_LIBRARY}
						${QT_QTXMLPATTERNS_LIBRARY}
						${CLUCENE_LIBRARYS})

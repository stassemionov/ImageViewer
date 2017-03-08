cmake_minimum_required(VERSION 2.8.11)
project(HistogramBuilder)
find_package(Qt5 REQUIRED Widgets LinguistTools)
find_package(OpenCV REQUIRED core imgproc)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(SOURCE_PATH "./source")
set(HEADER_PATH "./include")
set(SOURCE_FILES ${SOURCE_PATH}/histogrambuilder.cpp
				 ${SOURCE_PATH}/imageplace.cpp
				 ${SOURCE_PATH}/main.cpp
				 ${SOURCE_PATH}/mainwindow.cpp)
set(HEADER_FILES ${HEADER_PATH}/histogrambuilder.h
				 ${HEADER_PATH}/imageplace.h
				 ${HEADER_PATH}/mainwindow.h)
set(QRC_FILES 	"./resources/resources.qrc")
set(UI_FILES	"./forms/mainwindow.ui")
set(TS_FILES  	"./translations/HistogramBuilder_ru_RU.ts")

include_directories(${HEADER_PATH}
					${OpenCV_INCLUDE_DIRS})

# Translation
set(FILES_TO_TRANSLATE ${SOURCE_FILES} ${UI_FILES})
option(UPDATE_TRANSLATIONS
	  "Update source translation (files translations/*.ts)."
	   ON)

if(UPDATE_TRANSLATIONS)
	message(STATUS "UPDATE_TRANSLATIONS option is set.")
	qt5_create_translation(QM_FILES
						   ${FILES_TO_TRANSLATE}
						   ${TS_FILES})
else()
	qt5_add_translation(QM_FILES ${TS_FILES})
endif()

add_custom_target(translations DEPENDS ${QM_FILES})
set_property(DIRECTORY PROPERTY CLEAN_NO_CUSTOM TRUE)

# Executable
qt5_wrap_cpp(	  MOC_WRAPPERS ${HEADER_FILES})
qt5_wrap_ui( 	  UIC_WRAPPERS ${UI_FILES})
qt5_add_resources(QRC_WRAPPERS ${QRC_FILES}
				  OPTIONS -no-compress)

add_definitions(-DQT_NO_CAST_FROM_ASCII)
add_executable(HistogramBuilder
		 	   	${SOURCE_FILES}
			   	${HEADER_FILES}
				${MOC_WRAPPERS}
				${UIC_WRAPPERS}
				${QRC_WRAPPERS}
				${FILES_TO_TRANSLATE})
target_compile_features(HistogramBuilder PUBLIC cxx_nullptr)
if (CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(HistogramBuilder PUBLIC -Wall -O3)
endif()
if (MSVC)
    target_compile_options(HistogramBuilder PUBLIC /W3 /Ox)
endif()

target_link_libraries(HistogramBuilder Qt5::Widgets
					   					opencv_core
			 							opencv_imgproc)
# Installation
# For user (.exe + .qm)
install(TARGETS HistogramBuilder
		RUNTIME
		DESTINATION user/bin
		COMPONENT user)
install(FILES ${QM_FILES}
		DESTINATION user/translations
		COMPONENT user)
# For developer (.exe + .qm + .ts)
install(TARGETS HistogramBuilder
		RUNTIME
		DESTINATION developer/bin
		COMPONENT developer)
install(FILES ${QM_FILES} ${TS_FILES}
		DESTINATION developer/translations
		COMPONENT developer)

get_property(LIB_QT5CORE_PATH
			 TARGET 	Qt5::Core
			 PROPERTY 	IMPORTED_LOCATION_RELEASE)
get_filename_component(QT_LIBRARY_DIR
					   "${LIB_QT5CORE_PATH}"
					   DIRECTORY)

set(EXE_PATH_USR
	user/bin/HistogramBuilder${CMAKE_EXECUTABLE_SUFFIX})
set(EXE_PATH_DVP
	developer/bin/HistogramBuilder${CMAKE_EXECUTABLE_SUFFIX})

include(DeployQt4)
install_qt4_executable("${EXE_PATH_USR}" "" "" "${QT_LIBRARY_DIR}")
install_qt4_executable("${EXE_PATH_DVP}" "" "" "${QT_LIBRARY_DIR}")


#-------------------------------------------------
#
# Project created by QtCreator 2017-02-20T17:50:56
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11 -O3 -Wall -fopenmp

TARGET = ImageViewer
TEMPLATE = app

LIBS += -fopenmp

INCLUDEPATH +=\
    $$PWD/include

SOURCES +=\
    source/main.cpp \
    source/mainwindow.cpp \
    source/labelviewer.cpp \
    source/service.cpp \
    source/history.cpp \
    source/mainwindow_editor.cpp \
    source/filtercustomizer.cpp

HEADERS +=\
    include/mainwindow.h \
    include/labelviewer.h \
    include/service.h \
    include/history.h \
    include/filtercustomizer.h

FORMS +=\
    interface/mainwindow.ui \
    interface/properties_dialog.ui \
    interface/filtercustomizer.ui

RESOURCES += \
    resources/resources.qrc

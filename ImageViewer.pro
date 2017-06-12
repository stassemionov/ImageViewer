#-------------------------------------------------
#
# Project created by QtCreator 2017-02-20T17:50:56
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -std=c++11 -O2 -Wall

TARGET = ImageViewer
TEMPLATE = app

INCLUDEPATH +=\
    $$PWD/include

SOURCES +=\
    source/main.cpp \
    source/mainwindow.cpp \
    source/labelviewer.cpp \
    source/service.cpp \
    source/history.cpp \
    source/mainwindow_editor.cpp

HEADERS +=\
    include/mainwindow.h \
    include/labelviewer.h \
    include/service.h \
    include/history.h

FORMS +=\
    interface/mainwindow.ui \
    interface/properties_dialog.ui

RESOURCES += \
    resources/resources.qrc

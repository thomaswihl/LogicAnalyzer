#-------------------------------------------------
#
# Project created by QtCreator 2013-10-19T19:12:41
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = LogicAnalyser
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    serialport.cpp

HEADERS  += mainwindow.h \
    serialport.h

FORMS    += mainwindow.ui

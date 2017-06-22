#-------------------------------------------------
#
# Project created by QtCreator 2017-01-19T22:15:37
#
#-------------------------------------------------

QT       += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = QtRCONclient
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
    RCon.cpp \
    qtrconclient.cpp

HEADERS  += \
    RCon.h \
    qtrconclient.h

FORMS    += \
    qtrconclient.ui

win32:RC_FILE = myapp.rc

RESOURCES += \
    resources.qrc

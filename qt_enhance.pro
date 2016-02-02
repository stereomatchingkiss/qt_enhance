#-------------------------------------------------
#
# Project created by QtCreator 2016-02-02T05:34:02
#
#-------------------------------------------------

QT       += widgets

TARGET = qt_enhance
TEMPLATE = lib
CONFIG += staticlib

SOURCES += img_region_selector.cpp

HEADERS += img_region_selector.hpp
unix {
    target.path = /usr/lib
    INSTALLS += target
}

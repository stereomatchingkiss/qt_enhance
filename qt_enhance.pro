#-------------------------------------------------
#
# Project created by QtCreator 2016-02-02T05:34:02
#
#-------------------------------------------------

QT       += widgets

TARGET = qt_enhance
TEMPLATE = lib
CONFIG += staticlib

SOURCES += gui/img_region_selector.cpp \
    gui/rubber_band.cpp

HEADERS += gui/img_region_selector.hpp \
    gui/rubber_band.hpp
unix {
    target.path = /usr/lib
    INSTALLS += target
}

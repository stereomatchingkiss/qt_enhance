#-------------------------------------------------
#
# Project created by QtCreator 2016-02-02T05:34:02
#
#-------------------------------------------------

QT       += widgets

TARGET = qt_enhance
TEMPLATE = lib
CONFIG += staticlib

SOURCES += img_region_selector.cpp \
    rubber_band.cpp

HEADERS += img_region_selector.hpp \
    rubber_band.hpp
unix {
    target.path = /usr/lib
    INSTALLS += target
}

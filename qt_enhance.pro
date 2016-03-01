#-------------------------------------------------
#
# Project created by QtCreator 2016-02-02T05:34:02
#
#-------------------------------------------------

QT       += widgets network core

TARGET = qt_enhance
TEMPLATE = lib
CONFIG += staticlib

include(../pri/boost.pri)

SOURCES += gui/img_region_selector.cpp \
    gui/rubber_band.cpp \
    network/download_info.cpp \
    network/download_manager.cpp

HEADERS += gui/img_region_selector.hpp \
    gui/rubber_band.hpp \
    network/download_info.hpp \
    network/download_manager.hpp
unix {
    target.path = /usr/lib
    INSTALLS += target
}

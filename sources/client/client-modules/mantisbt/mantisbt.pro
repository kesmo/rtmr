#-------------------------------------------------
#
# Project created by QtCreator 2011-05-01T12:20:09
#
#-------------------------------------------------
TARGET = mantisbt
QT += xml network
TARGET_EXT = .cmo

TEMPLATE = lib
CONFIG += plugin

include(qtsoap-2.7_1-opensource/src/qtsoap.pri)
include (../../common/common.pri)

DEFINES += CLIENT_MODULE_LIBRARY

unix:target.path = $$EXTERNALS_MODULES_DIR

SOURCES += ../bugtracker.cpp \
    mantisbt.cpp \
    mantismodule.cpp
HEADERS += ../bugtracker.h \
    mantisbt.h \
    mantismodule.h
INCLUDEPATH += .. \
    ../../common \
    ../../client-app \
    ../../client-lib

OBJECTS_DIR += build

!mac {
    QMAKE_EXTENSION_SHLIB = cmo
}

#!win32:QMAKE_LFLAGS += --export-dynamic
#QMAKE_LN_SHLIB = :

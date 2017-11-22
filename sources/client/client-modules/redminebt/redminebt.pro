# -------------------------------------------------
# Project created by QtCreator 2012-02-25T15:10:19
# -------------------------------------------------
TARGET = redminebt
QT += xml network
TARGET_EXT = .cmo
TEMPLATE = lib
CONFIG += plugin

include (../../common/common.pri)

DEFINES += CLIENT_MODULE_LIBRARY

unix:target.path = $$EXTERNALS_MODULES_DIR

SOURCES += ../bugtracker.cpp \
    redminebt.cpp \
    redminemodule.cpp

HEADERS += ../bugtracker.h \
    redminebt.h \
    redminemodule.h

INCLUDEPATH += .. \
    ../../common \
    ../../client-app \
    ../../client-lib

OBJECTS_DIR += build

!mac:QMAKE_EXTENSION_SHLIB = cmo

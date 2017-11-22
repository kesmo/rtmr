# -------------------------------------------------
# Project created by QtCreator 2011-04-25T21:32:39
# -------------------------------------------------
TARGET = bugzillabt
QT += xml network
TARGET_EXT = .cmo

TEMPLATE = lib
CONFIG += plugin

include (../../common/common.pri)

DEFINES += CLIENT_MODULE_LIBRARY

unix:target.path = $$EXTERNALS_MODULES_DIR

SOURCES += ../bugtracker.cpp \
    bugzillabt.cpp \
    qxtxmlrpc/qxtxmlrpcresponseparser.cpp \
    qxtxmlrpc/qxtxmlrpcrequestcomposer.cpp \
    qxtxmlrpc/qxtxmlrpcclient.cpp \
    bugzillamodule.cpp
HEADERS += ../bugtracker.h \
    bugzillabt.h \
    qxtxmlrpc/qxtxmlrpcresponseparser.h \
    qxtxmlrpc/qxtxmlrpcrequestcomposer.h \
    qxtxmlrpc/qxtxmlrpcclient.h \
    bugzillamodule.h
INCLUDEPATH += .. \
    ../../common \
    ../../client-lib \
    ../../client-app

OBJECTS_DIR += build

!mac {
    QMAKE_EXTENSION_SHLIB = cmo
}

#!win32:QMAKE_LFLAGS += --export-dynamic
#QMAKE_LN_SHLIB = :

# -------------------------------------------------
# Project created by QtCreator 2012-02-25T15:10:19
# -------------------------------------------------
TARGET = jirabt
QT += xml \
    network
TARGET_EXT = .cmo
TEMPLATE = lib
CONFIG += plugin
include (../../common/common.pri)
DEFINES += CLIENT_MODULE_LIBRARY QJSON_EXPORT=
unix:target.path = $$EXTERNALS_MODULES_DIR
SOURCES += ../bugtracker.cpp \
    jirabt.cpp \
    jiramodule.cpp \
    qjson/serializerrunnable.cpp \
    qjson/serializer.cpp \
    qjson/qobjecthelper.cpp \
    qjson/parserrunnable.cpp \
    qjson/parser.cpp \
    qjson/json_scanner.cpp \
    qjson/json_parser.cc
HEADERS += ../bugtracker.h \
    jirabt.h \
    jiramodule.h \
    qjson/stack.hh \
    qjson/serializerrunnable.h \
    qjson/serializer.h \
    qjson/qobjecthelper.h \
    qjson/qjson_export.h \
    qjson/qjson_debug.h \
    qjson/position.hh \
    qjson/parserrunnable.h \
    qjson/parser_p.h \
    qjson/parser.h \
    qjson/location.hh \
    qjson/json_scanner.h \
    qjson/json_parser.hh
INCLUDEPATH += .. \
    ../../common \
    ../../client-app \
    ../../client-lib
OBJECTS_DIR += build
!mac:QMAKE_EXTENSION_SHLIB = cmo
#!win32:QMAKE_LFLAGS += --export-dynamic
OTHER_FILES += qjson/json_parser.yy

#-------------------------------------------------
#
# Project created by QtCreator 2012-12-08T13:48:17
#
#-------------------------------------------------

TARGET = std-automation
QT += network

TARGET_EXT = .cmo
TEMPLATE = lib
CONFIG += plugin
include (../../common/common.pri)
DEFINES += CLIENT_MODULE_LIBRARY
unix:target.path = $$EXTERNALS_MODULES_DIR

Debug {
    LIBS += -L../../client-entities -L../../client-entities/Debug -L../../client-lib/Debug
    win32:PRE_TARGETDEPS += ../../client-entities/Debug/client-entities.lib
    unix:PRE_TARGETDEPS += ../../client-entities/libclient-entities.a

} else {
    LIBS += -L../../client-entities -L../../client-entities/Release -L../../client-lib/Release
    win32:PRE_TARGETDEPS += ../../client-entities/Release/client-entities.lib
    unix:PRE_TARGETDEPS += ../../client-entities/libclient-entities.a
}
win32:LIBS += -lUser32
LIBS += -lclient-entities -lrtmr

SOURCES += stdautomation.cpp

HEADERS += stdautomation.h

INCLUDEPATH += .. \
    ../../common \
    ../../client-app \
    ../../client-lib \
    ../../client-entities

OBJECTS_DIR += build

!mac:QMAKE_EXTENSION_SHLIB = cmo

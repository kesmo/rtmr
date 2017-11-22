# -------------------------------------------------
# Project created by QtCreator 2011-08-28T13:34:14
# -------------------------------------------------
TARGET = rtmr
QT += network gui

TEMPLATE = app

DEFINES += AUTOMATION_LIB

include (../common/common.pri)

unix:target.path = $$BINDIR

#DEFINES += ELEVATED_UAC

win32 {
    contains(DEFINES, ELEVATED_UAC) {
        CONFIG -= embed_manifest_exe
        Debug{
            QMAKE_POST_LINK += $$quote(mt.exe -nologo -manifest \"$${PWD}\\rtmr.exe.manifest\" -outputresource:\"$${PWD}\\Debug/rtmr.exe\")
        } else {
            contains(DEFINES, AUTOMATION_LIB) {
                QMAKE_POST_LINK += $$quote(mt.exe -nologo -manifest \"$${PWD}\\rtmr.exe.manifest\" -outputresource:\"$${PWD}\\Release/rtmr.exe\")
            }
        }
    }
}

SOURCES += main.cpp \
    client_launcher.cpp \
    gui/form_new_version_information.cpp \
    process_utils.cpp
HEADERS += client_launcher.h \
    gui/form_new_version_information.h \
    process_utils.h

FORMS += \
    gui/form_new_version_information.ui

mac {
    ICON = ../client-app/images/client.icns
}

win32:RC_FILE = ../client-app/resources.rc

OTHER_FILES += ../client-app/resources.rc
OTHER_FILES += languages/client-launcher_en.ts
OTHER_FILES += ../client-app/rtmrapp.exe.manifest

win32:LIBS += -lpsapi -lUser32
win32:DEFINES += PSAPI_VERSION=1

mac:LIBS += -framework Carbon

RESOURCES += ../client-app/resources.qrc \
    client-launcher-resources.qrc

OBJECTS_DIR = build
MOC_DIR = build

Debug {
    LIBS += -L../client-lib/Debug
}else{
    LIBS += -L../client-lib/Release
}
unix:LIBS += -lproc

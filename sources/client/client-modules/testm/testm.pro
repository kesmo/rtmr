QT += network
DEFINES += RTMR_LIBRARY_IMPORTS \
    _RTMR \
    _POSTGRES
DEFINES += CLIENT_MODULE_LIBRARY \
    APP_VERSION=\\\"1.9.0.0\\\"
DEFINES -= UNICODE
TARGET = testm
TARGET_EXT = .cmo
TEMPLATE = lib
CONFIG += plugin
HEADERS += testm.h \
    ../../client-app/records/xmlprojectdatas.h \
    ../../client-app/records/user.h \
    ../../client-app/records/testrequirement.h \
    ../../client-app/records/testhierarchy.h \
    ../../client-app/records/testcontentfile.h \
    ../../client-app/records/testcontent.h \
    ../../client-app/records/testcampaign.h \
    ../../client-app/records/test.h \
    ../../client-app/records/requirementhierarchy.h \
    ../../client-app/records/requirementcontent.h \
    ../../client-app/records/requirement.h \
    ../../client-app/records/record.h \
    ../../client-app/records/projectversion.h \
    ../../client-app/records/projectparameter.h \
    ../../client-app/records/projectgrant.h \
    ../../client-app/records/project.h \
    ../../client-app/records/parameter.h \
    ../../client-app/records/hierarchy.h \
    ../../client-app/records/genericrecord.h \
    ../../client-app/records/executiontestparameter.h \
    ../../client-app/records/executiontest.h \
    ../../client-app/records/executionrequirement.h \
    ../../client-app/records/executioncampaignparameter.h \
    ../../client-app/records/executioncampaign.h \
    ../../client-app/records/executionaction.h \
    ../../client-app/records/campaign.h \
    ../../client-app/records/bug.h \
    ../../client-app/records/action.h
SOURCES += testm.cpp \
    ../../client-app/records/xmlprojectdatas.cpp \
    ../../client-app/records/user.cpp \
    ../../client-app/records/testrequirement.cpp \
    ../../client-app/records/testhierarchy.cpp \
    ../../client-app/records/testcontentfile.cpp \
    ../../client-app/records/testcontent.cpp \
    ../../client-app/records/testcampaign.cpp \
    ../../client-app/records/test.cpp \
    ../../client-app/records/requirementhierarchy.cpp \
    ../../client-app/records/requirementcontent.cpp \
    ../../client-app/records/requirement.cpp \
    ../../client-app/records/record.cpp \
    ../../client-app/records/projectversion.cpp \
    ../../client-app/records/projectparameter.cpp \
    ../../client-app/records/projectgrant.cpp \
    ../../client-app/records/project.cpp \
    ../../client-app/records/parameter.cpp \
    ../../client-app/records/hierarchy.cpp \
    ../../client-app/records/genericrecord.cpp \
    ../../client-app/records/executiontestparameter.cpp \
    ../../client-app/records/executiontest.cpp \
    ../../client-app/records/executionrequirement.cpp \
    ../../client-app/records/executioncampaignparameter.cpp \
    ../../client-app/records/executioncampaign.cpp \
    ../../client-app/records/executionaction.cpp \
    ../../client-app/records/campaign.cpp \
    ../../client-app/records/bug.cpp \
    ../../client-app/records/action.cpp
INCLUDEPATH += .. \
    ../../common \
    ../../client-lib \
    ../../client-app
OBJECTS_DIR += build
QMAKE_EXTENSION_SHLIB = cmo
QMAKE_LFLAGS += --export-dynamic
OTHER_FILES += 
release|Release:LIBS += -L../../client-lib/Release
LIBS += -lrtmr

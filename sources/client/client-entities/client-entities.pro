#-------------------------------------------------
#
# Project created by QtCreator 2012-12-27T17:30:59
#
#-------------------------------------------------

#QT       -= gui
QT += network \
    xml

TARGET = client-entities
TEMPLATE = lib
CONFIG += staticlib

include (../common/common.pri)
DEFINES += RTMR_LIBRARY_IMPORTS
DEFINES += AUTOMATION_LIB \
    AUTOMATION_LIBRARY_IMPORTS

LIBS += -lrtmr
Debug {
    wni32:QMAKE_CFLAGS_DEBUG += /Zi
    LIBS += -L../client-lib/Debug
}else {
    LIBS += -L../client-lib/Release
}

SOURCES += record.cpp \
    action.cpp \
    campaign.cpp \
    executionaction.cpp \
    executioncampaign.cpp \
    executionrequirement.cpp \
    executiontest.cpp \
    project.cpp \
    projectversion.cpp \
    requirement.cpp \
    requirementcontent.cpp \
    test.cpp \
    testcampaign.cpp \
    testcontent.cpp \
    testrequirement.cpp \
    user.cpp \
    genericrecord.cpp \
    projectgrant.cpp \
    testcontentfile.cpp \
    projectparameter.cpp \
    hierarchy.cpp \
    testhierarchy.cpp \
    requirementhierarchy.cpp \
    executioncampaignparameter.cpp \
    xmlprojectdatas.cpp \
    bug.cpp \
    parameter.cpp \
    executiontestparameter.cpp \
    automatedaction.cpp \
    customfielddesc.cpp \
    customfield.cpp \
    customtestfield.cpp \
    customrequirementfield.cpp \
    automatedactionvalidation.cpp \
    session.cpp \
    parent.cpp

HEADERS += genericrecord.h \
    projectgrant.h \
    testcontentfile.h \
    hierarchy.h \
    projectparameter.h \
    record.h \
    action.h \
    campaign.h \
    executionaction.h \
    executioncampaign.h \
    executionrequirement.h \
    executiontest.h \
    project.h \
    projectversion.h \
    requirement.h \
    requirementcontent.h \
    test.h \
    testcampaign.h \
    testcontent.h \
    testrequirement.h \
    user.h \
    testhierarchy.h \
    requirementhierarchy.h \
    executioncampaignparameter.h \
    xmlprojectdatas.h \
    bug.h \
    parameter.h \
    executiontestparameter.h \
    automatedaction.h \
    customfielddesc.h \
    customfield.h \
    customtestfield.h \
    customrequirementfield.h \
    automatedactionvalidation.h \
    session.h \
    parent.h

INCLUDEPATH +=  ../client-lib \
    ../client-modules \
    ../common

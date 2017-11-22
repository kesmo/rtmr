# -------------------------------------------------
# Project created by QtCreator 2009-08-22T07:23:20
# -------------------------------------------------
TARGET = rtmr
QT += xml network
DEFINES += APP_VERSION=\\\"1.11.0.0\\\"
DEFINES += NETSCAPE_PLUGIN

# DEFINES += AUTOMATION_LIB AUTOMATION_LIBRARY_IMPORTS
include (../common/common.pri)

include (./qtbrowserplugin/qtbrowserplugin.pri)

unix:CFLAGS += -fPIC

unix:target.path = $$BINDIR
mac:ICON = images/client.icns
win32:RC_FILE = npresources.rc
win32:DEFINES += _CRT_SECURE_NO_WARNINGS
SOURCES += records/genericrecord.cpp \
    gui/forms/form_user_projects_grants.cpp \
    records/projectgrant.cpp \
    gui/forms/form_bug.cpp \
    gui/forms/form_options.cpp \
    records/testcontentfile.cpp \
    gui/forms/form_executions_reports.cpp \
    objects/Trinome.cpp \
    objects/TrinomeArray.cpp \
    objects/DataObject.cpp \
    objects/Comparable.cpp \
    gui/forms/form_project_version.cpp \
    records/projectparameter.cpp \
    gui/components/record_text_edit_toolbar.cpp \
    gui/components/record_text_edit_widget.cpp \
    records/hierarchy.cpp \
    gui/components/record_text_edit.cpp \
    gui/components/record_text_edit_container.cpp \
    gui/components/records_table_widget.cpp \
    gui/components/records_tree_model.cpp \
    gui/components/records_tree_view.cpp \
    gui/forms/form_campaign.cpp \
    gui/forms/form_campaign_wizard.cpp \
    gui/forms/form_execution_campaign.cpp \
    gui/forms/form_execution_test.cpp \
    gui/forms/form_logon.cpp \
    gui/forms/form_manage_users.cpp \
    gui/forms/form_new_user.cpp \
    gui/forms/form_new_version.cpp \
    gui/forms/form_project.cpp \
    gui/forms/form_project_selection.cpp \
    gui/forms/form_requirement.cpp \
    gui/forms/form_test.cpp \
    gui/mainwindow.cpp \
    records/record.cpp \
    records/action.cpp \
    records/campaign.cpp \
    records/executionaction.cpp \
    records/executioncampaign.cpp \
    records/executionrequirement.cpp \
    records/executiontest.cpp \
    records/project.cpp \
    records/projectversion.cpp \
    records/requirement.cpp \
    records/requirementcontent.cpp \
    records/test.cpp \
    records/testcampaign.cpp \
    records/testcontent.cpp \
    records/testrequirement.cpp \
    records/user.cpp \
    imaging/GradientColor.cpp \
    imaging/Chart.cpp \
    imaging/Image.cpp \
    imaging/Rectangle3D.cpp \
    imaging/GraphicImage.cpp \
    gui/forms/form_projects_reports.cpp \
    gui/forms/form_search_project.cpp \
    gui/forms/form_change_password.cpp \
    records/testhierarchy.cpp \
    records/requirementhierarchy.cpp \
    records/executioncampaignparameter.cpp \
    records/xmlprojectdatas.cpp \
    records/bug.cpp \
    gui/forms/form_execution_bugs.cpp \
    records/parameter.cpp \
    records/executiontestparameter.cpp \
    gui/forms/form_data_import.cpp \
    gui/forms/form_project_bugs.cpp \
    gui/forms/form_data_export.cpp \
    ../client-launcher/process_utils.cpp \
    gui/components/test_action_attachments_manager.cpp \
    records/automatedaction.cpp \
    gui/components/abstract_project_widget.cpp \
    session.cpp
HEADERS += records/genericrecord.h \
    gui/forms/form_user_projects_grants.h \
    records/projectgrant.h \
    gui/forms/form_bug.h \
    gui/forms/form_options.h \
    records/testcontentfile.h \
    gui/forms/form_executions_reports.h \
    objects/Trinome.h \
    objects/TrinomeArray.h \
    objects/DataObject.h \
    objects/Comparable.h \
    gui/forms/form_project_version.h \
    records/hierarchy.h \
    gui/components/record_text_edit.h \
    gui/components/record_text_edit_container.h \
    gui/components/records_table_widget.h \
    gui/components/records_tree_model.h \
    gui/components/records_tree_view.h \
    gui/forms/form_campaign.h \
    gui/forms/form_campaign_wizard.h \
    gui/forms/form_execution_campaign.h \
    gui/forms/form_execution_test.h \
    gui/forms/form_logon.h \
    gui/forms/form_manage_users.h \
    gui/forms/form_new_user.h \
    gui/forms/form_new_version.h \
    gui/forms/form_project.h \
    gui/forms/form_project_selection.h \
    gui/forms/form_test.h \
    gui/mainwindow.h \
    gui/forms/form_requirement.h \
    gui/components/record_text_edit_toolbar.h \
    gui/components/record_text_edit_widget.h \
    records/projectparameter.h \
    records/record.h \
    records/action.h \
    records/campaign.h \
    records/executionaction.h \
    records/executioncampaign.h \
    records/executionrequirement.h \
    records/executiontest.h \
    records/project.h \
    records/projectversion.h \
    records/requirement.h \
    records/requirementcontent.h \
    records/test.h \
    records/testcampaign.h \
    records/testcontent.h \
    records/testrequirement.h \
    records/user.h \
    imaging/GradientColor.h \
    imaging/Chart.h \
    imaging/Chain.h \
    imaging/Image.h \
    imaging/Rectangle3D.h \
    imaging/GraphicImage.h \
    gui/forms/form_projects_reports.h \
    gui/forms/form_search_project.h \
    gui/forms/form_change_password.h \
    records/testhierarchy.h \
    records/requirementhierarchy.h \
    records/executioncampaignparameter.h \
    records/xmlprojectdatas.h \
    records/bug.h \
    gui/forms/form_execution_bugs.h \
    records/parameter.h \
    records/executiontestparameter.h \
    gui/forms/form_data_import.h \
    gui/forms/form_project_bugs.h \
    gui/forms/form_data_export.h \
    ../client-modules/clientmodule.h \
    ../client-launcher/process_utils.h \
    gui/components/test_action_attachments_manager.h \
    records/automatedaction.h \
    gui/components/abstract_project_widget.h \
    session.h
FORMS += gui/forms/Form_User_Projects_Grants.ui \
    gui/forms/Form_Bug.ui \
    gui/forms/Form_Options.ui \
    gui/forms/Form_Executions_Reports.ui \
    gui/forms/Form_Project_Version.ui \
    gui/forms/Form_Campaign.ui \
    gui/forms/Form_Campaign_Wizard.ui \
    gui/forms/Form_Execution_Action.ui \
    gui/forms/Form_Execution_Campaign.ui \
    gui/forms/Form_Execution_Test.ui \
    gui/forms/Form_Logon.ui \
    gui/forms/Form_Manage_Users.ui \
    gui/forms/Form_New_User.ui \
    gui/forms/Form_New_Version.ui \
    gui/forms/Form_Project.ui \
    gui/forms/Form_Project_Selection.ui \
    gui/forms/Form_Requirement.ui \
    gui/forms/Form_Test.ui \
    gui/forms/Form_Projects_Reports.ui \
    gui/forms/Form_Search_Project.ui \
    gui/forms/Form_Change_Password.ui \
    gui/forms/Form_Execution_Bugs.ui \
    gui/forms/Form_Data_Import.ui \
    gui/forms/Form_Project_Bugs.ui \
    gui/forms/Form_Data_Export.ui
INCLUDEPATH += ../client-lib \
    ../common \
    ../automation-lib \
    ../client-modules \
    ./records
LIBS += -lrtmr
release|Release:LIBS += -L../client-lib/Release
debug|Debug:LIBS += -L../client-lib/Debug
mac:LIBS += -L../client-lib
contains(DEFINES, AUTOMATION_LIB):win32:LIBS += -L../automation-lib/Release -lautomation-lib
win32:LIBS += -lpsapi -lUser32
win32:DEFINES += PSAPI_VERSION=1
mac:LIBS += -framework Carbon
unix:LIBS += -lproc
RESOURCES += resources.qrc
OBJECTS_DIR = build
MOC_DIR = build
OTHER_FILES += standard.css \
    resources.rc \
    rtmrapp.exe.manifest \
    npresources.rc

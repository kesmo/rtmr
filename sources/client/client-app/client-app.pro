# -------------------------------------------------
# Project created by QtCreator 2009-08-22T07:23:20
# -------------------------------------------------
TARGET = rtmrapp
QT += network \
    xml
TEMPLATE = app

DEFINES += RTMR_LIBRARY_IMPORTS
DEFINES += AUTOMATION_LIB
DEFINES += AUTOMATION_LIBRARY_IMPORTS

#DEFINES += ELEVATED_UAC


win32 {
    contains(DEFINES, ELEVATED_UAC) {
        CONFIG -= embed_manifest_exe
        Debug{
            QMAKE_POST_LINK += $$quote(mt.exe -nologo -manifest \"$${PWD}\\rtmrapp.exe.manifest\" -outputresource:\"$${PWD}\\Debug/rtmrapp.exe\")
        } else {
            QMAKE_POST_LINK += $$quote(mt.exe -nologo -manifest \"$${PWD}\\rtmrapp.exe.manifest\" -outputresource:\"$${PWD}\\Release/rtmrapp.exe\")
        }
    }
}
include (../common/common.pri)
unix:target.path = $$BINDIR
mac:ICON = images/client.icns
win32:RC_FILE = resources.rc
SOURCES += gui/forms/form_user_projects_grants.cpp \
    gui/forms/form_bug.cpp \
    gui/forms/form_options.cpp \
    gui/forms/form_executions_reports.cpp \
    objects/Trinome.cpp \
    objects/TrinomeArray.cpp \
    objects/DataObject.cpp \
    objects/Comparable.cpp \
    gui/forms/form_project_version.cpp \
    gui/components/record_text_edit_toolbar.cpp \
    gui/components/record_text_edit_widget.cpp \
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
    main.cpp \
    imaging/GradientColor.cpp \
    imaging/Chart.cpp \
    imaging/Image.cpp \
    imaging/Rectangle3D.cpp \
    imaging/GraphicImage.cpp \
    gui/forms/form_projects_reports.cpp \
    gui/forms/form_search_project.cpp \
    gui/forms/form_change_password.cpp \
    gui/forms/form_execution_bugs.cpp \
    gui/forms/form_data_import.cpp \
    gui/forms/form_project_bugs.cpp \
    gui/forms/form_data_export.cpp \
    ../client-launcher/process_utils.cpp \
    gui/components/test_action_attachments_manager.cpp \
    gui/components/abstract_project_widget.cpp \
    gui/components/records_table_model.cpp \
    gui/forms/form_manage_customfields.cpp \
    gui/forms/form_customfielddesc.cpp \
    gui/components/records_table_view.cpp \
    gui/components/custom_fields_controls_manager.cpp \
    gui/components/combobox_delegate.cpp \
    gui/components/records_table_view_delegate.cpp \
    gui/components/abstractdataselector.cpp

HEADERS += gui/forms/form_user_projects_grants.h \
    gui/forms/form_bug.h \
    gui/forms/form_options.h \
    gui/forms/form_executions_reports.h \
    objects/Trinome.h \
    objects/TrinomeArray.h \
    objects/DataObject.h \
    objects/Comparable.h \
    gui/forms/form_project_version.h \
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
    imaging/GradientColor.h \
    imaging/Chart.h \
    imaging/Chain.h \
    imaging/Image.h \
    imaging/Rectangle3D.h \
    imaging/GraphicImage.h \
    gui/forms/form_projects_reports.h \
    gui/forms/form_search_project.h \
    gui/forms/form_change_password.h \
    gui/forms/form_execution_bugs.h \
    gui/forms/form_data_import.h \
    gui/forms/form_project_bugs.h \
    gui/forms/form_data_export.h \
    ../client-modules/clientmodule.h \
    ../client-launcher/process_utils.h \
    gui/components/test_action_attachments_manager.h \
    gui/components/abstract_project_widget.h \
    gui/components/records_table_model.h \
    gui/forms/form_manage_customfields.h \
    gui/forms/form_customfielddesc.h \
    gui/components/records_table_view.h \
    gui/components/custom_fields_controls_manager.h \
    gui/components/combobox_delegate.h \
    gui/components/records_table_view_delegate.h \
    gui/components/abstractdataselector.h

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
    gui/forms/Form_Data_Export.ui \
    gui/forms/Form_Manage_CustomFields.ui \
    gui/forms/Form_CustomFieldDesc.ui
INCLUDEPATH += ../client-lib \
    ../common \
    ../automation-lib \
    ../client-modules \
    ../client-entities
LIBS += -L../client-entities -lrtmr -lclient-entities
Debug {
    LIBS += -L../client-lib/Debug \
        -L../automation-lib/Debug \
        -L../client-entities/Debug

    win32:PRE_TARGETDEPS += ../client-entities/Debug/client-entities.lib
    unix:PRE_TARGETDEPS += ../client-entities/libclient-entities.a

} else {
    LIBS += -L../client-lib/Release \
        -L../automation-lib/Release \
        -L../client-entities/Release

    win32:PRE_TARGETDEPS += ../client-entities/Release/client-entities.lib
    unix:PRE_TARGETDEPS += ../client-entities/libclient-entities.a

}
mac:LIBS += -L../client-lib
win32:contains(DEFINES, AUTOMATION_LIB) {
    LIBS += -lautomation-lib
    DEFINES += AUTOMATION_ACTIVATED
}
win32:LIBS += -lpsapi \
    -lUser32
win32:DEFINES += PSAPI_VERSION=1
mac:LIBS += -framework \
    Carbon
unix:LIBS += -lproc
RESOURCES += resources.qrc
OBJECTS_DIR = build
MOC_DIR = build
OTHER_FILES += standard.css \
    resources.rc \
    rtmrapp.exe.manifest

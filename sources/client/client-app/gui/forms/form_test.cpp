/*****************************************************************************
Copyright (C) 2012 Emmanuel Jorge ejorge@free.fr

This file is part of R.T.M.R.

R.T.M.R is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

R.T.M.R is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with R.T.M.R.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/

#include "form_test.h"
#include "ui_Form_Test.h"
#include "action.h"
#include "utilities.h"
#include "testrequirement.h"
#include "testcontent.h"
#include "testcontentfile.h"
#include "requirementhierarchy.h"
#include "session.h"
#include "form_bug.h"

#include <QtGui>
#include <QDomDocument>
#include <QDomNamedNodeMap>
#include <QDomNode>
#include <QDomNodeList>
#include <QProcess>
#include "automatedactionvalidation.h"
#include "gui/components/records_table_view_delegate.h"

#ifdef AUTOMATION_ACTIVATED
#include "WinUser.h"

static int freeWindowHierarchy(windows_hierarchy** in_window_hierarchy_list)
{
    if (in_window_hierarchy_list && (*in_window_hierarchy_list))
    {
        freeWindowHierarchy(&(*in_window_hierarchy_list)->m_child_window);
        free(*in_window_hierarchy_list);
        *in_window_hierarchy_list = NULL;
    }

    return NOERR;
}


int freeMessagesList(log_message_list** in_messages_list)
{
    if (in_messages_list && (*in_messages_list))
    {
        freeMessagesList(&(*in_messages_list)->m_next_message);
        freeWindowHierarchy(&(*in_messages_list)->m_window_hierarchy);
        free(*in_messages_list);
        *in_messages_list = NULL;
    }
    return NOERR;
}





int DefaultMessageCallback(HWND in_window_id, bool in_must_be_active_window, char* in_error_msg)
{
    HWND tmp_active_window = GetActiveWindow();

    LOG_TRACE(Session::instance()->getClientSession(), "%s : Active window id=%lu, current window id=%lu\n", __FUNCTION__, tmp_active_window, in_window_id);
    if (in_must_be_active_window && tmp_active_window != in_window_id)
    {
        WINDOWINFO tmp_window_info;
        CHAR tmp_window_class_name[1024];
        CHAR tmp_window_name[1024];
        CHAR tmp_cwindow_class_name[1024];
        CHAR tmp_cwindow_name[1024];

        if (GetWindowInfo(tmp_active_window, &tmp_window_info) == TRUE){
            GetClassNameA(tmp_active_window, tmp_window_class_name, 1024);
            GetWindowTextA(tmp_active_window, tmp_window_name, 1024);

            if (GetWindowInfo(in_window_id, &tmp_window_info) == TRUE){
                GetClassNameA(in_window_id, tmp_cwindow_class_name, 1024);
                GetWindowTextA(in_window_id, tmp_cwindow_name, 1024);

                sprintf(in_error_msg, "DefaultMessageCallback : target window %lu (class=%s name=%s) is not the active window %lu (class=%s name=%s).\n",
                        in_window_id,
                        tmp_cwindow_class_name,
                        tmp_cwindow_name,
                        tmp_active_window,
                        tmp_window_class_name,
                        tmp_window_name
                        );
            }else{
                sprintf(in_error_msg, "DefaultMessageCallback : target window %lu (unknow class/name) is not the active window %lu (class=%s name=%s).\n",
                        in_window_id,
                        tmp_active_window,
                        tmp_window_class_name,
                        tmp_window_name
                        );
            }
        }else{
            sprintf(in_error_msg, "DefaultMessageCallback : target window %lu (unknow class/name) is not the active window %lu (unknow class/name).\n",
                    in_window_id,
                    tmp_active_window
                    );
        }

        return -1;
    }

    return NOERR;
}


int RunMessageCallbacks(log_message_list* in_log_message, HWND in_window_id, char* in_error_msg)
{
    int tmp_return = NOERR;

    if (in_log_message && in_log_message->m_custom_data_ptr){
        AutomatedAction* tmp_automated_action = static_cast<AutomatedAction*>(in_log_message->m_custom_data_ptr);
        if (tmp_automated_action){
            QList<AutomatedActionValidation*> tmp_validations = tmp_automated_action->getChilds();

            ClientModule        *tmp_module = NULL;
            AutomationModule    *tmp_automation_module = NULL;
            AutomationCallbackFunction *tmp_callback = NULL;
            AutomatedActionValidation* tmp_validation = NULL;
            QMap < QString, ClientModule*>	tmp_modules_map = Session::instance()->externalsModules().value(ClientModule::AutomationPlugin);

            windows_hierarchy* tmp_current_window = in_log_message->m_window_hierarchy;
            while(tmp_current_window && tmp_current_window->m_window_id != in_window_id){
                tmp_current_window = tmp_current_window->m_child_window;
            }

            if (!tmp_validations.isEmpty()){
                for(int tmp_index = 0;  tmp_index < tmp_validations.count() && tmp_return == NOERR; ++tmp_index){

                    tmp_validation = tmp_validations[tmp_index];
                    tmp_module = tmp_modules_map[tmp_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_NAME)];
                    if (tmp_module){
                        if (tmp_module->getModuleVersion() >= QString(tmp_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_VERSION))){
                            tmp_automation_module = static_cast<AutomationModule*>(tmp_module);
                            if (tmp_automation_module){
                                QMap<QString, AutomationCallbackFunction*> tmp_module_functions_list = tmp_automation_module->getFunctionsMap();
                                tmp_callback = tmp_module_functions_list[tmp_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME)];
                                if (tmp_callback && tmp_callback->getEntryPoint()){
                                    tmp_return = tmp_callback->getEntryPoint()(in_window_id, tmp_current_window && tmp_current_window->m_is_active == TRUE, tmp_validation, in_error_msg, NULL);
                                }else{
                                    sprintf(in_error_msg, "RunMessageCallbacks : The function %s of the module %s is not available.\n",
                                            tmp_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME),
                                            tmp_module->getModuleName().toStdString().c_str());
                                    return -1;
                                }
                            }
                        }else{
                            sprintf(in_error_msg, "RunMessageCallbacks : The version %s of the module %s is too old (must be >= %s).\n",
                                    tmp_module->getModuleVersion().toStdString().c_str(),
                                    tmp_module->getModuleName().toStdString().c_str(),
                                    tmp_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_VERSION));
                            return -1;
                        }
                    }else{
                        sprintf(in_error_msg, "RunMessageCallbacks : There is no module named %s.\n",
                                tmp_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_NAME));
                        return -1;
                    }
                }
            }else{
                return DefaultMessageCallback(in_window_id, tmp_current_window && tmp_current_window->m_is_active == TRUE, in_error_msg);
            }
        }
    }

    return tmp_return;
}


int AutomationCallbackErrorFunction(const char* in_error_msg)
{
    QMessageBox::critical(NULL, Form_Test::tr("Erreur"), QString::fromLocal8Bit(in_error_msg));

    return NOERR;
}

#endif


QVariant FunctionMessageType(Record* record, int role){
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
        switch ((EventMessageType)atoi(record->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE))){
        case StartRecord:
            return Form_Test::tr("Début");
        case StopRecord:
            return Form_Test::tr("Fin");
        case Keyboard:
            return Form_Test::tr("Clavier");
        case Mouse:
            return Form_Test::tr("Souris");
        case None:
        default:
            return Form_Test::tr("Inconnu");
        }

        break;
    }

    return QVariant();
}


QString getKeyLabel(int scancode){

    int vkcode = 0;

#if (defined(_WINDOWS) || defined (WIN32))

    vkcode = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
    if (vkcode == 0)
        vkcode = scancode;

    // 0 to 9
    if (vkcode >= 0x30 && vkcode <= 0x39)
        return QString("[%1]").arg(QChar('0' + vkcode - 0x30));

    // 0 to 9 : numpad
    if (vkcode >= 0x60 && vkcode <= 0x69)
        return QString("[%1]").arg(QChar('0' + vkcode - 0x60));

    // A to Z
    if (vkcode >= 0x41 && vkcode <= 0x5A)
        return QString("[%1]").arg(QChar('A' + vkcode - 0x41));

    // F1 to F24
    if (vkcode >= VK_F1 && vkcode <= VK_F24)
        return QString("[F%1]").arg(QString::number(vkcode - VK_F1 + 1));

    switch(vkcode){
    case VK_BACK:
        return "[Backspace]";
    case VK_TAB:
        return "[Tabulation]";
    case VK_CLEAR:
        return "[Suppr]";
    case VK_RETURN:
        return "[Entrée]";
    case VK_SHIFT:
        return "[Majuscule]";
    case VK_LSHIFT:
        return "[Majuscule gauche]";
    case VK_RSHIFT:
        return "[Majuscule droite]";
    case VK_CONTROL:
        return "[Ctrl]";
    case VK_LCONTROL:
        return "[Ctrl gauche]";
    case VK_RCONTROL:
        return "[Ctrl droite]";
    case VK_MENU:
        return "[Alt]";
    case VK_LMENU:
        return "[Alt gauche]";
    case VK_RMENU:
        return "[Alt droite]";
    case VK_PAUSE:
        return "[Pause]";
    case VK_CAPITAL:
        return "[Majuscule]";
    case VK_ESCAPE:
        return "[Echap]";
    case VK_SPACE:
        return "[Espace]";
    case VK_PRIOR:
        return "[Page up]";
    case VK_NEXT:
        return "[Page down]";
    case VK_END:
        return "[Fin]";
    case VK_LEFT:
        return "[Flêche gauche]";
    case VK_UP:
        return "[Flêche haut]";
    case VK_RIGHT:
        return "[Flêche droite]";
    case VK_DOWN:
        return "[Flêche bas]";
    case VK_SELECT:
        return "[Select]";
    case VK_PRINT:
        return "[Impr]";
    case VK_INSERT:
        return "[Inser]";
    case VK_DELETE:
        return "[Suppr]";
    case VK_NUMLOCK:
        return "[Verr Num]";
    case VK_SCROLL:
        return "[Scroll]";

    }
#endif

    return QString("[%1]").arg(vkcode);
}


QVariant FunctionMessageData(Record* record, int role){
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
        switch ((EventMessageType)atoi(record->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE))){
        case Keyboard:
        case Mouse:
        {
            QStringList tmp_data_values = QString(record->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA)).split(AutomatedAction::FieldSeparator);
            if (tmp_data_values.count() == 3)
            {
                switch (atoi(tmp_data_values[0].toStdString().c_str()))
                {
                // Mouse
                case EVT_MSG_MOUSE_BUTTON_LEFT_DOWN:
                    return Form_Test::tr("Appuyer bouton gauche en x=%1 et y=%2").arg(tmp_data_values[1]).arg(tmp_data_values[2]);
                case EVT_MSG_MOUSE_BUTTON_LEFT_UP:
                    return Form_Test::tr("Relâcher bouton gauche en x=%1 et y=%2").arg(tmp_data_values[1]).arg(tmp_data_values[2]);
                case EVT_MSG_MOUSE_BUTTON_RIGHT_DOWN:
                    return Form_Test::tr("Appui bouton droit en x=%1 et y=%2").arg(tmp_data_values[1]).arg(tmp_data_values[2]);
                case EVT_MSG_MOUSE_BUTTON_RIGHT_UP:
                    return Form_Test::tr("Relâcher bouton droit en x=%1 et y=%2").arg(tmp_data_values[1]).arg(tmp_data_values[2]);
                case EVT_MSG_MOUSE_MOVE:
                    return Form_Test::tr("Déplacer en x=%1 et y=%2").arg(tmp_data_values[1]).arg(tmp_data_values[2]);

                    // Keyboard
                case EVT_MSG_KEYBOARD_KEY_DOWN:
                    return Form_Test::tr("Appuyer touche %1 (%2)").arg(getKeyLabel(tmp_data_values[1].toInt())).arg(tmp_data_values[2]);
                case EVT_MSG_KEYBOARD_KEY_UP:
                    return Form_Test::tr("Relâcher touche %1 (%2)").arg(getKeyLabel(tmp_data_values[1].toInt())).arg(tmp_data_values[2]);
                case EVT_MSG_KEYBOARD_SYSTEM_KEY_DOWN:
                    return Form_Test::tr("Appuyer touche système %1 (%2)").arg(getKeyLabel(tmp_data_values[1].toInt())).arg(tmp_data_values[2]);
                case EVT_MSG_KEYBOARD_SYSTEM_KEY_UP:
                    return Form_Test::tr("Relâcher touche système %1 (%2)").arg(getKeyLabel(tmp_data_values[1].toInt())).arg(tmp_data_values[2]);
                }
            }

            return record->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA);
            break;
        }
        case None:
        case StartRecord:
        case StopRecord:
        default:
            break;
        }

        break;
    }

    return QVariant();
}


QVariant FunctionMessageDelay(Record* record, int role){
    switch(role)
    {
    case Qt::DisplayRole:
    case Qt::EditRole:
    case Qt::ToolTipRole:
        int tmp_delay = atoi(record->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY));
        return Form_Test::tr("%1 sec %2 ms").arg(tmp_delay / 1000).arg(tmp_delay % 1000);
        break;
    }

    return QVariant();
}


QVariant FunctionMessageCallback(Record* record, int role){

    AutomatedAction* tmp_automated_action = dynamic_cast<AutomatedAction*>(record);
    AutomatedActionValidation* tmp_automated_action_validation = NULL;

    if (tmp_automated_action){
        QList<AutomatedActionValidation*> tmp_automated_action_validations = tmp_automated_action->getChilds();
        QString tmp_text;

        switch(role)
        {
        case Qt::DisplayRole:
        case Qt::EditRole:
            if (tmp_automated_action_validations.isEmpty()){
                tmp_text = "DefaultMessageCallback";
            }else{
                for(int tmp_index= 0; tmp_index < tmp_automated_action_validations.count(); ++tmp_index){
                    tmp_automated_action_validation = tmp_automated_action_validations[tmp_index];
                    if (tmp_index < tmp_automated_action_validations.count() - 1)
                    tmp_text += QString("%1\n")
                            .arg(tmp_automated_action_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME));
                    else
                        tmp_text += QString("%1")
                                .arg(tmp_automated_action_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME));
                }
            }
            return tmp_text;

        case Qt::ToolTipRole:
            if (tmp_automated_action_validations.isEmpty()){
                tmp_text = Form_Test::tr("<p><b>DefaultMessageCallback</b> : fonction par défaut qui vérifie que la fenêtre active est la même que celle enregistrée lors de l'exécution de l'action</p>");
            }else{
                foreach(AutomatedActionValidation* tmp_automated_action_validation, tmp_automated_action_validations){
                    QString tmp_module_name(tmp_automated_action_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_NAME));
                    QString tmp_module_version(tmp_automated_action_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_VERSION));
                    QString tmp_function_name(tmp_automated_action_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME));
                    AutomationCallbackFunction* tmp_callback = Session::instance()->getAutomationCallbackFunction(tmp_module_name, tmp_module_version, tmp_function_name);
                    tmp_text += Form_Test::tr("<p><b>%1</b> (module %2 version %3) : %4</p>")
                            .arg(tmp_function_name)
                            .arg(tmp_module_name)
                            .arg(tmp_module_version)
                            .arg(tmp_callback ? tmp_callback->getDescription() : Form_Test::tr("<i>pas de description</i>"));
                }
            }
            return tmp_text;
        }
    }

    return QVariant();
}


/**
  Constructeur
**/
Form_Test::Form_Test(QWidget *parent) :
    AbstractProjectWidget(parent),
    CustomFieldsControlsManager(),
    _m_ui(new Ui::Form_Test)
#ifdef AUTOMATION_ACTIVATED
    ,_m_automation_playback_messages(NULL)
#endif
{
    setAttribute(Qt::WA_DeleteOnClose);
    _m_modifiable = false;

    _m_ui->setupUi(this);

    _m_ui->test_description->addTextToolBar(RecordTextEditToolBar::Small);

    foreach(RequirementCategory *tmp_requiremetn_category, Session::instance()->requirementsCategories())
    {
        _m_ui->test_category->addItem(TR_CUSTOM_MESSAGE(tmp_requiremetn_category->getValueForKey(REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_LABEL)), tmp_requiremetn_category->getValueForKey(REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_ID));
    }

    foreach(TestType *tmp_test_type, Session::instance()->testsTypes())
    {
        _m_ui->test_type->addItem(TR_CUSTOM_MESSAGE(tmp_test_type->getValueForKey(TESTS_TYPES_TABLE_TEST_TYPE_LABEL)), tmp_test_type->getValueForKey(TESTS_TYPES_TABLE_TEST_TYPE_ID));
    }

    QList< QPair<QString, QString> >  tmp_automated_actions_table_headers;
    tmp_automated_actions_table_headers << qMakePair<QString, QString>(tr("Type de message"), AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE);
    tmp_automated_actions_table_headers << qMakePair<QString, QString>(tr("Données"), AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA);
    tmp_automated_actions_table_headers << qMakePair<QString, QString>(tr("Délai"), AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY);
    tmp_automated_actions_table_headers << qMakePair<QString, QString>(tr("Vérifications"), QString::null);

    _m_previous_test_content = NULL;
    _m_next_test_content = NULL;
    _m_test_content = NULL;
    _m_bugtracker = NULL;
//    _m_automated_program_record_process = NULL;
//    _m_automated_program_playback_process = NULL;
    _m_is_automated_program_recording = false;
    _m_is_automated_program_playbacking = false;
    _m_automated_actions_model = new RecordsTableModel<AutomatedAction>(tmp_automated_actions_table_headers, QList<AutomatedAction*>(), this);
    _m_automated_actions_model->setColumnDataFunctionForColumn(0, &FunctionMessageType);
    _m_automated_actions_model->setColumnDataFunctionForColumn(1, &FunctionMessageData);
    _m_automated_actions_model->setColumnDataFunctionForColumn(2, &FunctionMessageDelay);
    _m_automated_actions_model->setColumnDataFunctionForColumn(3, &FunctionMessageCallback);
    _m_ui->automated_actions_list->setModel(_m_automated_actions_model);
    _m_ui->automated_actions_list->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(_m_ui->automated_actions_list, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showAutomatedActionValidationContextMenu(QPoint)));
    _m_ui->automated_actions_list->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    _m_ui->automated_actions_list->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);

    connect(_m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(save()));
    connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));

    connect(_m_ui->test_name, SIGNAL(textChanged(const QString &)), this, SLOT(setModified()));
    connect(_m_ui->test_description->textEditor(), SIGNAL(textChanged()), this, SLOT(setModified()));
    connect(_m_ui->test_category, SIGNAL(currentIndexChanged(int)), this, SLOT(setModified()));
    connect(_m_ui->test_type, SIGNAL(currentIndexChanged(int)), this, SLOT(setModified()));
    connect(_m_ui->test_limit, SIGNAL(stateChanged(int)), this, SLOT(setModified()));
    connect(_m_ui->test_priority_level, SIGNAL(valueChanged(int)), this, SLOT(setModified()));
    connect(_m_ui->automation_command_line, SIGNAL(textChanged(const QString &)), this, SLOT(setModified()));
    connect(_m_ui->automation_command_parameters, SIGNAL(textChanged(const QString &)), this, SLOT(setModified()));

    connect(_m_ui->button_show_original_test, SIGNAL(clicked(bool)), this, SLOT(showOriginalTestInfosClicked()));

    connect(_m_ui->button_add, SIGNAL(clicked()), this, SLOT(addAction()));
    connect(_m_ui->button_del, SIGNAL(clicked()), this, SLOT(deleteSelectedAction()));
    connect(_m_ui->button_up, SIGNAL(clicked()), this, SLOT(moveSelectedActionUp()));
    connect(_m_ui->button_down, SIGNAL(clicked()), this, SLOT(moveSelectedActionDown()));

    connect(_m_ui->button_copy, SIGNAL(clicked()), this, SLOT(copySelectedActionsToClipboard()));
    connect(_m_ui->button_cut, SIGNAL(clicked()), this, SLOT(cutSelectedActionsToClipboard()));
    connect(_m_ui->button_paste, SIGNAL(clicked()), this, SLOT(pasteClipboardDataToActionsList()));
    connect(_m_ui->button_paste_plain_text, SIGNAL(clicked()), this, SLOT(pasteClipboardPlainTextToActionsList()));

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));

    connect(_m_ui->button_select_command_line, SIGNAL(clicked()), this, SLOT(selectExternalCommandForAutomation()));

#ifdef AUTOMATION_ACTIVATED
    _m_ui->button_start_record->setVisible(true);
    _m_ui->button_stop_record->setVisible(true);
    _m_ui->button_start_playback->setVisible(true);
    _m_ui->button_stop_playback->setVisible(true);
#else
    _m_ui->button_start_record->setVisible(false);
    _m_ui->button_stop_record->setVisible(false);
    _m_ui->button_start_playback->setVisible(false);
    _m_ui->button_stop_playback->setVisible(false);
#endif

    connect(_m_ui->button_start_record, SIGNAL(clicked()), this, SLOT(launchStartRecordSystemEvents()));
    connect(_m_ui->button_stop_record, SIGNAL(clicked()), this, SLOT(stopRecordProcess()));
    connect(_m_ui->button_start_playback, SIGNAL(clicked()), this, SLOT(launchStartPlaybackSystemEvents()));
    connect(_m_ui->button_stop_playback, SIGNAL(clicked()), this, SLOT(stopPlaybackProcess()));

    _m_ui->actions_list->setRemoveSelectedRowsOnKeypressEvent(false);
    connect(_m_ui->actions_list, SIGNAL(itemSelectionChanged()), this, SLOT(actionSelectionChanged()));
    connect(_m_ui->actions_list, SIGNAL(recordslistDrop(QList<Record*>, int)), this, SLOT(actionsDrop(QList<Record*>, int)));
    connect(_m_ui->actions_list, SIGNAL(lastRowAndColumnReach()), this, SLOT(addActionAfterLastAction()));
    connect(_m_ui->actions_list, SIGNAL(linesCopied()), this, SLOT(copySelectedActionsToClipboard()));
    connect(_m_ui->actions_list, SIGNAL(linesCut()), this, SLOT(cutSelectedActionsToClipboard()));
    connect(_m_ui->actions_list, SIGNAL(linesPaste()), this, SLOT(pasteClipboardDataToActionsList()));
    connect(_m_ui->actions_list, SIGNAL(linesPastePlainText()), this, SLOT(pasteClipboardPlainTextToActionsList()));

    connect(_m_ui->requirements_list, SIGNAL(recordslistDrop(QList<Record*>, int)), this, SLOT(requirementsDrop(QList<Record*>, int)));
    connect(_m_ui->requirements_list, SIGNAL(rowRemoved(int)), this, SLOT(deletedRequirementAtIndex(int)));
    connect(_m_ui->requirements_list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showRequirementAtIndex(QModelIndex)));

    connect(_m_ui->previous_version_button, SIGNAL(clicked()), this, SLOT(loadPreviousTestContent()));
    connect(_m_ui->next_version_button, SIGNAL(clicked()), this, SLOT(loadNextTestContent()));

    _m_ui->files_list->setMimeType("text/uri-list");
    connect(_m_ui->files_list, SIGNAL(urlsListDrop(QList<QString>,int)), this, SLOT(filesDrop(QList<QString>,int)));
    connect(_m_ui->files_list, SIGNAL(rowRemoved(int)), this, SLOT(deletedFileAtIndex(int)));
    connect(_m_ui->files_list, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(openFileAtIndex(QTableWidgetItem*)));

    connect(_m_ui->bugs_list, SIGNAL(rowRemoved(int)), this, SLOT(deletedBugAtIndex(int)));
    connect(_m_ui->bugs_list, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(openBugAtIndex(QTableWidgetItem*)));

    connect(_m_ui->tabs, SIGNAL(currentChanged(int)), this, SLOT(changeTab(int)));

    _m_ui->test_id->setVisible(Session::instance()->getClientSession()->m_debug);

    _m_ui->actions_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    loadPluginsViews();
    loadCustomFieldsView(_m_ui->tool_box, Session::instance()->customTestsFieldsDesc());
}


/**
  Destructeur
**/
Form_Test::~Form_Test()
{
    stopRecordProcess();
    stopPlaybackProcess();

    destroyPluginsViews();

    delete _m_ui;
    delete _m_previous_test_content;
    delete _m_next_test_content;
    delete _m_test_content;
    delete _m_automated_actions_model;


    if (_m_bugtracker != NULL)
    {
        ClientModule *tmp_bugtracker_module = Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).value(_m_test->projectVersion()->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE));
        if (tmp_bugtracker_module != NULL)
        {
            static_cast<BugtrackerModule*>(tmp_bugtracker_module)->destroyBugtracker(_m_bugtracker);
        }
    }

    qDeleteAll(_m_actions);
    qDeleteAll(_m_removed_actions);

    qDeleteAll(_m_requirements);
    qDeleteAll(_m_removed_requirements);

    qDeleteAll(_m_removed_files);

    qDeleteAll(_m_bugs);
    qDeleteAll(_m_removed_bugs);

    qDeleteAll(_m_custom_tests_fields);
}


void Form_Test::loadNextTestContent()
{
    TestContent	*tmp_test_content = _m_next_test_content;

    if (maybeClose())
    {
        if (_m_test_content != NULL
                && _m_next_test_content != NULL
                && compare_values(_m_test_content->getIdentifier(), _m_next_test_content->getIdentifier()) == 0)
            tmp_test_content = _m_test_content;

        loadTestContent(tmp_test_content);
    }
}


/**
  Charger un test
**/
void Form_Test::loadTest(TestHierarchy *in_test)
{
    QStringList             tmp_actions_headers;
    QStringList             tmp_requirements_headers;
    QStringList             tmp_bugs_headers;
    int                     tmp_load_content_result = NOERR;

    _m_ui->test_name->setFocus();

    _m_ui->test_description->textEditor()->setCompletionFromList(in_test->projectVersion()->project()->parametersNames());

    tmp_actions_headers << tr("Description") << tr("Résultat attendu");
    _m_ui->actions_list->setHorizontalHeaderLabels(tmp_actions_headers);
    _m_ui->actions_list->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    _m_ui->automated_actions_list->horizontalHeader()->setStretchLastSection(true);

    tmp_requirements_headers << tr("Description") << tr("Catégorie") << tr("Version");
    _m_ui->requirements_list->setHorizontalHeaderLabels(tmp_requirements_headers);
    _m_ui->requirements_list->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

    tmp_bugs_headers << tr("Date") << tr("Résumé") << tr("Priorité") << tr("Sévérité") << tr("Id. externe");
    _m_ui->bugs_list->setHorizontalHeaderLabels(tmp_bugs_headers);
    _m_ui->bugs_list->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

    _m_ui->lock_widget->setVisible(false);

    _m_test = in_test;
    if (_m_test != NULL)
    {
        setWindowTitle(tr("Scénario : %1").arg(_m_test->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)));

        if (_m_bugtracker == NULL)
        {
            if (is_empty_string(_m_test->projectVersion()->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST)) == FALSE)
            {
                ClientModule *tmp_bugtracker_module = Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).value(_m_test->projectVersion()->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE));
                if (tmp_bugtracker_module != NULL)
                {
                    _m_bugtracker = static_cast<BugtrackerModule*>(tmp_bugtracker_module)->createBugtracker();
                }
            }
        }

        if (_m_bugtracker != NULL)
            _m_bugtracker->setBaseUrl(QUrl(_m_test->projectVersion()->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST)));

        _m_ui->test_id->setText(QString(_m_test->getIdentifier()) + "(" + QString(_m_test->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID)) + ")");

        if (_m_test_content != NULL)
            delete _m_test_content;

        _m_test_content = new TestContent(_m_test->projectVersion());
        if (_m_test->original() != NULL)
            tmp_load_content_result = _m_test_content->loadRecord(_m_test->original()->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID));
        else
            tmp_load_content_result = _m_test_content->loadRecord(_m_test->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID));

        if (tmp_load_content_result == NOERR)
        {
            loadTestContent(_m_test_content);
            actionSelectionChanged();
        }
        else
        {
            QMessageBox::critical(this, tr("Erreur lors de chargement de l'enregistrement"), Session::instance()->getErrorMessage(tmp_load_content_result));
            cancel();
        }
    }

}


void Form_Test::loadTestContent(TestContent *in_test_content)
{
    Action               	*tmp_action = NULL, *tmp_associated_action = NULL;
    int                     tmp_actions_index = 0;
    QList<Action*>			tmp_actions_list;

    TestRequirement      *tmp_test_requirement = NULL;
    int                     tmp_requirements_index = 0;

    TestContentFile      *tmp_file = NULL;
    int                     tmp_files_index = 0;

    Bug		          *tmp_bug = NULL;
    int                     tmp_bugs_index = 0;

    _m_modifiable = false;

    if (_m_test->original() == NULL && in_test_content == _m_test_content)
    {
        if (_m_test->isWritable())
        {
            _m_test_content->lockRecord(true);
            if (_m_test_content->lockRecordStatus() == RECORD_STATUS_LOCKED)
            {
                _m_ui->lock_widget->setVisible(true);
                net_get_field(NET_MESSAGE_TYPE_INDEX+1, Session::instance()->getClientSession()->m_response, Session::instance()->getClientSession()->m_column_buffer, SEPARATOR_CHAR);
                _m_ui->label_lock_by->setText(tr("Verrouillé par ") + QString(Session::instance()->getClientSession()->m_column_buffer));
            }
            else
                _m_modifiable = true;
        }
    }

    _m_ui->previous_version_button->setVisible(false);
    _m_ui->next_version_button->setVisible(false);

    for (tmp_actions_index = 0; tmp_actions_index < _m_ui->actions_list->rowCount(); tmp_actions_index++)
    {
        removeActionWidgetsAtIndex(tmp_actions_index);
    }
    _m_ui->actions_list->setRowCount(0);
    _m_ui->requirements_list->setRowCount(0);
    _m_ui->files_list->setRowCount(0);
    _m_ui->bugs_list->setRowCount(0);

    qDeleteAll(_m_actions);
    _m_actions.clear();
    qDeleteAll(_m_removed_actions);
    _m_removed_actions.clear();

    qDeleteAll(_m_requirements);
    _m_requirements.clear();
    qDeleteAll(_m_removed_requirements);
    _m_removed_requirements.clear();


    qDeleteAll(_m_removed_files);
    _m_removed_files.clear();

    qDeleteAll(_m_bugs);
    _m_bugs.clear();
    qDeleteAll(_m_removed_bugs);
    _m_removed_bugs.clear();

    qDeleteAll(_m_custom_tests_fields);
    _m_custom_tests_fields.clear();

    destroyAttachments();

    if (in_test_content != NULL)
    {
        _m_previous_test_content = in_test_content->previousTestContent();
        _m_next_test_content = in_test_content->nextTestContent();

        _m_ui->previous_version_button->setVisible(_m_previous_test_content != NULL);
        _m_ui->next_version_button->setVisible(_m_next_test_content != NULL);

        // Charger les pièces jointes (avant les actions)
        _m_files = in_test_content->loadFiles();
        if (_m_files.count() > 0)
        {
            _m_ui->files_list->setRowCount(_m_files.count());
            for (tmp_files_index = 0; tmp_files_index < _m_files.count(); tmp_files_index++)
            {
                tmp_file = _m_files.at(tmp_files_index);
                setFileForRow(tmp_file, tmp_files_index);
            }
        }

        // Charger les actions associees
        if (_m_test_content->isAutomatedTest())
        {
            tmp_actions_index = 0;
            _m_automated_actions_model->setRecordsList(in_test_content->loadAutomatedActions());
            _m_ui->automated_actions_list->resizeColumnsToContents();
        }
        else
        {
            tmp_actions_list = in_test_content->loadActions();
            if (tmp_actions_list.count() > 0)
            {
                tmp_actions_index = 0;
                foreach (tmp_action, tmp_actions_list)
                {
                    _m_actions.append(tmp_action);
                    _m_ui->actions_list->insertRow(tmp_actions_index);
                    setActionAtIndex(tmp_action, tmp_actions_index);
                    tmp_actions_index++;

                    tmp_action->loadAssociatedActionsForVersion(_m_test->getValueForKey(TESTS_HIERARCHY_VERSION));
                    foreach (tmp_associated_action, tmp_action->associatedTestActions())
                    {
                        _m_actions.append(tmp_associated_action);
                        _m_ui->actions_list->insertRow(tmp_actions_index);
                        setActionAtIndex(tmp_associated_action, tmp_actions_index);
                        tmp_actions_index++;
                    }
                }
            }
        }


        // Charger les exigences associees
        _m_requirements = in_test_content->loadTestRequirementsForProjectVersion(_m_test->projectVersion());
        if (_m_requirements.count() > 0)
        {
            _m_ui->requirements_list->setRowCount(_m_requirements.count());
            for (tmp_requirements_index = 0; tmp_requirements_index < _m_requirements.count(); tmp_requirements_index++)
            {
                tmp_test_requirement = _m_requirements[tmp_requirements_index];
                setRequirementForRow(tmp_test_requirement, tmp_requirements_index);
            }
            _m_ui->requirements_list->resizeColumnsToContents();
        }

        // Charger les anomalies
        _m_bugs = _m_test->loadBugs();
        if (_m_bugs.count() > 0)
        {
            _m_ui->bugs_list->setRowCount(_m_bugs.count());
            for (tmp_bugs_index = 0; tmp_bugs_index < _m_bugs.count(); tmp_bugs_index++)
            {
                tmp_bug = _m_bugs.at(tmp_bugs_index);
                setBugForRow(tmp_bug, tmp_bugs_index);
            }
            _m_ui->bugs_list->resizeColumnsToContents();
        }

        // Charger les champs personnalises
        _m_custom_tests_fields = _m_test->loadCustomFields();
        QList<CustomFieldDesc*> customFieldsDesc = Session::instance()->customTestsFieldsDesc();

        foreach(CustomFieldDesc* customFieldDesc, customFieldsDesc)
        {
            bool foundTestField = false;
            foreach(CustomTestField* customTestField, _m_custom_tests_fields)
            {
                if (compare_values(customTestField->getValueForKey(CUSTOM_TEST_FIELDS_TABLE_CUSTOM_FIELD_DESC_ID),
                                   customFieldDesc->getIdentifier()) == 0)
                {
                    customTestField->setFieldDesc(customFieldDesc);
                    customTestField->setTestContent(_m_test_content);
                    popCustomFieldValue(customFieldDesc, customTestField, CUSTOM_TEST_FIELDS_TABLE_FIELD_VALUE, _m_modifiable);
                    foundTestField = true;
                    break;
                }
            }

            if (!foundTestField)
            {
                CustomTestField* customTestField = new CustomTestField();
                customTestField->setFieldDesc(customFieldDesc);
                customTestField->setTestContent(_m_test_content);
                customTestField->setValueForKey(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE), CUSTOM_TEST_FIELDS_TABLE_FIELD_VALUE);
                popCustomFieldValue(customFieldDesc, customTestField, CUSTOM_TEST_FIELDS_TABLE_FIELD_VALUE, _m_modifiable);
                _m_custom_tests_fields.append(customTestField);
            }
        }

        _m_ui->version_label->setText(ProjectVersion::formatProjectVersionNumber(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_VERSION)));

        if (is_empty_string(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_CATEGORY_ID)) == FALSE)
        {
            _m_ui->test_category->setCurrentIndex(_m_ui->test_category->findData(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_CATEGORY_ID)));
        }
        else
            _m_ui->test_category->setCurrentIndex(0);

        if (is_empty_string(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_TYPE)) == FALSE)
        {
            _m_ui->test_type->setCurrentIndex(_m_ui->test_type->findData(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_TYPE)));
        }
        else
            _m_ui->test_type->setCurrentIndex(0);

        _m_ui->test_limit->setChecked(compare_values(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE), YES) == 0);


    }

    _m_ui->title->setText(tr("Scénario : %1").arg(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_SHORT_NAME)));
    _m_ui->test_name->setText(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_SHORT_NAME));
    _m_ui->test_description->textEditor()->setHtml(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION));
    _m_ui->test_priority_level->setValue(QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_PRIORITY_LEVEL)).toInt());
    _m_ui->automation_command_line->setText(QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND)));
    _m_ui->automation_command_parameters->setText(QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND_PARAMETERS)));
    _m_modified = false;

    updateControls(_m_test->original() != NULL);
}


/**
  Modifier la saisie sur les controles si le test est une copie
**/
void Form_Test::updateControls(bool in_original_test_indic)
{
    if (_m_ui->test_name->text().isEmpty())
        _m_ui->tool_box->setCurrentWidget(_m_ui->page_informations_detaillees);
    else if (_m_test_content->isAutomatedTest() && _m_ui->automation_command_line->text().isEmpty())
        _m_ui->tool_box->setCurrentWidget(_m_ui->page_automatisation);
    else
        _m_ui->tool_box->setCurrentWidget(_m_ui->page_actions);

    _m_ui->button_show_original_test->setVisible(in_original_test_indic);

    _m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(_m_modifiable);

    _m_ui->test_priority_level->setDisabled(!_m_modifiable);
    _m_ui->requirements_list->setDisabled(!_m_modifiable);
    _m_ui->files_list->setDisabled(!_m_modifiable);
    _m_ui->bugs_list->setDisabled(!_m_modifiable);
    _m_ui->button_add->setDisabled(!_m_modifiable);
    _m_ui->button_del->setDisabled(!_m_modifiable);
    _m_ui->button_up->setDisabled(!_m_modifiable);
    _m_ui->button_down->setDisabled(!_m_modifiable);
    _m_ui->test_name->setReadOnly(!_m_modifiable);
    _m_ui->test_description->textEditor()->setReadOnly(!_m_modifiable);
    _m_ui->test_category->setEnabled(_m_modifiable);
    _m_ui->test_type->setEnabled(_m_modifiable);
    _m_ui->test_limit->setEnabled(_m_modifiable);
    _m_ui->automation_command_line->setEnabled(_m_modifiable);
    _m_ui->automation_command_parameters->setEnabled(_m_modifiable);

    // Test automatisé
    if (_m_test_content->isAutomatedTest())
    {
        if (_m_ui->tool_box->widget(1) != _m_ui->page_automatisation)
            _m_ui->tool_box->insertItem(1, _m_ui->page_automatisation, tr("Automatisation"));

        _m_ui->actions_widget->setVisible(false);
        _m_ui->automation_widget->setVisible(true);
        _m_ui->page_automatisation->setVisible(true);
        _m_ui->test_pixmap_label->setPixmap(QPixmap(":/images/48x48/pellicule_auto.png"));

        if (_m_automated_actions_model->rowCount() > 0)
            _m_ui->tabs->setTabText(0, tr("Actions automatisées (%1)").arg(QString::number(_m_automated_actions_model->rowCount())));
        else
            _m_ui->tabs->setTabText(0, tr("Actions automatisées"));

        _m_ui->button_start_playback->setEnabled(_m_automated_actions_model->rowCount() > 0);
    }
    else
    {
        _m_ui->actions_widget->setVisible(true);
        _m_ui->automation_widget->setVisible(false);
        _m_ui->page_automatisation->setVisible(false);
        _m_ui->test_pixmap_label->setPixmap(QPixmap(":/images/48x48/pellicule.png"));

        if (_m_ui->tool_box->widget(1) == _m_ui->page_automatisation)
            _m_ui->tool_box->removeItem(1);

        if (_m_actions.count() > 0)
            _m_ui->tabs->setTabText(0, tr("Actions (%1)").arg(QString::number(_m_actions.count())));
        else
            _m_ui->tabs->setTabText(0, tr("Actions"));
    }

    if (_m_requirements.count() > 0)
        _m_ui->tabs->setTabText(1, tr("Exigences (%1)").arg(QString::number(_m_requirements.count())));
    else
        _m_ui->tabs->setTabText(1, tr("Exigences"));

    if (_m_files.count() > 0)
        _m_ui->tabs->setTabText(2, tr("Pièces jointes (%1)").arg(QString::number(_m_files.count())));
    else
        _m_ui->tabs->setTabText(2, tr("Pièces jointes"));

    if (_m_bugs.count() > 0)
        _m_ui->tabs->setTabText(3, tr("Anomalies (%1)").arg(QString::number(_m_bugs.count())));
    else
        _m_ui->tabs->setTabText(3, tr("Anomalies"));

    if (_m_modifiable == false)
    {
        _m_ui->test_description->toolBar()->hide();
        _m_ui->actions_text_toolbar->hide();
    }
    else
    {
        _m_ui->test_description->toolBar()->show();
        _m_ui->actions_text_toolbar->show();
    }

    actionSelectionChanged();
}


void Form_Test::save()
{
    if (saveTest())
    {
        savePluginsDatas();
        _m_modified = false;
        emit testSaved(_m_test);

        // Tout recharger
        loadTestContent(_m_test_content);
    }
}


/**
  Enregistrer le test en cours de modification
**/
bool Form_Test::saveTest()
{
    char                tmp_priority_level[32];
    Action		*tmp_action = NULL;
    AutomatedAction		*tmp_automated_action = NULL;
    TestRequirement	 *tmp_test_requirement = NULL;
    TestRequirement	*tmp_new_test_requirement = NULL;
    TestContentFile	*tmp_file = NULL;
    Bug			*tmp_bug = NULL;
    int                 tmp_compare_versions_result = 0;

    QMessageBox         *tmp_msg_box;
    QPushButton         *tmp_update_button;
    QPushButton         *tmp_conserv_button;
    QPushButton         *tmp_cancel_button;

    TestContent		*tmp_old_content = NULL;

    bool                tmp_upgrade_version = false;

    int                 tmp_save_result = NOERR;

    const char		*tmp_previous_action_id = NULL;

    QVariant		tmp_test_category;
    QVariant		tmp_test_type;

    CustomTestField		*tmp_custom_test_field = NULL;

    // Mettre a jour le contenu du test
    tmp_compare_versions_result = compare_values(_m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_VERSION), _m_test->getValueForKey(TESTS_HIERARCHY_VERSION));
    if (tmp_compare_versions_result < 0)
    {
        // La version du contenu du test est antérieure a la version du test
        tmp_msg_box = new QMessageBox(QMessageBox::Question, tr("Confirmation..."),
                                      tr("La version du contenu du cas de test (%1) est antérieure à la version courante (%2).\nVoulez-vous mettre à niveau la version du contenu vers la version courante ?").arg(ProjectVersion::formatProjectVersionNumber(_m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_VERSION))).arg(ProjectVersion::formatProjectVersionNumber(_m_test->getValueForKey(TESTS_HIERARCHY_VERSION))));
        tmp_update_button = tmp_msg_box->addButton(tr("Mettre à niveau"), QMessageBox::YesRole);
        tmp_conserv_button = tmp_msg_box->addButton(tr("Conserver la version"), QMessageBox::NoRole);
        tmp_cancel_button = tmp_msg_box->addButton(tr("Annuler"), QMessageBox::RejectRole);

        tmp_msg_box->exec();
        if (tmp_msg_box->clickedButton() == tmp_update_button)
        {
            delete tmp_msg_box;
            tmp_upgrade_version = true;
            tmp_old_content = _m_test_content;
            _m_test_content = tmp_old_content->copy();
        }
        else if(tmp_msg_box->clickedButton() == tmp_cancel_button)
        {
            delete tmp_msg_box;
            return false;
        }
    }

    _m_test_content->setValueForKey(_m_ui->test_name->text().toStdString().c_str(), TESTS_CONTENTS_TABLE_SHORT_NAME);
    _m_test_content->setValueForKey(_m_ui->test_description->textEditor()->toHtml().toStdString().c_str(), TESTS_CONTENTS_TABLE_DESCRIPTION);
    sprintf(tmp_priority_level, "%i", _m_ui->test_priority_level->value());
    _m_test_content->setValueForKey(tmp_priority_level, TESTS_CONTENTS_TABLE_PRIORITY_LEVEL);
#ifdef AUTOMATION_ACTIVATED
    _m_test_content->setValueForKey(_m_ui->automation_command_line->text().toStdString().c_str(), TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND);
    _m_test_content->setValueForKey(_m_ui->automation_command_parameters->text().toStdString().c_str(), TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND_PARAMETERS);
#endif

    tmp_test_type =_m_ui->test_type->itemData(_m_ui->test_type->currentIndex());
    if (tmp_test_type.isValid())
        _m_test_content->setValueForKey(tmp_test_type.toString().toStdString().c_str(), TESTS_CONTENTS_TABLE_TYPE);

    tmp_test_category =_m_ui->test_category->itemData(_m_ui->test_category->currentIndex());
    if (tmp_test_category.isValid())
        _m_test_content->setValueForKey(tmp_test_category.toString().toStdString().c_str(), TESTS_CONTENTS_TABLE_CATEGORY_ID);

    if (_m_ui->test_limit->isChecked())
        _m_test_content->setValueForKey(YES, TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE);
    else
        _m_test_content->setValueForKey(NO, TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE);


    tmp_save_result = cl_transaction_start(Session::instance()->getClientSession());

    if (tmp_save_result == NOERR)
        tmp_save_result = _m_test_content->saveRecord();

    if (tmp_save_result == NOERR)
    {
        _m_test->setDataFromTestContent(_m_test_content);
        tmp_save_result = _m_test->saveRecord();
    }

    if (!tmp_upgrade_version)
    {
        // Mettre a jour les actions a supprimer
        for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_removed_actions.count(); tmp_index++)
        {
            tmp_action = _m_removed_actions[tmp_index];
            if (tmp_action != NULL && compare_values(tmp_action->getValueForKey(ACTIONS_TABLE_TEST_CONTENT_ID), _m_test_content->getIdentifier()) == 0)
            {
                tmp_save_result = tmp_action->deleteRecord();
            }
        }

#ifdef AUTOMATION_ACTIVATED
        _m_automated_actions_model->submit();
#endif

        // Mettre a jour les exigences a supprimer
        for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_removed_requirements.count(); tmp_index++)
        {
            tmp_test_requirement = _m_removed_requirements[tmp_index];
            if (tmp_test_requirement != NULL)
            {
                tmp_save_result = tmp_test_requirement->deleteRecord();
            }
        }


        // Mettre a jour les pièces jointes a supprimer
        for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_removed_files.count(); tmp_index++)
        {
            tmp_file = _m_removed_files[tmp_index];
            if (tmp_file != NULL)
            {
                tmp_save_result = tmp_file->deleteRecord();
            }
        }


        // Mettre a jour les anomalies a supprimer
        for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_removed_bugs.count(); tmp_index++)
        {
            tmp_bug = _m_removed_bugs[tmp_index];
            if (tmp_bug != NULL)
            {
                tmp_save_result = tmp_bug->deleteRecord();
            }
        }

        // Mettre a jour les anomalies existantes
        for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_bugs.count(); tmp_index++)
        {
            tmp_bug = _m_bugs[tmp_index];
            if (tmp_bug != NULL)
            {
                tmp_save_result = tmp_bug->saveRecord();
            }
        }
    }
    else
    {
#ifdef AUTOMATION_ACTIVATED

        // Mettre a jour les actions automatisees
        tmp_previous_action_id = NULL;
        for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_automated_actions_model->rowCount(); tmp_index++)
        {
            tmp_automated_action = _m_automated_actions_model->recordAtIndex(tmp_index);
            if (tmp_automated_action != NULL){
                tmp_automated_action = tmp_automated_action->copy(_m_test_content);

                tmp_automated_action->setValueForKey(tmp_previous_action_id, AUTOMATED_ACTIONS_TABLE_PREVIOUS_ACTION_ID);

                tmp_save_result = tmp_automated_action->saveRecord();
                tmp_previous_action_id = tmp_automated_action->getIdentifier();
            }
        }
#endif
    }


    // Mettre a jour les exigences
    for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_requirements.count(); tmp_index++)
    {
        tmp_test_requirement = _m_requirements[tmp_index];
        if (tmp_test_requirement != NULL)
        {
            if (tmp_upgrade_version)
            {
                tmp_new_test_requirement = new TestRequirement;
                tmp_new_test_requirement->setValueForKey(_m_test_content->getIdentifier(), TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID);
                tmp_new_test_requirement->setValueForKey(tmp_test_requirement->getValueForKey(TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID), TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);
                tmp_save_result = tmp_new_test_requirement->saveRecord();
            }
            else
            {
                tmp_save_result = tmp_test_requirement->saveRecord();
            }
        }
    }

    // Enregistrer les pieces jointes apres le commit de la transaction
    // car la creation des blobs côtés serveur est déjà faite dans une transaction
    for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_files.count(); tmp_index++)
    {
        tmp_file = _m_files[tmp_index];
        if (tmp_file != NULL)
        {
            if (tmp_upgrade_version){
                tmp_file = tmp_file->copy(_m_test_content);
            }

            tmp_save_result = tmp_file->saveRecord();
        }
    }

    // Mettre a jour les actions
    tmp_previous_action_id = NULL;
    for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_actions.count(); tmp_index++)
    {
        tmp_action = _m_actions[tmp_index];
        if (tmp_action != NULL
                && (compare_values(tmp_action->getValueForKey(ACTIONS_TABLE_TEST_CONTENT_ID), _m_test_content->getIdentifier()) == 0
                    || (tmp_upgrade_version && tmp_old_content != NULL && compare_values(tmp_action->getValueForKey(ACTIONS_TABLE_TEST_CONTENT_ID), tmp_old_content->getIdentifier()) == 0)))
        {
            if (tmp_upgrade_version)
                tmp_action = tmp_action->copy(_m_test_content);

            tmp_action->setValueForKey(tmp_previous_action_id, ACTIONS_TABLE_PREVIOUS_ACTION_ID);

            // Gestion des resources (images)
            // Colonne description
            tmp_save_result = saveTextEditAttachments(_m_test_content, textEditAt(tmp_index, 0));
            if (tmp_save_result == NOERR)
            {
                // Colonne resultat attendu
                tmp_save_result = saveTextEditAttachments(_m_test_content, textEditAt(tmp_index, 1));
                if (tmp_save_result == NOERR)
                {
                    writeActionDataFromTextEditAtIndex(tmp_action, tmp_index);

                    tmp_save_result = tmp_action->saveRecord();
                    tmp_previous_action_id = tmp_action->getIdentifier();
                }
            }
        }
    }


    if (tmp_old_content != NULL)
        delete tmp_old_content;

    // Mettre a jour les champs personnalises
    for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_custom_tests_fields.count(); tmp_index++)
    {
        tmp_custom_test_field = _m_custom_tests_fields[tmp_index];
        if (tmp_custom_test_field != NULL)
        {
            if (tmp_upgrade_version)
                tmp_custom_test_field = tmp_custom_test_field->copy(_m_test_content);

            tmp_save_result = pushEnteredCustomFieldValue(tmp_custom_test_field->getFieldDesc(), tmp_custom_test_field, CUSTOM_TEST_FIELDS_TABLE_FIELD_VALUE);
            if (tmp_save_result == EMPTY_OBJECT)
            {
                QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), tr("Le champ personnalisé <b>%1</b> de l'onglet <b>%2</b> est obligatoire.")
                                      .arg(tmp_custom_test_field->getFieldDesc()->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_LABEL))
                                      .arg(tmp_custom_test_field->getFieldDesc()->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME)));
                return false;
            }
            else if (tmp_save_result == NOERR)
                tmp_save_result = tmp_custom_test_field->saveRecord();
        }
    }


    if (tmp_save_result == NOERR)
    {
        tmp_save_result = cl_transaction_commit(Session::instance()->getClientSession());
    }

    if (tmp_save_result != NOERR)
    {
        cl_transaction_rollback(Session::instance()->getClientSession());
        QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_save_result));
        return false;
    }

    qDeleteAll(_m_removed_actions);
    _m_removed_actions.clear();

    qDeleteAll(_m_removed_requirements);
    _m_removed_requirements.clear();

    qDeleteAll(_m_removed_files);
    _m_removed_files.clear();

    qDeleteAll(_m_removed_bugs);
    _m_removed_bugs.clear();

    _m_ui->version_label->setText(ProjectVersion::formatProjectVersionNumber(_m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_VERSION)));

    return true;
}


void Form_Test::cancel()
{
    QWidget::close();
}


void Form_Test::setActionAtIndex(Action *in_action, int in_row)
{
    QModelIndex         tmp_first_column_index = _m_ui->actions_list->model()->index(in_row, 0);
    QModelIndex         tmp_second_column_index = _m_ui->actions_list->model()->index(in_row, 1);

    RecordTextEdit    *tmp_first_column_content = NULL;
    RecordTextEdit    *tmp_second_column_content = NULL;

    QTableWidgetItem	*tmp_widget_item = NULL;

    bool		tmp_action_modifiable = true;

    removeActionWidgetsAtIndex(in_row);

    if (is_empty_string(in_action->getValueForKey(ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID)))
    {
        tmp_first_column_content = new RecordTextEdit;
        tmp_second_column_content = new RecordTextEdit;

        tmp_action_modifiable = _m_modifiable && (is_empty_string(in_action->getIdentifier()) || (compare_values(_m_test_content->getIdentifier(), in_action->getValueForKey(ACTIONS_TABLE_TEST_CONTENT_ID)) == 0));

        tmp_first_column_content->setCompletionFromList(_m_test->projectVersion()->project()->parametersNames());
        tmp_first_column_content->setHtml(in_action->getValueForKey(ACTIONS_TABLE_DESCRIPTION));
        tmp_first_column_content->setTabChangesFocus(true);
        _m_ui->actions_text_toolbar->connectToEditor(tmp_first_column_content);
        tmp_first_column_content->setReadOnly(!tmp_action_modifiable);
        // Charger les resources
        loadTextEditAttachments(tmp_first_column_content);

        tmp_second_column_content->setCompletionFromList(_m_test->projectVersion()->project()->parametersNames());
        tmp_second_column_content->setHtml(in_action->getValueForKey(ACTIONS_TABLE_WAIT_RESULT));
        tmp_second_column_content->setTabChangesFocus(true);
        _m_ui->actions_text_toolbar->connectToEditor(tmp_second_column_content);
        tmp_second_column_content->setReadOnly(!tmp_action_modifiable);

        _m_ui->actions_list->setIndexWidget(tmp_first_column_index, tmp_first_column_content);
        _m_ui->actions_list->setIndexWidget(tmp_second_column_index, tmp_second_column_content);
        tmp_first_column_content->setRecord(in_action);
        tmp_second_column_content->setRecord(in_action);
        // Charger les resources
        loadTextEditAttachments(tmp_second_column_content);

        connect(tmp_first_column_content, SIGNAL(focused(Record*)), this, SLOT(selectFirstColumnAction(Record*)));
        connect(tmp_first_column_content, SIGNAL(textChanged()), this, SLOT(actionModified()));
        connect(tmp_second_column_content, SIGNAL(focused(Record*)), this, SLOT(selectSecondColumnAction(Record*)));
        connect(tmp_second_column_content, SIGNAL(textChanged()), this, SLOT(actionModified()));
    }
    else
    {
        _m_ui->actions_list->setSpan(in_row, 0, 1, 2);
        tmp_widget_item = new QTableWidgetItem();
        tmp_widget_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_action));
        tmp_widget_item->setText(in_action->getValueForKey(ACTIONS_TABLE_SHORT_NAME));
        tmp_widget_item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        tmp_widget_item->setTextColor(Qt::white);
        _m_ui->actions_list->setItem(in_row, 0, tmp_widget_item);
        _m_ui->actions_list->resizeRowToContents(in_row);

        QLinearGradient tmp_linear_grad(0, 0, 0, tmp_widget_item->tableWidget()->rowHeight(in_row));
        tmp_linear_grad.setColorAt(0, "#96c479");
        tmp_linear_grad.setColorAt(0.1, "#a5d488");
        tmp_linear_grad.setColorAt(0.49, "#81b268");
        tmp_linear_grad.setColorAt(0.5, "#73a158");
        tmp_linear_grad.setColorAt(1, "#88b268");

        tmp_widget_item->setBackground(tmp_linear_grad);

    }

    updateRowsHeight();
}



void Form_Test::addActionAfterLastAction()
{
    _m_ui->actions_list->setCurrentIndex(_m_ui->actions_list->model()->index(_m_ui->actions_list->rowCount()-1, 0));

    addAction();
}


void Form_Test::pasteClipboardPlainTextToActionsList()
{
    const QClipboard *tmp_app_clipboard = QApplication::clipboard();
    const QMimeData *tmp_mime_data = tmp_app_clipboard->mimeData();

    if (tmp_mime_data != NULL)
    {
        if (tmp_mime_data->hasText())
        {
            QString tmp_clipboard_text = tmp_mime_data->text();
            QTextStream tmp_stream(&tmp_clipboard_text, QIODevice::ReadOnly);

            QList<char> tmp_fields_sep;
            QList<char> tmp_records_sep;
            QList<const char*> tmp_fields;

            tmp_fields_sep << '\t';
            tmp_records_sep << '\n';
            tmp_fields << ACTIONS_TABLE_DESCRIPTION;
            tmp_fields << ACTIONS_TABLE_WAIT_RESULT;

            QList<Action*> tmp_actions = Record::readDataFromDevice<Action>(tmp_stream, tmp_fields_sep, tmp_records_sep, tmp_fields);
            foreach(Action* tmp_action, tmp_actions)
            {
                tmp_action->setTestContent(_m_test_content);
                addAction(tmp_action);
            }
        }
    }
}


void Form_Test::pasteClipboardDataToActionsList()
{
    const QClipboard *tmp_app_clipboard = QApplication::clipboard();
    const QMimeData *tmp_mime_data = tmp_app_clipboard->mimeData();

    if (tmp_mime_data != NULL)
    {
        if (!tmp_mime_data->data("rtmr/data").isEmpty())
        {
            QByteArray tmp_clipboard_data = tmp_mime_data->data("rtmr/data");
            QTextStream tmp_stream(&tmp_clipboard_data, QIODevice::ReadOnly);

            QList<char> tmp_fields_sep;
            QList<char> tmp_records_sep;
            QList<const char*> tmp_fields;

            tmp_fields_sep << '\t';
            tmp_records_sep << '\n';
            tmp_fields << ACTIONS_TABLE_DESCRIPTION;
            tmp_fields << ACTIONS_TABLE_WAIT_RESULT;

            QList<Action*> tmp_actions = Record::readDataFromDevice<Action>(tmp_stream, tmp_fields_sep, tmp_records_sep, tmp_fields);
            foreach(Action* tmp_action, tmp_actions)
            {
                tmp_action->setTestContent(_m_test_content);
                addAction(tmp_action);
            }
        }
        else
        {
            pasteClipboardPlainTextToActionsList();
        }
    }
}


void Form_Test::copySelectedActionsToClipboard()
{
    QString tmp_plain_text_buffer;
    QString tmp_buffer;
    QClipboard* tmp_app_clipboard = QApplication::clipboard();
    QMimeData *tmp_mime_data = new QMimeData();
    Action *tmp_action = NULL;
    QTextDocument tmp_doc;
    QTextCodec *tmp_codec = QTextCodec::codecForLocale();

    foreach(QModelIndex tmp_index, _m_ui->actions_list->selectionModel()->selectedIndexes())
    {
        if (tmp_index.row() < _m_actions.count() && tmp_index.column() == 0)
        {
            tmp_action = _m_actions[tmp_index.row()];
            if (tmp_action != NULL)
            {
                writeActionDataFromTextEditAtIndex(tmp_action, tmp_index.row());

                // Formated
                tmp_buffer += '\"';
                tmp_buffer += QString(tmp_action->getValueForKey(ACTIONS_TABLE_DESCRIPTION)).replace('\"', "\"\"");
                tmp_buffer += "\"\t\"";
                tmp_buffer += QString(tmp_action->getValueForKey(ACTIONS_TABLE_WAIT_RESULT)).replace('\"', "\"\"");
                tmp_buffer += '\"';
                tmp_buffer += '\n';

                // Not formated
                tmp_plain_text_buffer += '\"';
                tmp_doc.setHtml(tmp_action->getValueForKey(ACTIONS_TABLE_DESCRIPTION));
                tmp_plain_text_buffer += tmp_doc.toPlainText().replace('\"', "\"\"");
                tmp_plain_text_buffer += "\"\t\"";
                tmp_doc.setHtml(tmp_action->getValueForKey(ACTIONS_TABLE_WAIT_RESULT));
                tmp_plain_text_buffer += tmp_doc.toPlainText().replace('\"', "\"\"");
                tmp_plain_text_buffer += '\"';
                tmp_plain_text_buffer += '\n';
            }
        }
    }

    tmp_mime_data->setText(tmp_plain_text_buffer);
    tmp_mime_data->setData("rtmr/data", tmp_codec->fromUnicode(tmp_buffer));

    tmp_app_clipboard->setMimeData(tmp_mime_data, QClipboard::Clipboard);
}



void Form_Test::cutSelectedActionsToClipboard()
{
    copySelectedActionsToClipboard();
    deleteSelectedAction();
}



void Form_Test::selectFirstColumnAction(Record *in_action)
{
    selectAction(in_action, 0);
}


void Form_Test::selectSecondColumnAction(Record *in_action)
{
    selectAction(in_action, 1);
}


void Form_Test::selectAction(Record *in_action, int in_column)
{
    int	tmp_index = 0;
    foreach(Action *tmp_action, _m_actions)
    {
        if (tmp_action == in_action)
        {
            _m_ui->actions_list->selectionModel()->setCurrentIndex(_m_ui->actions_list->model()->index(tmp_index, in_column), QItemSelectionModel::ClearAndSelect);
            QWidget *tmp_widget =_m_ui->actions_list->indexWidget(_m_ui->actions_list->model()->index(tmp_index, in_column));
            if (tmp_widget != NULL)
                tmp_widget->setFocus();

            return;
        }
        tmp_index++;
    }
}


void Form_Test::actionModified()
{
    setModified();
    updateRowsHeight();
}

void Form_Test::updateRowsHeight()
{
    QSizeF				tmp_first_size, tmp_second_size;
    QModelIndex     	tmp_first_column_index, tmp_second_column_index;
    RecordTextEdit    *tmp_first_column_content = NULL;
    RecordTextEdit    *tmp_second_column_content = NULL;

    for (int tmp_index = 0; tmp_index < _m_ui->actions_list->rowCount(); tmp_index++)
    {
        tmp_first_column_index = _m_ui->actions_list->model()->index(tmp_index, 0);
        tmp_first_column_content = ::qobject_cast< RecordTextEdit *>(_m_ui->actions_list->indexWidget(tmp_first_column_index));

        tmp_second_column_index = _m_ui->actions_list->model()->index(tmp_index, 1);
        tmp_second_column_content = ::qobject_cast< RecordTextEdit *>(_m_ui->actions_list->indexWidget(tmp_second_column_index));

        if (tmp_first_column_content != NULL && tmp_second_column_content != NULL)
        {
            tmp_first_size = tmp_first_column_content->document()->size();
            tmp_second_size = tmp_second_column_content->document()->size();

            if (tmp_first_size.height() > tmp_second_size.height())
                _m_ui->actions_list->setRowHeight(tmp_index, tmp_first_size.height() + 10);
            else
                _m_ui->actions_list->setRowHeight(tmp_index, tmp_second_size.height() + 10);
        }
        else
            _m_ui->actions_list->resizeRowToContents(tmp_index);
    }

    if (_m_ui->actions_list->currentRow() == _m_ui->actions_list->rowCount() - 1)
    {
        // Scroll automatique sur la dernière ligne
        _m_ui->actions_list->scrollToBottom();
    }
    else
    {
        // Scroll automatique sur la ligne courante
        if (_m_ui->actions_list->rowHeight(_m_ui->actions_list->currentRow()) > _m_ui->actions_list->height())
            _m_ui->actions_list->scrollTo(_m_ui->actions_list->currentIndex(), QAbstractItemView::PositionAtBottom);
        else
            _m_ui->actions_list->scrollTo(_m_ui->actions_list->currentIndex(), QAbstractItemView::EnsureVisible);
    }
}


void Form_Test::removeActionWidgetsAtIndex(int in_row)
{
    QModelIndex     	tmp_first_column_index, tmp_second_column_index;
    RecordTextEdit    *tmp_first_column_content = NULL;
    RecordTextEdit    *tmp_second_column_content = NULL;

    tmp_first_column_index = _m_ui->actions_list->model()->index(in_row, 0);
    if (tmp_first_column_index.isValid())
    {
        tmp_first_column_content = ::qobject_cast< RecordTextEdit *>(_m_ui->actions_list->indexWidget(tmp_first_column_index));
        if (tmp_first_column_content != NULL)
        {
            _m_ui->actions_text_toolbar->disconnectFromEditor(tmp_first_column_content);
        }
    }

    tmp_second_column_index = _m_ui->actions_list->model()->index(in_row, 1);
    if (tmp_second_column_index.isValid())
    {
        tmp_second_column_content = ::qobject_cast< RecordTextEdit *>(_m_ui->actions_list->indexWidget(tmp_second_column_index));
        if (tmp_second_column_content != NULL)
        {
            _m_ui->actions_text_toolbar->disconnectFromEditor(tmp_second_column_content);
        }
    }
}

void Form_Test::addAction()
{
    addAction(new Action(_m_test_content));
}


void Form_Test::addAction(Action *in_action)
{
    QModelIndex         tmp_current_index = _m_ui->actions_list->selectionModel()->currentIndex();
    int                 tmp_row = 0;
    bool		tmp_current_action_modifiable = false;

    setModified();

    if (tmp_current_index.isValid())
    {
        tmp_row = tmp_current_index.row();
        while (!tmp_current_action_modifiable)
        {
            tmp_row++;
            if (tmp_row < _m_actions.count())
                tmp_current_action_modifiable = compare_values(_m_actions[tmp_row]->getValueForKey(ACTIONS_TABLE_TEST_CONTENT_ID), _m_test_content->getIdentifier()) == 0;
            else
                tmp_current_action_modifiable = true;
        }
    }
    else
        tmp_row = _m_ui->actions_list->rowCount();

    _m_actions.insert(tmp_row, in_action);
    _m_ui->actions_list->insertRow(tmp_row);

    setActionAtIndex(in_action, tmp_row);

    selectAction(in_action);
}


void Form_Test::deleteSelectedAction()
{
    Action		*tmp_action = NULL;
    int             tmp_row = 0;
    QModelIndexList tmp_selected_indexes = _m_ui->actions_list->selectionModel()->selectedIndexes();

    if (!tmp_selected_indexes.isEmpty())
    {
        setModified();

        while (tmp_selected_indexes.count() > 0)
        {
            if (tmp_selected_indexes[0].isValid())
            {
                tmp_row = tmp_selected_indexes[0].row();
                removeActionWidgetsAtIndex(tmp_row);
                _m_ui->actions_list->removeRow(tmp_row);
                tmp_action = _m_actions.takeAt(tmp_row);
                _m_removed_actions.append(tmp_action);

                for (int tmp_index = 0; tmp_index < tmp_action->associatedTestActions().count(); tmp_index++)
                {
                    removeActionWidgetsAtIndex(tmp_row);
                    _m_ui->actions_list->removeRow(tmp_row);
                    _m_actions.removeAt(tmp_row);
                }
            }
            tmp_selected_indexes = _m_ui->actions_list->selectionModel()->selectedIndexes();
        }

        if (tmp_row >= _m_ui->actions_list->rowCount())
            tmp_row--;

        if (tmp_row < _m_ui->actions_list->rowCount())
            _m_ui->actions_list->setCurrentIndex(_m_ui->actions_list->model()->index(tmp_row, 0));
    }
}


void Form_Test::moveSelectedActionUp()
{
    QModelIndex         tmp_current_index = _m_ui->actions_list->selectionModel()->currentIndex();
    int                 tmp_current_row = 0, tmp_previous_row = 0,  tmp_previous_column = 0,tmp_dest_row = 0;
    int					tmp_index = 0;
    Action				*tmp_current_action = NULL;
    QList<Action*>		tmp_current_action_list;
    bool				tmp_link_action = true;

    if (tmp_current_index.isValid())
    {
        tmp_current_row = tmp_current_index.row();
        tmp_previous_column = tmp_current_index.column();
        tmp_current_action = _m_actions[tmp_current_row];
        tmp_current_action_list = tmp_current_action->associatedTestActions();

        tmp_previous_row = tmp_current_row;
        while (tmp_previous_row >= 0 && tmp_link_action)
        {
            tmp_previous_row--;
            tmp_link_action = (compare_values(_m_test_content->getIdentifier(), _m_actions[tmp_previous_row]->getValueForKey(ACTIONS_TABLE_TEST_CONTENT_ID)) != 0);
        }

        if (tmp_previous_row >= 0)
        {
            setModified();
            writeActionDataFromTextEditAtIndex(tmp_current_action, tmp_current_row);

            for(tmp_index = 0; tmp_index <= tmp_current_action_list.count(); tmp_index++)
            {
                removeActionWidgetsAtIndex(tmp_current_row);
                _m_ui->actions_list->removeRow(tmp_current_row);
                _m_actions.removeAt(tmp_current_row);
            }

            _m_actions.insert(tmp_previous_row, tmp_current_action);
            _m_ui->actions_list->insertRow(tmp_previous_row);
            setActionAtIndex(tmp_current_action, tmp_previous_row);

            for(tmp_index = 0; tmp_index < tmp_current_action_list.count(); tmp_index++)
            {
                tmp_dest_row = tmp_previous_row + tmp_index + 1;
                _m_actions.insert(tmp_dest_row, tmp_current_action_list[tmp_index]);
                _m_ui->actions_list->insertRow(tmp_dest_row);
                setActionAtIndex(tmp_current_action_list[tmp_index], tmp_dest_row);
            }

            _m_ui->actions_list->setCurrentIndex(_m_ui->actions_list->model()->index(tmp_previous_row, tmp_previous_column));
        }
    }
}


void Form_Test::moveSelectedActionDown()
{
    QModelIndex         tmp_current_index = _m_ui->actions_list->selectionModel()->currentIndex();
    int                 tmp_current_row = 0, tmp_next_row = 0, tmp_previous_column = 0, tmp_dest_row = 0;
    int					tmp_index = 0;
    Action				*tmp_current_action = NULL, *tmp_next_action = NULL;
    QList<Action*>		tmp_current_action_list;

    if (tmp_current_index.isValid())
    {
        tmp_current_row = tmp_current_index.row();
        tmp_previous_column = tmp_current_index.column();
        tmp_current_action = _m_actions[tmp_current_row];
        tmp_current_action_list = tmp_current_action->associatedTestActions();

        tmp_next_row = tmp_current_row + tmp_current_action_list.count() + 1;

        if (tmp_next_row < _m_ui->actions_list->rowCount())
        {
            tmp_next_action = _m_actions[tmp_next_row];

            setModified();
            writeActionDataFromTextEditAtIndex(tmp_current_action, tmp_current_row);

            for(tmp_index = 0; tmp_index <= tmp_current_action_list.count(); tmp_index++)
            {
                removeActionWidgetsAtIndex(tmp_current_row);
                _m_ui->actions_list->removeRow(tmp_current_row);
                _m_actions.removeAt(tmp_current_row);
            }

            tmp_dest_row = tmp_current_row + tmp_next_action->associatedTestActions().count() + 1;
            _m_actions.insert(tmp_dest_row, tmp_current_action);
            _m_ui->actions_list->insertRow(tmp_dest_row);
            setActionAtIndex(tmp_current_action, tmp_dest_row);

            for(tmp_index = 0; tmp_index < tmp_current_action_list.count(); tmp_index++)
            {
                tmp_dest_row++;
                _m_actions.insert(tmp_dest_row, tmp_current_action_list[tmp_index]);
                _m_ui->actions_list->insertRow(tmp_dest_row);
                setActionAtIndex(tmp_current_action_list[tmp_index], tmp_dest_row);
            }

            _m_ui->actions_list->setCurrentIndex(_m_ui->actions_list->model()->index(tmp_current_row + tmp_next_action->associatedTestActions().count() + 1, tmp_previous_column));

        }
    }
}



void Form_Test::actionSelectionChanged()
{
    QModelIndexList     tmp_selected_indexes = _m_ui->actions_list->selectionModel()->selectedIndexes();
    bool	    tmp_action_modifiable =  false;
    bool	    tmp_link_action = false;
    int		    tmp_row = 0;

    _m_ui->button_add->setEnabled(_m_modifiable);

    if (!tmp_selected_indexes.isEmpty())
    {
        foreach (QModelIndex tmp_index, tmp_selected_indexes)
        {
            if (tmp_index.isValid())
            {
                tmp_row = tmp_index.row();
                tmp_link_action |= (compare_values(_m_test_content->getIdentifier(), _m_actions[tmp_row]->getValueForKey(ACTIONS_TABLE_TEST_CONTENT_ID)) != 0);
            }
        }

        tmp_action_modifiable = _m_modifiable && !tmp_link_action;

        _m_ui->button_add->setEnabled(_m_modifiable && !tmp_link_action);
        _m_ui->button_del->setEnabled(tmp_action_modifiable);

        _m_ui->button_up->setEnabled(tmp_action_modifiable && tmp_row > 0);
        _m_ui->button_down->setEnabled(tmp_action_modifiable && tmp_row + _m_actions[tmp_row]->associatedTestActions().count() < _m_ui->actions_list->rowCount() - 1);

        _m_ui->button_copy->setEnabled(true);
        _m_ui->button_cut->setEnabled(tmp_action_modifiable);
    }
    else
    {
        _m_ui->button_del->setEnabled(false);

        _m_ui->button_up->setEnabled(false);
        _m_ui->button_down->setEnabled(false);
        _m_ui->button_copy->setEnabled(false);
        _m_ui->button_cut->setEnabled(false);

    }
}


void Form_Test::actionsDrop(QList<Record*> in_list, int in_row)
{
    TestHierarchy   *tmp_test = NULL;
    int		    tmp_action_index = in_row;
    Action	    *tmp_action = NULL;

    if (_m_modifiable && (in_row >= _m_actions.count() ||
                          compare_values(_m_test_content->getIdentifier(), _m_actions[in_row]->getValueForKey(ACTIONS_TABLE_TEST_CONTENT_ID)) == 0))
    {
        foreach(Record *tmp_record, in_list)
        {
            // L'objet déposé est un test
            if (tmp_record->getEntityDefSignatureId() == TESTS_HIERARCHY_SIG_ID)
            {
                // Le test déposé n'est pas celui en cours d'édition
                if (compare_values(_m_test->getIdentifier(), tmp_record->getIdentifier()) != 0 &&
                        compare_values(_m_test->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID), tmp_record->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID)) != 0)
                {
                    tmp_test = (TestHierarchy*)_m_test->projectVersion()->findItemWithId(tmp_record->getIdentifier(), TESTS_HIERARCHY_SIG_ID);
                    if (tmp_test != NULL)
                    {
                        tmp_action = new Action(_m_test_content);
                        if (tmp_action->setAssociatedTest(tmp_test))
                        {
                            setModified();

                            _m_actions.insert(tmp_action_index, tmp_action);
                            _m_ui->actions_list->insertRow(tmp_action_index);

                            setActionAtIndex(tmp_action, tmp_action_index);
                            tmp_action_index++;

                            foreach(Action *tmp_associated_action, tmp_action->associatedTestActions())
                            {
                                _m_actions.insert(tmp_action_index, tmp_associated_action);
                                _m_ui->actions_list->insertRow(tmp_action_index);

                                setActionAtIndex(tmp_associated_action, tmp_action_index);
                                tmp_action_index++;
                            }
                        }
                        else
                        {
                            delete tmp_action;
                            QApplication::restoreOverrideCursor();
                            QMessageBox::warning(this, tr("Redondance cyclique..."), tr("Les actions du scénario que vous voulez insérer font directement ou indirectement référence aux actions du scénario en cours d'édition."));
                        }
                    }
                }
            }
        }
    }
}


void Form_Test::requirementsDrop(QList<Record*> in_list, int /* in_row */)
{
    TestRequirement  		*tmp_test_requirement = NULL;
    TestRequirement  		*tmp_current_test_requirement = NULL;
    QTableWidgetItem    	*tmp_item = NULL;
    bool                	tmp_item_exists = false;

    if (_m_modifiable)
    {
        foreach(Record *tmp_record, in_list)
        {
            if (tmp_record->getEntityDefSignatureId() == REQUIREMENTS_HIERARCHY_SIG_ID)
            {
                if (tmp_record != NULL)
                {
                    // Verifier le projet
                    if (compare_values(tmp_record->getValueForKey(REQUIREMENTS_HIERARCHY_PROJECT_ID), _m_test->getValueForKey(TESTS_HIERARCHY_PROJECT_ID)) == 0)
                    {
                        tmp_item_exists = false;
                        for (int tmp_index = 0; tmp_item_exists == false && tmp_index < _m_ui->requirements_list->rowCount(); tmp_index++)
                        {
                            tmp_item = _m_ui->requirements_list->item(tmp_index, 0);
                            tmp_current_test_requirement = (TestRequirement*)tmp_item->data(Qt::UserRole).value<void*>();
                            tmp_item_exists = (tmp_current_test_requirement != NULL
                                    && compare_values(
                                        tmp_record->getValueForKey(REQUIREMENTS_HIERARCHY_ORIGINAL_REQUIREMENT_CONTENT_ID),
                                        tmp_current_test_requirement->getValueForKey(TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID)) == 0);
                        }

                        if (tmp_item_exists == false)
                        {
                            setModified();
                            tmp_test_requirement = new TestRequirement();
                            tmp_test_requirement->setRequirementShortName(tmp_record->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME));
                            tmp_test_requirement->setRequirementCategory(tmp_record->getValueForKey(REQUIREMENTS_HIERARCHY_CATEGORY_ID));
                            tmp_test_requirement->setRequirementVersion(tmp_record->getValueForKey(REQUIREMENTS_HIERARCHY_CONTENT_VERSION));
                            tmp_test_requirement->setValueForKey(_m_test_content->getIdentifier(), TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID);
                            tmp_test_requirement->setValueForKey(tmp_record->getValueForKey(REQUIREMENTS_HIERARCHY_ORIGINAL_REQUIREMENT_CONTENT_ID), TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);
                            _m_requirements.insert(_m_requirements.count(), tmp_test_requirement);
                            _m_ui->requirements_list->insertRow(_m_ui->requirements_list->rowCount());
                            setRequirementForRow(tmp_test_requirement, _m_ui->requirements_list->rowCount() - 1);
                        }
                    }
                }
            }
        }
    }
}


void Form_Test::deletedRequirementAtIndex(int in_row)
{
    setModified();
    _m_removed_requirements.append(_m_requirements.takeAt(in_row));
}

void Form_Test::setRequirementForRow(TestRequirement *in_requirement, int in_row)
{
    QTableWidgetItem    *tmp_first_column_item = NULL;
    QTableWidgetItem    *tmp_second_column_item = NULL;
    QTableWidgetItem    *tmp_third_column_item = NULL;

    if (in_requirement != NULL)
    {
        // Premiere colonne
        tmp_first_column_item = new QTableWidgetItem;
        tmp_first_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_requirement));
        tmp_first_column_item->setText(in_requirement->requirementShortName());
        tmp_first_column_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        _m_ui->requirements_list->setItem(in_row, 0, tmp_first_column_item);

        // Deuxieme colonne
        tmp_second_column_item = new QTableWidgetItem;
        tmp_second_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_requirement));
        tmp_second_column_item->setText(_m_ui->test_category->itemText(_m_ui->test_category->findData(in_requirement->requirementCategory())));
        tmp_second_column_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        _m_ui->requirements_list->setItem(in_row, 1, tmp_second_column_item);

        // Troisième colonne
        tmp_third_column_item = new QTableWidgetItem;
        tmp_third_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_requirement));
        tmp_third_column_item->setText(ProjectVersion::formatProjectVersionNumber(in_requirement->requirementVersion()));
        tmp_third_column_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        _m_ui->requirements_list->setItem(in_row, 2, tmp_third_column_item);

    }
}

RecordTextEdit* Form_Test::textEditAt(int in_row, int in_column)
{
    QModelIndex     	tmp_column_index;
    RecordTextEdit    *tmp_column_content = NULL;

    tmp_column_index = _m_ui->actions_list->model()->index(in_row, in_column);
    if (tmp_column_index.isValid())
    {
        tmp_column_content = ::qobject_cast< RecordTextEdit *>(_m_ui->actions_list->indexWidget(tmp_column_index));
    }

    return tmp_column_content;
}

void Form_Test::writeActionDataFromTextEditAtIndex(Action *in_action, int in_index)
{
    QModelIndex     	tmp_first_column_index, tmp_second_column_index;
    RecordTextEdit    *tmp_first_column_content = NULL;
    RecordTextEdit    *tmp_second_column_content = NULL;

    tmp_first_column_index = _m_ui->actions_list->model()->index(in_index, 0);
    if (tmp_first_column_index.isValid())
    {
        tmp_first_column_content = ::qobject_cast< RecordTextEdit *>(_m_ui->actions_list->indexWidget(tmp_first_column_index));
        if (tmp_first_column_content != NULL)
            in_action->setValueForKey(tmp_first_column_content->toHtml().toStdString().c_str(), ACTIONS_TABLE_DESCRIPTION);
    }

    tmp_second_column_index = _m_ui->actions_list->model()->index(in_index, 1);
    if (tmp_second_column_index.isValid())
    {
        tmp_second_column_content = ::qobject_cast< RecordTextEdit *>(_m_ui->actions_list->indexWidget(tmp_second_column_index));
        if (tmp_second_column_content != NULL)
            in_action->setValueForKey(tmp_second_column_content->toHtml().toStdString().c_str(), ACTIONS_TABLE_WAIT_RESULT);
    }
}


void Form_Test::filesDrop(QList<QString> in_list, int /*in_row*/)
{
    TestContentFile		*tmp_file = NULL;

    if (_m_modifiable)
    {
        setModified();
        foreach(QString tmp_filename, in_list)
        {
            tmp_file = new TestContentFile(_m_test_content);
            tmp_file->setSourceFile(tmp_filename.toStdString().c_str());
            _m_files.insert(_m_files.count(), tmp_file);
            _m_ui->files_list->insertRow(_m_ui->files_list->rowCount());
            setFileForRow(tmp_file, _m_ui->files_list->rowCount() - 1);
        }
    }
}


void Form_Test::setFileForRow(TestContentFile *in_file, int in_row)
{
    QTableWidgetItem    *tmp_column_item = NULL;

    if (in_file != NULL)
    {
        tmp_column_item = new QTableWidgetItem;
        tmp_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_file));
        tmp_column_item->setText(in_file->getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME));
        _m_ui->files_list->setItem(in_row, 0, tmp_column_item);
    }
}


void Form_Test::setBugForRow(Bug *in_bug, int in_row)
{
    QTableWidgetItem    *tmp_first_column_item = NULL;
    QTableWidgetItem    *tmp_second_column_item = NULL;
    QTableWidgetItem    *tmp_third_column_item = NULL;
    QTableWidgetItem    *tmp_fourth_column_item = NULL;
    QTableWidgetItem    *tmp_fifth_column_item = NULL;

    QString		tmp_tool_tip;
    QString		tmp_url;
    QString		tmp_external_link;
    QDateTime		tmp_date_time;

    QLabel              *tmp_external_link_label = NULL;

    if (in_bug != NULL)
    {
        tmp_date_time = QDateTime::fromString(QString(in_bug->getValueForKey(BUGS_TABLE_CREATION_DATE)).left(16), "yyyy-MM-dd hh:mm");
        tmp_tool_tip += "<p><b>" + tr("Date de création") + "</b> : " + tmp_date_time.toString("dddd dd MMMM yyyy à hh:mm") + "</p>";
        tmp_tool_tip += "<p><b>" + tr("Résumé") + "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_SHORT_NAME)) + "</p>";
        tmp_tool_tip += "<p><b>" + tr("Priorité") + "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_PRIORITY)) + "</p>";
        tmp_tool_tip += "<p><b>" + tr("Gravité") + "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_SEVERITY)) + "</p>";
        tmp_tool_tip += "<p><b>" + tr("Plateforme") + "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_PLATFORM)) + "</p>";
        tmp_tool_tip += "<p><b>" + tr("Système d'exploitation") + "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_SYSTEM)) + "</p>";
        tmp_tool_tip += "<p><b>" + tr("Description") + "</b> :<br>" + QString(in_bug->getValueForKey(BUGS_TABLE_DESCRIPTION)).replace('\n', "<BR>") + "</p>";
        if (is_empty_string(in_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID)) == FALSE)
        {
            tmp_tool_tip += "<p><b>" + tr("Identifiant externe") + "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID)) + "</p>";
            if (_m_bugtracker != NULL)
            {
                tmp_url = _m_bugtracker->urlForBugWithId(in_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID));
                if (!tmp_url.isEmpty())
                    tmp_external_link = "<a href=\"" + tmp_url + "\">" + tmp_url + "</a>";
            }

            if (tmp_external_link.isEmpty())
                tmp_external_link = QString(in_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID));
        }

        // Premirère colonne (date de création du bug)
        tmp_first_column_item = new QTableWidgetItem;
        tmp_first_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_bug));
        tmp_first_column_item->setText(tmp_date_time.toString("dddd dd MMMM yyyy à hh:mm"));
        tmp_first_column_item->setToolTip(tmp_tool_tip);
        _m_ui->bugs_list->setItem(in_row, 0, tmp_first_column_item);

        // Seconde colonne (résumé)
        tmp_second_column_item = new QTableWidgetItem;
        tmp_second_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_bug));
        tmp_second_column_item->setText(in_bug->getValueForKey(BUGS_TABLE_SHORT_NAME));
        tmp_second_column_item->setToolTip(tmp_tool_tip);
        _m_ui->bugs_list->setItem(in_row, 1, tmp_second_column_item);

        // Troisième colonne (priorité)
        tmp_third_column_item = new QTableWidgetItem;
        tmp_third_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_bug));
        tmp_third_column_item->setText(in_bug->getValueForKey(BUGS_TABLE_PRIORITY));
        tmp_third_column_item->setToolTip(tmp_tool_tip);
        _m_ui->bugs_list->setItem(in_row, 2, tmp_third_column_item);

        // Quatrième colonne (sévérité)
        tmp_fourth_column_item = new QTableWidgetItem;
        tmp_fourth_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_bug));
        tmp_fourth_column_item->setText(in_bug->getValueForKey(BUGS_TABLE_SEVERITY));
        tmp_fourth_column_item->setToolTip(tmp_tool_tip);
        _m_ui->bugs_list->setItem(in_row, 3, tmp_fourth_column_item);

        // Cinquième colonne (id externe)
        tmp_external_link_label = new QLabel(tmp_external_link);
        tmp_external_link_label->setOpenExternalLinks(true);
        tmp_fifth_column_item = new QTableWidgetItem;
        tmp_fifth_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_bug));
        tmp_fifth_column_item->setToolTip(tmp_tool_tip);
        _m_ui->bugs_list->setItem(in_row, 4, tmp_fifth_column_item);
        _m_ui->bugs_list->setCellWidget(in_row, 4, tmp_external_link_label);
    }
}


void Form_Test::deletedFileAtIndex(int in_index)
{
    setModified();
    _m_removed_files.append(_m_files.takeAt(in_index));
}


void Form_Test::deletedBugAtIndex(int in_index)
{
    setModified();
    _m_removed_bugs.append(_m_bugs.takeAt(in_index));
}


void Form_Test::openBugAtIndex(QTableWidgetItem *in_item)
{
    Bug		    *tmp_bug = NULL;
    Form_Bug	    *tmp_form_bug = NULL;

    if (in_item != NULL)
    {
        tmp_bug = (Bug*)in_item->data(Qt::UserRole).value<void*>();
        if (tmp_bug != NULL)
        {
            tmp_form_bug = new Form_Bug(_m_test->projectVersion(), NULL, NULL, tmp_bug);
            tmp_form_bug->show();
        }
    }
}

void Form_Test::loadPreviousTestContent()
{
    TestContent	*tmp_test_content = _m_previous_test_content;

    if (maybeClose())
    {
        if (_m_test_content != NULL
                && _m_previous_test_content != NULL
                && compare_values(_m_test_content->getIdentifier(), _m_previous_test_content->getIdentifier()) == 0)
            tmp_test_content = _m_test_content;

        loadTestContent(tmp_test_content);
    }
}



void Form_Test::openFileAtIndex(QTableWidgetItem *in_item)
{
    TestContentFile		*tmp_file = NULL;
    QString			tmp_filename;
    const char			*tmp_filename_str = NULL;
    int				tmp_return = NOERR;
    QString			tmp_temp_dir = QDesktopServices::storageLocation(QDesktopServices::TempLocation);
    net_session		*tmp_session = Session::instance()->getClientSession();

    if (in_item != NULL)
    {
        tmp_file = (TestContentFile*)in_item->data(Qt::UserRole).value<void*>();
        if (tmp_file != NULL)
        {
            if (is_empty_string(tmp_file->getIdentifier()))
            {

            }
            else
            {
                tmp_filename_str = tmp_file->getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME);
                if (is_empty_string(tmp_filename_str) == FALSE)
                {
#if QT_VERSION >= 0x040800
                    if (QApplication::queryKeyboardModifiers() & Qt::ControlModifier)
#else
                    if (QApplication::keyboardModifiers() & Qt::ControlModifier)
#endif
                    {
                        tmp_filename = QFileDialog::getSaveFileName(this, tr("Enregistrer le fichier sous..."), tmp_filename_str);
                        if (tmp_filename.isEmpty() == false)
                        {
                            tmp_return = cl_get_blob(tmp_session, tmp_file->getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_LO_ID), tmp_filename.toStdString().c_str());
                            if (tmp_return != NOERR)
                            {
                                QMessageBox::critical(this, tr("Erreur lors de la récupération du fichier"), Session::instance()->getErrorMessage(tmp_return));
                            }
                        }
                    }
                    else
                    {
                        tmp_filename = tmp_temp_dir + "/" + tmp_filename_str;
                        tmp_return = cl_get_blob(tmp_session, tmp_file->getValueForKey(TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_LO_ID), tmp_filename.toStdString().c_str());
                        if (tmp_return != NOERR)
                        {
                            QMessageBox::critical(this, tr("Erreur lors de la récupération du fichier"), Session::instance()->getErrorMessage(tmp_return));
                        }
                        else
                        {
                            QDesktopServices::openUrl(QUrl("file:///" + tmp_filename));
                        }
                    }
                }
            }
        }
    }
}


void Form_Test::showEvent(QShowEvent * event)
{
    QWidget::showEvent(event);

    updateRowsHeight();
}


void Form_Test::closeEvent(QCloseEvent *in_event)
{
    if (maybeClose())
        in_event->accept();
    else
        in_event->ignore();
}


void Form_Test::showOriginalTestInfosClicked()
{
    emit showOriginalTestInfos(_m_test);
}


void Form_Test::setModified()
{
    _m_modified = true;
}


bool Form_Test::maybeClose()
{
    int		tmp_confirm_choice = 0;
    bool    tmp_return = true;

    if (_m_test_content != NULL)
    {
        if (_m_modified)
        {
            tmp_confirm_choice = QMessageBox::question(
                        this,
                        tr("Confirmation..."),
                        tr("Le test a été modifié. Voulez-vous enregistrer les modifications ?"),
                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                        QMessageBox::Cancel);

            if (tmp_confirm_choice == QMessageBox::Yes)
                tmp_return = saveTest();
            else if (tmp_confirm_choice == QMessageBox::Cancel)
                tmp_return = false;
        }

        if (tmp_return)
            _m_test_content->unlockRecord();
    }

    return tmp_return;
}


void Form_Test::showRequirementAtIndex(QModelIndex in_index)
{
    TestRequirement      		*tmp_test_requirement = NULL;
    QTableWidgetItem            *tmp_column_item = NULL;

    if (in_index.isValid())
    {
        tmp_column_item = _m_ui->requirements_list->item(in_index.row(), 0);
        if (tmp_column_item != NULL)
        {
            tmp_test_requirement = (TestRequirement*)tmp_column_item->data(Qt::UserRole).value<void*>();
            if (tmp_test_requirement != NULL)
            {
                emit showRequirementWithOriginalContentId(tmp_test_requirement->getValueForKey(TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID));
            }
        }
    }
}



void Form_Test::changeTab(int in_tab_index)
{
    if (in_tab_index == 0)
    {
        updateRowsHeight();
    }
}


void Form_Test::selectExternalCommandForAutomation()
{
    QString tmp_filename = QFileDialog::getOpenFileName(
                this,
                tr("Sélectionner un fichier"), QString(),"Executables (*.exe)");

    if (tmp_filename.isEmpty() == false)
    {
        _m_ui->automation_command_line->setText(tmp_filename);
    }
}


void Form_Test::startRecordSystemEvents()
{
#ifdef AUTOMATION_ACTIVATED

    _m_ui->button_start_record->setEnabled(false);
    _m_ui->button_stop_record->setEnabled(true);
    _m_ui->button_start_playback->setEnabled(false);

    int tmp_result = automation_start_record(Session::instance()->getClientSession(), _m_automated_program_record_process.pid()->dwProcessId, _m_automated_program_record_process.pid()->dwThreadId, &AutomationCallbackErrorFunction, _m_ui->automated_action_record_mouse_move->isChecked());
    if (tmp_result != NOERR)
    {
        QMessageBox::critical(this, tr("Erreur %1").arg(tmp_result), QString::fromLocal8Bit(Session::instance()->getClientSession()->m_last_error_msg));

        _m_ui->button_start_record->setEnabled(true);
        _m_ui->button_stop_record->setEnabled(false);
        _m_ui->button_start_playback->setEnabled(_m_automated_actions_model->rowCount() > 0);
    }
    else
    {
        _m_is_automated_program_recording = true;
    }

#endif
}


void Form_Test::execProcessError(QProcess::ProcessError /*in_process_error*/)
{
    QProcess *tmp_process = static_cast<QProcess*>(sender());
    QMessageBox::critical(this, tr("Erreur"), tr("Le progamme %1 n'a pû être démarré correctement : %2.").arg(_m_ui->automation_command_parameters->text()).arg(tmp_process->errorString()));
}


void Form_Test::launchStartRecordSystemEvents()
{
    QString tmp_exec_file = _m_ui->automation_command_line->text();
    if (tmp_exec_file.isEmpty())
    {
        QMessageBox::critical(this, tr("Erreur"), tr("Veuillez sélectionner une application à lancer dans la section Automatisation."));
    }
    else
    {
        if (!QFile::exists(tmp_exec_file))
        {
            QMessageBox::critical(this, tr("Erreur"), tr("L'executable' %1 n'existe pas.").arg(tmp_exec_file));
        }
        else
        {
            stopRecordProcess();

            connect(&_m_automated_program_record_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(execProcessError(QProcess::ProcessError)));
            connect(&_m_automated_program_record_process, SIGNAL(finished(int)), this, SLOT(stopRecordProcess()));

            _m_automated_program_record_process.start(tmp_exec_file, _m_ui->automation_command_parameters->text().split(' ', QString::SkipEmptyParts));
            if(_m_automated_program_record_process.waitForStarted()){
                startRecordSystemEvents();
            }
        }
    }
}


void Form_Test::stopRecordSystemEvents()
{
#ifdef AUTOMATION_ACTIVATED

    if (_m_is_automated_program_recording)
    {
        log_message_list* tmp_messages = automation_stop_record(Session::instance()->getClientSession());
        log_message_list* tmp_current_message = tmp_messages;
        int tmp_actions_count = 0;
        AutomatedAction* tmp_automated_action = NULL;
        QString tmp_windows_id;
        windows_hierarchy* tmp_window = NULL;
        QList<AutomatedAction*> tmp_actions;

        _m_ui->button_start_record->setEnabled(true);
        _m_ui->button_stop_record->setEnabled(false);

        _m_ui->button_start_playback->setEnabled(_m_automated_actions_model->rowCount() > 0);

        while (tmp_current_message != NULL)
        {
            tmp_automated_action = new AutomatedAction(_m_test_content);
            tmp_automated_action->setValueForKey(QString::number(tmp_current_message->m_message_type).toStdString().c_str(), AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE);
            tmp_automated_action->setValueForKey(QString::number(tmp_current_message->m_time).toStdString().c_str(), AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY);

            if (tmp_current_message->m_window_hierarchy)
            {
                tmp_window = tmp_current_message->m_window_hierarchy;
                tmp_windows_id = "";
                while(tmp_window){
                    tmp_windows_id += QString("%1%2%3%4%5%6%7%8%9%10%11%12%13%14%15%16%17%18")
                            .arg(tmp_window->m_window_atom)
                            .arg(AutomatedAction::FieldSeparator)
                            .arg(QString::fromLocal8Bit(tmp_window->m_window_class_name))
                            .arg(AutomatedAction::FieldSeparator)
                            .arg(QString::fromLocal8Bit(tmp_window->m_window_name))
                            .arg(AutomatedAction::FieldSeparator)
                            .arg(tmp_window->m_ctrl_id)
                            .arg(AutomatedAction::FieldSeparator)
                            .arg(tmp_window->m_is_active)
                            .arg(AutomatedAction::FieldSeparator)
                            .arg(tmp_window->m_window_rect.left)
                            .arg(AutomatedAction::FieldSeparator)
                            .arg(tmp_window->m_window_rect.top)
                            .arg(AutomatedAction::FieldSeparator)
                            .arg(tmp_window->m_window_rect.right)
                            .arg(AutomatedAction::FieldSeparator)
                            .arg(tmp_window->m_window_rect.bottom)
                            .arg(AutomatedAction::WindowSeparator);

                    tmp_window = tmp_window->m_child_window;
                }
                tmp_automated_action->setValueForKey(tmp_windows_id.toStdString().c_str(), AUTOMATED_ACTIONS_TABLE_WINDOW_ID);


                switch(tmp_current_message->m_message_type)
                {
                case Keyboard:
                    tmp_automated_action->setValueForKey(QString("%1%2%3%4%5")
                                                         .arg(tmp_current_message->m_message)
                                                         .arg(AutomatedAction::FieldSeparator)
                                                         .arg(tmp_current_message->m_keyboard_vk)
                                                         .arg(AutomatedAction::FieldSeparator)
                                                         .arg(tmp_current_message->m_keyboard_repeat).toStdString().c_str()
                                                         , AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA);
                    break;

                case Mouse:
                    tmp_automated_action->setValueForKey(QString("%1%2%3%4%5")
                                                         .arg(tmp_current_message->m_message)
                                                         .arg(AutomatedAction::FieldSeparator)
                                                         .arg(tmp_current_message->m_mouse_x)
                                                         .arg(AutomatedAction::FieldSeparator)
                                                         .arg(tmp_current_message->m_mouse_y).toStdString().c_str(), AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA);
                    break;
                }

            }
            tmp_actions.append(tmp_automated_action);

            tmp_current_message = tmp_current_message->m_next_message;
            ++tmp_actions_count;
        }

        free_messages_list(&tmp_messages);

        _m_automated_actions_model->setRecordsList(tmp_actions, true);
        _m_ui->automated_actions_list->resizeColumnsToContents();
    }

    _m_is_automated_program_recording = false;

#endif

}



void Form_Test::launchStartPlaybackSystemEvents()
{
    QString tmp_exec_file = _m_ui->automation_command_line->text();
    if (tmp_exec_file.isEmpty())
    {
        QMessageBox::critical(this, tr("Erreur"), tr("Veuillez sélectionner une application à lancer dans la section Automatisation."));
    }
    else
    {
        if (!QFile::exists(tmp_exec_file))
        {
            QMessageBox::critical(this, tr("Erreur"), tr("L'executable' %1 n'existe pas.").arg(tmp_exec_file));
        }
        else
        {
            stopPlaybackProcess();

            connect(&_m_automated_program_playback_process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(execProcessError(QProcess::ProcessError)));
            connect(&_m_automated_program_playback_process, SIGNAL(finished(int)), this, SLOT(stopPlaybackProcess()));

            _m_automated_program_playback_process.start(tmp_exec_file, _m_ui->automation_command_parameters->text().split(' ', QString::SkipEmptyParts));
            if(_m_automated_program_playback_process.waitForStarted()){
                startPlaybackSystemEvents();
            }
        }
    }
}


void Form_Test::startPlaybackSystemEvents()
{
#ifdef AUTOMATION_ACTIVATED

    log_message_list* tmp_current_message = NULL;
    windows_hierarchy* tmp_window_hierarchy = NULL;
    windows_hierarchy* tmp_child_window_hierarchy = NULL;

    QString tmp_windows_id;
    QStringList tmp_windows_ids;
    int tmp_windows_count = 0;
    int tmp_window_index = 0;

    _m_automation_playback_messages = (log_message_list*)malloc(sizeof(log_message_list));
    tmp_current_message = _m_automation_playback_messages;
    memset(tmp_current_message, 0, sizeof(log_message_list));
    tmp_current_message->m_message_type = None;

    foreach(AutomatedAction* tmp_automated_action, _m_automated_actions_model->getRecordsList())
    {
        QStringList tmp_message_data(QString(tmp_automated_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA)).split(AutomatedAction::FieldSeparator));

        // Liste des fenetres
        tmp_window_hierarchy = (windows_hierarchy*)malloc(sizeof(windows_hierarchy));
        memset(tmp_window_hierarchy, 0, sizeof(windows_hierarchy));
        tmp_child_window_hierarchy = tmp_window_hierarchy;

        tmp_windows_id = tmp_automated_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_WINDOW_ID);
        tmp_windows_ids = tmp_windows_id.split(AutomatedAction::WindowSeparator, QString::SkipEmptyParts);
        tmp_windows_count = tmp_windows_ids.count();
        tmp_window_index = 0;

        foreach(QString tmp_window, tmp_windows_ids){
            QStringList tmp_window_info = tmp_window.split(AutomatedAction::FieldSeparator);
            if (tmp_window_info.count() == 9){
                tmp_child_window_hierarchy->m_window_atom = tmp_window_info[0].toInt();
                strcpy(tmp_child_window_hierarchy->m_window_class_name, tmp_window_info[1].toLocal8Bit().data());
                strcpy(tmp_child_window_hierarchy->m_window_name, tmp_window_info[2].toLocal8Bit().data());
                tmp_child_window_hierarchy->m_ctrl_id = tmp_window_info[3].toInt();
                tmp_child_window_hierarchy->m_is_active = tmp_window_info[4].toInt();
                tmp_child_window_hierarchy->m_window_rect.left = tmp_window_info[5].toInt();
                tmp_child_window_hierarchy->m_window_rect.top = tmp_window_info[6].toInt();
                tmp_child_window_hierarchy->m_window_rect.right = tmp_window_info[7].toInt();
                tmp_child_window_hierarchy->m_window_rect.bottom = tmp_window_info[8].toInt();

                if (tmp_window_index < tmp_windows_count - 1){
                    tmp_child_window_hierarchy->m_child_window = (windows_hierarchy*)malloc(sizeof(windows_hierarchy));
                    memset(tmp_child_window_hierarchy->m_child_window, 0, sizeof(windows_hierarchy));
                    tmp_child_window_hierarchy = tmp_child_window_hierarchy->m_child_window;
                }
            }

            ++tmp_window_index;
        }
        tmp_current_message->m_window_hierarchy = tmp_window_hierarchy;

        tmp_current_message->m_message_type = (EventMessageType)atoi(tmp_automated_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE));
        if (tmp_message_data.count() == 3)
        {
            switch (tmp_current_message->m_message_type)
            {
            case Keyboard:
                tmp_current_message->m_message = tmp_message_data[0].toInt();
                tmp_current_message->m_keyboard_vk = tmp_message_data[1].toInt();
                tmp_current_message->m_keyboard_repeat = tmp_message_data[2].toInt();

                break;

            case Mouse:
                tmp_current_message->m_message = tmp_message_data[0].toInt();
                tmp_current_message->m_mouse_x = tmp_message_data[1].toInt();
                tmp_current_message->m_mouse_y = tmp_message_data[2].toInt();
                break;
            }
        }

        tmp_current_message->m_time = atol(tmp_automated_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY));

        tmp_current_message->m_callback = &RunMessageCallbacks;
        tmp_current_message->m_custom_data_ptr = tmp_automated_action;

        tmp_current_message->m_next_message = (log_message_list*)malloc(sizeof(log_message_list));
        memset(tmp_current_message->m_next_message, 0, sizeof(log_message_list));
        tmp_current_message = tmp_current_message->m_next_message;
        tmp_current_message->m_message_type = None;
    }

    _m_ui->button_start_record->setEnabled(false);
    _m_ui->button_start_playback->setEnabled(false);
    _m_ui->button_stop_playback->setEnabled(true);

    int tmp_result = automation_start_playback(Session::instance()->getClientSession(), _m_automation_playback_messages, _m_automated_program_playback_process.pid()->dwProcessId, _m_automated_program_playback_process.pid()->dwThreadId, &AutomationCallbackErrorFunction);
    if (tmp_result != NOERR)
    {
        QMessageBox::critical(this, tr("Erreur %1").arg(tmp_result), QString::fromLocal8Bit(Session::instance()->getClientSession()->m_last_error_msg));
        _m_ui->button_start_record->setEnabled(true);
        _m_ui->button_start_playback->setEnabled(true);
        _m_ui->button_stop_playback->setEnabled(false);
    }
    else
    {
        _m_is_automated_program_playbacking = true;
    }


#endif

}


void Form_Test::stopPlaybackSystemEvents()
{
#ifdef AUTOMATION_ACTIVATED
    if (_m_is_automated_program_playbacking)
    {
        int tmp_result = automation_stop_playback(Session::instance()->getClientSession());
        if (tmp_result != NOERR)
        {
            QMessageBox::critical(this, tr("Erreur %1").arg(tmp_result), QString::fromLocal8Bit(Session::instance()->getClientSession()->m_last_error_msg));
        }


        _m_ui->button_start_record->setEnabled(true);
        _m_ui->button_start_playback->setEnabled(_m_automated_actions_model->rowCount() > 0);
        _m_ui->button_stop_playback->setEnabled(false);

        freeMessagesList(&_m_automation_playback_messages);
    }

    _m_is_automated_program_playbacking = false;

#endif
}


void Form_Test::loadPluginsViews()
{
    QMap < QString, ClientModule*>	tmp_modules_map = Session::instance()->externalsModules().value(ClientModule::TestPlugin);

    TestModule   *tmp_test_module = NULL;

    foreach(ClientModule *tmp_module, tmp_modules_map)
    {
        tmp_test_module = static_cast<TestModule*>(tmp_module);

        tmp_test_module->loadTestModuleDatas(_m_test);

        QWidget	    *tmp_module_view = tmp_test_module->createView(this);
        if (tmp_module_view != NULL)
        {
            _m_views_modules_map[tmp_test_module] = tmp_module_view;
            _m_ui->tabs->addTab(tmp_module_view, tmp_module->getModuleName());
        }
    }
}


void Form_Test::savePluginsDatas()
{
    QMap<TestModule*, QWidget*>::iterator tmp_module_iterator;

    for(tmp_module_iterator = _m_views_modules_map.begin(); tmp_module_iterator != _m_views_modules_map.end(); tmp_module_iterator++)
    {
        tmp_module_iterator.key()->saveTestModuleDatas();
    }
}


void Form_Test::destroyPluginsViews()
{
    QMap<TestModule*, QWidget*>::iterator tmp_module_iterator;

    for(tmp_module_iterator = _m_views_modules_map.begin(); tmp_module_iterator != _m_views_modules_map.end(); tmp_module_iterator++)
    {
        tmp_module_iterator.key()->destroyView(tmp_module_iterator.value());
    }
}


void Form_Test::clipboardDataChanged()
{
    _m_ui->button_paste->setEnabled(!QApplication::clipboard()->text().isEmpty());
    _m_ui->button_paste_plain_text->setEnabled(!QApplication::clipboard()->text().isEmpty());
}


void Form_Test::showAutomatedActionValidationContextMenu(const QPoint & position)
{
#ifdef AUTOMATION_ACTIVATED
    QMenu *tmp_menu = NULL;
    QMenu *tmp_function_menu = NULL;
    QMenu*  tmp_module_menu = NULL;
    QMenu*  tmp_automated_action_menu = NULL;
    QAction *tmp_action = NULL;
    QModelIndex tmp_model_index = _m_ui->automated_actions_list->indexAt(position);
    AutomatedAction*  tmp_automated_action = static_cast< AutomatedAction* >(tmp_model_index.internalPointer());
    AutomatedActionValidation* tmp_automated_action_validation = NULL;
    AutomationModule* tmp_automation_module = NULL;
    AutomationCallbackFunction* tmp_callback = NULL;

    QMap < QString, ClientModule*>	tmp_modules_map = Session::instance()->externalsModules().value(ClientModule::AutomationPlugin);

    tmp_menu = new QMenu();
    tmp_automated_action_menu = tmp_menu->addMenu(tr("Toutes les actions"));
    tmp_function_menu = tmp_automated_action_menu->addMenu(tr("Ajouter une fonction de validation"));
    foreach(ClientModule* tmp_module, tmp_modules_map.values()){
        tmp_automation_module = static_cast<AutomationModule*>(tmp_module);
        if (tmp_automation_module){
            tmp_module_menu = tmp_function_menu->addMenu(tr("Module %1").arg(tmp_automation_module->getModuleName()));
            QMap<QString, AutomationCallbackFunction*> tmp_module_functions_list = tmp_automation_module->getFunctionsMap();
            foreach(tmp_callback, tmp_module_functions_list.values()){
                tmp_action = tmp_module_menu->addAction(tr("Fonction %1").arg(tmp_callback->getName()));
                tmp_action->setToolTip(QString("<p>%1</p>").arg(tmp_callback->getDescription()));
                connect(tmp_action, SIGNAL(hovered()), this, SLOT(showAutomatedActionValidationToolTip()));
                tmp_action->setData(QVariant::fromValue<void*>(tmp_callback));
                tmp_action->setProperty("type", ALL_ACTIONS_ADD_FUNCTION);
            }
            if (tmp_module_menu->isEmpty()){
                tmp_action = tmp_module_menu->addAction(tr("Aucune fonction disponible"));
                tmp_action->setFont(QFont(tmp_action->font().family(), -1, -1, true));
                tmp_action->setProperty("type", ACTION_NONE);
              }
        }
    }
    if (tmp_function_menu->isEmpty()){
        tmp_action = tmp_function_menu->addAction(tr("Aucun module disponible"));
        tmp_action->setFont(QFont(tmp_action->font().family(), -1, -1, true));
        tmp_action->setProperty("type", ACTION_NONE);
      }

    tmp_action = tmp_automated_action_menu->addAction(tr("Enlever toutes les fonctions de validation"));
    tmp_action->setProperty("type", ALL_ACTIONS_REMOVE_FUNCTION);



    if (tmp_automated_action){
        QList<AutomatedActionValidation*> tmp_childs = tmp_automated_action->getChilds();
        int tmp_index = 0;

        tmp_automated_action_menu = tmp_menu->addMenu(tr("Action sélectionnée"));

        for(tmp_index = 0; tmp_index < tmp_childs.count(); ++tmp_index){

            tmp_automated_action_validation = tmp_childs[tmp_index];
            tmp_function_menu = tmp_automated_action_menu->addMenu(tmp_automated_action_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME));

            if (tmp_index > 0){
                tmp_action = tmp_function_menu->addAction(tr("Monter"));
                tmp_action->setProperty("record", QVariant::fromValue<void*>(tmp_automated_action_validation));
                tmp_action->setProperty("type", CURRENT_ACTION_MOVE_FUNCTION_UP);
            }

            if (tmp_index < tmp_childs.count() - 1){
                tmp_action = tmp_function_menu->addAction(tr("Déscendre"));
                tmp_action->setProperty("record", QVariant::fromValue<void*>(tmp_automated_action_validation));
                tmp_action->setProperty("type", CURRENT_ACTION_MOVE_FUNCTION_DOWN);
            }

            tmp_function_menu->addSeparator();
            tmp_action = tmp_function_menu->addAction(tr("Enlever"));
            tmp_action->setProperty("record", QVariant::fromValue<void*>(tmp_automated_action_validation));
            tmp_action->setProperty("type", CURRENT_ACTION_REMOVE_FUNCTION);
        }

        if (!tmp_automated_action_menu->isEmpty())
            tmp_automated_action_menu->addSeparator();

        tmp_function_menu = tmp_automated_action_menu->addMenu(tr("Ajouter une fonction de validation"));
        foreach(ClientModule* tmp_module, tmp_modules_map.values()){
            tmp_automation_module = static_cast<AutomationModule*>(tmp_module);
            if (tmp_automation_module){
                tmp_module_menu = tmp_function_menu->addMenu(tr("Module %1").arg(tmp_automation_module->getModuleName()));
                QMap<QString, AutomationCallbackFunction*> tmp_module_functions_list = tmp_automation_module->getFunctionsMap();
                foreach(tmp_callback, tmp_module_functions_list.values()){
                    tmp_action = tmp_module_menu->addAction(tr("Fonction %1").arg(tmp_callback->getName()));
                    tmp_action->setToolTip(QString("<p>%1</p>").arg(tmp_callback->getDescription()));
                    connect(tmp_action, SIGNAL(hovered()), this, SLOT(showAutomatedActionValidationToolTip()));
                    tmp_action->setData(QVariant::fromValue<void*>(tmp_callback));
                    tmp_action->setProperty("type", CURRENT_ACTION_ADD_FUNCTION);
                }
                if (tmp_module_menu->isEmpty()){
                    tmp_action = tmp_module_menu->addAction(tr("Aucune fonction disponible"));
                    tmp_action->setFont(QFont(tmp_action->font().family(), -1, -1, true));
                    tmp_action->setProperty("type", ACTION_NONE);
                  }
            }
        }
        if (tmp_function_menu->isEmpty()){
            tmp_action = tmp_function_menu->addAction(tr("Aucun module disponible"));
            tmp_action->setFont(QFont(tmp_action->font().family(), -1, -1, true));
            tmp_action->setProperty("type", ACTION_NONE);
          }
    }

    tmp_action = tmp_menu->exec(QCursor::pos());
    if (tmp_action){
        tmp_automated_action_validation = static_cast<AutomatedActionValidation*>(tmp_action->property("record").value<void*>());
        if (tmp_automated_action_validation && tmp_automated_action){
            int tmp_child_index = tmp_automated_action->getChilds().indexOf(tmp_automated_action_validation);
            switch (tmp_action->property("type").toInt()) {
            case CURRENT_ACTION_REMOVE_FUNCTION:
                tmp_automated_action->removeChild(tmp_automated_action_validation);
                break;
            case CURRENT_ACTION_MOVE_FUNCTION_UP:
                tmp_automated_action->moveChild(tmp_child_index, tmp_child_index - 1);
                break;
            case CURRENT_ACTION_MOVE_FUNCTION_DOWN:
                tmp_automated_action->moveChild(tmp_child_index, tmp_child_index + 1);
                break;
            default:
                break;
            }
        }else{
            switch (tmp_action->property("type").toInt()) {
            case CURRENT_ACTION_ADD_FUNCTION:
                tmp_callback = (AutomationCallbackFunction*)tmp_action->data().value<void*>();
                if (tmp_callback && tmp_automated_action){
                    tmp_automated_action_validation = new AutomatedActionValidation();
                    tmp_automated_action_validation->setValueForKey(tmp_callback->getModule()->getModuleName().toStdString().c_str(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_NAME);
                    tmp_automated_action_validation->setValueForKey(tmp_callback->getModule()->getModuleVersion().toStdString().c_str(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_VERSION);
                    tmp_automated_action_validation->setValueForKey(tmp_callback->getName().toStdString().c_str(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME);
                    tmp_automated_action->addChild(tmp_automated_action_validation);
                }
                break;

            case ALL_ACTIONS_ADD_FUNCTION:

                foreach(tmp_automated_action, _m_automated_actions_model->getRecordsList()){
                    tmp_automated_action_validation = new AutomatedActionValidation();
                    tmp_automated_action_validation->setValueForKey(tmp_callback->getModule()->getModuleName().toStdString().c_str(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_NAME);
                    tmp_automated_action_validation->setValueForKey(tmp_callback->getModule()->getModuleVersion().toStdString().c_str(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_VERSION);
                    tmp_automated_action_validation->setValueForKey(tmp_callback->getName().toStdString().c_str(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME);
                    tmp_automated_action->addChild(tmp_automated_action_validation);
                }
                break;

            case ALL_ACTIONS_REMOVE_FUNCTION:
                foreach(tmp_automated_action, _m_automated_actions_model->getRecordsList()){
                    tmp_automated_action->removeAllChilds();
                }

                break;
            }

        }
    }

    delete tmp_menu;

    _m_ui->automated_actions_list->resizeRowsToContents();

#endif
}


void Form_Test::showAutomatedActionValidationToolTip()
{
    QToolTip::showText(QCursor::pos(), dynamic_cast<QAction*>(sender())->toolTip());
}


void Form_Test::stopRecordProcess()
{
    if (_m_is_automated_program_recording)
        stopRecordSystemEvents();

    if (_m_automated_program_record_process.state() == QProcess::Running)
        _m_automated_program_record_process.terminate();
}


void Form_Test::stopPlaybackProcess()
{
    if (_m_is_automated_program_playbacking)
        stopPlaybackSystemEvents();

    if (_m_automated_program_playback_process.state() == QProcess::Running)
        _m_automated_program_playback_process.terminate();
}

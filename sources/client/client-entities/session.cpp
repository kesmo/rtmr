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

#include "session.h"

Session* Session::_m_unique_session_instance = NULL;

Session::Session()
{
    _m_user = NULL;
    m_session = NULL;

}


Session::~Session()
{
    unloadUserInfos();
    cl_disconnect(&m_session);
    Record::init(m_session);
}


Session* Session::instance()
{
    if (!_m_unique_session_instance){
        _m_unique_session_instance = new Session();
    }
    return _m_unique_session_instance;
}


void Session::initialize()
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    qApp->setApplicationVersion(APP_VERSION);

    readSettings();
}


/**
  Charger l'utilisateur connecte
**/
int Session::loadUserInfos(const char* in_username)
{
    int                         tmp_result = NOERR;
    User                     	**tmp_users = NULL;
    unsigned long               tmp_users_count = 0;
    char                        *tmp_statement = m_session->m_where_clause_buffer;

    unsigned long               tmp_requirements_categories_count;
    RequirementCategory      **tmp_requirements_categories;

    unsigned long               tmp_tests_results_count;
    TestResult               **tmp_tests_results;

    unsigned long               tmp_actions_results_count;
    ActionResult             **tmp_actions_results;

    unsigned long              tmp_requirements_status_count;
    RequirementStatus      **tmp_requirements_status;

    unsigned long              tmp_tests_types_count;
    TestType                **tmp_tests_types;

    unloadUserInfos();

    sprintf(tmp_statement, "%s='%s'", USERS_TABLE_USERNAME, in_username);

    tmp_users = Record::loadRecords<User>(m_session, &users_table_def, tmp_statement, NULL, &tmp_users_count);
    if (tmp_users != NULL)
    {
        if (tmp_users_count == 1)
        {
            _m_user = tmp_users[0];
        tmp_users = Record::freeRecords<User>(tmp_users, 0);
        }
        else
        {
            _m_user = NULL;
        tmp_users = Record::freeRecords<User>(tmp_users, tmp_users_count);
        }
    }

    tmp_requirements_categories = Record::loadRecords<RequirementCategory>(m_session, &requirements_categories_table_def, NULL, NULL, &tmp_requirements_categories_count);
    if (tmp_requirements_categories != NULL)
    {
        for(unsigned long tmp_index = 0; tmp_index < tmp_requirements_categories_count; tmp_index++)
        {
            _m_requirements_categories_list.append(tmp_requirements_categories[tmp_index]);
        }

        tmp_requirements_categories = Record::freeRecords<RequirementCategory>(tmp_requirements_categories, 0);
    }


    tmp_tests_results = Record::loadRecords<TestResult>(m_session, &tests_results_table_def, NULL, NULL, &tmp_tests_results_count);
    if (tmp_tests_results != NULL)
    {
        for(unsigned long tmp_index = 0; tmp_index < tmp_tests_results_count; tmp_index++)
                {
            _m_tests_results_list.append(tmp_tests_results[tmp_index]);
                }
        tmp_tests_results = Record::freeRecords<TestResult>(tmp_tests_results, 0);
    }

    tmp_actions_results = Record::loadRecords<ActionResult>(m_session, &actions_results_table_def, NULL, NULL, &tmp_actions_results_count);
    if (tmp_actions_results != NULL)
    {
        for(unsigned long tmp_index = 0; tmp_index < tmp_actions_results_count; tmp_index++)
                {
            _m_actions_results_list.append(tmp_actions_results[tmp_index]);
                }
        tmp_actions_results = Record::freeRecords<ActionResult>(tmp_actions_results, 0);
    }

    tmp_requirements_status = Record::loadRecords<RequirementStatus>(m_session, &status_table_def, NULL, NULL, &tmp_requirements_status_count);
    if (tmp_requirements_status != NULL)
    {
        for(unsigned long tmp_index = 0; tmp_index < tmp_requirements_status_count; tmp_index++)
        {
            _m_requirements_status_list.append(tmp_requirements_status[tmp_index]);
        }
        tmp_requirements_status = Record::freeRecords<RequirementStatus>(tmp_requirements_status, 0);
    }

    tmp_tests_types = Record::loadRecords<TestType>(m_session, &tests_types_table_def, NULL, NULL, &tmp_tests_types_count);
    if (tmp_tests_types != NULL)
    {
        for(unsigned long tmp_index = 0; tmp_index < tmp_tests_types_count; tmp_index++)
        {
            _m_tests_types_list.append(tmp_tests_types[tmp_index]);
        }
        tmp_tests_types = Record::freeRecords<TestType>(tmp_tests_types, 0);
    }

    loadCustomFields();

    return tmp_result;
}

void Session::loadCustomFields()
{
    sprintf(Session::instance()->getClientSession()->m_where_clause_buffer, "%s='%s'", CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_ENTITY, CUSTOM_FIELDS_TEST);
    _m_custom_tests_fields_desc = CustomFieldDesc::loadRecordsList(Session::instance()->getClientSession()->m_where_clause_buffer);

    sprintf(Session::instance()->getClientSession()->m_where_clause_buffer, "%s='%s'", CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_ENTITY, CUSTOM_FIELDS_REQUIERMENT);
    _m_custom_requirements_fields_desc = CustomFieldDesc::loadRecordsList(Session::instance()->getClientSession()->m_where_clause_buffer);
}


/**
  Renvoie l'utilisateur connecte
**/
User* Session::user() const
{
    return _m_user;
}

void Session::unloadUserInfos()
{
    delete _m_user;
    _m_user = NULL;

    qDeleteAll(_m_requirements_categories_list);
    _m_requirements_categories_list.clear();

    qDeleteAll(_m_tests_results_list);
    _m_tests_results_list.clear();

    qDeleteAll(_m_actions_results_list);
    _m_actions_results_list.clear();

    qDeleteAll(_m_requirements_status_list);
    _m_requirements_status_list.clear();

    qDeleteAll(_m_tests_types_list);
    _m_tests_types_list.clear();

    qDeleteAll(_m_custom_tests_fields_desc);
    _m_custom_tests_fields_desc.clear();

    qDeleteAll(_m_custom_requirements_fields_desc);
    _m_custom_requirements_fields_desc.clear();

}



bool Session::addExternalModule(ClientModule *in_external_module)
{
    if (in_external_module != NULL)
    {
    if (!_m_externals_modules[in_external_module->getModuleType()].contains(in_external_module->getModuleName()))
    {
        _m_externals_modules[in_external_module->getModuleType()].insert(in_external_module->getModuleName(), in_external_module);
        return true;
    }
    }

    return false;
}



void Session::readSettings()
{
    QSettings			tmp_settings("rtmr", "options");

    /* Couleurs */
    QColor tmp_test_execution_ok_color = tmp_settings.value("test_execution_ok_color", QVariant(QColor(Qt::darkGreen))).value<QColor>();
    QColor tmp_test_execution_ko_color = tmp_settings.value("test_execution_ko_color", QVariant(QColor(Qt::red))).value<QColor>();
    QColor tmp_test_execution_incompleted_color = tmp_settings.value("test_execution_incompleted_color", QVariant(QColor(Qt::blue))).value<QColor>();
    QColor tmp_test_execution_by_passed_color = tmp_settings.value("test_execution_by_passed_color", QVariant(QColor(Qt::darkGray))).value<QColor>();

    ExecutionTest::initDefaultColors(
            tmp_test_execution_ok_color,
            tmp_test_execution_ko_color,
            tmp_test_execution_incompleted_color,
            tmp_test_execution_by_passed_color);

    /* Bugtrackers */
    foreach(ClientModule *tmp_module, externalsModules().value(ClientModule::BugtrackerPlugin).values())
    {
    QString	tmp_module_name = tmp_module->getModuleName();
    QString	tmp_setting_prefix = tmp_module_name.toLower().trimmed();
    std::string tmp_std_string = tmp_settings.value(tmp_setting_prefix + "_password", "").toString().toStdString();
    const char* tmp_crypted_pwd = decrypt_str(tmp_std_string.c_str());

    _m_bugtrackers_credentials[tmp_module_name] = QPair<QString, QString>(tmp_settings.value(tmp_setting_prefix + "_login", "").toString(),
        tmp_crypted_pwd);

    delete tmp_crypted_pwd;
    }

}



void Session::loadLanguage(const QString& in_language)
{

    if(_m_lang != in_language)
    {
        QSettings settings("rtmr", "options");

        settings.setValue("prefered_language", in_language);

        _m_lang = in_language;

        QLocale locale = QLocale(_m_lang);
        QLocale::setDefault(locale);

        qApp->removeTranslator(&_m_qt_translator);
        if (_m_qt_translator.load(":/languages/qt_" + in_language))
            qApp->installTranslator(&_m_qt_translator);

        qApp->removeTranslator(&_m_server_translator);
        if (_m_server_translator.load(QString(":/languages/server_") + in_language))
            qApp->installTranslator(&_m_server_translator);
        else if (in_language != "fr" && _m_server_translator.load(QString(":/languages/server_en")))
            qApp->installTranslator(&_m_server_translator);

        qApp->removeTranslator(&_m_client_translator);
        if (_m_client_translator.load(QString(":/languages/client_") + in_language))
            qApp->installTranslator(&_m_client_translator);
        else if (in_language != "fr" && _m_client_translator.load(QString(":/languages/client_en")))
            qApp->installTranslator(&_m_client_translator);
    }
}


void Session::setBugtrackerCredential(QString in_module_name, QPair<QString, QString> in_login_password)
{
    _m_bugtrackers_credentials[in_module_name] = in_login_password;
}


AutomationCallbackFunction* Session::getAutomationCallbackFunction(const QString &moduleName, const QString &moduleVersion, const QString &functionName)
{
    ClientModule        *tmp_module = NULL;
    AutomationModule    *tmp_automation_module = NULL;
    QMap < QString, ClientModule*>	tmp_modules_map = Session::instance()->externalsModules().value(ClientModule::AutomationPlugin);

    tmp_module = tmp_modules_map[moduleName];
    if (tmp_module){
        if (tmp_module->getModuleVersion() >= moduleVersion){
            tmp_automation_module = static_cast<AutomationModule*>(tmp_module);
            if (tmp_automation_module){
                QMap<QString, AutomationCallbackFunction*> tmp_module_functions_map = tmp_automation_module->getFunctionsMap();
                if (tmp_module_functions_map.contains(functionName))
                    return tmp_module_functions_map[functionName];
            }
        }
    }
    return NULL;
}

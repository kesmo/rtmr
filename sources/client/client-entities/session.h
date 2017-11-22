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

#ifndef SESSION_H
#define SESSION_H

#include <QProgressDialog>
#include "user.h"
#include "requirementcontent.h"
#include "executionaction.h"
#include "executiontest.h"
#include "customfielddesc.h"

#define TR_SERVER_MESSAGES_CONTEXT "Server_Messages"
#define TR_CUSTOM_MESSAGES_CONTEXT "Custom_Messages"

#define TR_SERVER_MESSAGE(message)  qApp->translate(TR_SERVER_MESSAGES_CONTEXT, message)

#define TR_CUSTOM_MESSAGE(message)  qApp->translate(TR_CUSTOM_MESSAGES_CONTEXT, message)

#include <clientmodule.h>

class Session : public QObject
{
    public:
        static Session* instance();
        ~Session();

        void initialize();

        void readSettings();
        void loadLanguage(const QString& in_language);

        bool addExternalModule(ClientModule *in_external_module);
        const QMap<ClientModule::ModuleType, QMap< QString, ClientModule* > > & externalsModules() const {return _m_externals_modules;}

        int loadUserInfos(const char* in_username);
        void loadCustomFields();

        void unloadUserInfos();
        User* user() const;
        const QList<RequirementCategory*> & requirementsCategories() const {return _m_requirements_categories_list;}
        const QList<TestResult*> &  testsResults() const {return _m_tests_results_list;}
        const QList<ActionResult*> &  actionsResults() const {return _m_actions_results_list;}
        const QList<RequirementStatus*> &  requirementsStatus() const {return _m_requirements_status_list;}
        const QList<TestType*> &  testsTypes() const {return _m_tests_types_list;}
        const QList<CustomFieldDesc*> &  customTestsFieldsDesc() const {return _m_custom_tests_fields_desc;}
        const QList<CustomFieldDesc*> &  customRequirementsFieldsDesc() const {return _m_custom_requirements_fields_desc;}

        void setLastErrorMessage(const char* in_error_mess){sprintf(m_session->m_last_error_msg, "%s\n", in_error_mess);}
        void setLastErrorMessage(const int in_error_id){sprintf(m_session->m_last_error_msg, "%s\n", cl_get_error_message(m_session, in_error_id));}
        QString getLastErrorMessage() const {return TR_SERVER_MESSAGE(m_session->m_last_error_msg);}
        QString getErrorMessage(const int in_error_id) const {return TR_SERVER_MESSAGE(cl_get_error_message(m_session, in_error_id));}
        net_session* getClientSession() const {return m_session;}
        net_session** getClientSessionPtr() {return &m_session;}
        const QString& getLangage() const {return _m_lang;}
        const QString& getApplicationVersion() const {return _m_app_version;}

        QMap<QString, QPair<QString, QString> > getBugtrackersCredentials() const {return _m_bugtrackers_credentials;}
        void setBugtrackerCredential(QString in_module_name, QPair<QString, QString> in_login_password);

        AutomationCallbackFunction* getAutomationCallbackFunction(const QString &moduleName, const QString &moduleVersion, const QString &functionName);

    private:
        Session();
        static Session* _m_unique_session_instance;

        QMap<ClientModule::ModuleType, QMap< QString, ClientModule* > >      _m_externals_modules;

        net_session                  *m_session;

        User                     *_m_user;
        QList<RequirementCategory*>                 _m_requirements_categories_list;
        QList<TestResult*>                 _m_tests_results_list;
        QList<ActionResult*>                 _m_actions_results_list;
        QList<RequirementStatus*>                 _m_requirements_status_list;
        QList<TestType*>                 _m_tests_types_list;
        QMap<QString, QPair<QString, QString> >	_m_bugtrackers_credentials;
        QList<CustomFieldDesc*> _m_custom_tests_fields_desc;
        QList<CustomFieldDesc*> _m_custom_requirements_fields_desc;

        /* Traduction */
        QString         _m_lang;
        QTranslator	    _m_qt_translator;
        QTranslator	    _m_client_translator;
        QTranslator	    _m_server_translator;

        QString _m_app_version;

};

#endif // SESSION_H

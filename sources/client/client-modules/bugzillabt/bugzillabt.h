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

#ifndef BUGZILLABT_H_
#define BUGZILLABT_H_

#include "bugtracker.h"
#include "qxtxmlrpc/qxtxmlrpcclient.h"

#define	    WEBSERVICE_URL_SUFFIX	"xmlrpc.cgi"

/**
\brief Bugzilla bugtracker implementation.
*/
class Bugzillabt: public Bugtracker
{

    Q_OBJECT

public:
    Bugzillabt();
    ~Bugzillabt();

    virtual void setBaseUrl(QUrl in_base_url);
    virtual void setWebserviceUrl(QUrl in_url);

    void login();
    void logout();

    void getVersion();

    void getPriorities();
    void getSeverities();
    void getReproducibilities();

    void getPlatforms();
    void getOperatingSystems();
    void getAvailableProjectsIds();

    void getProjectsInformations(QList<QString> projects_ids);
    void getProjectComponents(QString project_id);
    void getProjectVersions(QString project_id);

    void addBug(QString project_id, QString project_name, QString component, QString version, QString revision, QString platform, QString system, QString summary, QString description, QVariant priority, QVariant severity, QVariant reproducibility);
    void getBugs(QList<QString> in_bugs_ids_list);

    virtual QString urlForBugWithId(QString in_bug_id);

private:
    QxtXmlRpcClient		    *_m_xmlrpc_client;
    QList<QPair<QString,QString> >  _m_cookies_list;

private Q_SLOTS:
    void loginBugzilla(QVariant retValue);
    void logoutBugzilla(QVariant retValue);

    void loadBugzillaVersion(QVariant retValue);

    void loadBugzillaPriorities(QVariant retValue);
    void loadBugzillaSeverities(QVariant retValue);

    void loadBugzillaPlatforms(QVariant retValue);
    void loadBugzillaOsTypes(QVariant retValue);
    void loadBugzillaAvailableProjectsId(QVariant retValue);

    void loadBugzillaProjectsInformations(QVariant retValue);
    void loadBugzillaProjectComponents(QVariant retValue);
    void loadBugzillaProjectVersions(QVariant retValue);

    void bugzillaIssueCreated(QVariant retValue);

    void loadBugzillaBugsInformations(QVariant retValue);

    void xmlRpcNetworkError(QNetworkReply::NetworkError networkError, QString errorMsg);
    void xmlRpcFault(int faultCode, QString faultString, QString response);
    void xmlRpcParseError(QString errorMsg, QString response);

};

#endif /* BUGZILLABT_H_ */

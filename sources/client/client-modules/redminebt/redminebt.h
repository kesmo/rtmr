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

#ifndef REDMINEBT_H
#define REDMINEBT_H

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QDomDocument>
#include "bugtracker.h"

class Redminebt : public Bugtracker
{
    Q_OBJECT

public:
    Redminebt();
    ~Redminebt();

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

private Q_SLOTS:

    void loginRedmine(QDomDocument xmlDocument);

    void slotAuthenticationRequired(QNetworkReply* networkReply, QAuthenticator* authenticator);
    void loadRedmineVersion(QDomDocument xmlDocument);

    void loadRedminePriorities(QDomDocument xmlDocument);
    void loadRedmineSeverities(QDomDocument xmlDocument);

    void loadRedminePlatforms(QDomDocument xmlDocument);
    void loadRedmineOsTypes(QDomDocument xmlDocument);
    void loadRedmineAvailableProjectsId(QDomDocument xmlDocument);

    void loadRedmineProjectsInformations(QDomDocument xmlDocument);
    void loadRedmineProjectComponents(QDomDocument xmlDocument);
    void loadRedmineProjectVersions(QDomDocument xmlDocument);

    void RedmineIssueCreated(QDomDocument xmlDocument);

    void loadRedmineBugsInformations(QDomDocument xmlDocument);

    void requestFinished(QNetworkReply* networkReply);

private:
    QNetworkAccessManager   *_m_network_manager;

    typedef void (Redminebt::*XmlReplyFunction)(QDomDocument);

    XmlReplyFunction _m_current_reply_function;

    bool _m_authentication;
    // Map status id to a pair : name, is_closed
    QMap< QString, QPair<QString, bool> > _m_statuses;
    QMap< QString, QString > _m_categories;
    QMap< QString, QString > _m_versions;
};

#endif // REDMINEBT_H

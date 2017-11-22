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
#include "jirabt.h"
#include "qjson/parser.h"
#include "qjson/serializer.h"

#include <entities.h>

#include <QDebug>
#include <QSettings>

Jirabt::Jirabt()
    : Bugtracker(),
      _m_network_manager(NULL),
      _m_current_reply_function(NULL)
{
    _m_network_manager = new QNetworkAccessManager(this);

    connect(_m_network_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(_m_network_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));

#ifndef QT_NO_OPENSSL
    connect(_m_network_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif

}


Jirabt::~Jirabt()
{
    delete _m_network_manager;
}


void Jirabt::setBaseUrl(QUrl in_base_url)
{
    Bugtracker::setBaseUrl(in_base_url);

    if (_m_base_url.toString().endsWith('/'))
        setWebserviceUrl(_m_base_url.toString() + WEBSERVICE_URL_SUFFIX);
    else
        setWebserviceUrl(_m_base_url.toString() + "/" + WEBSERVICE_URL_SUFFIX);
}



void Jirabt::login()
{
    emit loginSignal();
}


void Jirabt::logout()
{
    emit logoutSignal();
}


void Jirabt::getVersion()
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_webservice_url.toString() + "/serverInfo";

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Jirabt::loadJiraVersion;
    _m_network_manager->get(networkRequest);
}


void Jirabt::getPriorities()
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_webservice_url.toString() + "/priority";

    networkRequest.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(_m_username).arg(_m_password).toAscii()).toBase64());

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Jirabt::loadJiraPriorities;
    _m_network_manager->get(networkRequest);

}


void Jirabt::getSeverities()
{
    // Not available
    emit severities(QMap<QString,QString>());
}



void Jirabt::getReproducibilities()
{
    // Not available
    emit reproducibilities(QMap<QString,QString>());
}

void Jirabt::getPlatforms()
{
    // Not available
    emit platforms(QList<QString>());
}


void Jirabt::getOperatingSystems()
{
    // Not available
    emit operatingSystems(QList<QString>());
}


void Jirabt::getAvailableProjectsIds()
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_webservice_url.toString() + "/project";

    networkRequest.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(_m_username).arg(_m_password).toAscii()).toBase64());

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Jirabt::loadJiraAvailableProjectsId;
    _m_network_manager->get(networkRequest);
}


void Jirabt::getProjectsInformations(QList<QString> /* projects_ids */)
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_webservice_url.toString() + "/project";

    networkRequest.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(_m_username).arg(_m_password).toAscii()).toBase64());

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Jirabt::loadJiraProjectsInformations;
    _m_network_manager->get(networkRequest);
}


void Jirabt::getProjectComponents(QString project_id)
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_webservice_url.toString() + "/project/" + project_id;

    networkRequest.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(_m_username).arg(_m_password).toAscii()).toBase64());

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Jirabt::loadJiraProjectComponents;
    _m_network_manager->get(networkRequest);
}


void Jirabt::getProjectVersions(QString project_id)
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_webservice_url.toString() + "/project/" + project_id;

    networkRequest.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(_m_username).arg(_m_password).toAscii()).toBase64());

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Jirabt::loadJiraProjectVersions;
    _m_network_manager->get(networkRequest);
}


void Jirabt::addBug(QString project_id, QString /* project_name */, QString component, QString version, QString /*revision*/, QString /*platform*/, QString /*system*/, QString summary, QString description, QVariant priority, QVariant /*severity*/, QVariant /* reproducibility */)
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_webservice_url.toString( )+ "/issue";
    QJson::Serializer serializer;
    QVariantMap issue;
    QVariantMap issueFields;
    QVariantMap issueProject;
    QVariantMap issueType;
    QVariantMap issuePriority;
    QVariantList issueComponents;
    QVariantMap issueComponent;
    QVariantList issueVersions;
    QVariantMap issueVersion;

    networkRequest.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(_m_username).arg(_m_password).toAscii()).toBase64());

    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    networkRequest.setUrl(requestUrl);

    issueVersion["name"] = version;
    issueVersions.append(issueVersion);
    issueFields["versions"] = issueVersions;

    issueComponent["name"] = component;
    issueComponents.append(issueComponent);
    issueFields["components"] = issueComponents;

    issueProject["key"] = project_id;
    issueFields["project"] = issueProject;

    issueType["id"] = "1";
    issueFields["issuetype"] = issueType;

    issuePriority["id"] = priority;
    issueFields["priority"] = issuePriority;

    issueFields["summary"] = summary;
    issueFields["description"] = description;

    issue["fields"] = issueFields;

    QByteArray jsonIssue = serializer.serialize(issue);

    _m_current_reply_function = &Jirabt::JiraIssueCreated;
    _m_network_manager->post(networkRequest, jsonIssue);
}


void Jirabt::getBugs(QList<QString> in_bugs_ids_list)
{
    _m_current_reply_function = &Jirabt::loadJiraBugsInformations;

    foreach(QString bug_id, in_bugs_ids_list)
    {
        QNetworkRequest networkRequest;
        QUrl requestUrl = _m_webservice_url.toString() + "/issue/" + bug_id;

        networkRequest.setRawHeader("Authorization", "Basic " + QByteArray(QString("%1:%2").arg(_m_username).arg(_m_password).toAscii()).toBase64());

        networkRequest.setUrl(requestUrl);

        _m_network_manager->get(networkRequest);
    }
}



void Jirabt::loginJira(QString /*jsonString*/)
{
    emit loginSignal();
}


void Jirabt::slotAuthenticationRequired(QNetworkReply* /* networkReply */, QAuthenticator* authenticator)
{
    authenticator->setUser(_m_username);
    authenticator->setPassword(_m_password);
}


void Jirabt::loadJiraVersion(QString jsonString)
{
    QJson::Parser parser;
    bool ok;

    QVariantMap jsonMap = parser.parse(jsonString.toAscii(), &ok).toMap();
    if (ok)
        emit version(jsonMap["version"].toString());
    else
        emit error("<b>qjson was unable to parse</b>: " + jsonString);
}


void Jirabt::loadJiraProjectsInformations(QString jsonString)
{
    QList< QPair<QString, QString> >	tmp_infos;
    QJson::Parser parser;
    bool ok;

    QVariantList jsonList = parser.parse(jsonString.toAscii(), &ok).toList();
    if (ok)
    {
        foreach(QVariant project, jsonList)
        {
            tmp_infos.append(QPair<QString, QString>(project.toMap().value("key").toString(), project.toMap().value("name").toString()));
        }
    }
    else
        emit error("<b>qjson was unable to parse</b>: " + jsonString);

    emit projectsInformations(tmp_infos);
}



void Jirabt::loadJiraPriorities(QString jsonString)
{
    QMap <QString, QString>	tmp_priorities;
    QJson::Parser parser;
    bool ok;

    QVariantList jsonList = parser.parse(jsonString.toAscii(), &ok).toList();
    if (ok)
    {
        foreach(QVariant tmp_priority, jsonList)
        {
            QVariantMap tmp_priority_fields = tmp_priority.toMap();
            tmp_priorities[tmp_priority_fields["id"].toString()] = tmp_priority_fields["name"].toString();
        }
    }
    else
        emit error("<b>qjson was unable to parse</b>: " + jsonString);

    emit priorities(tmp_priorities);

}

void Jirabt::loadJiraSeverities(QString /*jsonString*/)
{

}


void Jirabt::loadJiraPlatforms(QString /*jsonString*/)
{

}


void Jirabt::loadJiraOsTypes(QString /*jsonString*/)
{

}


void Jirabt::loadJiraAvailableProjectsId(QString jsonString)
{
    QList<QString>	tmp_projects_ids;
    QJson::Parser parser;
    bool ok;

    QVariantList jsonList = parser.parse(jsonString.toAscii(), &ok).toList();
    if (ok)
    {
        foreach(QVariant project, jsonList)
        {
            tmp_projects_ids.append(project.toMap().value("key").toString());
        }
    }
    else
        emit error("<b>qjson was unable to parse</b>: " + jsonString);

    emit availableProjectsIds(tmp_projects_ids);
}




void Jirabt::loadJiraProjectComponents(QString jsonString)
{
    QList<QString>	tmp_projects_components;
    QJson::Parser parser;
    bool ok;

    QVariantMap jsonMap = parser.parse(jsonString.toAscii(), &ok).toMap();
    if (ok)
    {
        QVariantList components = jsonMap["components"].toList();
        foreach(QVariant component, components)
        {
            tmp_projects_components.append(component.toMap().value("name").toString());
        }
    }
    else
        emit error("<b>qjson was unable to parse</b>: " + jsonString);

    emit projectComponents(tmp_projects_components);
}


void Jirabt::loadJiraProjectVersions(QString jsonString)
{
    QList<QString>	tmp_project_versions;
    QJson::Parser parser;
    bool ok;

    QVariantMap jsonMap = parser.parse(jsonString.toAscii(), &ok).toMap();
    if (ok)
    {
        QVariantList versions = jsonMap["versions"].toList();
        foreach(QVariant version, versions)
        {
            tmp_project_versions.append(version.toMap().value("name").toString());
        }
    }
    else
        emit error("<b>qjson was unable to parse</b>: " + jsonString);


    emit projectVersions(tmp_project_versions);
}


void Jirabt::loadJiraBugsInformations(QString jsonString)
{
    QMap< QString, QMap<Bugtracker::BugProperty, QString> >	tmp_infos;
    QJson::Parser parser;
    bool ok;

    QVariantMap jsonMap = parser.parse(jsonString.toAscii(), &ok).toMap();
    if (ok)
    {
        QVariantMap issueFields = jsonMap["fields"].toMap();
        QString tmp_id = jsonMap["key"].toString();

        QMap<BugProperty, QString>  tmp_bug_infos;

        tmp_bug_infos[BugId] = tmp_id;
        tmp_bug_infos[Description] = issueFields["description"].toString();
        tmp_bug_infos[Priority] = issueFields["priority"].toMap().value("name").toString();
        tmp_bug_infos[Summary] = issueFields["summary"].toString();
        tmp_bug_infos[CreationTime] = issueFields["created"].toMap().value("value").toString();
        tmp_bug_infos[ModificationTime] = issueFields["updated"].toMap().value("value").toString();
        int status = issueFields["status"].toMap().value("id").toInt();
        if (status < 5)
            tmp_bug_infos[Status] = BUG_STATUS_OPENED;
        else
            tmp_bug_infos[Status] = BUG_STATUS_CLOSED;

        tmp_infos[tmp_id] = tmp_bug_infos;
    }
    else
        emit error("<b>qjson was unable to parse</b>: " + jsonString);

    emit bugsInformations(tmp_infos);
}



void Jirabt::JiraIssueCreated(QString jsonString)
{
    QJson::Parser parser;
    bool ok;
    QVariantMap jsonMap = parser.parse(jsonString.toAscii(), &ok).toMap();
    if (ok)
        emit bugCreated(jsonMap["key"].toString());
    else
        emit error("<b>qjson was unable to parse</b>: " + jsonString);
}



QString Jirabt::urlForBugWithId(QString in_bug_id)
{
    return _m_base_url.toString() + "/browse/" + in_bug_id;
}


void Jirabt::requestFinished(QNetworkReply* networkReply)
{
    if (networkReply->error() != QNetworkReply::NoError)
    {
        emit error("<b>Network error</b>: " + networkReply->errorString() + " (" + QString::number(networkReply->error()) + ")");
    }
    else
    {
        QString jsonString = networkReply->readAll();
        qDebug() << Q_FUNC_INFO << jsonString;
        (this->*_m_current_reply_function)(jsonString);
    }
}

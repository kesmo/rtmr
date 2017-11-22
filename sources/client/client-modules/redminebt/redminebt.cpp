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
#include "redminebt.h"
#include <entities.h>

#include <QDebug>
#include <QSettings>

Redminebt::Redminebt()
    : Bugtracker(),
    _m_network_manager(NULL),
    _m_current_reply_function(NULL),
    _m_authentication(false)
{
    _m_network_manager = new QNetworkAccessManager(this);

    connect(_m_network_manager, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(slotAuthenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(_m_network_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(requestFinished(QNetworkReply*)));

#ifndef QT_NO_OPENSSL
    connect(_m_network_manager, SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif

}


Redminebt::~Redminebt()
{
    delete _m_network_manager;
}


void Redminebt::login()
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_base_url;

    requestUrl.setPath(_m_base_url.path() + "/issue_statuses.xml");

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Redminebt::loginRedmine;
    _m_network_manager->get(networkRequest);
}


void Redminebt::logout()
{
    emit logoutSignal();
}


void Redminebt::getVersion()
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_base_url;

    requestUrl.setPath(_m_base_url.path());

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Redminebt::loadRedmineVersion;
    _m_network_manager->get(networkRequest);
}


void Redminebt::getPriorities()
{
    // Priorities are read from settings (ini or registry) because Redmine REST API does not provide webservice that retreive priorities list
    // Location of those settings depends of the platform as describe the Qt doc below :
    //    On Unix systems, if the file format is NativeFormat, the following files are used by default:
    //
    //    $HOME/.config/MySoft/Star Runner.conf (Qt for Embedded Linux: $HOME/Settings/MySoft/Star Runner.conf)
    //    $HOME/.config/MySoft.conf (Qt for Embedded Linux: $HOME/Settings/MySoft.conf)
    //    /etc/xdg/MySoft/Star Runner.conf
    //    /etc/xdg/MySoft.conf
    //    On Mac OS X versions 10.2 and 10.3, these files are used by default:
    //
    //    $HOME/Library/Preferences/com.MySoft.Star Runner.plist
    //    $HOME/Library/Preferences/com.MySoft.plist
    //    /Library/Preferences/com.MySoft.Star Runner.plist
    //    /Library/Preferences/com.MySoft.plist
    //    On Windows, NativeFormat settings are stored in the following registry paths:
    //
    //    HKEY_CURRENT_USER\Software\MySoft\Star Runner
    //    HKEY_CURRENT_USER\Software\MySoft
    //    HKEY_LOCAL_MACHINE\Software\MySoft\Star Runner
    //    HKEY_LOCAL_MACHINE\Software\MySoft

    QSettings tmp_settings("rtmr", "redmine");

    QMap<QString,QString> tmp_priorities;

    int tmp_size = tmp_settings.beginReadArray("priorities");
    if (tmp_size == 0)
    {
	tmp_settings.endArray();

	tmp_priorities.insert("3", "Low");
	tmp_priorities.insert("4", "Normal");
	tmp_priorities.insert("5", "High");
	tmp_priorities.insert("6", "Urgent");
	tmp_priorities.insert("7", "Immediate");

	tmp_settings.beginWriteArray("priorities");

	tmp_settings.setArrayIndex(0);
	tmp_settings.setValue("value", "3");
	tmp_settings.setValue("label", "Low");

	tmp_settings.setArrayIndex(1);
	tmp_settings.setValue("value", "4");
	tmp_settings.setValue("label", "Normal");

	tmp_settings.setArrayIndex(2);
	tmp_settings.setValue("value", "5");
	tmp_settings.setValue("label", "High");

	tmp_settings.setArrayIndex(3);
	tmp_settings.setValue("value", "6");
	tmp_settings.setValue("label", "Urgent");

	tmp_settings.setArrayIndex(4);
	tmp_settings.setValue("value", "7");
	tmp_settings.setValue("label", "Immediate");

	tmp_settings.endArray();
    }
    else
    {
	for (int i = 0; i < tmp_size; ++i)
	{
	    tmp_settings.setArrayIndex(i);
	    tmp_priorities.insert(tmp_settings.value("value").toString(), tmp_settings.value("label").toString());
	}
	tmp_settings.endArray();
    }

    emit priorities(tmp_priorities);
}


void Redminebt::getSeverities()
{
    // Not available
    emit severities(QMap<QString,QString>());
}



void Redminebt::getReproducibilities()
{
    // Not available
    emit reproducibilities(QMap<QString,QString>());
}

void Redminebt::getPlatforms()
{
    // Not available
    emit platforms(QList<QString>());
}


void Redminebt::getOperatingSystems()
{
    // Not available
    emit operatingSystems(QList<QString>());
}


void Redminebt::getAvailableProjectsIds()
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_base_url;

    requestUrl.setPath(_m_base_url.path() + "/projects.xml");

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Redminebt::loadRedmineAvailableProjectsId;
    _m_network_manager->get(networkRequest);
}


void Redminebt::getProjectsInformations(QList<QString> /* projects_ids */)
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_base_url;

    requestUrl.setPath(_m_base_url.path() + "/projects.xml");

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Redminebt::loadRedmineProjectsInformations;
    _m_network_manager->get(networkRequest);
}


void Redminebt::getProjectComponents(QString project_id)
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_base_url;

    requestUrl.setPath(_m_base_url.path() + "/projects/" + project_id + "/issue_categories.xml");

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Redminebt::loadRedmineProjectComponents;
    _m_network_manager->get(networkRequest);
}


void Redminebt::getProjectVersions(QString project_id)
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_base_url;

    requestUrl.setPath(_m_base_url.path() + "/projects/" + project_id + "/versions.xml");

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Redminebt::loadRedmineProjectVersions;
    _m_network_manager->get(networkRequest);
}


void Redminebt::addBug(QString project_id, QString /* project_name */, QString component, QString version, QString /*revision*/, QString /*platform*/, QString /*system*/, QString summary, QString description, QVariant priority, QVariant /*severity*/, QVariant /* reproducibility */)
{
    QNetworkRequest networkRequest;
    QUrl requestUrl = _m_base_url;

    QDomDocument xmlDoc;

    QDomElement xmlIssueTag = xmlDoc.createElement("issue");
    xmlDoc.appendChild(xmlIssueTag);

    QDomElement xmlSubjectTag = xmlDoc.createElement("subject");
    xmlSubjectTag.appendChild(xmlDoc.createTextNode(summary.replace(">","&gt;").replace("<","&lt;")));
    xmlIssueTag.appendChild(xmlSubjectTag);

    QDomElement xmlProjectIdTag = xmlDoc.createElement("project_id");
    xmlProjectIdTag.appendChild(xmlDoc.createTextNode(project_id));
    xmlIssueTag.appendChild(xmlProjectIdTag);

    QDomElement xmlDescriptionTag = xmlDoc.createElement("description");
    xmlDescriptionTag.appendChild(xmlDoc.createTextNode(description.replace(">","&gt;").replace("<","&lt;")));
    xmlIssueTag.appendChild(xmlDescriptionTag);

    QDomElement xmlPriorityTag = xmlDoc.createElement("priority_id");
    xmlPriorityTag.appendChild(xmlDoc.createTextNode(priority.toString()));
    xmlIssueTag.appendChild(xmlPriorityTag);

    QString tmp_category_id = _m_categories.key(component);
    if (!tmp_category_id.isEmpty())
    {
	QDomElement xmlCategoryTag = xmlDoc.createElement("category_id");
	xmlCategoryTag.appendChild(xmlDoc.createTextNode(tmp_category_id));
	xmlIssueTag.appendChild(xmlCategoryTag);
    }

    QString tmp_version_id = _m_versions.key(version);
    if (!tmp_version_id.isEmpty())
    {
	QDomElement xmlVersionTag = xmlDoc.createElement("fixed_version_id");
	xmlVersionTag.appendChild(xmlDoc.createTextNode(tmp_version_id));
	xmlIssueTag.appendChild(xmlVersionTag);
    }


    qDebug() << Q_FUNC_INFO << xmlDoc.toString();

    requestUrl.setPath(_m_base_url.path() + "/issues.xml");
    networkRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");

    networkRequest.setUrl(requestUrl);

    _m_current_reply_function = &Redminebt::RedmineIssueCreated;
    _m_network_manager->post(networkRequest, xmlDoc.toByteArray());
}


void Redminebt::getBugs(QList<QString> in_bugs_ids_list)
{
    _m_current_reply_function = &Redminebt::loadRedmineBugsInformations;

    foreach(QString bug_id, in_bugs_ids_list)
    {
	QNetworkRequest networkRequest;
	QUrl requestUrl = _m_base_url;


	requestUrl.setPath(_m_base_url.path() + "/issues/" + bug_id + ".xml");

	networkRequest.setUrl(requestUrl);

	_m_network_manager->get(networkRequest);
    }
}



void Redminebt::loginRedmine(QDomDocument xmlDocument)
{
    // Récupérer la liste des status
    QDomNodeList xmlStatusNodes = xmlDocument.elementsByTagName("issue_status");

    _m_statuses.clear();

    for(int tmp_node_index = 0; tmp_node_index < xmlStatusNodes.count(); tmp_node_index++)
    {
        QDomNode	    tmp_status = xmlStatusNodes.at(tmp_node_index);

        if (!tmp_status.namedItem("id").isNull() && !tmp_status.namedItem("name").isNull())
        {
            if (!tmp_status.namedItem("is_closed").isNull())
                _m_statuses.insert(tmp_status.namedItem("id").toElement().text(), QPair<QString, bool>(tmp_status.namedItem("name").toElement().text(), tmp_status.namedItem("is_closed").toElement().text() == "true"));
            else
                _m_statuses.insert(tmp_status.namedItem("id").toElement().text(), QPair<QString, bool>(tmp_status.namedItem("name").toElement().text(), false));
        }
    }

    emit loginSignal();
}


void Redminebt::slotAuthenticationRequired(QNetworkReply* /* networkReply */, QAuthenticator* authenticator)
{
    if (!_m_authentication)
    {
	authenticator->setUser(_m_username);
	authenticator->setPassword(_m_password);
	_m_authentication = true;
    }
    else
    {
	_m_authentication = false;
    }
}


void Redminebt::loadRedmineVersion(QDomDocument /* xmlDocument */)
{
    QString versionInfos = "Unknow.<BR>Although, the server seems to respond correctly, Redmine version information is not available from Redmine services.<BR>See <a href=\"http://www.redmine.org/projects/redmine/wiki/Rest_api\">Redmin REST API</a>.";

    emit version(versionInfos);
}


void Redminebt::loadRedmineProjectsInformations(QDomDocument xmlDocument)
{
    QList< QPair<QString, QString> >	tmp_infos;

    // Récupérer la liste des projets
    QDomNodeList xmlProjectNodes = xmlDocument.elementsByTagName("project");

    for(int tmp_node_index = 0; tmp_node_index < xmlProjectNodes.count(); tmp_node_index++)
    {
	QDomNode	    tmp_project = xmlProjectNodes.at(tmp_node_index);

    if (!tmp_project.namedItem("identifier").isNull() && !tmp_project.namedItem("name").isNull())
	{
        tmp_infos.append(QPair<QString, QString>(tmp_project.namedItem("identifier").toElement().text(), tmp_project.namedItem("name").toElement().text()));
	}
    }

    emit projectsInformations(tmp_infos);
}



void Redminebt::loadRedminePriorities(QDomDocument /*xmlDocument*/)
{

}

void Redminebt::loadRedmineSeverities(QDomDocument /*xmlDocument*/)
{

}


void Redminebt::loadRedminePlatforms(QDomDocument /*xmlDocument*/)
{

}


void Redminebt::loadRedmineOsTypes(QDomDocument /*xmlDocument*/)
{

}


void Redminebt::loadRedmineAvailableProjectsId(QDomDocument xmlDocument)
{
    QList<QString>	tmp_projects_ids;

    // Récupérer la liste des ids des projets
    QDomNodeList xmlProjectNodes = xmlDocument.elementsByTagName("project");

    for(int tmp_node_index = 0; tmp_node_index < xmlProjectNodes.count(); tmp_node_index++)
    {
	QDomNode	    tmp_project = xmlProjectNodes.at(tmp_node_index);

    if (!tmp_project.namedItem("identifier").isNull())
        tmp_projects_ids.append(tmp_project.namedItem("identifier").toElement().text());
    }

    emit availableProjectsIds(tmp_projects_ids);
}




void Redminebt::loadRedmineProjectComponents(QDomDocument xmlDocument)
{
    QList<QString>	tmp_projects_components;

    // Récupérer la liste des composants du projet
    QDomNodeList xmlComponentNodes = xmlDocument.elementsByTagName("issue_category");

    _m_categories.clear();

    for(int tmp_node_index = 0; tmp_node_index < xmlComponentNodes.count(); tmp_node_index++)
    {
    QDomNode	    tmp_component = xmlComponentNodes.at(tmp_node_index);

    if (!tmp_component.namedItem("id").isNull() && !tmp_component.namedItem("name").isNull())
	{
        _m_categories.insert(tmp_component.namedItem("id").toElement().text(), tmp_component.namedItem("name").toElement().text());
        tmp_projects_components.append(tmp_component.namedItem("name").toElement().text());
	}
    }

    emit projectComponents(tmp_projects_components);
}


void Redminebt::loadRedmineProjectVersions(QDomDocument xmlDocument)
{
    QList<QString>	tmp_project_versions;

    // Récupérer la liste des versions du projet
    QDomNodeList xmlVersionNodes = xmlDocument.elementsByTagName("version");

    _m_versions.clear();

    for(int tmp_node_index = 0; tmp_node_index < xmlVersionNodes.count(); tmp_node_index++)
    {
        QDomNode	    tmp_version = xmlVersionNodes.at(tmp_node_index);

        if (!tmp_version.namedItem("id").isNull() && !tmp_version.namedItem("name").isNull())
        {
            _m_versions.insert(tmp_version.namedItem("id").toElement().text(), tmp_version.namedItem("name").toElement().text());
            tmp_project_versions.append(tmp_version.namedItem("name").toElement().text());
        }
    }

    emit projectVersions(tmp_project_versions);
}


void Redminebt::loadRedmineBugsInformations(QDomDocument xmlDocument)
{
    QMap< QString, QMap<Bugtracker::BugProperty, QString> >	tmp_bug_infos;
    // Récupérer la liste des versions du projet
    QDomNodeList xmlBugNodes = xmlDocument.elementsByTagName("issue");

    for(int tmp_node_index = 0; tmp_node_index < xmlBugNodes.count(); tmp_node_index++)
    {
	QDomNode	    tmp_bug_node = xmlBugNodes.at(tmp_node_index);

	QDomNode	    tmp_bug_node_id = tmp_bug_node.namedItem("id");

	if (!tmp_bug_node_id.isNull())
	{
	    QString tmp_bug_id = tmp_bug_node_id.toElement().text();
	    if (!tmp_bug_id.isEmpty())
	    {
		QMap<Bugtracker::BugProperty, QString> tmp_bug_properties;

		tmp_bug_properties[BugId] = tmp_bug_id;

		QDomNode	    tmp_bug_node_subject = tmp_bug_node.namedItem("subject");
		if (!tmp_bug_node_subject.isNull())
		    tmp_bug_properties[Summary] = tmp_bug_node_subject.toElement().text();

		QDomNode	    tmp_bug_node_description = tmp_bug_node.namedItem("description");
		if (!tmp_bug_node_description.isNull())
		    tmp_bug_properties[Description] = tmp_bug_node_description.toElement().text();

		QDomNode	    tmp_bug_node_category = tmp_bug_node.namedItem("category");
		if (!tmp_bug_node_category.isNull() && tmp_bug_node_category.isElement())
		{
		    QDomAttr tmp_bug_node_category_name = tmp_bug_node_category.toElement().attributeNode("name");
		    if (!tmp_bug_node_category_name.isNull())
			tmp_bug_properties[Component] = tmp_bug_node_category_name.value();
		}

		QDomNode	    tmp_bug_node_priority = tmp_bug_node.namedItem("priority");
		if (!tmp_bug_node_priority.isNull() && tmp_bug_node_priority.isElement())
		{
		    QDomAttr tmp_bug_node_priority_id = tmp_bug_node_priority.toElement().attributeNode("name");
		    if (!tmp_bug_node_priority_id.isNull())
			tmp_bug_properties[Priority] = tmp_bug_node_priority_id.value();
		}

		QDomNode	    tmp_bug_node_status = tmp_bug_node.namedItem("status");
		if (!tmp_bug_node_status.isNull() && tmp_bug_node_status.isElement())
		{
		    QDomAttr tmp_bug_node_status_id = tmp_bug_node_status.toElement().attributeNode("id");
		    if (!tmp_bug_node_status_id.isNull())
		    {
			QString tmp_status_str = tmp_bug_node_status_id.value();
			if (tmp_status_str.isEmpty() || !_m_statuses.contains(tmp_status_str) || !_m_statuses[tmp_status_str].second)
			    tmp_bug_properties[Status] = BUG_STATUS_OPENED;
			else
			    tmp_bug_properties[Status] = BUG_STATUS_CLOSED;
		    }
		}

		QDomNode	    tmp_bug_node_creation_time = tmp_bug_node.namedItem("created_on");
		if (!tmp_bug_node_creation_time.isNull())
		    tmp_bug_properties[CreationTime] = tmp_bug_node_creation_time.toElement().text();

		QDomNode	    tmp_bug_node_modification_time = tmp_bug_node.namedItem("updated_on");
		if (!tmp_bug_node_modification_time.isNull())
		    tmp_bug_properties[ModificationTime] = tmp_bug_node_modification_time.toElement().text();

		tmp_bug_infos[tmp_bug_id] = tmp_bug_properties;
	    }
	}
    }


    emit bugsInformations(tmp_bug_infos);
}



void Redminebt::RedmineIssueCreated(QDomDocument xmlDocument)
{
    QDomNodeList xmlBugNodes = xmlDocument.elementsByTagName("issue");

    for(int tmp_node_index = 0; tmp_node_index < xmlBugNodes.count(); tmp_node_index++)
    {
	QDomNode	    tmp_bug_node = xmlBugNodes.at(tmp_node_index);

	QDomNode	    tmp_bug_node_id = tmp_bug_node.namedItem("id");

	if (!tmp_bug_node_id.isNull())
	{
	    QString tmp_bug_id = tmp_bug_node_id.toElement().text();
	    if (!tmp_bug_id.isEmpty())
	    {
		emit bugCreated(tmp_bug_id);
	    }
	}
    }
}



QString Redminebt::urlForBugWithId(QString in_bug_id)
{
    return _m_base_url.toString() + "/issues/" + in_bug_id;
}


void Redminebt::requestFinished(QNetworkReply* networkReply)
{
    if (networkReply->error() != QNetworkReply::NoError)
    {
	emit error("<b>Network error</b>: " + networkReply->errorString() + " (" + QString::number(networkReply->error()) + ")");
    }
    else
    {
	QDomDocument xmlDoc;
	QString xmlErrorMsg;
	int xmlErrorLine, xmlErrorColumn;

	if (xmlDoc.setContent(networkReply, false, &xmlErrorMsg, &xmlErrorLine, &xmlErrorColumn))
	{
	    (this->*_m_current_reply_function)(xmlDoc);
	}
	else
	    emit error(QString("<b>Xml parse error</b>: %1 (line %2, column %3)")
		       .arg(xmlErrorMsg)
		       .arg(xmlErrorLine)
		       .arg(xmlErrorColumn));
    }
}

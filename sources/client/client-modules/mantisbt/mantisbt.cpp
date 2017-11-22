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

#include "mantisbt.h"
#include <entities.h>

#include <QtNetwork>

Mantisbt::Mantisbt() : Bugtracker()
{
    _m_soap_http = NULL;
}


Mantisbt::~Mantisbt()
{
	delete _m_soap_http;
}



void Mantisbt::setBaseUrl(QUrl in_base_url)
{
    Bugtracker::setBaseUrl(in_base_url);

    if (in_base_url.toString().endsWith('/'))
	setWebserviceUrl(in_base_url.toString() + WEBSERVICE_URL_SUFFIX);
    else
	setWebserviceUrl(in_base_url.toString() + "/" + WEBSERVICE_URL_SUFFIX);
}


void Mantisbt::setWebserviceUrl(QUrl in_url)
{
    Bugtracker::setWebserviceUrl(in_url);

    if (_m_soap_http != NULL)
	delete _m_soap_http;

    _m_soap_http = new QtSoapHttpTransport();

    if (_m_webservice_url.scheme() == "https")
    {
        _m_soap_http->setHost(_m_webservice_url.host(), true, _m_webservice_url.port());
#ifndef QT_NO_OPENSSL
    	connect(_m_soap_http->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif
    }
    else
    {
	_m_soap_http->setHost(_m_webservice_url.host(), false, _m_webservice_url.port());
    }
}


void Mantisbt::login()
{
	emit loginSignal();
}

void Mantisbt::logout()
{
	emit logoutSignal();
}


void Mantisbt::getVersion()
{
    QtSoapMessage		tmp_version_request;

	connect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisVersion()));

	tmp_version_request.setMethod("mc_version");
	_m_soap_http->submitRequest(tmp_version_request, _m_webservice_url.path());

}

void Mantisbt::getPriorities()
{
    QtSoapMessage		tmp_priorities_request;

    connect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisPriorities()));

    // Chargement des priorites
    tmp_priorities_request.setMethod("mc_enum_priorities");
    tmp_priorities_request.addMethodArgument("username", "", _m_username);
    tmp_priorities_request.addMethodArgument("password", "", _m_password);
    _m_soap_http->submitRequest(tmp_priorities_request, _m_webservice_url.path());
}


void Mantisbt::getSeverities()
{
    QtSoapMessage		tmp_severities_request;

    // Chargement des gravites
    connect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisSeverities()));

    tmp_severities_request.setMethod("mc_enum_severities");
    tmp_severities_request.addMethodArgument("username", "", _m_username);
    tmp_severities_request.addMethodArgument("password", "", _m_password);
    _m_soap_http->submitRequest(tmp_severities_request, _m_webservice_url.path());

}


void Mantisbt::getReproducibilities()
{
    QtSoapMessage		tmp_reproducibilities_request;

    connect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisReproducibilities()));

    // Chargement des reproductibilites
    tmp_reproducibilities_request.setMethod("mc_enum_reproducibilities");
    tmp_reproducibilities_request.addMethodArgument("username", "", _m_username);
    tmp_reproducibilities_request.addMethodArgument("password", "", _m_password);
    _m_soap_http->submitRequest(tmp_reproducibilities_request, _m_webservice_url.path());
}


void Mantisbt::getPlatforms()
{
    emit platforms(QList<QString>());
}


void Mantisbt::getOperatingSystems()
{
    emit operatingSystems(QList<QString>());
}


void Mantisbt::getAvailableProjectsIds()
{
    emit availableProjectsIds(QList<QString>());
}


void Mantisbt::getProjectsInformations(QList<QString> /* projects_ids */)
{
    QtSoapMessage		tmp_request;

    connect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisProjectsInformations()));

    tmp_request.setMethod("mc_projects_get_user_accessible");
    tmp_request.addMethodArgument("username", "", _m_username);
    tmp_request.addMethodArgument("password", "", _m_password);
    _m_soap_http->submitRequest(tmp_request, _m_webservice_url.path());
}


void Mantisbt::getProjectComponents(QString project_id)
{
    QtSoapMessage		tmp_request;

    connect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisProjectComponents()));

    tmp_request.setMethod("mc_project_get_categories");
    tmp_request.addMethodArgument("username", "", _m_username);
    tmp_request.addMethodArgument("password", "", _m_password);
    tmp_request.addMethodArgument("project_id", "", project_id);
    _m_soap_http->submitRequest(tmp_request, _m_webservice_url.path());
}


void Mantisbt::getProjectVersions(QString project_id)
{
    QtSoapMessage		tmp_request;

    connect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisProjectVersions()));

    tmp_request.setMethod("mc_project_get_versions");
    tmp_request.addMethodArgument("username", "", _m_username);
    tmp_request.addMethodArgument("password", "", _m_password);
    tmp_request.addMethodArgument("project_id", "", project_id);
    _m_soap_http->submitRequest(tmp_request, _m_webservice_url.path());
}


void Mantisbt::addBug(QString project_id, QString project_name, QString component, QString version, QString revision, QString platform, QString system, QString summary, QString description, QVariant priority, QVariant severity, QVariant reproducibility)
{
    QtSoapStruct 	*tmp_issue_data = new QtSoapStruct(QtSoapQName("issue"));
    QtSoapStruct 	*tmp_project = new QtSoapStruct(QtSoapQName("project"));
    QtSoapStruct 	*tmp_priority = new QtSoapStruct(QtSoapQName("priority"));
    QtSoapStruct 	*tmp_severity = new QtSoapStruct(QtSoapQName("severity"));
    QtSoapStruct 	*tmp_reproducibility = new QtSoapStruct(QtSoapQName("reproducibility"));
    QtSoapMessage	tmp_request;

    connect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(mantisIssueCreated()));

    tmp_request.setMethod("mc_issue_add");
    tmp_request.addMethodArgument("username", "", _m_username);
    tmp_request.addMethodArgument("password", "", _m_password);

    tmp_project->insert(new QtSoapSimpleType(QtSoapQName("id"), project_id));
    tmp_project->insert(new QtSoapSimpleType(QtSoapQName("name"), project_name));

    tmp_priority->insert(new QtSoapSimpleType(QtSoapQName("id"), priority.toInt()));
    tmp_severity->insert(new QtSoapSimpleType(QtSoapQName("id"), severity.toInt()));
    tmp_reproducibility->insert(new QtSoapSimpleType(QtSoapQName("id"), reproducibility.toInt()));

    tmp_issue_data->insert(tmp_project);
    tmp_issue_data->insert(tmp_priority);
    tmp_issue_data->insert(tmp_severity);
    tmp_issue_data->insert(tmp_reproducibility);
    tmp_issue_data->insert(new QtSoapSimpleType(QtSoapQName("category"), component));
    tmp_issue_data->insert(new QtSoapSimpleType(QtSoapQName("version"), version));
    tmp_issue_data->insert(new QtSoapSimpleType(QtSoapQName("build"), revision));
    tmp_issue_data->insert(new QtSoapSimpleType(QtSoapQName("platform"), platform));
    tmp_issue_data->insert(new QtSoapSimpleType(QtSoapQName("os"), system));
    tmp_issue_data->insert(new QtSoapSimpleType(QtSoapQName("summary"), summary));
    tmp_issue_data->insert(new QtSoapSimpleType(QtSoapQName("description"), description));

    tmp_request.addMethodArgument(tmp_issue_data);

    // Submit the method request to the web service.
    _m_soap_http->submitRequest(tmp_request, _m_webservice_url.path());
}


void Mantisbt::getBugs(QList<QString> in_bugs_ids_list)
{
    connect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisBugInformations()));

    foreach(QString tmp_bug_id, in_bugs_ids_list)
    {
        QtSoapMessage   tmp_request;

        tmp_request.setMethod("mc_issue_get");
        tmp_request.addMethodArgument("username", "", _m_username);
        tmp_request.addMethodArgument("password", "", _m_password);
        tmp_request.addMethodArgument("issue_id", "", tmp_bug_id);

	_m_soap_http->submitRequest(tmp_request, _m_webservice_url.path());
    }
}



void Mantisbt::loadMantisVersion()
{
    QtSoapMessage		tmp_response = _m_soap_http->getResponse();

    disconnect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisVersion()));

    if (tmp_response.isFault())
    {
        emit error("<b>Query failed</b> mc_version : " + tmp_response.faultString().value().toString()+"("+QString::number(tmp_response.faultCode())+")<br>"+tmp_response.faultDetail().toString());
    	return;
    }

    const QtSoapType &tmp_results = tmp_response.returnValue();

    if (!tmp_results.isValid())
    {
    	emit error("<b>Invalid return value</b> mc_version :<br>" + tmp_response.toXmlString());
    	return;
    }

    emit version(tmp_results.value().toString());

}


void Mantisbt::loadMantisPriorities()
{
    QtSoapMessage		tmp_response = _m_soap_http->getResponse();
    QMap<QString, QString>	tmp_list;

    disconnect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisPriorities()));

    if (tmp_response.isFault())
    {
        emit error("<b>Query failed</b> mc_enum_priorities : " + tmp_response.faultString().value().toString()+"("+QString::number(tmp_response.faultCode())+")<br>"+tmp_response.faultDetail().toString());
    	return;
    }

    const QtSoapType &tmp_results = tmp_response.returnValue();

    if (!tmp_results.isValid())
    {
    	emit error("<b>Invalid return value</b> mc_enum_priorities :<br>" + tmp_response.toXmlString());
    	return;
    }

    for(int tmp_index = 0; tmp_index < tmp_results.count(); tmp_index++)
    {
    	const QtSoapType &tmp_object_ref = tmp_results[tmp_index];
    	if (tmp_object_ref.isValid())
    	{
			const QtSoapType &tmp_priority_id = tmp_object_ref["id"];
			const QtSoapType &tmp_priority_name = tmp_object_ref["name"];
			if (tmp_priority_id.isValid() && tmp_priority_name.isValid())
			{
				tmp_list.insert(tmp_priority_id.value().toString(), tmp_priority_name.value().toString());
			}
    	}
    }

    emit priorities(tmp_list);
}


void Mantisbt::loadMantisSeverities()
{
    QtSoapMessage		tmp_response = _m_soap_http->getResponse();
    QMap<QString, QString>	tmp_list;

    disconnect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisSeverities()));

    if (tmp_response.isFault())
    {
        emit error("<b>Query failed</b> mc_enum_severities : " + tmp_response.faultString().value().toString()+"("+QString::number(tmp_response.faultCode())+")<br>"+tmp_response.faultDetail().toString());
    	return;
    }

    const QtSoapType &tmp_results = tmp_response.returnValue();

    if (!tmp_results.isValid())
    {
    	emit error("<b>Invalid return value</b> mc_enum_severities :<br>" + tmp_response.toXmlString());
    	return;
    }

    for(int tmp_index = 0; tmp_index < tmp_results.count(); tmp_index++)
    {
    	const QtSoapType &tmp_object_ref = tmp_results[tmp_index];
    	if (tmp_object_ref.isValid())
    	{
			const QtSoapType &tmp_severity_id = tmp_object_ref["id"];
			const QtSoapType &tmp_severity_name = tmp_object_ref["name"];
			if (tmp_severity_id.isValid() && tmp_severity_name.isValid())
			{
				tmp_list.insert(tmp_severity_id.value().toString(), tmp_severity_name.value().toString());
			}
    	}
    }

    emit severities(tmp_list);
}


void Mantisbt::loadMantisReproducibilities()
{
    QtSoapMessage		tmp_response = _m_soap_http->getResponse();
    QMap<QString, QString>	tmp_list;

    disconnect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisReproducibilities()));

    if (tmp_response.isFault())
    {
        emit error("<b>Query failed</b> mc_enum_reproducibilities : " + tmp_response.faultString().value().toString()+"("+QString::number(tmp_response.faultCode())+")<br>"+tmp_response.faultDetail().toString());
    	return;
    }

    const QtSoapType &tmp_results = tmp_response.returnValue();

    if (!tmp_results.isValid())
    {
    	emit error("<b>Invalid return value</b> mc_enum_reproducibilities :<br>" + tmp_response.toXmlString());
    	return;
    }

    for(int tmp_index = 0; tmp_index < tmp_results.count(); tmp_index++)
    {
    	const QtSoapType &tmp_object_ref = tmp_results[tmp_index];
    	if (tmp_object_ref.isValid())
    	{
			const QtSoapType &tmp_reproducibility_id = tmp_object_ref["id"];
			const QtSoapType &tmp_reproducibility_name = tmp_object_ref["name"];
			if (tmp_reproducibility_id.isValid() && tmp_reproducibility_name.isValid())
			{
				tmp_list.insert(tmp_reproducibility_id.value().toString(), tmp_reproducibility_name.value().toString());
			}
    	}
    }

    emit reproducibilities(tmp_list);

}


void Mantisbt::loadMantisProjectsInformations()
{
    QtSoapMessage                           tmp_response = _m_soap_http->getResponse();
    QList< QPair<QString,QString> >         tmp_list;

    disconnect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisProjectsInformations()));

    if (tmp_response.isFault())
    {
        emit error("<b>Query failed</b> mc_projects_get_user_accessible : " + tmp_response.faultString().value().toString()+"("+QString::number(tmp_response.faultCode())+")<br>"+tmp_response.faultDetail().toString());
        return;
    }

    const QtSoapType &tmp_results = tmp_response.returnValue();

    if (!tmp_results.isValid())
    {
        emit error("<b>Invalid return value</b> mc_projects_get_user_accessible :<br>" + tmp_response.toXmlString());
        return;
    }

    for(int tmp_index = 0; tmp_index < tmp_results.count(); tmp_index++)
    {
        const QtSoapType &tmp_object_ref = tmp_results[tmp_index];
        if (tmp_object_ref.isValid())
        {
            const QtSoapType &tmp_project_id = tmp_object_ref["id"];
            const QtSoapType &tmp_project_name = tmp_object_ref["name"];
            if (tmp_project_id.isValid() && tmp_project_name.isValid())
            {
                    tmp_list.append(QPair<QString,QString>(tmp_project_id.value().toString(), tmp_project_name.value().toString()));
            }
        }
    }

    emit projectsInformations(tmp_list);

}


void Mantisbt::loadMantisProjectComponents()
{
    QtSoapMessage          tmp_response = _m_soap_http->getResponse();
    QList<QString>         tmp_list;

    disconnect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisProjectComponents()));

    if (tmp_response.isFault())
    {
        emit error("<b>Query failed</b> mc_project_get_categories : " + tmp_response.faultString().value().toString()+"("+QString::number(tmp_response.faultCode())+")<br>"+tmp_response.faultDetail().toString());
        return;
    }

    const QtSoapType &tmp_results = tmp_response.returnValue();

    if (!tmp_results.isValid())
    {
        emit error("<b>Invalid return value</b> mc_project_get_categories :<br>" + tmp_response.toXmlString());
        return;
    }

    for(int tmp_index = 0; tmp_index < tmp_results.count(); tmp_index++)
    {
        const QtSoapType &tmp_object_ref = tmp_results[tmp_index];
        if (tmp_object_ref.isValid())
        {
            tmp_list.append(tmp_object_ref.value().toString());
        }
    }

    emit projectComponents(tmp_list);

}


void Mantisbt::loadMantisProjectVersions()
{
    QtSoapMessage          tmp_response = _m_soap_http->getResponse();
    QList<QString>         tmp_list;

    disconnect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisProjectVersions()));

    if (tmp_response.isFault())
    {
        emit error("<b>Query failed</b> mc_project_get_versions : " + tmp_response.faultString().value().toString()+"("+QString::number(tmp_response.faultCode())+")<br>"+tmp_response.faultDetail().toString());
        return;
    }

    const QtSoapType &tmp_results = tmp_response.returnValue();

    if (!tmp_results.isValid())
    {
        emit error("<b>Invalid return value</b> mc_project_get_versions :<br>" + tmp_response.toXmlString());
        return;
    }

    for(int tmp_index = 0; tmp_index < tmp_results.count(); tmp_index++)
    {
        const QtSoapType &tmp_object_ref = tmp_results[tmp_index];
        if (tmp_object_ref.isValid())
        {
            const QtSoapType &tmp_version_name = tmp_object_ref["name"];
            if (tmp_version_name.isValid())
            {
                tmp_list.append(tmp_version_name.value().toString());
            }
        }
    }

    emit projectVersions(tmp_list);
}


void Mantisbt::mantisIssueCreated()
{
    QtSoapMessage		tmp_response = _m_soap_http->getResponse();

    disconnect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(mantisIssueCreated()));

    if (tmp_response.isFault())
    {
        emit error("<b>Query failed</b> mc_add_issue : " + tmp_response.faultString().value().toString()+"("+QString::number(tmp_response.faultCode())+")<br>"+tmp_response.faultDetail().toString());
	return;
    }

    const QtSoapType &tmp_results = tmp_response.returnValue();
    if (!tmp_results.isValid())
    {
    	emit error("<b>Invalid return value</b> mc_add_issue :<br>" + tmp_response.toXmlString());
	return;
    }

    emit bugCreated(tmp_results.value().toString());
}


void Mantisbt::loadMantisBugsInformations()
{
    QMap< QString, QMap<BugProperty, QString> >     tmp_infos;
    QtSoapMessage                           tmp_response = _m_soap_http->getResponse();

    disconnect(_m_soap_http, SIGNAL(responseReady()), this, SLOT(loadMantisBugsInformations()));

    if (tmp_response.isFault())
    {
        emit error("<b>Query failed</b> mc_projects_get_user_accessible : " + tmp_response.faultString().value().toString()+"("+QString::number(tmp_response.faultCode())+")<br>"+tmp_response.faultDetail().toString());
        return;
    }

    const QtSoapType &tmp_results = tmp_response.returnValue();

    if (!tmp_results.isValid())
    {
        emit error("<b>Invalid return value</b> mc_projects_get_user_accessible :<br>" + tmp_response.toXmlString());
        return;
    }

    for(int tmp_index = 0; tmp_index < tmp_results.count(); tmp_index++)
    {
        const QtSoapType &tmp_object_ref = tmp_results[tmp_index];
        if (tmp_object_ref.isValid())
        {
	    QMap<BugProperty, QString>  tmp_bugs_infos;

            QString tmp_id = tmp_object_ref["id"].value().toString();
            const QtSoapType &tmp_status  = tmp_object_ref["status"];
            const QtSoapType &tmp_category = tmp_object_ref["category"];
            const QtSoapType &tmp_priority = tmp_object_ref["priority"];
            const QtSoapType &tmp_severity = tmp_object_ref["severity"];
            const QtSoapType &tmp_summary = tmp_object_ref["summary"];

            tmp_bugs_infos[BugId] = tmp_id;
            tmp_bugs_infos[Component] = tmp_category["name"].value().toString();
            tmp_bugs_infos[Priority] = tmp_priority["name"].value().toString();
            tmp_bugs_infos[Severity] = tmp_severity["name"].value().toString();
            tmp_bugs_infos[Summary] = tmp_summary["name"].value().toString();
            QString tmp_status_str = tmp_status["id"].value().toString();
            // 80 = resolved, 90 = closed
            if (tmp_status_str.isEmpty() || (tmp_status_str.compare("80") != 0 && tmp_status_str.compare("90") != 0))
                tmp_bugs_infos[Status] = BUG_STATUS_OPENED;
            else
                tmp_bugs_infos[Status] = BUG_STATUS_CLOSED;

            tmp_infos[tmp_id] = tmp_bugs_infos;
        }
    }

    emit bugsInformations(tmp_infos);
}


void Mantisbt::loadMantisBugInformations()
{
    QMap< QString, QMap<BugProperty, QString> >     tmp_infos;
    QtSoapMessage                            tmp_response = _m_soap_http->getResponse();

    if (tmp_response.isFault())
    {
        emit error("<b>Query failed</b> mc_issue_get : " + tmp_response.faultString().value().toString()+"("+QString::number(tmp_response.faultCode())+")<br>"+tmp_response.faultDetail().toString());
        return;
    }

    const QtSoapType &tmp_results = tmp_response.returnValue();

    if (!tmp_results.isValid())
    {
        emit error("<b>Invalid return value</b> mc_issue_get :<br>" + tmp_response.toXmlString());
        return;
    }

    QMap<BugProperty, QString>  tmp_bugs_infos;

    QString tmp_id = tmp_results["id"].value().toString();
    const QtSoapType &tmp_status  = tmp_results["status"];
    const QtSoapType &tmp_category = tmp_results["category"];
    const QtSoapType &tmp_priority = tmp_results["priority"];
    const QtSoapType &tmp_severity = tmp_results["severity"];
    const QtSoapType &tmp_summary = tmp_results["summary"];

    tmp_bugs_infos[BugId] = tmp_id;
    tmp_bugs_infos[Component] = tmp_category["name"].value().toString();
    tmp_bugs_infos[Priority] = tmp_priority["name"].value().toString();
    tmp_bugs_infos[Severity] = tmp_severity["name"].value().toString();
    tmp_bugs_infos[Summary] = tmp_summary["name"].value().toString();
    QString tmp_status_str = tmp_status["id"].value().toString();
    // 80 = resolved, 90 = closed
    if (tmp_status_str.isEmpty() || (tmp_status_str.compare("80") != 0 && tmp_status_str.compare("90") != 0))
        tmp_bugs_infos[Status] = BUG_STATUS_OPENED;
    else
        tmp_bugs_infos[Status] = BUG_STATUS_CLOSED;

    tmp_infos[tmp_id] = tmp_bugs_infos;

    emit bugsInformations(tmp_infos);
}



QString Mantisbt::urlForBugWithId(QString in_bug_id)
{
    return _m_base_url.toString() + "/view.php?id=" + in_bug_id;
}

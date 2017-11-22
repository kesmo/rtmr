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

#include "bugzillabt.h"
#include <entities.h>

Bugzillabt::Bugzillabt() : Bugtracker()
{
    _m_xmlrpc_client = NULL;
}


Bugzillabt::~Bugzillabt()
{
	delete _m_xmlrpc_client;
}


void Bugzillabt::setBaseUrl(QUrl in_base_url)
{
    Bugtracker::setBaseUrl(in_base_url);

    if (_m_base_url.toString().endsWith('/'))
	setWebserviceUrl(_m_base_url.toString() + WEBSERVICE_URL_SUFFIX);
    else
	setWebserviceUrl(_m_base_url.toString() + "/" + WEBSERVICE_URL_SUFFIX);
}


void Bugzillabt::setWebserviceUrl(QUrl in_url)
{
    Bugtracker::setWebserviceUrl(in_url);

    if (_m_xmlrpc_client != NULL)
	delete _m_xmlrpc_client;

    _m_xmlrpc_client = new QxtXmlRpcClient(in_url);

    if (_m_webservice_url.scheme() == "https")
    {
        _m_xmlrpc_client->setHost(_m_webservice_url.host(), true, _m_webservice_url.port());
#ifndef QT_NO_OPENSSL
    	connect(_m_xmlrpc_client->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(sslErrors(QNetworkReply*,QList<QSslError>)));
#endif
    }
    else
    {
	_m_xmlrpc_client->setHost(_m_webservice_url.host(), false, _m_webservice_url.port());
    }

    connect(_m_xmlrpc_client, SIGNAL(networkError(QNetworkReply::NetworkError, QString)), this, SLOT(xmlRpcNetworkError(QNetworkReply::NetworkError, QString)));
    connect(_m_xmlrpc_client, SIGNAL(fault(int, QString, QString)), this, SLOT(xmlRpcFault(int, QString, QString)));
    connect(_m_xmlrpc_client, SIGNAL(parseError(QString, QString)), this, SLOT(xmlRpcParseError(QString, QString)));
}


void Bugzillabt::login()
{
    QList<QVariant>			tmp_params;
    QMap<QString, QVariant>	tmp_auth;

    tmp_auth["login"] = _m_username;
    tmp_auth["password"] = _m_password;
    tmp_auth["remember"] = true;
    tmp_params.append(tmp_auth);

    connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loginBugzilla(QVariant)));

    _m_xmlrpc_client->removeAllRequestHeaderValues();
    _m_xmlrpc_client->call("User.login", tmp_params);
}


void Bugzillabt::logout()
{
    QList<QVariant>			tmp_params;

    connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(logoutBugzilla(QVariant)));

    _m_xmlrpc_client->call("User.logout", tmp_params);
}


void Bugzillabt::getVersion()
{
    QList<QVariant>			tmp_params;

	connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaVersion(QVariant)));

	_m_xmlrpc_client->removeAllRequestHeaderValues();
	_m_xmlrpc_client->call("Bugzilla.version", tmp_params);
}

void Bugzillabt::getPriorities()
{
    QList<QVariant>						tmp_params;
    QMap<QString, QVariant>				tmp_params_map;

    tmp_params_map["field"] = "priority";
    tmp_params.append(tmp_params_map);

    connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaPriorities(QVariant)));
    _m_xmlrpc_client->call("Bug.legal_values", tmp_params);

}
void Bugzillabt::getSeverities()
{
    QList<QVariant>						tmp_params;
    QMap<QString, QVariant>				tmp_params_map;

    tmp_params_map["field"] = "severity";
    tmp_params.append(tmp_params_map);

    connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaSeverities(QVariant)));
    _m_xmlrpc_client->call("Bug.legal_values", tmp_params);
}



void Bugzillabt::getReproducibilities()
{
    emit reproducibilities(QMap <QString, QString>());
}

void Bugzillabt::getPlatforms()
{
    QList<QVariant>						tmp_params;
    QMap<QString, QVariant>				tmp_params_map;

    tmp_params_map["field"] = "platform";
    tmp_params.append(tmp_params_map);

    connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaPlatforms(QVariant)));
    _m_xmlrpc_client->call("Bug.legal_values", tmp_params);
}


void Bugzillabt::getOperatingSystems()
{
    QList<QVariant>						tmp_params;
    QMap<QString, QVariant>				tmp_params_map;

    tmp_params_map["field"] = "op_sys";
    tmp_params.append(tmp_params_map);

    connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaOsTypes(QVariant)));
    _m_xmlrpc_client->call("Bug.legal_values", tmp_params);
}


void Bugzillabt::getAvailableProjectsIds()
{
    QList<QVariant>						tmp_params;

    connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaAvailableProjectsId(QVariant)));
    _m_xmlrpc_client->call("Product.get_enterable_products", tmp_params);
}


void Bugzillabt::getProjectsInformations(QList<QString> projects_ids)
{
    QList<QVariant>		tmp_projects_ids;
    QList<QVariant>		tmp_params;
    QMap<QString, QVariant>	tmp_params_map;

    foreach(QString tmp_project_id, projects_ids)
    {
	tmp_projects_ids.append(tmp_project_id);
    }

    tmp_params_map["ids"] = tmp_projects_ids;
    tmp_params.append(tmp_params_map);

    connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaProjectsInformations(QVariant)));
    _m_xmlrpc_client->call("Product.get", tmp_params);
}


void Bugzillabt::getProjectComponents(QString project_id)
{
    QList<QVariant>						tmp_params;
    QMap<QString, QVariant>				tmp_params_map;

    tmp_params_map["field"] = "component";
	tmp_params_map["product_id"] =  project_id;
	tmp_params.append(tmp_params_map);

	connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaProjectComponents(QVariant)));
	_m_xmlrpc_client->call("Bug.legal_values", tmp_params);

}


void Bugzillabt::getProjectVersions(QString project_id)
{
    QList<QVariant>						tmp_params;
    QMap<QString, QVariant>				tmp_params_map;

    tmp_params_map["field"] = "version";
    tmp_params_map["product_id"] =  project_id;
    tmp_params.append(tmp_params_map);

    connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaProjectVersions(QVariant)));
    _m_xmlrpc_client->call("Bug.legal_values", tmp_params);

}


void Bugzillabt::addBug(QString /* project_id */, QString project_name, QString component, QString version, QString /* revision */, QString platform, QString system, QString summary, QString description, QVariant priority, QVariant severity, QVariant /* reproducibility */)
{
    QList<QVariant>		tmp_params;
    QMap<QString, QVariant>	tmp_params_map;

    tmp_params_map["product"] = project_name;
    tmp_params_map["component"] = component;
    tmp_params_map["version"] = version;

    tmp_params_map["platform"] = platform;
    tmp_params_map["op_sys"] = system;

    tmp_params_map["summary"] = summary;
    tmp_params_map["description"] = description;
    tmp_params_map["priority"] = priority.toString();
    tmp_params_map["severity"] = severity.toString();
    tmp_params.append(tmp_params_map);

    connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(bugzillaIssueCreated(QVariant)));

    _m_xmlrpc_client->call("Bug.create", tmp_params);
}


void Bugzillabt::getBugs(QList<QString> in_bugs_ids_list)
{
    QList<QVariant>		tmp_bugs_ids;
    QList<QVariant>		tmp_params;
    QMap<QString, QVariant>	tmp_params_map;

    foreach(QString in_bug_id, in_bugs_ids_list)
    {
            tmp_bugs_ids.append(in_bug_id);
    }

    tmp_params_map["ids"] = tmp_bugs_ids;
    tmp_params.append(tmp_params_map);

    connect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaBugsInformations(QVariant)));
    _m_xmlrpc_client->call("Bug.get", tmp_params);
}



void Bugzillabt::loginBugzilla(QVariant /* retValue */)
{
    QString								tmp_cookie;
    QPair <QString, QString>			tmp_pair;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loginBugzilla(QVariant)));

    /* Gestion des cookies de session */
    _m_cookies_list = _m_xmlrpc_client->responseHeaderValuesForKey("Set-Cookie");
    foreach(tmp_pair, _m_cookies_list)
    {
        if (!tmp_cookie.isEmpty())
        	tmp_cookie.append(';');

    	tmp_cookie.append(tmp_pair.second.section(';', 0, 0));
    }

    if (!tmp_cookie.isEmpty())
    	_m_xmlrpc_client->addRequestHeaderValueForKey(QPair<QString, QString>("Cookie", tmp_cookie));

    emit loginSignal();
}


void Bugzillabt::logoutBugzilla(QVariant /* retValue */)
{
    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(logoutBugzilla(QVariant)));

    emit logoutSignal();
}


void Bugzillabt::loadBugzillaVersion(QVariant retValue)
{
    QMap<QString, QVariant>	tmp_results = retValue.toMap();
    QString					tmp_informations;
    QString 				tmp_key;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaVersion(QVariant)));

    foreach(tmp_key, tmp_results.keys())
    {
        tmp_informations = tmp_informations + QString("Clé = ") + tmp_key + QString(", Valeur = ") + tmp_results.value(tmp_key).toString();
        emit version(tmp_results.value(tmp_key).toString());
    }
}


void Bugzillabt::loadBugzillaProjectsInformations(QVariant retValue)
{
    QMap<QString, QVariant>		tmp_results = retValue.toMap();
    QList<QVariant>			tmp_values_list;
    QMap< QString, QVariant >		tmp_product_infos_map;
    QVariant				tmp_product_infos;
    QList< QPair<QString, QString> >	tmp_infos;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaProjectsInformations(QVariant)));

    // Récupérer la liste des produits disponibles dans Bugzilla
    tmp_values_list = tmp_results.value("products").toList();

    foreach(tmp_product_infos, tmp_values_list)
    {
	tmp_product_infos_map = tmp_product_infos.toMap();
	tmp_infos.append(QPair<QString, QString>(tmp_product_infos_map.value("id").toString(), tmp_product_infos_map.value("name").toString()));
    }

    emit projectsInformations(tmp_infos);
}



void Bugzillabt::loadBugzillaPriorities(QVariant retValue)
{
    QMap<QString, QVariant>			tmp_results = retValue.toMap();
    QList<QVariant>				tmp_values_list;

    QMap <QString, QString>			tmp_list;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaPriorities(QVariant)));

    tmp_values_list = tmp_results.value("values").toList();

    foreach(QVariant tmp_value, tmp_values_list)
    {
	tmp_list.insert(tmp_value.toString(), tmp_value.toString());
    }

    emit priorities(tmp_list);
}

void Bugzillabt::loadBugzillaSeverities(QVariant retValue)
{
    QMap<QString, QVariant>			tmp_results = retValue.toMap();
    QList<QVariant>				tmp_values_list;

   QMap <QString, QString>			tmp_list;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaSeverities(QVariant)));

    tmp_values_list = tmp_results.value("values").toList();

    foreach(QVariant tmp_value, tmp_values_list)
    {
	tmp_list.insert(tmp_value.toString(), tmp_value.toString());
    }

    emit severities(tmp_list);
}


void Bugzillabt::loadBugzillaPlatforms(QVariant retValue)
{
    QMap<QString, QVariant>	tmp_results = retValue.toMap();
    QList<QVariant>		tmp_values_list;

    QList<QString>		tmp_list;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaPlatforms(QVariant)));

    tmp_values_list = tmp_results.value("values").toList();

    foreach(QVariant tmp_value, tmp_values_list)
    {
	tmp_list.append(tmp_value.toString());
    }

    emit platforms(tmp_list);
}


void Bugzillabt::loadBugzillaOsTypes(QVariant retValue)
{
    QMap<QString, QVariant>	tmp_results = retValue.toMap();
    QList<QVariant>		tmp_values_list;

    QList<QString>		tmp_list;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaOsTypes(QVariant)));

    tmp_values_list = tmp_results.value("values").toList();

    foreach(QVariant tmp_value, tmp_values_list)
    {
    	tmp_list.append(tmp_value.toString());
    }

    emit operatingSystems(tmp_list);
}


void Bugzillabt::loadBugzillaAvailableProjectsId(QVariant retValue)
{
    QMap<QString, QVariant>	tmp_results = retValue.toMap();
    QList<QVariant>		tmp_values_list;
    QList<QString>		tmp_list;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaAvailableProjectsId(QVariant)));

    tmp_values_list = tmp_results.value("ids").toList();

    foreach(QVariant tmp_value, tmp_values_list)
    {
    	tmp_list.append(tmp_value.toString());
    }

    emit availableProjectsIds(tmp_list);
}




void Bugzillabt::loadBugzillaProjectComponents(QVariant retValue)
{
    QMap<QString, QVariant>	tmp_results = retValue.toMap();
    QList<QVariant>		tmp_values_list;

    QList<QString>		tmp_list;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaProjectComponents(QVariant)));

    tmp_values_list = tmp_results.value("values").toList();

    foreach(QVariant tmp_value, tmp_values_list)
    {
    	tmp_list.append(tmp_value.toString());
    }

    emit projectComponents(tmp_list);
}


void Bugzillabt::loadBugzillaProjectVersions(QVariant retValue)
{
    QMap<QString, QVariant>	tmp_results = retValue.toMap();
    QList<QVariant>		tmp_values_list;

    QList<QString>		tmp_list;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaProjectVersions(QVariant)));

    tmp_values_list = tmp_results.value("values").toList();

    foreach(QVariant tmp_value, tmp_values_list)
    {
	tmp_list.append(tmp_value.toString());
    }

    emit projectVersions(tmp_list);
}


void Bugzillabt::loadBugzillaBugsInformations(QVariant retValue)
{
    QMap<QString, QVariant>		tmp_results = retValue.toMap();
    QList<QVariant>			tmp_values_list;
    QMap< QString, QVariant >		tmp_bug_infos_map;
    QMap< QString, QVariant >		tmp_bug_internals_infos_map;
    QVariant				tmp_bug_infos;
    QMap< QString, QMap<BugProperty, QString> > 	tmp_infos;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(loadBugzillaBugsInformations(QVariant)));

    // Récupérer la liste des bugs disponibles dans Bugzilla
    tmp_values_list = tmp_results.value("bugs").toList();

    foreach(tmp_bug_infos, tmp_values_list)
    {
	QMap<BugProperty, QString>  tmp_bugs_infos;

	tmp_bug_infos_map = tmp_bug_infos.toMap();
        QString tmp_id = tmp_bug_infos_map.value("id").toString();

        bool tmp_is_open = tmp_bug_infos_map.value("is_open").toBool();
        tmp_bug_internals_infos_map = tmp_bug_infos_map.value("internals").toMap();

        tmp_bugs_infos[BugId] = tmp_id;
        tmp_bugs_infos[Component] = tmp_bug_infos_map.value("component").toString();
        tmp_bugs_infos[Priority] = tmp_bug_infos_map.value("priority").toString();
        tmp_bugs_infos[Severity] = tmp_bug_infos_map.value("severity").toString();
        tmp_bugs_infos[Summary] = tmp_bug_infos_map.value("summary").toString();
        if (tmp_is_open)
            tmp_bugs_infos[Status] = BUG_STATUS_OPENED;
        else
            tmp_bugs_infos[Status] = BUG_STATUS_CLOSED;

        tmp_infos[tmp_id] = tmp_bugs_infos;
    }

    emit bugsInformations(tmp_infos);
}



void Bugzillabt::bugzillaIssueCreated(QVariant retValue)
{
    QMap<QString, QVariant>	    tmp_results = retValue.toMap();
    QVariant			    tmp_bug_id;

    disconnect(_m_xmlrpc_client, SIGNAL(finished(QVariant)), this, SLOT(bugzillaIssueSaved(QVariant)));

    tmp_bug_id = tmp_results.value("id");

    emit bugCreated(QString::number(tmp_bug_id.toInt()));
}


void Bugzillabt::xmlRpcNetworkError(QNetworkReply::NetworkError networkError,QString errorMsg)
{
    emit error("<b>XmlRpcNetworkError</b>: " + errorMsg + " (" + QString::number(networkError) + ")");
}


void Bugzillabt::xmlRpcFault(int faultCode, QString faultString, QString response)
{
    emit error("<b>XmlRpcFault</b>: " + faultString + " ("+  QString::number(faultCode) + ")<br><b>Response</b> : "+response);
}


void Bugzillabt::xmlRpcParseError(QString errorMsg, QString response)
{
    emit error("<b>XmlRpcParseError</b>: " + errorMsg + "<br><b>Response</b> : "+response);
}



QString Bugzillabt::urlForBugWithId(QString in_bug_id)
{
    return _m_base_url.toString() + "/show_bug.cgi?id=" + in_bug_id;
}

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

#ifndef BUGTRACKER_H
#define BUGTRACKER_H

#ifndef QT_NO_OPENSSL
#include <QSslError>
#endif

#include <QNetworkReply>
#include <QMap>

/**
\brief Main class for bugtrackers.
*/
class Bugtracker : public QObject
{

    Q_OBJECT

public:
    Bugtracker();
    virtual ~Bugtracker() = 0;


	/**
	\brief List of properties for projects.
	*/
    enum ProjectProperty {
	ProjectId,
	ProjectName,
	ProjectComponent,
	ProjectVersion,
	ProjectRevision,
    };

	/**
	\brief List of properties for bugs.
	*/
    enum BugProperty {
	BugId,
	Component,
	Platform,
	System,
	Summary,
	Description,
	Priority,
	Severity,
	Reproducibility,
	CreationTime,
        ModificationTime,
        Status
    };

	/**
	Defines the base url to access the bugtracker.
	*/
  virtual void setBaseUrl(QUrl in_base_url);
  
	/**
	Defines the webservice url to request the bugtracker.
	*/
  virtual void setWebserviceUrl(QUrl in_url);
  
	/**
	Defines the user informations used to access the bugtracker.
	*/
  virtual void setCredential(QString username, QString password);

	/**
	Log in to the bugtracker using Bugtracker::_m_username and Bugtracker::_m_password defined with Bugtracker::setCredential.<br>
	This function must call directly or indirectly Bugtracker::loginSignal signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void login() = 0;
  
	/**
	Log out the bugtracker.<br>
	This function must call directly or indirectly Bugtracker::logoutSignal signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void logout() = 0;

	/**
	Retrieve the bugtracker version.<br>
	This function must call directly or indirectly Bugtracker::version signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getVersion() = 0;

	/**
	Retrieve the available priorities list of the bugtracker.<br>
	This function must call directly or indirectly Bugtracker::priorities signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getPriorities() = 0;

	/**
	Retrieve the available severities list of the bugtracker.<br>
	This function must call directly or indirectly Bugtracker::severities signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getSeverities() = 0;

	/**
	Retrieve the available reproducibilities list of the bugtracker.<br>
	This function must call directly or indirectly Bugtracker::reproducibilities signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getReproducibilities() = 0;


	/**
	Retrieve the available platforms list of the bugtracker.<br>
	This function must call directly or indirectly Bugtracker::platforms signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getPlatforms() = 0;

	/**
	Retrieve the available operating systems list of the bugtracker.<br>
	This function must call directly or indirectly Bugtracker::operatingSystems signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getOperatingSystems() = 0;

	/**
	Retrieve the available projects ids list of the bugtracker.<br>
	This function must call directly or indirectly Bugtracker::availableProjectsIds signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getAvailableProjectsIds() = 0;


	/**
	Retrieve the projects informations from projects ids list parameter.<br>
	This function must call directly or indirectly Bugtracker::projectsInformations signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getProjectsInformations(QList<QString> projects_ids) = 0;

	/**
	Retrieve the components of the project from project id parameter.<br>
	This function must call directly or indirectly Bugtracker::projectComponents signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getProjectComponents(QString project_id) = 0;

	/**
	Retrieve the versions of the project from project id parameter.<br>
	This function must call directly or indirectly Bugtracker::projectVersions signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getProjectVersions(QString project_id) = 0;


	/**
	Add a new bug.<br>
	This function must call directly or indirectly Bugtracker::bugCreated signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void addBug(QString project_id, QString project_name, QString component, QString version, QString revision, QString platform, QString system, QString summary, QString description, QVariant priority, QVariant severity, QVariant reproducibility) = 0;

	/**
	Retrieve the informations of the bug from bug id parameter.<br>
	This function must call directly or indirectly Bugtracker::bugsInformations signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getBug(QString in_bug_id);

	/**
	Retrieve the informations of the bugs from bugs ids list parameter.<br>
	This function must call directly or indirectly Bugtracker::bugsInformations signal function if it is successful otherwise it should call Bugtracker::error signal function.
	*/
  virtual void getBugs(QList<QString> in_bugs_ids_list) = 0;


	/**
	Retrieve the url of the bug from bug id parameter.<br>
	*/
  virtual QString urlForBugWithId(QString in_bug_id) = 0;

public slots:
#ifndef QT_NO_OPENSSL
	/**
	This signal must be emitted if ssl request has failed.
	*/
  virtual void sslErrors(QNetworkReply *in_network_reply, QList<QSslError> in_ssl_errors_list);
#endif
Q_SIGNALS:
	
	/**
	This signal must be emitted when login request is successful.
	*/
	void loginSignal();
	
	/**
	This signal must be emitted when logout request is successful.
	*/
	void logoutSignal();

	
	/**
	This signal must be emitted when version request is successful.
	*/
	void version(QString);

	
	/**
	This signal must be emitted when priorities request is successful.
	*/
	void priorities(QMap <QString, QString>);
	
	/**
	This signal must be emitted when severities request is successful.
	*/
	void severities(QMap <QString, QString>);
	
	/**
	This signal must be emitted when reproducibilities request is successful.
	*/
	void reproducibilities(QMap <QString, QString>);

	
	/**
	This signal must be emitted when reproducibilities request is successful.
	*/
	void platforms(QList<QString>);
	
	/**
	This signal must be emitted when operatingSystems request is successful.
	*/
	void operatingSystems(QList<QString>);
	
	/**
	This signal must be emitted when availableProjectsIds request is successful.
	*/
	void availableProjectsIds(QList<QString>);

	
	/**
	This signal must be emitted when projectsInformations request is successful.
	It return a list of pairs containing for each project : id, name
	*/
	void projectsInformations(QList< QPair<QString, QString> >);
	
	/**
	This signal must be emitted when projectComponents request is successful.
	*/
	void projectComponents(QList<QString>);
	
	/**
	This signal must be emitted when projectVersions request is successful.
	*/
	void projectVersions(QList<QString>);

	/**
	This signal must be emitted when bugCreated request is successful.
	*/
	void bugCreated(QString);

	/**
	This signal must be emitted when bugsInformations request is successful.
	*/
	void bugsInformations(QMap< QString, QMap<Bugtracker::BugProperty, QString> >);

	/**
	This signal must be emitted when request has failed.
	*/
	void error(QString);

protected:
	/**
	Base url.
	*/
	QUrl	_m_base_url;
	
	/**
	Webservice url.
	*/
	QUrl	_m_webservice_url;
	
	/**
	Username used for log in to the bugtracker.
	*/
	QString	_m_username;
	
	/**
	Password used for log in to the bugtracker.
	*/
	QString	_m_password;
};

#endif // BUGTRACKER_H

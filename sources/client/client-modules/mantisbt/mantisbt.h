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

#ifndef MANTISBT_H_
#define MANTISBT_H_

#include "bugtracker.h"
#include "qtsoap.h"


#define	    WEBSERVICE_URL_SUFFIX	"api/soap/mantisconnect.php"

/**
\brief Mantis bugtracker implementation.
*/
class Mantisbt: public Bugtracker
{

    Q_OBJECT

public:
    Mantisbt();
    ~Mantisbt();

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

private Q_SLOTS:
	void loadMantisVersion();

	void loadMantisPriorities();
	void loadMantisSeverities();
	void loadMantisReproducibilities();

        void loadMantisProjectsInformations();
        void loadMantisProjectComponents();
        void loadMantisProjectVersions();

        void loadMantisBugInformations();
        void loadMantisBugsInformations();

	void mantisIssueCreated();

private:
    QtSoapHttpTransport	    *_m_soap_http;

};

#endif /* MANTISBT_H_ */

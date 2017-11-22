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

#ifndef FORM_BUG_H
#define FORM_BUG_H

#include <QtGui>
#include <QMap>
#include "ui_Form_Bug.h"
#include "executiontest.h"
#include "executionaction.h"
#include "projectversion.h"
#include "projectgrant.h"
#include "bugtracker.h"
#include "gui/components/abstract_project_widget.h"

QT_FORWARD_DECLARE_CLASS(Ui_Form_Bug);

class Form_Bug : public AbstractProjectWidget
{
    Q_OBJECT

public:
    Form_Bug(ProjectVersion *in_project_version, ExecutionTest *in_execution_test, ExecutionAction *in_execution_action, Bug *in_bug = NULL, QWidget *parent = 0, Qt::WindowFlags in_windows_flags = Qt::Dialog);
    ~Form_Bug();

    void loadBugtrackerProjectsInformations(QList<QString> in_projects_ids);
    void loadBugtrackerPriorities();
    void loadBugtrackerSeverities();
    void loadBugtrackerReproducibilities();
    void loadBugtrackerProjectComponents();
    void loadBugtrackerProjectVersions();
    void loadBugtrackerPlatforms();
    void loadBugtrackerOsTypes();
    void loadBugtrackerAvailableProjectsIds();
    void addBugtrackerIssue();
    void logoutBugtracker();

    bool generateBugDescriptionUntilAction(ExecutionTest *in_root_test, QString & description, QString in_prefix, ExecutionTest *in_test, ExecutionAction *in_action);

public Q_SLOTS:
    void save();
    void loginBugtracker();

    void bugtrackerLogin();
    void bugtrackerLogout();

    void bugtrackerPriorities(QMap<QString, QString>);
    void bugtrackerSeverities(QMap<QString, QString>);
    void bugtrackerReproducibilities(QMap<QString, QString>);

    void bugtrackerPlatforms(QList<QString>);
    void bugtrackerOperatingSystems(QList<QString>);
    void bugtrackerAvailableProjectsIds(QList<QString>);

    void bugtrackerProjectsInformations(QList< QPair<QString, QString> >);
    void bugtrackerProjectComponents(QList<QString>);
    void bugtrackerProjectVersions(QList<QString>);

    void bugtrackerBugCreated(QString);

    void bugtrackerError(QString);

    void updateBugFromBugtracker(QMap< QString, QMap<Bugtracker::BugProperty, QString> > in_bugs_list);

    void openExecutionCampaign();

Q_SIGNALS:
	void bugSaved();

private:
    Ui_Form_Bug 			*_m_ui;

    ProjectVersion       *_m_project_version;
    ExecutionTest        *_m_execution_test;
    ExecutionAction      *_m_execution_action;

    Bug			 *_m_bug;

    bool		 _m_new_bug;

    Bugtracker		 *_m_bugtracker;

    bool		_m_bugtracker_connection_active;

    QString		_m_project_id;
    QString		_m_project_name;

    void updateControls();
    void saveBug(QString in_bugtracker_bug_id);
};

#endif // FORM_BUG_H

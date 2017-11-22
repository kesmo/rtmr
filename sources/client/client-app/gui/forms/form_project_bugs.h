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

#ifndef FORM_PROJECT_BUGS_H
#define FORM_PROJECT_BUGS_H

#include "projectversion.h"
#include "bug.h"
#include "gui/components/abstract_project_widget.h"

#include <bugtracker.h>

#include <QWidget>
#include <QTableWidgetItem>

namespace Ui {
    class Form_Project_Bugs;
}

class Form_Project_Bugs : public AbstractProjectWidget {
    Q_OBJECT
public:
    Form_Project_Bugs(ProjectVersion *in_project_version, QWidget *parent = 0);
    ~Form_Project_Bugs();

public Q_SLOTS:
    void loadBugsList();
    void filterBugsList();

    void bugtrackerLogin();
    void launchUpdateBugsListFromBugtracker();
    void updateBugsListFromBugtracker(QMap< QString, QMap<Bugtracker::BugProperty, QString> >);
    void bugtrackerError(QString errorMsg);

    void deletedSelectedBugs(QList<Record*> in_bugs_list);
    void openBugAtIndex(QTableWidgetItem *in_item);

    void cancel();

Q_SIGNALS:
    void closed();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_Project_Bugs *_m_ui;

    ProjectVersion *_m_project_version;
    QList<Bug*>		_m_bugs_list;
    Bugtracker		 *_m_bugtracker;
    bool                _m_bugtracker_connection_active;

    bool            _m_has_bugtracker_error;

    void setBugForRow(Bug *in_bug, int in_row);
    void clearBugsList();
    void initExternalBugtracker();

};

#endif // FORM_PROJECT_BUGS_H

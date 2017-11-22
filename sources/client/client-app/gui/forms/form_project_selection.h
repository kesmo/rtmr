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

#ifndef FORM_PROJECT_SELECTION_H
#define FORM_PROJECT_SELECTION_H

#include <QDialog>
#include "project.h"
#include "projectversion.h"
#include "ui_Form_Project_Selection.h"

QT_FORWARD_DECLARE_CLASS(Ui_Form_Project_Selection);

class Form_Project_Selection : public QDialog
{
    Q_OBJECT

public:
    Form_Project_Selection(QWidget *parent = 0);
    ~Form_Project_Selection();

signals:
    void projectSelected(ProjectVersion *in_project);

public slots:
    void accept();
    void newVersion();
    void selectedProjectChanged(int);
    void selectedVersionChanged(int);
    void selectVersion(ProjectVersion*);

    void deleteProject();
    void deleteProjectVersion();

private:
    Ui_Form_Project_Selection   *_m_ui;
    Project                  **_m_projects_list;
    QList<ProjectVersion*>	_m_versions_list;
};

#endif // FORM_PROJECT_SELECTION_H

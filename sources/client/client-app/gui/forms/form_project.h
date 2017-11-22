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

#ifndef FORM_PROJECT_H
#define FORM_PROJECT_H

#include <QDialog>
#include "project.h"
#include "projectversion.h"
#include "gui/components/record_text_edit_container.h"

QT_FORWARD_DECLARE_CLASS(Ui_Form_Project);

class Form_Project : public QDialog
{
    Q_OBJECT

public:
    Form_Project(ProjectVersion* in_project_record = NULL, QWidget *parent = 0);
    ~Form_Project();

signals:
    void projectSelected(ProjectVersion *in_project);

public Q_SLOTS:
    void accept();

private:
    Ui_Form_Project   *_m_ui;
    ProjectVersion *_m_project_record;

    bool              _m_new_version;
};

#endif // FORM_PROJECT_H

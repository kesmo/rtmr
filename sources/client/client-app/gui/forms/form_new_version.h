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

#ifndef FORM_NEW_VERSION_H
#define FORM_NEW_VERSION_H

#include <QtGui/QDialog>
#include "projectversion.h"
#include "gui/components/record_text_edit_container.h"

namespace Ui {
    class Form_New_Version;
}

class Form_New_Version : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(Form_New_Version)
public:
    explicit Form_New_Version(Project *in_project, QList<ProjectVersion*> in_versions_list, QWidget *parent = 0);
    virtual ~Form_New_Version();

signals:
     void versionCreated(ProjectVersion *in_project_version);

public slots:
    void accept();
    void selectedVersionChanged(int);

private:
    Ui::Form_New_Version *_m_ui;

    Project          		*_m_project;
    QList<ProjectVersion*>	_m_versions_list;
};

#endif // FORM_NEW_VERSION_H

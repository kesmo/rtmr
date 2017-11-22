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

#ifndef FORM_SEARCH_PROJECT_H
#define FORM_SEARCH_PROJECT_H

#include <QtGui/QDialog>
#include <QList>
#include "projectversion.h"
#include "requirementhierarchy.h"
#include "testhierarchy.h"
#include "record.h"

namespace Ui {
    class Form_Search_Project;
}

class Form_Search_Project : public QDialog {
    Q_OBJECT
public:
    Form_Search_Project(ProjectVersion *in_project_version, QWidget *parent = 0);
    ~Form_Search_Project();

public slots:
    void accept();
    void updateControls();

    void launchTextualSearch();
    void launchTestsFromCurrentProjectVersionSearch();
    void launchRequirementsFromCurrentProjectVersionSearch();
    void launchNotCoveredRequirementsSearch();

signals:
    void foundRecords(const QList<Record*> & out_records_list);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_Search_Project *_m_ui;

    ProjectVersion	*_m_project_version;
};

#endif // FORM_SEARCH_PROJECT_H

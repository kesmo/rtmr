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

#ifndef FORM_USER_PROJECTS_GRANTS_H
#define FORM_USER_PROJECTS_GRANTS_H

#include <QtGui/QDialog>

namespace Ui {
    class Form_User_Projects_Grants;
}

class Form_User_Projects_Grants : public QDialog {
    Q_OBJECT
    Q_DISABLE_COPY(Form_User_Projects_Grants)
public:
    explicit Form_User_Projects_Grants(const char *in_username, QWidget *parent = 0);
    virtual ~Form_User_Projects_Grants();

    void loadProjectsGrants();
    void setProjectGrantAtIndex(char **in_project_grant, int in_index);

public Q_SLOTS:
    void accept();

protected:
    virtual void changeEvent(QEvent *e);

private:
    Ui::Form_User_Projects_Grants   *_m_ui;
    const char			    *_m_username;
    char			    ***_m_grants;
    unsigned long		    _m_rows_count;
    unsigned long		    _m_columns_count;

    bool			    *_m_existing_grant_array;
    bool			    *_m_modified_grant_array;

};

#endif // FORM_USER_PROJECTS_GRANTS_H

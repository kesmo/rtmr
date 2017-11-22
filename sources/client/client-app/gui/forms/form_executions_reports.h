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

#ifndef FORM_EXECUTIONS_REPORTS_H
#define FORM_EXECUTIONS_REPORTS_H

#include <QtGui>
#include "imaging/Chart.h"
#include "record.h"
#include "projectversion.h"
#include "ui_Form_Executions_Reports.h"
#include "gui/components/abstract_project_widget.h"

namespace Ui {
    class Form_Executions_Reports;
}

class Form_Executions_Reports : public AbstractProjectWidget
{
    Q_OBJECT

public:
    Form_Executions_Reports(ProjectVersion *in_project_version, QWidget *parent = 0);
    ~Form_Executions_Reports();

signals:
    void canceled();

public slots:
    void draw();
    void updateGraph();
    void save();
    void setTitleColor();
    void cancel();

protected:
    void init();
    TrinomeArray loadDatas();

private:
    Ui::Form_Executions_Reports			*_m_ui;

    ProjectVersion                  		*_m_projects_version;
    Chart					*_m_chart;
    QList<Trinome*>         _m_chart_data;

};

#endif // FORM_EXECUTIONS_REPORTS_H

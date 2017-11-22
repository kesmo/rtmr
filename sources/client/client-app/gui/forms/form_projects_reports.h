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

#ifndef FORMS_PROJECTS_REPORTS_H
#define FORMS_PROJECTS_REPORTS_H

#include <QtGui/QDialog>
#include "imaging/Chart.h"
#include "record.h"
#include "project.h"

namespace Ui {
    class Form_Projects_Reports;
}

class Form_Projects_Reports : public QDialog
{
    Q_OBJECT
    Q_DISABLE_COPY(Form_Projects_Reports)

public:
    explicit Form_Projects_Reports(QWidget *parent = 0);
    virtual ~Form_Projects_Reports();

public slots:
    void draw();
    void updateGraph();
    void saveAs();
    void setTitleColor();

protected:
    virtual void changeEvent(QEvent *e);
    void init();
    TrinomeArray loadDatas();

private:
    Ui::Form_Projects_Reports 	*_m_ui;

    Chart			*_m_chart;
    QList<Trinome*> _m_chart_data;
};

#endif // FORMS_PROJECTS_REPORTS_H

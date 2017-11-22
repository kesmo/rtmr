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

#ifndef FORM_EXECUTION_BUGS_H
#define FORM_EXECUTION_BUGS_H

#include <QDialog>
#include <QTableWidgetItem>

#include "executiontest.h"
#include "executionaction.h"

#include <bugtracker.h>

namespace Ui {
    class Form_Execution_Bugs;
}

class Form_Execution_Bugs : public QDialog {
    Q_OBJECT
public:
    Form_Execution_Bugs(ExecutionTest *in_execution_test, ExecutionAction *in_execution_action, QWidget *parent = 0);
    ~Form_Execution_Bugs();

public Q_SLOTS:
    void addBug();
    void updateBugsList();

    void deletedSelectedBugs(QList<Record*> in_bugs_list);
    void openBugAtIndex(QTableWidgetItem *in_item);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_Execution_Bugs	*_m_ui;

    ExecutionTest        *_m_execution_test;
    ExecutionAction      *_m_execution_action;

    QList<Bug*>		_m_bugs_list;

    Bugtracker		*_m_bugtracker;

    void setBugForRow(Bug *in_bug, int in_row);

};

#endif // FORM_EXECUTION_BUGS_H

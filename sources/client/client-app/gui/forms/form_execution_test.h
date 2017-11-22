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

#ifndef FORM_EXECUTION_TEST_H
#define FORM_EXECUTION_TEST_H

#include <QtGui/QWidget>

#include "executiontest.h"
#include "executionaction.h"
#include "testcontent.h"
#include "gui/components/record_text_edit_container.h"
#include "gui/components/test_action_attachments_manager.h"

namespace Ui {
    class Form_Execution_Test;
}

class Form_Execution_Test : public QWidget, public TestActionAttachmentsManager
{
    Q_OBJECT
    Q_DISABLE_COPY(Form_Execution_Test)
public:
    explicit Form_Execution_Test(QWidget *parent = 0);
    virtual ~Form_Execution_Test();

    void loadTest(ExecutionTest *in_execution_test, ExecutionAction *in_execution_action);

public Q_SLOTS:
    void modifyTest();
	void saveComments();
	void saveTestDescription();
	void validateResult();
	void inValidateResult();
	void bypass();
	void selectNext();
	void selectPrevious();
	void manageBugs();
	void initManageBugsButtonLabel();
        void modifyAction();
        void reinitExecutionsTestsParameters();

Q_SIGNALS:
	void resultValidated();
	void resultInValidated();
	void bypassed();
        void nextSelected();
        void previousSelected();

protected:
    virtual void changeEvent(QEvent *e);
    void initLayout();

    void setExecutionTestData(const char *in_result_id);
    void setExecutionActionData(const char *in_result_id);

private:
    Ui::Form_Execution_Test *_m_ui;

    ExecutionTest        *_m_execution_test;
    ExecutionAction      *_m_execution_action;
    TestContent		 *_m_test_content;

    QList<QString>					_m_current_parameters;

    void saveOriginalAction();
    void synchronizeExecutionsTestsParameters(QList<QString> in_parameters_list);

};

#endif // FORM_EXECUTION_TEST_H

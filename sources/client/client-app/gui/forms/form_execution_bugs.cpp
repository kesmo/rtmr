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

#include "form_execution_bugs.h"
#include "ui_Form_Execution_Bugs.h"
#include "form_bug.h"

#include "session.h"

#include <QMessageBox>

#include <clientmodule.h>

Form_Execution_Bugs::Form_Execution_Bugs(ExecutionTest *in_execution_test, ExecutionAction *in_execution_action, QWidget *parent) : QDialog(parent), _m_ui(new Ui::Form_Execution_Bugs)
{
    bool	tmp_write_access = in_execution_test != NULL && in_execution_test->projectTest() != NULL && in_execution_test->projectTest()->projectVersion() != NULL
				   && in_execution_test->projectTest()->projectVersion()->project() != NULL
				   && in_execution_test->projectTest()->projectVersion()->project()->projectGrants() != NULL
                   && compare_values(in_execution_test->projectTest()->projectVersion()->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC), PROJECT_GRANT_WRITE) == 0;

    setAttribute(Qt::WA_DeleteOnClose);

    _m_ui->setupUi(this);
    _m_ui->bugs_list->setRemoveSelectedRowsOnKeypressEvent(false);

    _m_execution_test = in_execution_test;
    _m_execution_action = in_execution_action;

    _m_bugtracker = NULL;

    if (_m_execution_test != NULL && _m_execution_test->projectTest() != NULL && _m_execution_test->projectTest()->projectVersion() != NULL)
    {
    setWindowTitle(tr("Anomalies du test : %1").arg(_m_execution_test->projectTest()->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)));

    if (is_empty_string(_m_execution_test->projectTest()->projectVersion()->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST)) == FALSE)
	{
        ClientModule *tmp_bugtracker_module = Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).value(_m_execution_test->projectTest()->projectVersion()->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE));
	    if (tmp_bugtracker_module != NULL)
	    {
		 _m_bugtracker = static_cast<BugtrackerModule*>(tmp_bugtracker_module)->createBugtracker();
		 if (_m_bugtracker != NULL)
             _m_bugtracker->setBaseUrl(QUrl(_m_execution_test->projectTest()->projectVersion()->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST)));
	    }
	}

    }

    connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(accept()));
    connect(_m_ui->bugs_list, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(openBugAtIndex(QTableWidgetItem*)));

    if (!tmp_write_access)
    {
	_m_ui->add_bug_button->setEnabled(false);
	_m_ui->bugs_list->setRemoveSelectedRowsOnKeypressEvent(false);
    }
    else
    {
	connect(_m_ui->add_bug_button, SIGNAL(clicked()), this, SLOT(addBug()));
	connect(_m_ui->bugs_list, SIGNAL(delKeyPressed(QList<Record*>)), this, SLOT(deletedSelectedBugs(QList<Record*>)));
    }

    updateBugsList();
}

Form_Execution_Bugs::~Form_Execution_Bugs()
{
    qDeleteAll(_m_bugs_list);

    if (_m_bugtracker != NULL && _m_execution_test != NULL && _m_execution_test->projectTest() != NULL && _m_execution_test->projectTest()->projectVersion() != NULL)
    {
    ClientModule *tmp_bugtracker_module = Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).value(_m_execution_test->projectTest()->projectVersion()->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE));
	if (tmp_bugtracker_module != NULL)
	{
		static_cast<BugtrackerModule*>(tmp_bugtracker_module)->destroyBugtracker(_m_bugtracker);
	}
    }

    delete _m_ui;
}

void Form_Execution_Bugs::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
	_m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void Form_Execution_Bugs::addBug()
{
    Form_Bug		*tmp_form_bug = new Form_Bug(NULL, _m_execution_test, _m_execution_action, NULL);

	connect(tmp_form_bug, SIGNAL(bugSaved()), this, SLOT(updateBugsList()));

	tmp_form_bug->show();
}



void Form_Execution_Bugs::updateBugsList()
{
    int			    tmp_bugs_index = 0;
    Bug			    *tmp_bug = NULL;
    QStringList             tmp_bugs_headers;

    _m_ui->bugs_list->clear();
    _m_bugs_list.clear();

    if (_m_execution_action != NULL)
    {
	_m_bugs_list = _m_execution_action->loadBugs();
    }
    else if (_m_execution_test != NULL)
    {
	_m_bugs_list = _m_execution_test->loadBugs();
    }

    _m_ui->bugs_list->setRowCount(_m_bugs_list.count());

    tmp_bugs_headers << tr("Date") << tr("Résumé") << tr("Priorité") << tr("Sévérité") << tr("Id. externe");
    _m_ui->bugs_list->setHorizontalHeaderLabels(tmp_bugs_headers);

    for (tmp_bugs_index = 0; tmp_bugs_index < _m_bugs_list.count(); tmp_bugs_index++)
    {
	tmp_bug = _m_bugs_list.at(tmp_bugs_index);
	setBugForRow(tmp_bug, tmp_bugs_index);
    }

    _m_ui->bugs_list->resizeColumnsToContents();
}



void Form_Execution_Bugs::setBugForRow(Bug *in_bug, int in_row)
{
    QTableWidgetItem    *tmp_first_column_item = NULL;
    QTableWidgetItem    *tmp_second_column_item = NULL;
    QTableWidgetItem    *tmp_third_column_item = NULL;
    QTableWidgetItem    *tmp_fourth_column_item = NULL;
    QTableWidgetItem     *tmp_fifth_column_item = NULL;

    QDateTime		tmp_date_time;
    QString		tmp_tool_tip;
    QString		tmp_url;
    QString		tmp_external_link;

    QLabel              *tmp_external_link_label = NULL;


    if (in_bug != NULL)
    {
        tmp_date_time = QDateTime::fromString(QString(in_bug->getValueForKey(BUGS_TABLE_CREATION_DATE)).left(16), "yyyy-MM-dd hh:mm");
    tmp_tool_tip += "<p><b>" + tr("Date de création")+ "</b> : " + tmp_date_time.toString("dddd dd MMMM yyyy à hh:mm") + "</p>";
    tmp_tool_tip += "<p><b>" + tr("Résumé") + "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_SHORT_NAME)) + "</p>";
    tmp_tool_tip += "<p><b>" + tr("Priorité") + "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_PRIORITY)) + "</p>";
    tmp_tool_tip += "<p><b>" + tr("Gravité") + "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_SEVERITY)) + "</p>";
    tmp_tool_tip += "<p><b>" + tr("Plateforme")+ "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_PLATFORM)) + "</p>";
    tmp_tool_tip += "<p><b>" + tr("Système d'exploitation")+ "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_SYSTEM)) + "</p>";
    tmp_tool_tip += "<p><b>" + tr("Description")+ "</b> :<br>" + QString(in_bug->getValueForKey(BUGS_TABLE_DESCRIPTION)).replace('\n', "<BR>") + "</p>";
    if (is_empty_string(in_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID)) == FALSE)
	{
	    if (_m_bugtracker != NULL)
	    {
        tmp_url = _m_bugtracker->urlForBugWithId(in_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID));
		if (!tmp_url.isEmpty())
		    tmp_external_link = "<a href=\"" + tmp_url + "\">" + tmp_url + "</a>";
	    }

	    if (tmp_external_link.isEmpty())
        tmp_external_link = QString(in_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID));
	}

    // Première colonne (date de création du bug)
	tmp_first_column_item = new QTableWidgetItem;
	tmp_first_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_bug));
	tmp_first_column_item->setText(tmp_date_time.toString("dddd dd MMMM yyyy à hh:mm"));
	tmp_first_column_item->setToolTip(tmp_tool_tip);
	_m_ui->bugs_list->setItem(in_row, 0, tmp_first_column_item);

	// Seconde colonne (résumé)
	tmp_second_column_item = new QTableWidgetItem;
	tmp_second_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_bug));
    tmp_second_column_item->setText(in_bug->getValueForKey(BUGS_TABLE_SHORT_NAME));
	tmp_second_column_item->setToolTip(tmp_tool_tip);
	_m_ui->bugs_list->setItem(in_row, 1, tmp_second_column_item);

	// Troisième colonne (priorité)
	tmp_third_column_item = new QTableWidgetItem;
	tmp_third_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_bug));
    tmp_third_column_item->setText(in_bug->getValueForKey(BUGS_TABLE_PRIORITY));
	tmp_third_column_item->setToolTip(tmp_tool_tip);
	_m_ui->bugs_list->setItem(in_row, 2, tmp_third_column_item);

	// Quatrième colonne (sévérité)
	tmp_fourth_column_item = new QTableWidgetItem;
	tmp_fourth_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_bug));
    tmp_fourth_column_item->setText(in_bug->getValueForKey(BUGS_TABLE_SEVERITY));
	tmp_fourth_column_item->setToolTip(tmp_tool_tip);
	_m_ui->bugs_list->setItem(in_row, 3, tmp_fourth_column_item);

	// Cinquième colonne (id externe)
        tmp_external_link_label = new QLabel(tmp_external_link);
        tmp_external_link_label->setOpenExternalLinks(true);
        tmp_fifth_column_item = new QTableWidgetItem;
        tmp_fifth_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_bug));
	tmp_fifth_column_item->setToolTip(tmp_tool_tip);
        _m_ui->bugs_list->setItem(in_row, 4, tmp_fifth_column_item);
        _m_ui->bugs_list->setCellWidget(in_row, 4, tmp_external_link_label);
    }
}


void Form_Execution_Bugs::openBugAtIndex(QTableWidgetItem *in_item)
{
    Bug		    *tmp_bug = NULL;
    Form_Bug	    *tmp_form_bug = NULL;

    if (in_item != NULL)
    {
	tmp_bug = (Bug*)in_item->data(Qt::UserRole).value<void*>();
	if (tmp_bug != NULL)
	{
	    tmp_form_bug = new Form_Bug(NULL, _m_execution_test, _m_execution_action, tmp_bug);
	    connect(tmp_form_bug, SIGNAL(bugSaved()), this, SLOT(updateBugsList()));
	    tmp_form_bug->show();
	}
    }
}


void Form_Execution_Bugs::deletedSelectedBugs(QList<Record*> in_bugs_list)
{
    if (in_bugs_list.count() > 0)
    {
	if (QMessageBox::question(this, tr("Confirmation"),
			tr("Etes-vous sûr(e) de vouloir supprimer %1 anomalie(s) sélectionnée(s) ?").arg(QString::number(in_bugs_list.count())),
				  QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
	{
	    foreach (Record *tmp_bug, in_bugs_list)
	    {
		tmp_bug->deleteRecord();
	    }

	    updateBugsList();
	}
    }
}



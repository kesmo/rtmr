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

#include "form_project_bugs.h"
#include "ui_Form_Project_Bugs.h"

#include "gui/forms/form_bug.h"

#include "session.h"

#include <QMessageBox>
#include <QLabel>

Form_Project_Bugs::Form_Project_Bugs(ProjectVersion *in_project_version, QWidget *parent) :
  AbstractProjectWidget(parent),
  _m_ui(new Ui::Form_Project_Bugs)
{
    setAttribute(Qt::WA_DeleteOnClose);

    setWindowTitle(tr("Liste des anomalies"));

    _m_project_version = in_project_version;
    _m_bugtracker = NULL;
    _m_bugtracker_connection_active = false;
    _m_has_bugtracker_error = false;

    _m_ui->setupUi(this);

    _m_ui->bugs_list->setRemoveSelectedRowsOnKeypressEvent(false);

    _m_ui->bug_status_filter->addItem(TR_CUSTOM_MESSAGE("Tous"), "");
    _m_ui->bug_status_filter->addItem(TR_CUSTOM_MESSAGE("Ouvert"), BUG_STATUS_OPENED);
    _m_ui->bug_status_filter->addItem(TR_CUSTOM_MESSAGE("Fermé"), BUG_STATUS_CLOSED);

    connect(_m_ui->bug_filter, SIGNAL(textChanged(QString)), this, SLOT(filterBugsList()));
    connect(_m_ui->bug_status_filter, SIGNAL(currentIndexChanged(int)), this, SLOT(filterBugsList()));

    connect(_m_ui->buttonBox->button(QDialogButtonBox::Close), SIGNAL(clicked()), this, SLOT(cancel()));
    connect(_m_ui->reload_bugs_button, SIGNAL(clicked()), this, SLOT(launchUpdateBugsListFromBugtracker()));
    connect(_m_ui->bugs_list, SIGNAL(delKeyPressed(QList<Record*>)), this, SLOT(deletedSelectedBugs(QList<Record*>)));
    connect(_m_ui->bugs_list, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(openBugAtIndex(QTableWidgetItem*)));

    loadBugsList();
    initExternalBugtracker();
}

Form_Project_Bugs::~Form_Project_Bugs()
{
    qDeleteAll(_m_bugs_list);

    if (_m_bugtracker != NULL)
    {
    ClientModule *tmp_bugtracker_module = Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).value(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE));
	if (tmp_bugtracker_module != NULL)
	{
		static_cast<BugtrackerModule*>(tmp_bugtracker_module)->destroyBugtracker(_m_bugtracker);
	}
    }

    delete _m_ui;
}

void Form_Project_Bugs::changeEvent(QEvent *e)
{
    QWidget::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
	_m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void Form_Project_Bugs::initExternalBugtracker()
{
    QString		    tmp_base_url = _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST);
    QString		    tmp_url = _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_URL);

    _m_bugtracker_connection_active = false;

    if (!tmp_base_url.isEmpty() || !tmp_url.isEmpty())
    {
	if (!tmp_base_url.isEmpty())
	    _m_ui->bugtracker_url->setText("<a href=\"" + tmp_base_url + "\">" + tmp_base_url + "</a>");
	else if (!tmp_url.isEmpty())
	    _m_ui->bugtracker_url->setText("<a href=\"" + tmp_url + "\">" + tmp_url + "</a>");

        ClientModule *tmp_bugtracker_module = Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).value(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE));
        if (tmp_bugtracker_module != NULL)
        {
            if (_m_bugtracker == NULL)
		_m_bugtracker = static_cast<BugtrackerModule*>(tmp_bugtracker_module)->createBugtracker();

	    if (_m_bugtracker != NULL)
	    {
		if (!tmp_base_url.isEmpty())
		    _m_bugtracker->setBaseUrl(tmp_base_url);

		if (!tmp_url.isEmpty())
		    _m_bugtracker->setWebserviceUrl(tmp_url);

		connect(_m_bugtracker, SIGNAL(loginSignal()), this, SLOT(bugtrackerLogin()));
		connect(_m_bugtracker, SIGNAL(error(QString)), this, SLOT(bugtrackerError(QString)));

        _m_bugtracker->setCredential(Session::instance()->getBugtrackersCredentials()[tmp_bugtracker_module->getModuleName()].first, Session::instance()->getBugtrackersCredentials()[tmp_bugtracker_module->getModuleName()].second);
		_m_bugtracker->login();
	    }
        }
    }

    _m_ui->url_externe_widget->setVisible(_m_bugtracker != NULL);
}


void Form_Project_Bugs::clearBugsList()
{
    QStringList             tmp_bugs_headers;

    _m_ui->bugs_list->clear();
    _m_ui->bugs_list->setRowCount(0);

    tmp_bugs_headers << tr("Date") << tr("Résumé") << tr("Priorité") << tr("Sévérité") << tr("Id. externe") << tr("Etat");
    _m_ui->bugs_list->setHorizontalHeaderLabels(tmp_bugs_headers);
}


void Form_Project_Bugs::loadBugsList()
{
    _m_bugs_list.clear();
    _m_bugs_list = _m_project_version->loadBugs();

    filterBugsList();
}


void Form_Project_Bugs::filterBugsList()
{
    int		tmp_index = 0;
    QString	tmp_filter = _m_ui->bug_filter->text();

    QString	tmp_status_str = _m_ui->bug_status_filter->itemData(_m_ui->bug_status_filter->currentIndex()).toString();

    clearBugsList();

    foreach (Bug *tmp_bug, _m_bugs_list)
    {
    if (QString(tmp_bug->getValueForKey(BUGS_TABLE_SHORT_NAME)).indexOf(tmp_filter) >= 0 || QString(tmp_bug->getValueForKey(BUGS_TABLE_DESCRIPTION)).indexOf(tmp_filter) >= 0)
	{
        if (tmp_status_str.isEmpty() || compare_values(tmp_status_str.toStdString().c_str(), tmp_bug->getValueForKey(BUGS_TABLE_STATUS)) == 0)
	    {
		_m_ui->bugs_list->insertRow(tmp_index);
		setBugForRow(tmp_bug, tmp_index++);
	    }
	}
    }

    _m_ui->bugs_list->resizeColumnsToContents();
}


void Form_Project_Bugs::setBugForRow(Bug *in_bug, int in_row)
{
    QTableWidgetItem    *tmp_first_column_item = NULL;
    QTableWidgetItem    *tmp_second_column_item = NULL;
    QTableWidgetItem    *tmp_third_column_item = NULL;
    QTableWidgetItem    *tmp_fourth_column_item = NULL;
    QTableWidgetItem     *tmp_fifth_column_item = NULL;
    QTableWidgetItem     *tmp_sixth_column_item = NULL;

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
        tmp_tool_tip += "<p><b>" + tr("Identifiant externe")+ "</b> : " + QString(in_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID)) + "</p>";
	    if (_m_bugtracker != NULL)
	    {
        tmp_url = _m_bugtracker->urlForBugWithId(in_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID));
		if (!tmp_url.isEmpty())
		    tmp_external_link = "<a href=\"" + tmp_url + "\">" + tmp_url + "</a>";
	    }

	    if (tmp_external_link.isEmpty())
        tmp_external_link = QString(in_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID));
	}

	// Premirère colonne (date de création du bug)
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

	// Sixième colonne (status)
	tmp_sixth_column_item = new QTableWidgetItem;
	tmp_sixth_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_bug));
        if (compare_values(in_bug->getValueForKey(BUGS_TABLE_STATUS), BUG_STATUS_OPENED) == 0)
            tmp_sixth_column_item->setText(tr("Ouvert"));
        else if (compare_values(in_bug->getValueForKey(BUGS_TABLE_STATUS), BUG_STATUS_CLOSED) == 0)
            tmp_sixth_column_item->setText(tr("Fermé"));
        else
            tmp_sixth_column_item->setText(tr("Non disponible"));

        tmp_sixth_column_item->setToolTip(tmp_tool_tip);
	_m_ui->bugs_list->setItem(in_row, 5, tmp_sixth_column_item);


    }
}


void Form_Project_Bugs::openBugAtIndex(QTableWidgetItem *in_item)
{
    Bug		    *tmp_bug = NULL;
    Form_Bug	    *tmp_form_bug = NULL;

    if (in_item != NULL)
    {
	tmp_bug = (Bug*)in_item->data(Qt::UserRole).value<void*>();
	if (tmp_bug != NULL)
	{
	    tmp_form_bug = new Form_Bug(_m_project_version, NULL, NULL, tmp_bug);
            connect(tmp_form_bug, SIGNAL(bugSaved()), this, SLOT(loadBugsList()));
	    tmp_form_bug->show();
	}
    }
}


void Form_Project_Bugs::deletedSelectedBugs(QList<Record*> in_bugs_list)
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

            loadBugsList();
	}
    }
}

void Form_Project_Bugs::cancel()
{
    QWidget::close();
}



void Form_Project_Bugs::updateBugsListFromBugtracker(QMap< QString, QMap<Bugtracker::BugProperty, QString> > in_bugs_list)
{
    QMap< QString, QMap<Bugtracker::BugProperty, QString> >::iterator tmp_bug_iterator;

    for(tmp_bug_iterator = in_bugs_list.begin(); tmp_bug_iterator != in_bugs_list.end(); tmp_bug_iterator++)
    {
        int tmp_bug_index = Record::indexOfMatchingValueInRecordsList<Bug>(_m_bugs_list, BUGS_TABLE_BUGTRACKER_BUG_ID, tmp_bug_iterator.key().toStdString().c_str());
	if (tmp_bug_index >= 0 && _m_bugs_list[tmp_bug_index] != NULL)
	{
	    // Mettre à jour le bug
	    QMap<Bugtracker::BugProperty, QString> tmp_bug_infos = tmp_bug_iterator.value();

            if (!tmp_bug_infos[Bugtracker::Priority].isEmpty())
                _m_bugs_list[tmp_bug_index]->setValueForKey(tmp_bug_infos[Bugtracker::Priority].toStdString().c_str(), BUGS_TABLE_PRIORITY);

            if (!tmp_bug_infos[Bugtracker::Severity].isEmpty())
                _m_bugs_list[tmp_bug_index]->setValueForKey(tmp_bug_infos[Bugtracker::Severity].toStdString().c_str(), BUGS_TABLE_SEVERITY);

            if (!tmp_bug_infos[Bugtracker::Summary].isEmpty())
                _m_bugs_list[tmp_bug_index]->setValueForKey(tmp_bug_infos[Bugtracker::Summary].toStdString().c_str(), BUGS_TABLE_SHORT_NAME);

            if (!tmp_bug_infos[Bugtracker::Status].isEmpty())
                _m_bugs_list[tmp_bug_index]->setValueForKey(tmp_bug_infos[Bugtracker::Status].toStdString().c_str(), BUGS_TABLE_STATUS);

            _m_bugs_list[tmp_bug_index]->saveRecord();
	}
    }

    filterBugsList();

}

void Form_Project_Bugs::launchUpdateBugsListFromBugtracker()
{
    _m_has_bugtracker_error = false;

    if (_m_bugtracker == NULL || !_m_bugtracker_connection_active)
    {
        initExternalBugtracker();
    }
    else
    {
        QStringList tmp_bugs_ids_list;

        foreach(Bug *tmp_bug, _m_bugs_list)
        {
            if (is_empty_string(tmp_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID)) == FALSE)
                tmp_bugs_ids_list << tmp_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID);
        }

        if (!tmp_bugs_ids_list.isEmpty())
            _m_bugtracker->getBugs(tmp_bugs_ids_list);
    }
}


void Form_Project_Bugs::bugtrackerLogin()
{
    _m_bugtracker_connection_active = true;

    connect(_m_bugtracker, SIGNAL(bugsInformations(QMap<QString,QMap<Bugtracker::BugProperty,QString> >)), this, SLOT(updateBugsListFromBugtracker(QMap< QString, QMap<Bugtracker::BugProperty, QString> >)));

    launchUpdateBugsListFromBugtracker();
}


void Form_Project_Bugs::bugtrackerError(QString errorMsg)
{   
    if (!_m_has_bugtracker_error)
    {
	_m_has_bugtracker_error = true;
    QMessageBox::critical(this, tr("Erreur"), tr("La synchronisation avec le gestionnaire d'anomalies (%1) n'a pu être réalisée.").arg(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_URL)) + "<br>" +  errorMsg+"<p><span style=\"color: red\"><b>"+tr("La requête a échouée.")+"</b></span></p>");
    }
}



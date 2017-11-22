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

#include "form_project_selection.h"
#include <QMessageBox>
#include <QComboBox>
#include <QPushButton>
#include "record.h"
#include "project.h"
#include "projectversion.h"
#include "form_new_version.h"
#include "client.h"
#include "session.h"

/**
  Constructeur
**/
Form_Project_Selection::Form_Project_Selection(QWidget *parent) : QDialog(parent)
{
    net_session		*tmp_session = Session::instance()->getClientSession();
    unsigned long   tmp_projects_count = 0;
    unsigned long   tmp_index = 0;

    char			tmp_where_str[256];

    QSettings		tmp_settings("rtmr", "");


    QString		tmp_last_opened_project_id = tmp_settings.value("last_opened_project_id", "").toString();

    int			tmp_select_project_index = 0;

    setAttribute(Qt::WA_DeleteOnClose);

    _m_ui = new  Ui_Form_Project_Selection();

    _m_ui->setupUi(this);
    _m_ui->projects_list->setFocus();

    connect(_m_ui->button_open, SIGNAL(clicked()), this, SLOT(accept()));
    connect(_m_ui->button_new_version, SIGNAL(clicked()), this, SLOT(newVersion()));
    connect(_m_ui->button_cancel, SIGNAL(clicked()), this, SLOT(reject()));

    if (cl_has_user_this_role(tmp_session, cl_current_user(tmp_session), "admin_role"))
    {
    	connect(_m_ui->button_delete_project, SIGNAL(clicked()), this, SLOT(deleteProject()));
    	connect(_m_ui->button_delete_project_version, SIGNAL(clicked()), this, SLOT(deleteProjectVersion()));
    }
    else
    {
    	_m_ui->button_delete_project->setVisible(false);
    	_m_ui->button_delete_project_version->setVisible(false);
    }

    sprintf(tmp_where_str, "%s IN (SELECT %s FROM %s WHERE %s.%s=%s.%s AND %s='%s')",
            PROJECTS_TABLE_PROJECT_ID,
            PROJECTS_GRANTS_TABLE_PROJECT_ID,
            PROJECTS_GRANTS_TABLE_SIG,
            PROJECTS_GRANTS_TABLE_SIG,
            PROJECTS_GRANTS_TABLE_PROJECT_ID,
            PROJECTS_TABLE_SIG,
            PROJECTS_TABLE_PROJECT_ID,
            PROJECTS_GRANTS_TABLE_USERNAME,
            cl_current_user(Session::instance()->getClientSession())
            );

    _m_projects_list = Record::loadRecords<Project>(Session::instance()->getClientSession(), &projects_table_def, tmp_where_str, PROJECTS_TABLE_SHORT_NAME, &tmp_projects_count);
    if (_m_projects_list != NULL)
    {
        for (tmp_index = 0; tmp_index < tmp_projects_count; tmp_index++)
        {
            if (_m_projects_list[tmp_index] != NULL)
            {
                _m_ui->projects_list->addItem(_m_projects_list[tmp_index]->getValueForKey(PROJECTS_TABLE_SHORT_NAME));
                _m_projects_list[tmp_index]->loadProjectDatas();

		if (!tmp_last_opened_project_id.isEmpty() && tmp_last_opened_project_id.compare(_m_projects_list[tmp_index]->getIdentifier()) == 0)
		{
		    tmp_select_project_index = tmp_index;
		    _m_ui->projects_list->setCurrentIndex(tmp_select_project_index);
		}
            }
        }
    }


    if (tmp_projects_count > 0)
	selectedProjectChanged(tmp_select_project_index);

    connect(_m_ui->projects_list, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedProjectChanged(int)));
    connect(_m_ui->projects_versions_list, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedVersionChanged(int)));
}


/**
  Destructeur
**/
Form_Project_Selection::~Form_Project_Selection()
{
    qDeleteAll(_m_versions_list);
    Record::freeRecords<Project>(_m_projects_list, _m_ui->projects_list->count());
    delete _m_ui;
}


/**
  Changement de selection de projet
**/
void Form_Project_Selection::selectedProjectChanged(int in_project_index)
{
    QSettings		tmp_settings("rtmr", "");
    QString		tmp_last_opened_project_id = tmp_settings.value("last_opened_project_id", "").toString();
    QString		tmp_last_opened_project_version = tmp_settings.value("last_opened_project_version", "").toString();

    int			tmp_select_project_version_index = 0;

    if (in_project_index >= 0 && _m_projects_list && _m_projects_list[in_project_index])
    {
        _m_ui->project_description->setText(_m_projects_list[in_project_index]->getValueForKey(PROJECTS_TABLE_DESCRIPTION));
        _m_ui->projects_versions_list->clear();
        qDeleteAll(_m_versions_list);
        _m_versions_list.clear();
        _m_versions_list = _m_projects_list[in_project_index]->loadProjectVersions();
	tmp_select_project_version_index = _m_versions_list.count() - 1;
        for (int tmp_version_index = 0; tmp_version_index < _m_versions_list.count(); tmp_version_index++)
        {
        _m_ui->projects_versions_list->addItem(ProjectVersion::formatProjectVersionNumber(_m_versions_list[tmp_version_index]->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION)));
	    if (!tmp_last_opened_project_id.isEmpty() && tmp_last_opened_project_id.compare(_m_projects_list[in_project_index]->getIdentifier()) == 0
        && !tmp_last_opened_project_version.isEmpty() && tmp_last_opened_project_version.compare(_m_versions_list[tmp_version_index]->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION)) == 0)
	    {
		tmp_select_project_version_index = tmp_version_index;
	    }
        }
        // Sélection de la dernière version
	_m_ui->projects_versions_list->setCurrentIndex(tmp_select_project_version_index);
        selectedVersionChanged(0);
    }
}

/**
  Changement de selection de version
**/
void Form_Project_Selection::selectedVersionChanged(int in_version_index)
{
    if (in_version_index >= 0 && in_version_index < _m_versions_list.count())
    {
        _m_ui->project_version_description->setText(_m_versions_list[in_version_index]->getValueForKey(PROJECTS_VERSIONS_TABLE_DESCRIPTION));
    }
}


/**
  Validation par le bouton OK
**/
void Form_Project_Selection::accept()
{
    if (_m_ui->projects_versions_list->currentIndex() >= 0 && _m_ui->projects_versions_list->currentIndex() < _m_versions_list.count())
        selectVersion(_m_versions_list[_m_ui->projects_versions_list->currentIndex()]);
}


/**
  Saisie d'une nouvelle version du projet
**/
void Form_Project_Selection::newVersion()
{
    Form_New_Version    *tmp_form = NULL;

    if (_m_ui->projects_list->currentIndex() >= 0)
    {
        tmp_form = new Form_New_Version(_m_projects_list[_m_ui->projects_list->currentIndex()], _m_versions_list, this);
        connect(tmp_form, SIGNAL(versionCreated(ProjectVersion*)), this, SLOT(selectVersion(ProjectVersion*)));

        tmp_form->show();
    }
}


void Form_Project_Selection::selectVersion(ProjectVersion *in_project_version)
{
    Project*		tmp_projet = in_project_version->project()->duplicate();
    ProjectVersion*	tmp_projet_version = in_project_version->clone<ProjectVersion>();
    QSettings		tmp_settings("rtmr", "");

    tmp_projet_version->setProject(tmp_projet);

    // Conserver le dernier projet ouvert dans les preferences utilisateur
    tmp_settings.setValue("last_opened_project_id", tmp_projet->getIdentifier());
    tmp_settings.setValue("last_opened_project_version", tmp_projet_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION));

    QDialog::accept();
    emit projectSelected(tmp_projet_version);
}


void Form_Project_Selection::deleteProject()
{
	Project*		tmp_projet = NULL;
	int				tmp_save_result = NOERR;

	if (_m_ui->projects_list->currentIndex() >= 0)
	{
		tmp_projet = _m_projects_list[_m_ui->projects_list->currentIndex()];
		if (QMessageBox::question(this,
				tr("Confirmation..."),
                tr("Etes-vous sûr(e) de vouloir supprimer le projet %1 ?").arg(tmp_projet->getValueForKey(PROJECTS_TABLE_SHORT_NAME)),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		{
			tmp_save_result = tmp_projet->deleteRecord();
		    if (tmp_save_result != NOERR)
		    {
            QMessageBox::critical(this, tr("Erreur lors de la suppression"), Session::instance()->getErrorMessage(tmp_save_result));
		    }
		    else
				QDialog::accept();
		}
	}
}


void Form_Project_Selection::deleteProjectVersion()
{
	Project*			tmp_projet = NULL;
	ProjectVersion*		tmp_projet_version = NULL;
	int					tmp_save_result = NOERR;

	if (_m_ui->projects_list->currentIndex() >= 0 && _m_ui->projects_versions_list->currentIndex() >= 0)
	{
		tmp_projet = _m_projects_list[_m_ui->projects_list->currentIndex()];
		tmp_projet_version = _m_versions_list[_m_ui->projects_versions_list->currentIndex()];
		if (QMessageBox::question(
				this,
				tr("Confirmation..."),
                tr("Etes-vous sûr(e) de vouloir supprimer la version %1 du projet %2 ?").arg(ProjectVersion::formatProjectVersionNumber(tmp_projet_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION))).arg(QString(tmp_projet->getValueForKey(PROJECTS_TABLE_SHORT_NAME))),
				QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
		{
			tmp_save_result = tmp_projet_version->deleteRecord();
		    if (tmp_save_result != NOERR)
		    {
            QMessageBox::critical(this, tr("Erreur lors de la suppression"), Session::instance()->getErrorMessage(tmp_save_result));
		    }
		    else
				QDialog::accept();
		}
	}
}

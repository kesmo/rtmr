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

#include "form_new_version.h"
#include "ui_Form_New_Version.h"
#include <QMessageBox>
#include <QComboBox>
#include "utilities.h"
#include "session.h"
#include "test.h"

/**
  Constructeur
**/
Form_New_Version::Form_New_Version(Project *in_project, QList<ProjectVersion*> in_versions_list, QWidget *parent) : QDialog(parent), _m_ui(new Ui::Form_New_Version)
{
    setAttribute(Qt::WA_DeleteOnClose);

    _m_ui->setupUi(this);

    _m_ui->project_version_info->addTextToolBar(RecordTextEditToolBar::Small);
    _m_project = in_project;
    _m_versions_list = in_versions_list;

    // Init UI
    _m_ui->project_name->setText(_m_project->getValueForKey(PROJECTS_TABLE_SHORT_NAME));
    _m_ui->project_description->setHtml(_m_project->getValueForKey(PROJECTS_TABLE_DESCRIPTION));

    for (int tmp_version_index = 0; tmp_version_index < _m_versions_list.count(); tmp_version_index++)
    {
    _m_ui->projects_versions_list->addItem(ProjectVersion::formatProjectVersionNumber(_m_versions_list[tmp_version_index]->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION)));
    }
    _m_ui->projects_versions_list->setCurrentIndex(_m_versions_list.count() - 1);
    selectedVersionChanged(_m_versions_list.count() - 1);

    connect(_m_ui->projects_versions_list, SIGNAL(currentIndexChanged(int)), this, SLOT(selectedVersionChanged(int)));

    connect(_m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}


/**
  Destructeur
**/
Form_New_Version::~Form_New_Version()
{
    delete _m_ui;
}


/**
  Changement de selection de version
**/
void Form_New_Version::selectedVersionChanged(int in_version_index)
{
    QStringList tmp_version_numbers;
    ProjectVersion   *tmp_version = NULL;

    if (in_version_index >= 0 && in_version_index < _m_versions_list.count())
    {
        tmp_version = _m_versions_list[in_version_index];

        tmp_version_numbers = QString(tmp_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION)).split(".");
        if (tmp_version_numbers.count() > 0)
            _m_ui->major_version_number->setValue(tmp_version_numbers[0].toInt());
        if (tmp_version_numbers.count() > 1)
            _m_ui->medium_version_number->setValue(tmp_version_numbers[1].toInt());
        if (tmp_version_numbers.count() > 2)
            _m_ui->minor_version_number->setValue(tmp_version_numbers[2].toInt());
        if (tmp_version_numbers.count() > 3)
            _m_ui->maintenance_version_number->setValue(tmp_version_numbers[3].toInt());

        _m_ui->project_src_version_info->setHtml(_m_versions_list[in_version_index]->getValueForKey(PROJECTS_VERSIONS_TABLE_DESCRIPTION));
    }
}

/**
  Validation par le bouton OK
**/
void Form_New_Version::accept()
{
    QString         tmp_selected_version, tmp_version, tmp_major_version, tmp_medium_version, tmp_minor_version, tmp_maintenance_version;

    ProjectVersion  **tmp_versions = NULL;
    ProjectVersion  *tmp_selected_project_version = NULL;
    ProjectVersion  *tmp_new_project_version = NULL;

    net_session	    *tmp_session = Session::instance()->getClientSession();
    char            ***tmp_copy_return_results = NULL;
    unsigned long   tmp_rows_count = 0;
    unsigned long   tmp_columns_count = 0;

    int		    tmp_save_return = NOERR;

    tmp_major_version.setNum(_m_ui->major_version_number->value());
    tmp_medium_version.setNum(_m_ui->medium_version_number->value());
    tmp_minor_version.setNum(_m_ui->minor_version_number->value());
    tmp_maintenance_version.setNum(_m_ui->maintenance_version_number->value());

    tmp_version = QString("%1.%2.%3.%4")
		  .arg(tmp_major_version, 2, '0')
		  .arg(tmp_medium_version, 2, '0')
		  .arg(tmp_minor_version, 2, '0')
		  .arg(tmp_maintenance_version, 2, '0');

    if (_m_ui->projects_versions_list->currentIndex() >= 0)
    {
	tmp_selected_project_version = _m_versions_list[_m_ui->projects_versions_list->currentIndex()];
    tmp_selected_version = tmp_selected_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION);
	if (tmp_selected_version == tmp_version)
	{
	    QMessageBox::critical(this, tr("Donnée non cohérante"), tr("La version existe déjà."));
	    return;
	}
	else if (tmp_selected_version > tmp_version)
	{
	    QMessageBox::critical(this, tr("Donnée non cohérante"), tr("La nouvelle version est antérieure é la version sélectionnée."));
	    return;
	}

	sprintf(tmp_session->m_last_query, "select copy_project_version(%s,'%s','%s');",
        tmp_selected_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID),
        tmp_selected_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION),
		tmp_version.toStdString().c_str());

	tmp_copy_return_results = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
	if (tmp_copy_return_results != NULL)
	{
	    cl_free_rows_columns_array(&tmp_copy_return_results, tmp_rows_count, tmp_columns_count);

	    sprintf(tmp_session->m_last_query, "%s=%s AND %s='%s'",
		    PROJECTS_VERSIONS_TABLE_PROJECT_ID,
            tmp_selected_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID),
		    PROJECTS_VERSIONS_TABLE_VERSION,
		    tmp_version.toStdString().c_str());
	    tmp_versions = Record::loadRecords<ProjectVersion>(tmp_session, &projects_versions_table_def, tmp_session->m_last_query, NULL, &tmp_rows_count);
	    if (tmp_versions != NULL)
	    {
		if (tmp_rows_count == 1)
		{
		    tmp_new_project_version = tmp_versions[0];
            tmp_new_project_version->setValueForKey(tmp_selected_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE), PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE);
            tmp_new_project_version->setValueForKey(tmp_selected_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST), PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST);
            tmp_new_project_version->setValueForKey(tmp_selected_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_URL), PROJECTS_VERSIONS_TABLE_BUG_TRACKER_URL);
            tmp_new_project_version->setValueForKey(tmp_selected_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_ID), PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_ID);
            tmp_new_project_version->setValueForKey(tmp_selected_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_VERSION), PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_VERSION);
		    Record::freeRecords<ProjectVersion>(tmp_versions, 0);
		}
		else
		    Record::freeRecords<ProjectVersion>(tmp_versions, tmp_rows_count);
	    }
	}
    }
    else
    {
	tmp_new_project_version = new ProjectVersion();
    tmp_new_project_version->setValueForKey(tmp_version.toStdString().c_str(), PROJECTS_VERSIONS_TABLE_VERSION);

    }

    if (tmp_new_project_version != NULL)
    {
	tmp_new_project_version->setProject(_m_project->duplicate());
    tmp_new_project_version->setValueForKey(_m_ui->project_version_info->textEditor()->toHtml().toStdString().c_str(), PROJECTS_VERSIONS_TABLE_DESCRIPTION);
	tmp_save_return = tmp_new_project_version->saveRecord();
	if (tmp_save_return == NOERR)
	{
	    QDialog::accept();
	    emit versionCreated(tmp_new_project_version);
	    return;
	}
    }

    QMessageBox::critical(this, tr("Erreur d'enregistrement"), Session::instance()->getErrorMessage(tmp_save_return));
}

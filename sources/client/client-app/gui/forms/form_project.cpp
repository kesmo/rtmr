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

#include "form_project.h"
#include "ui_Form_Project.h"

#include "client.h"
#include "constants.h"
#include "session.h"
#include "projectgrant.h"

#include <QtGui>


/**
  Constructeur
**/
Form_Project::Form_Project(ProjectVersion* in_project_record, QWidget *parent) : QDialog(parent)
{
    QStringList tmp_version_numbers;

    setAttribute(Qt::WA_DeleteOnClose);
    _m_project_record = in_project_record;

    _m_ui = new  Ui_Form_Project();

    _m_ui->setupUi(this);
    _m_ui->project_description->addTextToolBar(RecordTextEditToolBar::Small);
    _m_ui->project_version_info->addTextToolBar(RecordTextEditToolBar::Small);
    _m_ui->project_name->setFocus();

    if (_m_project_record != NULL)
    {
        _m_ui->project_name->setText(_m_project_record->project()->getValueForKey(PROJECTS_TABLE_SHORT_NAME));
        _m_ui->project_description->textEditor()->setHtml(_m_project_record->project()->getValueForKey(PROJECTS_TABLE_DESCRIPTION));
        tmp_version_numbers = QString(_m_project_record->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION)).split(".");
        if (tmp_version_numbers.count() > 0)
            _m_ui->major_version_number->setValue(tmp_version_numbers[0].toInt());
        if (tmp_version_numbers.count() > 1)
            _m_ui->medium_version_number->setValue(tmp_version_numbers[1].toInt());
        if (tmp_version_numbers.count() > 2)
            _m_ui->minor_version_number->setValue(tmp_version_numbers[2].toInt());
        if (tmp_version_numbers.count() > 3)
            _m_ui->maintenance_version_number->setValue(tmp_version_numbers[3].toInt());

        setWindowTitle(_m_project_record->project()->getValueForKey(PROJECTS_TABLE_SHORT_NAME));
        _m_new_version = false;
    }
    else
    {
        _m_project_record = new ProjectVersion(new Project);
    //_m_project_record->project()->setValueForKey(Session::instance()->user()->getIdentifier(), PROJECTS_TABLE_OWNER_ID);
        _m_ui->major_version_number->setValue(1);
        setWindowTitle(tr("Nouveau projet"));
        _m_new_version = true;
    }

    connect(_m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}



/**
  Destruction
**/
Form_Project::~Form_Project()
{
	delete _m_ui;
	delete _m_project_record;
}


/**
  Validation par le bouton OK
**/
void Form_Project::accept()
{
    QString         tmp_version, tmp_major_version, tmp_medium_version, tmp_minor_version, tmp_maintenance_version;
    int             tmp_return = NOERR;
    ProjectGrant    *tmp_project_grants = NULL;
    ProjectVersion  *tmp_new_version = NULL;

    /* Affecter les nouvelles valeurs */
    if (_m_ui->project_name->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Donnée obligatoire non saisi"), tr("Le nom du projet est nécessaire."));
        return;
    }
    else
    {
        _m_project_record->project()->setValueForKey((char*)_m_ui->project_name->text().toStdString().c_str(), PROJECTS_TABLE_SHORT_NAME);
        _m_project_record->project()->setValueForKey((char*)_m_ui->project_description->textEditor()->toHtml().toStdString().c_str(), PROJECTS_TABLE_DESCRIPTION);

        tmp_major_version.setNum(_m_ui->major_version_number->value());
        tmp_medium_version.setNum(_m_ui->medium_version_number->value());
        tmp_minor_version.setNum(_m_ui->minor_version_number->value());
        tmp_maintenance_version.setNum(_m_ui->maintenance_version_number->value());

	tmp_version = QString("%1.%2.%3.%4")
		      .arg(tmp_major_version, 2, '0')
		      .arg(tmp_medium_version, 2, '0')
		      .arg(tmp_minor_version, 2, '0')
		      .arg(tmp_maintenance_version, 2, '0');

        _m_project_record->setValueForKey((char*)tmp_version.toStdString().c_str(), PROJECTS_VERSIONS_TABLE_VERSION);
        _m_project_record->setValueForKey((char*)_m_ui->project_version_info->textEditor()->toHtml().toStdString().c_str(), PROJECTS_VERSIONS_TABLE_DESCRIPTION);

        if (_m_new_version)
        {
            tmp_return = _m_project_record->project()->insertRecord();
            if (tmp_return == NOERR)
            {
    	    	tmp_return = _m_project_record->saveRecord();
                if (tmp_return == NOERR)
                {
        	    	tmp_project_grants = new ProjectGrant();
                    tmp_project_grants->setValueForKey(_m_project_record->project()->getIdentifier(), PROJECTS_GRANTS_TABLE_PROJECT_ID);
            tmp_project_grants->setValueForKey(Session::instance()->getClientSession()->m_username, PROJECTS_GRANTS_TABLE_USERNAME);
                    tmp_project_grants->setValueForKey(PROJECT_GRANT_WRITE, PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC);
                    tmp_project_grants->setValueForKey(PROJECT_GRANT_WRITE, PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC);
                    tmp_project_grants->setValueForKey(PROJECT_GRANT_WRITE, PROJECTS_GRANTS_TABLE_MANAGE_CAMPAIGNS_INDIC);
                    tmp_project_grants->setValueForKey(PROJECT_GRANT_WRITE, PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC);

        	    	tmp_return = tmp_project_grants->saveRecord();
        	    	if (tmp_return == NOERR)
        	    		_m_project_record->project()->loadProjectGrants();
                }
            }
        }
        else
	    tmp_return = _m_project_record->saveRecord();

        if (tmp_return == NOERR)
        {
        tmp_new_version = ProjectVersion::loadVersionForProject(_m_project_record->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), _m_project_record->project());
	    if (tmp_new_version != NULL)
	    {
		emit projectSelected(tmp_new_version);
		QDialog::accept();
	    }
	    return;
	}

    QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_return));
    }
}

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

#include "form_user_projects_grants.h"
#include "ui_Form_User_Projects_Grants.h"

#include "session.h"

#include "client.h"
#include "utilities.h"
#include "constants.h"
#include "projectgrant.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QRadioButton>
#include "QMessageBox"

Form_User_Projects_Grants::Form_User_Projects_Grants(const char *in_username, QWidget *parent) : QDialog(parent), _m_ui(new Ui::Form_User_Projects_Grants)
{
    QStringList             tmp_headers;

    setAttribute(Qt::WA_DeleteOnClose);

    _m_ui->setupUi(this);

    _m_ui->title->setText(tr("Gestion des droits ") + QString(in_username));

    tmp_headers << tr("Projet") << tr("Gestion des scénarios") << tr("Gestion des exigences") << tr("Gestions des campagnes") << tr("Gestion des executions");
    _m_ui->projects_grants_table_widget->setHorizontalHeaderLabels(tmp_headers);
    _m_ui->projects_grants_table_widget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    _m_username = in_username;

    loadProjectsGrants();

}

Form_User_Projects_Grants::~Form_User_Projects_Grants()
{
    cl_free_rows_columns_array(&_m_grants, _m_rows_count, _m_columns_count);
    delete _m_ui;

    delete _m_existing_grant_array;
    delete _m_modified_grant_array;
}

void Form_User_Projects_Grants::changeEvent(QEvent *e)
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

void Form_User_Projects_Grants::loadProjectsGrants()
{
    char                tmp_statement_str[512];
    unsigned long       tmp_row_index = 0;

    unsigned long       tmp_last_row = 0;

    sprintf(tmp_statement_str, "SELECT %s.%s, %s, %s, %s, %s, %s FROM %s LEFT OUTER JOIN %s ON (%s.%s=%s.%s and %s='%s') ORDER BY %s;",
	    PROJECTS_TABLE_SIG,
	    PROJECTS_TABLE_PROJECT_ID,
	    PROJECTS_TABLE_SHORT_NAME,
	    PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC,
	    PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC,
	    PROJECTS_GRANTS_TABLE_MANAGE_CAMPAIGNS_INDIC,
	    PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC,
	    PROJECTS_TABLE_SIG,
	    PROJECTS_GRANTS_TABLE_SIG,
	    PROJECTS_TABLE_SIG,
	    PROJECTS_TABLE_PROJECT_ID,
	    PROJECTS_GRANTS_TABLE_SIG,
	    PROJECTS_GRANTS_TABLE_PROJECT_ID,
	    PROJECTS_GRANTS_TABLE_USERNAME,
	    _m_username,
	    PROJECTS_TABLE_SHORT_NAME
	    );

    _m_grants = cl_run_sql(Session::instance()->getClientSession(), tmp_statement_str, &_m_rows_count, &_m_columns_count);
    if (_m_grants != NULL)
    {
	_m_existing_grant_array = new bool[_m_rows_count];
	_m_modified_grant_array = new bool[_m_rows_count];

	for (tmp_row_index = 0; tmp_row_index < _m_rows_count; tmp_row_index++)
	{
	    if (_m_grants[tmp_row_index] != NULL)
	    {
		_m_ui->projects_grants_table_widget->insertRow(tmp_last_row);

		setProjectGrantAtIndex(_m_grants[tmp_row_index], tmp_last_row);

		tmp_last_row++;
	    }
	}
    }
}


void Form_User_Projects_Grants::setProjectGrantAtIndex(char **in_project_grant, int in_index)
{
    QTableWidgetItem    *tmp_first_column_item = NULL;

    QWidget             *tmp_widget = NULL;
    QVBoxLayout         *tmp_grid_layout = NULL;
    QRadioButton        *tmp_radio_button_none = NULL;
    QRadioButton        *tmp_radio_button_read = NULL;
    QRadioButton        *tmp_radio_button_write = NULL;
    QModelIndex         tmp_model_index;

    bool		tmp_existing = true;

    tmp_first_column_item = new QTableWidgetItem;
    tmp_first_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_project_grant));
    tmp_first_column_item->setText(in_project_grant[1]);
    _m_ui->projects_grants_table_widget->setItem(in_index, 0, tmp_first_column_item);

    for(int tmp_index = 2; tmp_index <= 5; tmp_index++)
    {
		tmp_existing = (tmp_existing && is_empty_string(in_project_grant[tmp_index]) == false);

		tmp_model_index = _m_ui->projects_grants_table_widget->model()->index(in_index, tmp_index - 1);
		tmp_widget = new QWidget(_m_ui->projects_grants_table_widget);
		tmp_grid_layout = new QVBoxLayout(tmp_widget);
		tmp_radio_button_none = new QRadioButton(tr("Aucun"), tmp_widget);
		tmp_radio_button_none->setChecked(compare_values(in_project_grant[tmp_index], PROJECT_GRANT_NONE) == 0);
		tmp_grid_layout->addWidget(tmp_radio_button_none);
		tmp_radio_button_read = new QRadioButton(tr("Lecture"), tmp_widget);
		tmp_radio_button_read->setChecked(compare_values(in_project_grant[tmp_index], PROJECT_GRANT_READ) == 0);
		tmp_grid_layout->addWidget(tmp_radio_button_read);
		tmp_radio_button_write = new QRadioButton(tr("Ecriture"), tmp_widget);
		tmp_radio_button_write->setChecked(compare_values(in_project_grant[tmp_index], PROJECT_GRANT_WRITE) == 0);
		tmp_grid_layout->addWidget(tmp_radio_button_write);
		_m_ui->projects_grants_table_widget->setCellWidget(in_index, tmp_index - 1, tmp_widget);
    }

    _m_existing_grant_array[in_index] = tmp_existing;
    _m_modified_grant_array[in_index] = false;
}



void Form_User_Projects_Grants::accept()
{
    unsigned long       tmp_row_index = 0;
    QModelIndex     	tmp_column_index;
    QWidget				*tmp_widget = NULL;
    QRadioButton        *tmp_radio_button = NULL;
    ProjectGrant		*tmp_project_grants = NULL;
    int					tmp_status = NOERR;

    for (tmp_row_index = 0; tmp_row_index < _m_rows_count && tmp_status == NOERR; tmp_row_index++)
    {
		if (_m_grants[tmp_row_index] != NULL)
		{
	    	tmp_project_grants = new ProjectGrant();
            tmp_project_grants->setValueForKey(_m_grants[tmp_row_index][0], PROJECTS_GRANTS_TABLE_PROJECT_ID);
            tmp_project_grants->setValueForKey(_m_username, PROJECTS_GRANTS_TABLE_USERNAME);
			tmp_project_grants->cloneColumns();

	        for(int tmp_index = 2; tmp_index <= 5; tmp_index++)
	        {
	        	if (is_empty_string(_m_grants[tmp_row_index][tmp_index]))
                    tmp_project_grants->setValueForKeyAtIndex(PROJECT_GRANT_NONE, tmp_index);
	        	else
                    tmp_project_grants->setValueForKeyAtIndex(_m_grants[tmp_row_index][tmp_index], tmp_index);

				tmp_column_index = _m_ui->projects_grants_table_widget->model()->index(tmp_row_index, tmp_index - 1);
				if (tmp_column_index.isValid())
				{
					tmp_widget = _m_ui->projects_grants_table_widget->indexWidget(tmp_column_index);
					if (tmp_widget != NULL)
					{
						/* Aucun */
						tmp_radio_button = ::qobject_cast< QRadioButton *>(tmp_widget->layout()->itemAt(0)->widget());
						if (tmp_radio_button != NULL)
						{
							if (tmp_radio_button->isChecked() && compare_values(_m_grants[tmp_row_index][tmp_index], PROJECT_GRANT_NONE) != 0)
							{
							    _m_modified_grant_array[tmp_row_index] = true;
                                tmp_project_grants->setValueForKeyAtIndex(PROJECT_GRANT_NONE, tmp_index);
							}
							else
							{
								/* Lecture */
								tmp_radio_button = ::qobject_cast< QRadioButton *>(tmp_widget->layout()->itemAt(1)->widget());
								if (tmp_radio_button != NULL)
								{
									if (tmp_radio_button->isChecked() && compare_values(_m_grants[tmp_row_index][tmp_index], PROJECT_GRANT_READ) != 0)
									{
									    _m_modified_grant_array[tmp_row_index] = true;
                                        tmp_project_grants->setValueForKeyAtIndex(PROJECT_GRANT_READ, tmp_index);
									}
									else
									{
										/* Ecriture */
										tmp_radio_button = ::qobject_cast< QRadioButton *>(tmp_widget->layout()->itemAt(2)->widget());
										if (tmp_radio_button != NULL)
										{
											if (tmp_radio_button->isChecked() && compare_values(_m_grants[tmp_row_index][tmp_index], PROJECT_GRANT_WRITE) != 0)
											{
											    _m_modified_grant_array[tmp_row_index] = true;
                                                tmp_project_grants->setValueForKeyAtIndex(PROJECT_GRANT_WRITE, tmp_index);
											}
										}
									}
								}
							}
						}
					}
				}
	        }

	        /* Enregistrement des données */
	        if (_m_modified_grant_array[tmp_row_index])
	        {
	        	if (_m_existing_grant_array[tmp_row_index])
	        	{
                    if (compare_values(tmp_project_grants->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC), PROJECT_GRANT_NONE) == 0
                            && compare_values(tmp_project_grants->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC), PROJECT_GRANT_NONE) == 0
                            && compare_values(tmp_project_grants->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_CAMPAIGNS_INDIC), PROJECT_GRANT_NONE) == 0
                            && compare_values(tmp_project_grants->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC), PROJECT_GRANT_NONE) == 0)
					{
						tmp_status = tmp_project_grants->deleteRecord();
					}
					else
					{
						tmp_status = tmp_project_grants->saveRecord();
					}
	        	}
	        	else
	        	{
	        		tmp_status = tmp_project_grants->insertRecord();
	        	}
	        }
		}
    }

    if (tmp_status == NOERR)
    {
    	QDialog::accept();
    }
    else
    {
        QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_status));
    }
}

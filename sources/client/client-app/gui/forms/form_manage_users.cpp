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

#include "form_manage_users.h"
#include "ui_Form_Manage_Users.h"
#include "client.h"
#include "session.h"
#include "utilities.h"
#include "constants.h"
#include <QRadioButton>
#include <QMessageBox>
#include "form_new_user.h"
#include "form_user_projects_grants.h"
#include "form_change_password.h"

/**
  Constructeur
**/
Form_Manage_Users::Form_Manage_Users(QWidget *parent) : QDialog(parent), _m_ui(new Ui::Form_Manage_Users)
{
    QStringList             tmp_headers;

    setAttribute(Qt::WA_DeleteOnClose);

    _m_ui->setupUi(this);

    tmp_headers << tr("Utilisateur") << tr("Rôle Administrateur") << tr("Rôle rédacteur") << tr("Rôle lecteur");
    _m_ui->users_list->setHorizontalHeaderLabels(tmp_headers);
    _m_ui->users_list->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    loadUsers();

    connect(_m_ui->users_list, SIGNAL(itemSelectionChanged()), this, SLOT(userSelectionChanged()));

    connect(_m_ui->add_user_button, SIGNAL(clicked()), this, SLOT(addUser()));
    connect(_m_ui->change_password_button, SIGNAL(clicked()), this, SLOT(changePassword()));
    connect(_m_ui->user_grants_button, SIGNAL(clicked()), this, SLOT(manageUserGrants()));
    connect(_m_ui->del_users_button, SIGNAL(clicked()), this, SLOT(removeSelectedUsers()));

    userSelectionChanged();
}


/**
  Destructeur
**/
Form_Manage_Users::~Form_Manage_Users()
{
    qDeleteAll(_m_users_list);
    delete _m_ui;
}


void Form_Manage_Users::loadUsers()
{
    DBUser				*tmp_user = NULL;
    unsigned long       tmp_rows_count = 0;
    unsigned long       tmp_columns_count = 0;
    unsigned long       tmp_row_index = 0;

    unsigned long       tmp_last_row = 0;

    char                ***tmp_users = cl_run_sql(Session::instance()->getClientSession(), "SELECT username, admin_role, writer_role, reader_role FROM users_roles WHERE username not like 'dba' ORDER BY username ASC;", &tmp_rows_count, &tmp_columns_count);

    if (tmp_users != NULL)
    {
        for (tmp_row_index = 0; tmp_row_index < tmp_rows_count; tmp_row_index++)
        {
            if (tmp_users[tmp_row_index] != NULL && is_empty_string(tmp_users[tmp_row_index][0]) == FALSE && is_empty_string(tmp_users[tmp_row_index][1]) == FALSE
                    && is_empty_string(tmp_users[tmp_row_index][2]) == FALSE && is_empty_string(tmp_users[tmp_row_index][3]) == FALSE)
            {
                tmp_user = new DBUser(tmp_users[tmp_row_index][0],
                        tmp_users[tmp_row_index][1][0] == '1',
                        tmp_users[tmp_row_index][2][0] == '1',
                        tmp_users[tmp_row_index][3][0] == '1');

                _m_ui->users_list->insertRow(tmp_last_row);

                setUserAtIndex(tmp_user, tmp_last_row);
                _m_users_list.append(tmp_user);

                tmp_last_row++;
            }
        }

        cl_free_rows_columns_array(&tmp_users, tmp_rows_count, 4);
    }
}

void Form_Manage_Users::setUserAtIndex(DBUser *in_user, int in_index)
{
    QTableWidgetItem    *tmp_first_column_item = NULL;

    QWidget		*tmp_checkbox_parent = NULL;
    QRadioButton        *tmp_checkbox = NULL;
    QModelIndex         tmp_checkbox_index;
    QButtonGroup	*tmp_button_group = new QButtonGroup(this);

    tmp_first_column_item = new QTableWidgetItem;
    tmp_first_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_user));
    tmp_first_column_item->setText(in_user->username());
    _m_ui->users_list->setItem(in_index, 0, tmp_first_column_item);

    tmp_checkbox_index = _m_ui->users_list->model()->index(in_index, 1);
    tmp_checkbox_parent = new QWidget;
    tmp_checkbox_parent->setContentsMargins(0, 0, 0, 0);
    tmp_checkbox = new QRadioButton;
    tmp_checkbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tmp_button_group->addButton(tmp_checkbox);
    tmp_checkbox_parent->setLayout(new QGridLayout);
    tmp_checkbox_parent->layout()->setContentsMargins(0, 0, 0, 0);
    tmp_checkbox_parent->layout()->addWidget(tmp_checkbox);
    tmp_checkbox->setChecked(in_user->isAdmin() ? Qt::Checked : Qt::Unchecked);
    _m_ui->users_list->setIndexWidget(tmp_checkbox_index, tmp_checkbox_parent);

    tmp_checkbox_index = _m_ui->users_list->model()->index(in_index, 2);
    tmp_checkbox_parent = new QWidget;
    tmp_checkbox_parent->setContentsMargins(0, 0, 0, 0);
    tmp_checkbox = new QRadioButton;
    tmp_checkbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tmp_button_group->addButton(tmp_checkbox);
    tmp_checkbox_parent->setLayout(new QGridLayout);
    tmp_checkbox_parent->layout()->setContentsMargins(0, 0, 0, 0);
    tmp_checkbox_parent->layout()->addWidget(tmp_checkbox);
    tmp_checkbox->setChecked(in_user->isWriter() ? Qt::Checked : Qt::Unchecked);
    _m_ui->users_list->setIndexWidget(tmp_checkbox_index, tmp_checkbox_parent);

    tmp_checkbox_index = _m_ui->users_list->model()->index(in_index, 3);
    tmp_checkbox_parent = new QWidget;
    tmp_checkbox_parent->setContentsMargins(0, 0, 0, 0);
    tmp_checkbox = new QRadioButton;
    tmp_checkbox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    tmp_button_group->addButton(tmp_checkbox);
    tmp_checkbox_parent->setLayout(new QGridLayout);
    tmp_checkbox_parent->layout()->setContentsMargins(0, 0, 0, 0);
    tmp_checkbox_parent->layout()->addWidget(tmp_checkbox);
    tmp_checkbox->setChecked(in_user->isReader() ? Qt::Checked : Qt::Unchecked);
    _m_ui->users_list->setIndexWidget(tmp_checkbox_index, tmp_checkbox_parent);
}


void Form_Manage_Users::accept()
{
    DBUser				*tmp_current_user = NULL;
    QTableWidgetItem 	*tmp_current_item = NULL;
    QModelIndex     	tmp_column_index;
    QWidget       	*tmp_widget = NULL;
    QRadioButton       	*tmp_checkbox = NULL;
    int					tmp_result = NOERR;

    net_session			*tmp_session = Session::instance()->getClientSession();

    foreach(DBUser *tmp_user, _m_removed_users_list)
    {
        if (tmp_user->isNewUser() == false)
        {
            tmp_result = cl_remove_user(tmp_session, tmp_user->username());
        }
    }

    for(int tmp_index = 0; tmp_index < _m_ui->users_list->rowCount() && tmp_result == NOERR; tmp_index++)
    {
        tmp_current_item = _m_ui->users_list->item(tmp_index, 0);
        if (tmp_current_item != NULL)
        {
            tmp_current_user = (DBUser*)tmp_current_item->data(Qt::UserRole).value<void*>();
            if (tmp_current_user != NULL)
            {
                if (tmp_current_user->isNewUser())
                {
                    tmp_result = cl_add_user(tmp_session, tmp_current_user->username(), tmp_current_user->password(), NULL);
                }

                if (tmp_result == NOERR)
                {
                    // Recuperer l'etat de la case a cocher ADMIN
                    tmp_column_index = _m_ui->users_list->model()->index(tmp_index, 1);
                    if (tmp_column_index.isValid())
                    {
                        tmp_widget = _m_ui->users_list->indexWidget(tmp_column_index);
                        if (tmp_widget != NULL)
                        {
                            tmp_checkbox = ::qobject_cast< QRadioButton *>(tmp_widget->layout()->itemAt(0)->widget());
                            if (tmp_checkbox != NULL)
                            {
                                if (tmp_checkbox->isChecked())
                                {
                                    if (tmp_current_user->isAdmin() == false || tmp_current_user->isNewUser())
                                        tmp_result = cl_add_role_to_user(tmp_session, "admin_role", tmp_current_user->username());
                                }
                                else if (tmp_current_user->isAdmin())
                                    tmp_result = cl_remove_role_from_user(tmp_session, "admin_role", tmp_current_user->username());

                            }
                        }
                    }

                    if (tmp_result == NOERR)
                    {
                        // Recuperer l'etat de la case a cocher WRITER
                        tmp_column_index = _m_ui->users_list->model()->index(tmp_index, 2);
                        if (tmp_column_index.isValid())
                        {
                            tmp_widget = _m_ui->users_list->indexWidget(tmp_column_index);
                            if (tmp_widget != NULL)
                            {
                                tmp_checkbox = ::qobject_cast< QRadioButton *>(tmp_widget->layout()->itemAt(0)->widget());
                                if (tmp_checkbox != NULL)
                                {
                                    if (tmp_checkbox->isChecked())
                                    {
                                        if (tmp_current_user->isWriter() == false || tmp_current_user->isNewUser())
                                            tmp_result = cl_add_role_to_user(tmp_session, "writer_role", tmp_current_user->username());
                                    }
                                    else if (tmp_current_user->isWriter())
                                        tmp_result = cl_remove_role_from_user(tmp_session, "writer_role", tmp_current_user->username());
                                }
                            }
                        }
                    }

                    if (tmp_result == NOERR)
                    {
                        // Recuperer l'etat de la case a cocher READER
                        tmp_column_index = _m_ui->users_list->model()->index(tmp_index, 3);
                        if (tmp_column_index.isValid())
                        {
                            tmp_widget = _m_ui->users_list->indexWidget(tmp_column_index);
                            if (tmp_widget != NULL)
                            {
                                tmp_checkbox = ::qobject_cast< QRadioButton *>(tmp_widget->layout()->itemAt(0)->widget());
                                if (tmp_checkbox != NULL)
                                {
                                    if (tmp_checkbox->isChecked())
                                    {
                                        if (tmp_current_user->isReader() == false || tmp_current_user->isNewUser())
                                            tmp_result = cl_add_role_to_user(tmp_session, "reader_role", tmp_current_user->username());
                                    }
                                    else if (tmp_current_user->isReader())
                                        tmp_result = cl_remove_role_from_user(tmp_session, "reader_role", tmp_current_user->username());
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    if (tmp_result != NOERR)
    {
        QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_result));
    }
    else
        QDialog::accept();
}


void Form_Manage_Users::addUser()
{
    Form_New_User	*tmp_dialog = new Form_New_User(this);

    connect(tmp_dialog, SIGNAL(userCreated(DBUser*)), this, SLOT(addUser(DBUser*)));
    tmp_dialog->show();
}


void Form_Manage_Users::changePassword()
{
    Form_Change_Password	*tmp_dialog = NULL;
    QModelIndex			tmp_current_index = _m_ui->users_list->selectionModel()->currentIndex();
    DBUser			*tmp_current_user = NULL;
    QTableWidgetItem		*tmp_current_item = NULL;

    if (tmp_current_index.isValid())
    {
        tmp_current_item = _m_ui->users_list->item(tmp_current_index.row(), 0);
        if (tmp_current_item != NULL)
        {
            tmp_current_user = (DBUser*)tmp_current_item->data(Qt::UserRole).value<void*>();
            if (tmp_current_user != NULL)
            {
                tmp_dialog = new Form_Change_Password(tmp_current_user->username(), this);
                tmp_dialog->show();
            }
        }
    }
}


void Form_Manage_Users::manageUserGrants()
{
    Form_User_Projects_Grants	*tmp_dialog = NULL;
    QModelIndex			tmp_current_index = _m_ui->users_list->selectionModel()->currentIndex();
    DBUser			*tmp_current_user = NULL;
    QTableWidgetItem		*tmp_current_item = NULL;

    if (tmp_current_index.isValid())
    {
        tmp_current_item = _m_ui->users_list->item(tmp_current_index.row(), 0);
        if (tmp_current_item != NULL)
        {
            tmp_current_user = (DBUser*)tmp_current_item->data(Qt::UserRole).value<void*>();
            if (tmp_current_user != NULL)
            {
                tmp_dialog = new Form_User_Projects_Grants(tmp_current_user->username(), this);
                tmp_dialog->show();
            }
        }
    }
}


void Form_Manage_Users::addUser(DBUser *in_user)
{
    int		tmp_last_row = _m_ui->users_list->rowCount();

    _m_ui->users_list->insertRow(tmp_last_row);
    setUserAtIndex(in_user, tmp_last_row);
    _m_users_list.append(in_user);
}


void Form_Manage_Users::removeSelectedUsers()
{
    QModelIndex             tmp_current_index = _m_ui->users_list->selectionModel()->currentIndex();
    QMessageBox             *tmp_msgbox = NULL;
    int                     tmp_return = 0;
    DBUser                  *tmp_current_user = NULL;
    QTableWidgetItem        *tmp_current_item = NULL;

    if (tmp_current_index.isValid())
    {
        tmp_msgbox = new QMessageBox(QMessageBox::Question, tr("Confirmation..."), tr("Etes-vous sûr(e) de vouloir supprimer l'utilisateur sélectionné ?"), QMessageBox::Yes | QMessageBox::Cancel, this);
        tmp_msgbox->setDefaultButton(QMessageBox::Cancel);
        tmp_return = tmp_msgbox->exec();
        if (tmp_return == QMessageBox::Yes)
        {
            tmp_current_item = _m_ui->users_list->item(tmp_current_index.row(), 0);
            if (tmp_current_item != NULL)
            {
                tmp_current_user = (DBUser*)tmp_current_item->data(Qt::UserRole).value<void*>();
                if (tmp_current_user != NULL)
                {
                    _m_removed_users_list.append(tmp_current_user);
                }
                _m_ui->users_list->model()->removeRow(tmp_current_index.row());
            }
        }
    }
}


void Form_Manage_Users::changeEvent(QEvent *e)
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


void Form_Manage_Users::userSelectionChanged()
{
    bool	tmp_items_selected = _m_ui->users_list->selectionModel()->selection().count() > 0;

    _m_ui->user_grants_button->setEnabled(tmp_items_selected);
    _m_ui->change_password_button->setEnabled(tmp_items_selected);
    _m_ui->del_users_button->setEnabled(tmp_items_selected);
}

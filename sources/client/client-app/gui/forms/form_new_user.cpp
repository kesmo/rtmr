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

#include "form_new_user.h"
#include "utilities.h"
#include <QMessageBox>

Form_New_User::Form_New_User(QWidget *parent) : QDialog(parent), _m_ui(new Ui::Form_New_User)
{
    setAttribute(Qt::WA_DeleteOnClose);

    _m_ui->setupUi(this);

    connect(_m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

Form_New_User::~Form_New_User()
{
    delete _m_ui;
}

void Form_New_User::accept()
{
    if (_m_ui->username->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Donnée obligatoire non saisi"), tr("Le nom de l'utilisateur est nécessaire."));
        return;
    }

    if (_m_ui->password->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Donnée obligatoire non saisi"), tr("Le mot de passe est nécessaire."));
        return;
    }

    if (_m_ui->password_confirm->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Donnée obligatoire non saisi"), tr("Le mot de passe de confirmation est nécessaire."));
        return;
    }


    if (compare_values(_m_ui->password->text().toStdString().c_str(), _m_ui->password_confirm->text().toStdString().c_str()) != 0)
    {
        QMessageBox::critical(this, tr("Donnée saisie incorrecte"), tr("Les mots de passe saisis ne sont pas identiques."));
        return;
    }


    QDialog::accept();
    emit userCreated(new DBUser(_m_ui->username->text().toStdString().c_str(), _m_ui->password->text().toStdString().c_str()));
}

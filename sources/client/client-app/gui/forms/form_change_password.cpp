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

#include "form_change_password.h"
#include "client.h"
#include "session.h"
#include "utilities.h"
#include "constants.h"
#include <QMessageBox>

Form_Change_Password::Form_Change_Password(const QString &username, QWidget *parent) : QDialog(parent), _m_ui(new Ui::Form_Change_Password)
{
    _m_username = username;

    setAttribute(Qt::WA_DeleteOnClose);

    _m_ui->setupUi(this);

    setWindowTitle(tr("Changement de mot de passe %1").arg(_m_username));
    _m_ui->title->setText(tr("Changement de mot de passe %1").arg(_m_username));

	connect(_m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

Form_Change_Password::~Form_Change_Password()
{
	delete _m_ui;
}


void Form_Change_Password::accept()
{
    net_session			*tmp_session = Session::instance()->getClientSession();
    int					tmp_result = NOERR;

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

    tmp_result = cl_modify_user_password(tmp_session, _m_username.toStdString().c_str(), _m_ui->password->text().toStdString().c_str());
	if (tmp_result != NOERR)
	{
        QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_result));
	}
	else
		QDialog::accept();
}

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

#include "form_new_version_information.h"
#include "ui_form_new_version_information.h"

#include <QPushButton>

Form_New_Version_Information::Form_New_Version_Information(QString version, QString informations, QWidget *parent) :
    QDialog(parent),
    _m_ui(new Ui::Form_New_Version_Information)
{
    _m_ui->setupUi(this);

    _m_ui->version->setText(tr("La version <b>%1</b> est disponible.").arg(version));

    if (informations.isEmpty())
	_m_ui->version_informations->setVisible(false);

    _m_ui->version_informations->setText(informations);

    connect(_m_ui->buttonBox->button(QDialogButtonBox::Yes), SIGNAL(clicked()), this, SLOT(accept()));
    connect(_m_ui->buttonBox->button(QDialogButtonBox::No), SIGNAL(clicked()), this, SLOT(reject()));
}


Form_New_Version_Information::~Form_New_Version_Information()
{
    delete _m_ui;
}

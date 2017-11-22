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

#include "form_data_export.h"
#include "ui_Form_Data_Export.h"

#include "form_data_import.h"

#include <QFileDialog>
#include <QMessageBox>

Form_Data_Export::Form_Data_Export(QWidget *parent) : QDialog(parent), _m_ui(new Ui::Form_Data_Export)
{
    QStringList	    tmp_formats_list;

    tmp_formats_list << tr("valeurs séparées par des virgules (csv)") << tr("valeurs séparées par des points virgules") << tr("valeurs séparées par des tabulations") << tr("personnalisé");

    _m_ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);

    _m_ui->file_formats->addItems(tmp_formats_list);

    connect(_m_ui->file_formats, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSeparators()));
    connect(_m_ui->select_destination_file_button, SIGNAL(clicked()), this, SLOT(selectFile()));

    updateSeparators();
}


Form_Data_Export::~Form_Data_Export()
{
    delete _m_ui;
}


void Form_Data_Export::changeEvent(QEvent *e)
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


/**
*  Mettre à jour les separateurs d'après le format de fichier sélectionné
**/
void Form_Data_Export::updateSeparators()
{
    switch (_m_ui->file_formats->currentIndex())
    {
    case 0:
        _m_ui->fields_separators->setText("','");
	_m_ui->records_separators->setText("13+10");
        break;
    case 1:
        _m_ui->fields_separators->setText("';'");
	_m_ui->records_separators->setText("13+10");
        break;
    case 2:
        _m_ui->fields_separators->setText("09");
	_m_ui->records_separators->setText("13+10");
        break;
    case 3:
    default:
        _m_ui->fields_separators->setText("");
        _m_ui->records_separators->setText("");
        break;

    }
}


void Form_Data_Export::selectFile()
{
    QString tmp_file_destination = QFileDialog::getSaveFileName(this, tr("Enregistrer le fichier sous..."));

    if (!tmp_file_destination.isEmpty())
    {
        _m_ui->filepath->setText(tmp_file_destination);
    }
}



void Form_Data_Export::accept()
{
    QString tmp_file_destination = _m_ui->filepath->text();
    QList<char> tmp_field_separators = Form_Data_Import::stringToSeparatorsList(_m_ui->fields_separators->text(), CHAR_SEPARATOR);
    QList<char> tmp_record_separators = Form_Data_Import::stringToSeparatorsList(_m_ui->records_separators->text(), CHAR_SEPARATOR);

    QByteArray  tmp_field_separator;
    QByteArray  tmp_record_separator;
    QByteArray  tmp_field_enclosing_char = _m_ui->text_separator->text().toAscii();

    foreach(char tmp_char, tmp_field_separators)
    {
        tmp_field_separator.append(tmp_char);
    }

    foreach(char tmp_char, tmp_record_separators)
    {
        tmp_record_separator.append(tmp_char);
    }

    if (tmp_file_destination.isEmpty())
    {
        QMessageBox::critical(this, tr("Fichier non sélectionné"), tr("Veuillez sélectionner un fichier de destination."));
    }
    else
    {
        emit startExport(tmp_file_destination, tmp_field_separator, tmp_record_separator, tmp_field_enclosing_char);
        QDialog::accept();
    }
}


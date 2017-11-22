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

#include "form_data_import.h"
#include "ui_Form_Data_Import.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QByteArray>

/**
*  Constructeur
**/
Form_Data_Import::Form_Data_Import(Hierarchy *in_parent, QStringList in_fields_list, const entity_def *in_entity, QWidget *parent) : QDialog(parent), _m_ui(new Ui::Form_Data_Import)
{
    QStringList	    tmp_formats_list;
    QStringList	    tmp_encodings_list;
    QStringList	    tmp_sources_list;

    tmp_formats_list << tr("valeurs séparées par des virgules (csv)") << tr("valeurs séparées par des points virgules") << tr("valeurs séparées par des tabulations") << tr("personnalisé");
    tmp_encodings_list << tr("Europe occidentale (ISO-8859-1)") << tr("Unicode (UTF-8)");
    tmp_sources_list << tr("Windows (CR+LF)") << tr("Autre (LF)");

    _m_columns_names = in_fields_list;
    _m_parent = in_parent;
    _m_entity = in_entity;

    setAttribute(Qt::WA_DeleteOnClose);
    _m_ui->setupUi(this);
    _m_ui->file_formats->addItems(tmp_formats_list);
    _m_ui->file_encodings->addItems(tmp_encodings_list);
    _m_ui->file_sources->addItems(tmp_sources_list);

    connect(_m_ui->file_formats, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSeparators()));
    connect(_m_ui->file_sources, SIGNAL(currentIndexChanged(int)), this, SLOT(updateSeparators()));

    connect(_m_ui->load_file_button, SIGNAL(clicked()), this, SLOT(loadFile()));
    connect(_m_ui->update_overview_button, SIGNAL(clicked()), this, SLOT(updateFileOverview()));

    updateSeparators();
}


/**
*  Destructeur
**/
Form_Data_Import::~Form_Data_Import()
{
    delete _m_ui;
}

void Form_Data_Import::changeEvent(QEvent *e)
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
void Form_Data_Import::updateSeparators()
{
    switch (_m_ui->file_formats->currentIndex())
    {
        case 0:
            _m_ui->fields_separators->setText("','");
            if (_m_ui->file_sources->currentIndex() == 0)
                _m_ui->records_separators->setText("13+10");
            else
                _m_ui->records_separators->setText("10");
            break;
        case 1:
            _m_ui->fields_separators->setText("';'");
            if (_m_ui->file_sources->currentIndex() == 0)
                _m_ui->records_separators->setText("13+10");
            else
                _m_ui->records_separators->setText("10");
            break;
        case 2:
            _m_ui->fields_separators->setText("09");
            if (_m_ui->file_sources->currentIndex() == 0)
                _m_ui->records_separators->setText("13+10");
            else
                _m_ui->records_separators->setText("10");
            break;
        case 3:
        default:
            _m_ui->fields_separators->setText("");
            _m_ui->records_separators->setText("");
            break;

    }
}


/**
*  Mettre à jour l'aperçu du fichier
**/
void Form_Data_Import::updateFileOverview()
{
    readFile(false);
}


/**
*  Importer les données du fichier
**/
void Form_Data_Import::accept()
{
    if (readFile(true))
        QDialog::accept();
}


/**
*  Lire le fichier
**/
bool Form_Data_Import::readFile(bool in_generate_record)
{
    char	    tmp_char;
    char	    tmp_text_separator = '\"';
    QList<char>	    tmp_fields_sep = stringToSeparatorsList(_m_ui->fields_separators->text(), CHAR_SEPARATOR);
    QList<char>	    tmp_records_sep = stringToSeparatorsList(_m_ui->records_separators->text(), CHAR_SEPARATOR);

    QString	    tmp_text_separator_str = _m_ui->text_separator->text();

    int tmp_field_char_index = 0;
    int tmp_record_char_index = 0;

    int		    tmp_column_number = 0;
    int		    tmp_row_number = 0;

    int		    tmp_first_line_of_data = _m_ui->first_line_number->value() - 1;

    QByteArray	    tmp_str;
    QByteArray	    tmp_field_sep_str;
    QByteArray	    tmp_record_sep_str;

    bool	    tmp_neof = false;
    bool	    tmp_first_char_of_field = true;
    bool	    tmp_first_char_is_quote = false;
    bool	    tmp_previous_char_is_quote = false;

    GenericRecord   *tmp_record = NULL;

    if (_m_file.fileName().isEmpty())
    {
        QMessageBox::critical(this, tr("Fichier non sélectionné"), tr("Veuillez sélectionner un fichier."));
    }
    else if (tmp_fields_sep.isEmpty())
    {
        QMessageBox::critical(this, tr("Données manquantes"), tr("Veuillez préciser quels sont les séparateurs de champs."));
    }
    else if (tmp_records_sep.isEmpty())
    {
        QMessageBox::critical(this, tr("Données manquantes"), tr("Veuillez préciser quels sont les séparateurs d'enregistrements."));
    }
    else if (_m_file.exists())
    {
        if (_m_file.open(QIODevice::ReadOnly))
        {
            _m_ui->file_overview_table->setRowCount(0);
            _m_ui->file_overview_table->setColumnCount(0);

            if (tmp_text_separator_str.isEmpty() == false)
                tmp_text_separator = tmp_text_separator_str.at(0).toAscii();

            if (in_generate_record)
                emit startImport(_m_parent);

            tmp_neof = _m_file.getChar(&tmp_char);
            while (tmp_neof)
            {
                // Gestion des double-quotes
                if (tmp_char==tmp_text_separator)
                {
                    if (tmp_first_char_of_field)
                    {
                        tmp_first_char_is_quote = true;
                    }
                    else
                    {
                        if (tmp_previous_char_is_quote)
                        {
                            tmp_str += tmp_char;
                            tmp_previous_char_is_quote = false;
                        }
                        else
                            tmp_previous_char_is_quote = true;
                    }

                    tmp_first_char_of_field = false;
                    tmp_neof = _m_file.getChar(&tmp_char);
                }
                // Separateur de champs
                else if (tmp_char == tmp_fields_sep[0])
                {
                    tmp_first_char_of_field = false;
                    tmp_field_sep_str = QByteArray();

                    while (tmp_neof && tmp_field_char_index < tmp_fields_sep.length() && tmp_char == tmp_fields_sep[tmp_field_char_index])
                    {
                        tmp_field_sep_str += tmp_char;
                        tmp_field_char_index++;
                        tmp_neof = _m_file.getChar(&tmp_char);
                    }

                    if (tmp_field_char_index < tmp_fields_sep.length())
                    {
                        tmp_str += tmp_field_sep_str;
                    }
                    // Enregistrement du champs
                    else
                    {
                        if (!tmp_first_char_is_quote || (tmp_first_char_is_quote && tmp_previous_char_is_quote))
                        {
                            if (tmp_row_number >= tmp_first_line_of_data)
                            {
                                setFieldAt(tmp_str, tmp_row_number - tmp_first_line_of_data, tmp_column_number);

                                if (in_generate_record)
                                {
                                    if (tmp_column_number == 0)
                                        tmp_record = new GenericRecord(_m_entity);

                                    setFieldRecordAt(tmp_record, tmp_str, tmp_column_number);
                                }
                            }

                            tmp_column_number++;

                            tmp_str.clear();

                            tmp_first_char_is_quote = false;
                            tmp_first_char_of_field = true;
                        }
                        else
                        {
                            tmp_str += tmp_field_sep_str;
                        }
                    }

                    tmp_previous_char_is_quote = false;
                    tmp_field_char_index = 0;
                }
                // Separateur d'enregistrements
                else if (tmp_char == tmp_records_sep[0])
                {
                    tmp_first_char_of_field = false;
                    tmp_record_sep_str = QByteArray();

                    while (tmp_neof && tmp_record_char_index < tmp_records_sep.length() && tmp_char == tmp_records_sep[tmp_record_char_index])
                    {
                        tmp_record_sep_str += tmp_char;
                        tmp_record_char_index++;
                        tmp_neof = _m_file.getChar(&tmp_char);
                    }

                    if (tmp_record_char_index < tmp_records_sep.length())
                    {
                        tmp_str += tmp_record_sep_str;
                    }
                    // Enregistrement du champs
                    else
                    {
                        if (!tmp_first_char_is_quote || (tmp_first_char_is_quote && tmp_previous_char_is_quote))
                        {
                            if (tmp_row_number >= tmp_first_line_of_data)
                            {
                                setFieldAt(tmp_str, tmp_row_number - tmp_first_line_of_data, tmp_column_number);

                                if (in_generate_record)
                                {
                                    if (tmp_record == NULL)
                                        tmp_record = new GenericRecord(_m_entity);

                                    if (setFieldRecordAt(tmp_record, tmp_str, tmp_column_number))
                                        emit importRecord(_m_parent, tmp_record, false);
                                }
                            }

                            tmp_row_number++;

                            tmp_column_number = 0;
                            tmp_str.clear();

                            tmp_first_char_is_quote = false;
                            tmp_first_char_of_field = true;
                        }
                        else
                        {
                            tmp_str += tmp_record_sep_str;
                        }
                    }

                    tmp_previous_char_is_quote = false;
                    tmp_record_char_index = 0;
                }
                else
                {
                    tmp_previous_char_is_quote = false;
                    tmp_first_char_of_field = false;
                    tmp_str += tmp_char;
                    tmp_neof = _m_file.getChar(&tmp_char);
                }
            }

            if (tmp_str.isEmpty() == false)
            {
                setFieldAt(tmp_str, tmp_row_number, tmp_column_number);
                if (in_generate_record)
                {
                    if (tmp_record == NULL)
                        tmp_record = new GenericRecord(_m_entity);

                    if (setFieldRecordAt(tmp_record, tmp_str, tmp_column_number))
                        emit importRecord(_m_parent, tmp_record, true);
                }
            }
            else if (tmp_column_number > 0 && in_generate_record)
            {
                emit importRecord(_m_parent, tmp_record, true);
            }
            else if (in_generate_record)
                emit importRecord(_m_parent, NULL, true);

            // Fermer le fichier
            _m_file.close();

            updateMappingTable();

            return true;

        }
        else
            QMessageBox::critical(this, tr("Ouverture du fichier impossible"), tr("Le fichier %1 ne peut être ouvert en lecture.").arg(_m_file.fileName()));
    }
    else
        QMessageBox::critical(this, tr("Fichier non trouvé"), tr("Le fichier %1 n'existe pas.").arg(_m_file.fileName()));

    return false;
}


void Form_Data_Import::setFieldAt(QByteArray in_field_value, int in_row, int in_column)
{
    QTableWidgetItem        *tmp_widget_item = NULL;
    QString		    tmp_value;

    if (_m_ui->file_encodings->currentIndex() == 0)
        tmp_value = QString::fromLatin1(in_field_value);
    else
        tmp_value = QString::fromUtf8(in_field_value);

    while (_m_ui->file_overview_table->rowCount() <= in_row)
        _m_ui->file_overview_table->insertRow(_m_ui->file_overview_table->rowCount());

    while (_m_ui->file_overview_table->columnCount() <= in_column)
        _m_ui->file_overview_table->insertColumn(_m_ui->file_overview_table->columnCount());

    tmp_widget_item = new QTableWidgetItem;
    tmp_widget_item->setText(tmp_value);
    _m_ui->file_overview_table->setItem(in_row, in_column, tmp_widget_item);
}

bool Form_Data_Import::setFieldRecordAt(GenericRecord *in_record, QByteArray in_field_value, int in_column)
{
    QComboBox	*tmp_combo = NULL;
    QModelIndex tmp_index;
    int		tmp_combo_index = 0;
    QString	tmp_value;

    if (_m_ui->fields_mapping_table->rowCount() > in_column)
    {
        tmp_index = _m_ui->fields_mapping_table->model()->index(in_column, 0);
        if (tmp_index.isValid())
        {
            tmp_combo = ::qobject_cast< QComboBox *>(_m_ui->fields_mapping_table->indexWidget(tmp_index));
            if (tmp_combo != NULL)
            {
                tmp_combo_index = tmp_combo->currentIndex();
                if (tmp_combo_index > 0)
                {
                    if (_m_ui->file_encodings->currentIndex() == 0)
                        tmp_value = QString::fromLatin1(in_field_value);
                    else
                        tmp_value = QString::fromUtf8(in_field_value);

                    return (in_record->setValueForKeyAtIndex(tmp_value.toStdString().c_str(), tmp_combo_index - 1) == NOERR);
                }
            }
        }
    }

    return true;
}

/**
* Charger un fichier
**/
void Form_Data_Import::loadFile()
{

    QString tmp_filename = QFileDialog::getOpenFileName(
                this,
                tr("Sélectionner un fichier"));

    if (tmp_filename.isEmpty() == false)
    {
        _m_file.setFileName(tmp_filename);
        _m_ui->filename->setText(tmp_filename);
        updateFileOverview();
    }
}


QList<char> Form_Data_Import::stringToSeparatorsList(const QString & in_str, char in_separator)
{
    QList<char> tmp_sperators_list;

    QStringList tmp_seperators = in_str.split(in_separator, QString::SkipEmptyParts, Qt::CaseSensitive);
    foreach(QString tmp_separator, tmp_seperators)
    {
        int tmp_start_index = 0;
        int tmp_end_index = tmp_separator.length();

        if (tmp_separator.startsWith('\''))
        {
            tmp_start_index = 1;

            if (tmp_separator.endsWith('\''))
            {
                tmp_end_index--;
            }

            for(int tmp_index = tmp_start_index; tmp_index < tmp_end_index; tmp_index++)
            {
                tmp_sperators_list << tmp_separator[tmp_index].toAscii();
            }
        }
        else
        {
            tmp_sperators_list << tmp_separator.toInt();
        }
    }

    return tmp_sperators_list;
}



void Form_Data_Import::updateMappingTable()
{
    QComboBox		    *tmp_combobox = NULL;
    QStringList         tmp_columns_headers;
    QStringList         tmp_column_header;

    QStringList             tmp_rows_headers;

    for(int tmp_column_index = 0; tmp_column_index < _m_ui->file_overview_table->columnCount(); tmp_column_index++){
        tmp_columns_headers << tr("Champ %1").arg(tmp_column_index + 1);
    }
    _m_ui->file_overview_table->setHorizontalHeaderLabels(tmp_columns_headers);

    for(int tmp_row_index = 0; tmp_row_index < _m_ui->file_overview_table->rowCount(); tmp_row_index++){
        tmp_rows_headers << tr("Ligne %1").arg(tmp_row_index + 1);
    }
    _m_ui->file_overview_table->setVerticalHeaderLabels(tmp_rows_headers);


    if (_m_ui->file_overview_table->columnCount() != _m_ui->fields_mapping_table->rowCount())
    {
        _m_ui->fields_mapping_table->setRowCount(_m_ui->file_overview_table->columnCount());
        _m_ui->fields_mapping_table->setColumnCount(1);

        tmp_column_header << tr("Correspondance");
        _m_ui->fields_mapping_table->setHorizontalHeaderLabels(tmp_column_header);

        for(int tmp_row_index = 0; tmp_row_index < _m_ui->fields_mapping_table->rowCount(); tmp_row_index++)
        {
            tmp_combobox = new QComboBox;
            tmp_combobox->addItem(tr("Pas de correspondance"));
            tmp_combobox->addItems(_m_columns_names);
            if (tmp_row_index + 1 < tmp_combobox->count())
                tmp_combobox->setCurrentIndex(tmp_row_index + 1);
            _m_ui->fields_mapping_table->setCellWidget(tmp_row_index, 0, tmp_combobox);
        }

        _m_ui->fields_mapping_table->setVerticalHeaderLabels(tmp_columns_headers);
    }
}


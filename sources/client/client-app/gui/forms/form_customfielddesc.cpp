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

#include "form_customfielddesc.h"
#include "ui_Form_CustomFieldDesc.h"

Form_CustomFieldDesc::Form_CustomFieldDesc(CustomFieldDesc *customField, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Form_CustomFieldDesc),
    _m_custom_field(customField)
{
    const char *defaultFieldValue = _m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);

    ui->setupUi(this);
    ui->widgetRecordTextEditDefaultValue->addTextToolBar(RecordTextEditToolBar::Small);

    ui->lineEditFieldLabel->setText(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_LABEL));
    ui->checkBoxMandatory->setChecked(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_MANDATORY), YES) == 0);
    if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_PLAIN_TEXT) == 0)
    {
        ui->lineEditDefaultValue->setText(defaultFieldValue);
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_RICH_TEXT) == 0)
    {
        ui->widgetRecordTextEditDefaultValue->textEditor()->setHtml(defaultFieldValue);
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_CHECKBOX) == 0)
    {
        ui->checkBoxDefaultValue->setChecked(compare_values(defaultFieldValue, YES) == 0);
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_LIST) == 0)
    {

        QStringList values = QString(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_VALUES)).split(';', QString::SkipEmptyParts);
        for(int index = 0; index < values.count(); ++index)
        {
            addValue(values[index]);
        }
        ui->comboBoxDefaultValue->setCurrentIndex(ui->comboBoxDefaultValue->findText(defaultFieldValue));
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_INTEGER) == 0)
    {
        ui->spinBoxDefaultValue->setValue(QString(defaultFieldValue).toInt());
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_FLOAT) == 0)
    {
        ui->doubleSpinBoxDefaultValue->setValue(QString(defaultFieldValue).toDouble());
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_TIME) == 0)
    {
        ui->checkBoxCurrentTime->setChecked(is_empty_string(defaultFieldValue));
        ui->timeEditDefaultValue->setTime(QTime::fromString(defaultFieldValue, TimeFormat));
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_DATE) == 0)
    {
        ui->checkBoxCurrentDate->setChecked(is_empty_string(defaultFieldValue));
        ui->dateEditDefaultValue->setDate(QDate::fromString(defaultFieldValue, DateFormat));
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_DATETIME) == 0)
    {
        ui->checkBoxCurrentDateTime->setChecked(is_empty_string(defaultFieldValue));
        ui->dateTimeEditDefaultValue->setDateTime(QDateTime::fromString(defaultFieldValue, DateTimeFormat));
    }

    ui->comboBoxFieldType->addItem(tr("Texte"), CUSTOM_FIELDS_PLAIN_TEXT);
    ui->comboBoxFieldType->addItem(tr("Texte enrichi"), CUSTOM_FIELDS_RICH_TEXT);
    ui->comboBoxFieldType->addItem(tr("Boîte à cocher"), CUSTOM_FIELDS_CHECKBOX);
    ui->comboBoxFieldType->addItem(tr("Liste de valeurs"), CUSTOM_FIELDS_LIST);
    ui->comboBoxFieldType->addItem(tr("Nombre entier"), CUSTOM_FIELDS_INTEGER);
    ui->comboBoxFieldType->addItem(tr("Nombre décimal"), CUSTOM_FIELDS_FLOAT);
    ui->comboBoxFieldType->addItem(tr("Heure"), CUSTOM_FIELDS_TIME);
    ui->comboBoxFieldType->addItem(tr("Date"), CUSTOM_FIELDS_DATE);
    ui->comboBoxFieldType->addItem(tr("Date et heure"), CUSTOM_FIELDS_DATETIME);

    ui->comboBoxFieldType->setCurrentIndex(ui->comboBoxFieldType->findData(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE)));
    connect(ui->listWidgetFieldValues, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(changeValue(QListWidgetItem*)));

    connect(ui->comboBoxFieldType, SIGNAL(currentIndexChanged(int)), this, SLOT(changeFieldType()));

    connect(ui->pushButtonAddValue, SIGNAL(clicked()), this, SLOT(addValue()));
    connect(ui->pushButtonRemoveValue, SIGNAL(clicked()), this, SLOT(removeValue()));
    connect(ui->pushButtonValueUp, SIGNAL(clicked()), this, SLOT(putValueUp()));
    connect(ui->pushButtonValueDown, SIGNAL(clicked()), this, SLOT(putValueDown()));

    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    changeFieldType();
}

Form_CustomFieldDesc::~Form_CustomFieldDesc()
{
    delete ui;
}

void Form_CustomFieldDesc::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void Form_CustomFieldDesc::accept()
{
    _m_custom_field->setValueForKey(ui->checkBoxMandatory->isChecked() ? YES : NO, CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_MANDATORY);
    _m_custom_field->setValueForKey(ui->lineEditFieldLabel->text().toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_LABEL);
    _m_custom_field->setValueForKey(ui->comboBoxFieldType->itemData(ui->comboBoxFieldType->currentIndex()).toString().toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE);

    if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_PLAIN_TEXT) == 0)
    {
        _m_custom_field->setValueForKey(ui->lineEditDefaultValue->text().toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_RICH_TEXT) == 0)
    {
        _m_custom_field->setValueForKey(ui->widgetRecordTextEditDefaultValue->textEditor()->toHtml().toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_CHECKBOX) == 0)
    {
        _m_custom_field->setValueForKey(ui->checkBoxDefaultValue->isChecked() ? YES : NO, CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_LIST) == 0)
    {
        _m_custom_field->setValueForKey(ui->comboBoxDefaultValue->itemText(ui->comboBoxDefaultValue->currentIndex()).toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);

        QString value;
        QListWidgetItem *item;
        for(int index = 0; index < ui->listWidgetFieldValues->count(); ++index)
        {
            item = ui->listWidgetFieldValues->item(index);
            value += QString("%1;").arg(item->text());
        }
        _m_custom_field->setValueForKey(value.toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_VALUES);
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_INTEGER) == 0)
    {
        _m_custom_field->setValueForKey(ui->spinBoxDefaultValue->text().toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_FLOAT) == 0)
    {
        _m_custom_field->setValueForKey(ui->doubleSpinBoxDefaultValue->text().toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_TIME) == 0)
    {
        if (ui->checkBoxCurrentTime->isChecked())
            _m_custom_field->setValueForKey(NULL, CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
        else
            _m_custom_field->setValueForKey(ui->timeEditDefaultValue->time().toString(TimeFormat).toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_DATE) == 0)
    {
        if (ui->checkBoxCurrentDate->isChecked())
            _m_custom_field->setValueForKey(NULL, CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
        else
            _m_custom_field->setValueForKey(ui->dateEditDefaultValue->date().toString(DateFormat).toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
    }
    else if(compare_values(_m_custom_field->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_DATETIME) == 0)
    {
        if (ui->checkBoxCurrentDateTime->isChecked())
            _m_custom_field->setValueForKey(NULL, CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
        else
            _m_custom_field->setValueForKey(ui->dateTimeEditDefaultValue->dateTime().toString(DateTimeFormat).toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
    }

    emit saved(_m_custom_field);
    QDialog::accept();
}


void Form_CustomFieldDesc::reject()
{
    emit unsaved(_m_custom_field);
    QDialog::reject();
}


void Form_CustomFieldDesc::changeFieldType()
{
    ui->lineEditDefaultValue->setVisible(false);
    ui->widgetRecordTextEditDefaultValue->setVisible(false);
    ui->widgetCheckBox->setVisible(false);
    ui->widgetFieldValues->setVisible(false);
    ui->widgetSpinBox->setVisible(false);
    ui->widgetDoubleSpinBox->setVisible(false);
    ui->widgetTimeEdit->setVisible(false);
    ui->widgetDateEdit->setVisible(false);
    ui->widgetDateTimeEdit->setVisible(false);
    ui->comboBoxDefaultValue->setVisible(false);

    if(ui->comboBoxFieldType->itemData(ui->comboBoxFieldType->currentIndex()).toString().toStdString() == CUSTOM_FIELDS_PLAIN_TEXT)
    {
        ui->lineEditDefaultValue->setVisible(true);
    }
    else if(ui->comboBoxFieldType->itemData(ui->comboBoxFieldType->currentIndex()).toString().toStdString() == CUSTOM_FIELDS_RICH_TEXT)
    {
        ui->widgetRecordTextEditDefaultValue->setVisible(true);
    }
    else if(ui->comboBoxFieldType->itemData(ui->comboBoxFieldType->currentIndex()).toString().toStdString() == CUSTOM_FIELDS_CHECKBOX)
    {
        ui->widgetCheckBox->setVisible(true);
    }
    else if(ui->comboBoxFieldType->itemData(ui->comboBoxFieldType->currentIndex()).toString().toStdString() == CUSTOM_FIELDS_LIST)
    {
        ui->widgetFieldValues->setVisible(true);
        ui->comboBoxDefaultValue->setVisible(true);
    }
    else if(ui->comboBoxFieldType->itemData(ui->comboBoxFieldType->currentIndex()).toString().toStdString() == CUSTOM_FIELDS_INTEGER)
    {
        ui->widgetSpinBox->setVisible(true);
    }
    else if(ui->comboBoxFieldType->itemData(ui->comboBoxFieldType->currentIndex()).toString().toStdString() == CUSTOM_FIELDS_FLOAT)
    {
        ui->widgetDoubleSpinBox->setVisible(true);
    }
    else if(ui->comboBoxFieldType->itemData(ui->comboBoxFieldType->currentIndex()).toString().toStdString() == CUSTOM_FIELDS_TIME)
    {
        ui->widgetTimeEdit->setVisible(true);
    }
    else if(ui->comboBoxFieldType->itemData(ui->comboBoxFieldType->currentIndex()).toString().toStdString() == CUSTOM_FIELDS_DATE)
    {
        ui->widgetDateEdit->setVisible(true);
    }
    else if(ui->comboBoxFieldType->itemData(ui->comboBoxFieldType->currentIndex()).toString().toStdString() == CUSTOM_FIELDS_DATETIME)
    {
        ui->widgetDateTimeEdit->setVisible(true);
    }

    adjustSize();
}

void Form_CustomFieldDesc::addValue(const QString& value)
{
    QListWidgetItem *item = new QListWidgetItem(value, ui->listWidgetFieldValues);
    ui->listWidgetFieldValues->insertItem(ui->listWidgetFieldValues->count(), item);
    ui->comboBoxDefaultValue->insertItem(ui->listWidgetFieldValues->count(), value);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
}

void Form_CustomFieldDesc::addValue()
{
    addValue(tr("Nouvelle valeur"));
}


void Form_CustomFieldDesc::removeValue()
{
    int index = ui->listWidgetFieldValues->currentRow();
    if (index >= 0)
    {
        delete ui->listWidgetFieldValues->takeItem(index);
        ui->comboBoxDefaultValue->removeItem(index);
    }
}


void Form_CustomFieldDesc::putValueUp()
{
    int index = ui->listWidgetFieldValues->currentRow();
    if (index > 0)
    {
        QListWidgetItem *currentItem = ui->listWidgetFieldValues->item(index);
        QListWidgetItem *previousItem = ui->listWidgetFieldValues->item(index - 1);

        QString currentText = currentItem->text();
        QString previousText = previousItem->text();

        currentItem->setText(previousText);
        previousItem->setText(currentText);

        ui->comboBoxDefaultValue->setItemText(index, previousText);
        ui->comboBoxDefaultValue->setItemText(index - 1, currentText);

        ui->listWidgetFieldValues->setCurrentRow(index - 1);
    }
}


void Form_CustomFieldDesc::putValueDown()
{
    int index = ui->listWidgetFieldValues->currentRow();
    if (index >= 0 && index < ui->listWidgetFieldValues->count() - 1)
    {
        QListWidgetItem *currentItem = ui->listWidgetFieldValues->item(index);
        QListWidgetItem *nextItem = ui->listWidgetFieldValues->item(index + 1);

        QString currentText = currentItem->text();
        QString nextText = nextItem->text();

        currentItem->setText(nextText);
        nextItem->setText(currentText);

        ui->comboBoxDefaultValue->setItemText(index, nextText);
        ui->comboBoxDefaultValue->setItemText(index + 1, currentText);

        ui->listWidgetFieldValues->setCurrentRow(index + 1);
    }
}

void Form_CustomFieldDesc::changeValue(QListWidgetItem* item)
{
    for(int index = 0; index < ui->listWidgetFieldValues->count(); ++index)
    {
        if (ui->listWidgetFieldValues->item(index) == item)
            ui->comboBoxDefaultValue->setItemText(index, item->text());
    }
}

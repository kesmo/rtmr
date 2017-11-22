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

#include "custom_fields_controls_manager.h"

#include "session.h"

#include "gui/components/record_text_edit_widget.h"

#include <QGridLayout>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTimeEdit>
#include <QDateEdit>
#include <QDateTimeEdit>

CustomFieldsControlsManager::CustomFieldsControlsManager():
    _m_custom_fields_tab_widget(NULL)
{
}


void CustomFieldsControlsManager::loadCustomFieldsView(QToolBox* toolBox, const QList<CustomFieldDesc*>& customFieldsDesc)
{
    if (!customFieldsDesc.isEmpty())
    {
        QWidget* page_custom_fields = new QWidget();
        QGridLayout* gridLayout = new QGridLayout(page_custom_fields);
        _m_custom_fields_tab_widget = new QTabWidget(page_custom_fields);

        gridLayout->addWidget(_m_custom_fields_tab_widget, 0, 0, 1, 1);
        page_custom_fields->setLayout(gridLayout);

        foreach (CustomFieldDesc* customField, customFieldsDesc)
        {
            bool tabNameExists = false;
            int tmp_index = 0;
            for(; tmp_index < _m_custom_fields_tab_widget->count(); ++tmp_index)
            {
                if (compare_values(_m_custom_fields_tab_widget->tabText(tmp_index).toStdString().c_str(), customField->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME)) == 0){
                    tabNameExists = true;
                    break;
                }
            }

            if (!tabNameExists)
            {
                addCustomFieldsTab(customField->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME));
                addCustomFields(customField, tmp_index);
            }
            else
                addCustomFields(customField, tmp_index);
        }

        for(int tmp_index = 0; tmp_index < _m_custom_fields_tab_widget->count(); ++tmp_index)
        {
            QWidget *tabWidget = _m_custom_fields_tab_widget->widget(tmp_index);
            if (tabWidget && tabWidget->layout())
            {
                tabWidget->layout()->addItem(new QSpacerItem(20, 40, QSizePolicy::Preferred, QSizePolicy::Expanding));
            }
        }

        toolBox->addItem(page_custom_fields, QApplication::tr("Champs personnalisés"));
    }
}


void CustomFieldsControlsManager::addCustomFieldsTab(QString tabName)
{
    QScrollArea* scrollArea = new QScrollArea(_m_custom_fields_tab_widget);
    QVBoxLayout *vBoxLayout = new QVBoxLayout(scrollArea);

    scrollArea->setLayout(vBoxLayout);

    _m_custom_fields_tab_widget->addTab(scrollArea, tabName);
}


void CustomFieldsControlsManager::addCustomFields(CustomFieldDesc* customFieldDesc, int tabIndex)
{
    QWidget *tabWidget = _m_custom_fields_tab_widget->widget(tabIndex);
    if (tabWidget && tabWidget->layout())
    {
        QHBoxLayout* hBoxLayout = new QHBoxLayout();
        QLabel* label = new QLabel(tabWidget);
        const char* fieldValue = customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);

        hBoxLayout->setParent(tabWidget->layout());

        label->setText(QString("%1 : ").arg(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_LABEL)));
        label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

        if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_MANDATORY), YES) == 0)
        {
            QFont font;
            font.setBold(true);
            label->setFont(font);
        }

        hBoxLayout->addWidget(label);

        if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_PLAIN_TEXT) == 0)
        {
            QLineEdit* lineEdit = new QLineEdit(tabWidget);
            lineEdit->setText(fieldValue);
            hBoxLayout->addWidget(lineEdit);
            _m_custom_fields_controls_map[customFieldDesc] = lineEdit;
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_RICH_TEXT) == 0)
        {
            RecordTextEditWidget* textEdit = new RecordTextEditWidget(tabWidget);
            textEdit->addTextToolBar(RecordTextEditToolBar::Small);
            textEdit->textEditor()->setHtml(fieldValue);
            hBoxLayout->addWidget(textEdit);
            _m_custom_fields_controls_map[customFieldDesc] = textEdit;
            label->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_CHECKBOX) == 0)
        {
            QCheckBox* checkBox = new QCheckBox(tabWidget);
            checkBox->setChecked(compare_values(fieldValue, YES) == 0);
            hBoxLayout->addWidget(checkBox);
            hBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
            _m_custom_fields_controls_map[customFieldDesc] = checkBox;
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_LIST) == 0)
        {
            QComboBox* comboBox = new QComboBox(tabWidget);
            QStringList values = QString(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_VALUES)).split(';', QString::SkipEmptyParts);
            for(int index = 0; index < values.count(); ++index)
            {
                comboBox->addItem(values[index]);
            }
            comboBox->setCurrentIndex(comboBox->findText(fieldValue));
            hBoxLayout->addWidget(comboBox);
            _m_custom_fields_controls_map[customFieldDesc] = comboBox;
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_INTEGER) == 0)
        {
            QSpinBox* spinBox = new QSpinBox(tabWidget);
            spinBox->setValue(QString(fieldValue).toInt());
            spinBox->setMinimum(-999999999);
            spinBox->setMaximum(999999999);
            hBoxLayout->addWidget(spinBox);
            hBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
            _m_custom_fields_controls_map[customFieldDesc] = spinBox;
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_FLOAT) == 0)
        {
            QDoubleSpinBox* doubleSpinBox = new QDoubleSpinBox(tabWidget);
            doubleSpinBox->setValue(QString(fieldValue).toDouble());
            doubleSpinBox->setMinimum(-1e+09);
            doubleSpinBox->setMaximum(1e+09);
            hBoxLayout->addWidget(doubleSpinBox);
            hBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
            _m_custom_fields_controls_map[customFieldDesc] = doubleSpinBox;
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_TIME) == 0)
        {
            QTimeEdit* timeEdit = new QTimeEdit(tabWidget);
            if (is_empty_string(fieldValue))
                timeEdit->setTime(QTime::currentTime());
            else
                timeEdit->setTime(QTime::fromString(fieldValue, TimeFormat));

            hBoxLayout->addWidget(timeEdit);
            hBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
            _m_custom_fields_controls_map[customFieldDesc] = timeEdit;
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_DATE) == 0)
        {
            QDateEdit* dateEdit = new QDateEdit(tabWidget);
            if (is_empty_string(fieldValue))
                dateEdit->setDate(QDate::currentDate());
            else
                dateEdit->setDate(QDate::fromString(fieldValue, DateFormat));

            hBoxLayout->addWidget(dateEdit);
            hBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
            _m_custom_fields_controls_map[customFieldDesc] = dateEdit;
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_DATETIME) == 0)
        {
            QDateTimeEdit* dateTimeEdit = new QDateTimeEdit(tabWidget);
            if (is_empty_string(fieldValue))
                dateTimeEdit->setDateTime(QDateTime::currentDateTime());
            else
                dateTimeEdit->setDateTime(QDateTime::fromString(fieldValue, DateTimeFormat));

            hBoxLayout->addWidget(dateTimeEdit);
            hBoxLayout->addSpacerItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
            _m_custom_fields_controls_map[customFieldDesc] = dateTimeEdit;
        }


        tabWidget->layout()->addItem(hBoxLayout);

    }
}


void CustomFieldsControlsManager::popCustomFieldValue(const CustomFieldDesc* customFieldDesc, Record* in_custom_field_record, const char* in_value_key, bool enable_control)
{
    QWidget* widget = _m_custom_fields_controls_map[customFieldDesc];
    if (widget)
    {
        const char* fieldValue = in_custom_field_record->getValueForKey(in_value_key);

        if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_PLAIN_TEXT) == 0)
        {
            qobject_cast<QLineEdit*>(widget)->setText(fieldValue);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_RICH_TEXT) == 0)
        {
            dynamic_cast<RecordTextEditWidget*>(widget)->textEditor()->setHtml(fieldValue);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_CHECKBOX) == 0)
        {
            qobject_cast<QCheckBox*>(widget)->setChecked(compare_values(fieldValue, YES) == 0);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_LIST) == 0)
        {
            qobject_cast<QComboBox*>(widget)->setCurrentIndex(qobject_cast<QComboBox*>(widget)->findText(fieldValue));
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_INTEGER) == 0)
        {
            qobject_cast<QSpinBox*>(widget)->setValue(QString(fieldValue).toInt());
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_FLOAT) == 0)
        {
            qobject_cast<QDoubleSpinBox*>(widget)->setValue(QString(fieldValue).toDouble());
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_TIME) == 0)
        {
            QTimeEdit* timeEdit = qobject_cast<QTimeEdit*>(widget);
            if (is_empty_string(fieldValue))
                timeEdit->setTime(QTime::currentTime());
            else
                timeEdit->setTime(QTime::fromString(fieldValue, TimeFormat));
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_DATE) == 0)
        {
            QDateEdit* dateEdit = qobject_cast<QDateEdit*>(widget);
            if (is_empty_string(fieldValue))
                dateEdit->setDate(QDate::currentDate());
            else
                dateEdit->setDate(QDate::fromString(fieldValue, DateFormat));
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_DATETIME) == 0)
        {
            QDateTimeEdit* dateTimeEdit = qobject_cast<QDateTimeEdit*>(widget);

            if (is_empty_string(fieldValue))
                dateTimeEdit->setDateTime(QDateTime::currentDateTime());
            else
                dateTimeEdit->setDateTime(QDateTime::fromString(fieldValue, DateTimeFormat));
        }

        widget->setEnabled(enable_control);
    }
}

int CustomFieldsControlsManager::pushEnteredCustomFieldValue(const CustomFieldDesc* customFieldDesc, Record* in_custom_field_record, const char* in_value_key)
{
    QWidget* widget = _m_custom_fields_controls_map[customFieldDesc];
    if (widget)
    {
        if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_PLAIN_TEXT) == 0)
        {
            in_custom_field_record->setValueForKey(qobject_cast<QLineEdit*>(widget)->text().toStdString().c_str(), in_value_key);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_RICH_TEXT) == 0)
        {
            in_custom_field_record->setValueForKey(dynamic_cast<RecordTextEditWidget*>(widget)->textEditor()->toHtml().toStdString().c_str(), in_value_key);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_CHECKBOX) == 0)
        {
            in_custom_field_record->setValueForKey(qobject_cast<QCheckBox*>(widget)->isChecked() ? YES : NO, in_value_key);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_LIST) == 0)
        {
            in_custom_field_record->setValueForKey(qobject_cast<QComboBox*>(widget)->itemText(qobject_cast<QComboBox*>(widget)->currentIndex()).toStdString().c_str(), in_value_key);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_INTEGER) == 0)
        {
            in_custom_field_record->setValueForKey(qobject_cast<QSpinBox*>(widget)->text().toStdString().c_str(), in_value_key);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_FLOAT) == 0)
        {
            in_custom_field_record->setValueForKey(qobject_cast<QDoubleSpinBox*>(widget)->text().toStdString().c_str(), in_value_key);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_TIME) == 0)
        {
            in_custom_field_record->setValueForKey(qobject_cast<QTimeEdit*>(widget)->time().toString(TimeFormat).toStdString().c_str(), in_value_key);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_DATE) == 0)
        {
            in_custom_field_record->setValueForKey(qobject_cast<QDateEdit*>(widget)->date().toString(DateFormat).toStdString().c_str(), in_value_key);
        }
        else if(compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE), CUSTOM_FIELDS_DATETIME) == 0)
        {
            in_custom_field_record->setValueForKey(qobject_cast<QDateTimeEdit*>(widget)->dateTime().toString(DateTimeFormat).toStdString().c_str(), in_value_key);
        }

        // Check that if the field is mandatory, its value is not empty
        if (compare_values(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_MANDATORY), YES) == 0)
        {
            if (is_empty_string(in_custom_field_record->getValueForKey(in_value_key)))
                return EMPTY_OBJECT;
        }
    }

    return NOERR;
}

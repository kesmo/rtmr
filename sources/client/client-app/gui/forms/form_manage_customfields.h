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

#ifndef FORM_MANAGE_CUSTOMFIELDS_H
#define FORM_MANAGE_CUSTOMFIELDS_H


#include "customfielddesc.h"

#include "gui/components/records_table_model.h"

#include <QDialog>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QTableView>
#include <QSortFilterProxyModel>

namespace Ui {
class Form_Manage_CustomFields;
}


class Form_Manage_CustomFields : public QDialog
{
    Q_OBJECT
    
public:
    explicit Form_Manage_CustomFields(const char *fieldEntity, QWidget *parent = 0);
    ~Form_Manage_CustomFields();
    
public Q_SLOTS:
    void addTab();
    void removeSelectedTab();

    void addField();
    void editField();
    void removeField();
    void renameSelectedTab();

    void accept();

    void acceptAddedField(CustomFieldDesc* customField);
    void rejectAddedField(CustomFieldDesc* customField);

private:
    Ui::Form_Manage_CustomFields *_m_ui;

    const char *_m_field_entity;

    QMap<QPushButton*, QPair<QLineEdit*, QTableView*> >	    _m_rename_tab_button_map;

    QMap<QPushButton*, QTableView* >	    _m_add_field_buttons_map;
    QMap<QPushButton*, QTableView* >	    _m_remove_field_buttons_map;

    QMap<QTableView*, QSortFilterProxyModel*> _m_custom_fields_model_proxy;

    RecordsTableModel<CustomFieldDesc>   *_m_custom_fields_model;

    void addTab(QString tabName);

};

#endif // FORM_MANAGE_CUSTOMFIELDS_H

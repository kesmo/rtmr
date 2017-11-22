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

#ifndef FORM_DATA_IMPORT_H
#define FORM_DATA_IMPORT_H

#include <QDialog>
#include <QFile>
#include <QCloseEvent>
#include <QPair>
#include "hierarchy.h"
#include "genericrecord.h"

#define CHAR_SEPARATOR		'+'
#define PARENT_HIERARCHY_SEPARATOR          '\\'

namespace Ui {
    class Form_Data_Import;
}

class Form_Data_Import : public QDialog {

    Q_OBJECT

public:
    Form_Data_Import(Hierarchy *in_parent, QStringList in_fields_list, const entity_def *in_entity, QWidget *parent = 0);
    ~Form_Data_Import();

    static QList<char> stringToSeparatorsList(const QString & in_str, char in_separator);

Q_SIGNALS:
    void startImport(Hierarchy *in_parent);
    void importRecord(Hierarchy *in_parent, GenericRecord *out_record, bool in_last_record);

public slots:
    virtual void accept();

protected slots:
	void updateSeparators();
	void updateFileOverview();
	void loadFile();
	void updateMappingTable();

protected:
    void changeEvent(QEvent *e);
    bool setFieldRecordAt(GenericRecord *in_record, QByteArray in_field_value, int in_column);
    void setFieldAt(QByteArray in_field_value, int in_row, int in_column);
    bool readFile(bool in_generate_record);

private:
    Ui::Form_Data_Import *_m_ui;

    Hierarchy		*_m_parent;
    QFile		_m_file;
    QStringList		_m_columns_names;

    const entity_def		*_m_entity;
};

#endif // FORM_DATA_IMPORT_H

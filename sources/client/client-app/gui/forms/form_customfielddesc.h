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

#ifndef FORM_CUSTOMFIELDDESC_H
#define FORM_CUSTOMFIELDDESC_H

#include "customfielddesc.h"

#include <QDialog>
#include <QListWidgetItem>

namespace Ui {
    class Form_CustomFieldDesc;
}

class Form_CustomFieldDesc : public QDialog {
    Q_OBJECT
public:

    Form_CustomFieldDesc(CustomFieldDesc* customField, QWidget *parent = 0);
    ~Form_CustomFieldDesc();


public Q_SLOTS:
    virtual void accept();
    virtual void reject();

    void changeFieldType();

    void addValue();
    void removeValue();
    void putValueUp();
    void putValueDown();

    void changeValue(QListWidgetItem* item);

signals:
    void saved(CustomFieldDesc*);
    void unsaved(CustomFieldDesc*);

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_CustomFieldDesc *ui;

    CustomFieldDesc *_m_custom_field;

    void addValue(const QString& value);

};

#endif // FORM_CUSTOMFIELDDESC_H

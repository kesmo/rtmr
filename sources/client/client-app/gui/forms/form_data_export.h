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

#ifndef FORM_DATA_EXPORT_H
#define FORM_DATA_EXPORT_H

#include "hierarchy.h"
#include <QDialog>

namespace Ui {
    class Form_Data_Export;
}

class Form_Data_Export : public QDialog {
    Q_OBJECT
public:
    Form_Data_Export(QWidget *parent = 0);
    ~Form_Data_Export();

protected:
    void changeEvent(QEvent *e);

Q_SIGNALS:
    void startExport(QString in_filepath, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char);

public slots:
    virtual void accept();

protected slots:
        void updateSeparators();
        void selectFile();

private:
    Ui::Form_Data_Export *_m_ui;
};

#endif // FORM_DATA_EXPORT_H

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

#ifndef FORM_LOGON_H
#define FORM_LOGON_H

#include <QDialog>

QT_FORWARD_DECLARE_CLASS(Ui_Form_Logon);

class Form_Logon : public QDialog
{
    Q_OBJECT

public:
    Form_Logon(QWidget *parent = 0);
    ~Form_Logon();

public Q_SLOTS:
    void accept();

private:
    Ui_Form_Logon   *_m_ui;

    void readSettings();
    void writeSettings();

};

#endif // FORM_LOGON_H

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

#include "session.h"
#include "form_logon.h"
#include "ui_Form_Logon.h"

#include "client.h"
#include "constants.h"

#include <QtGui>
#include <QtGui>


/**
  Constructeur
**/
Form_Logon::Form_Logon(QWidget *parent) : QDialog(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    _m_ui = new  Ui_Form_Logon;

    _m_ui->setupUi(this);

    readSettings();

    if (_m_ui->username->text().isEmpty())
    {
        _m_ui->username->setFocus();
        _m_ui->username->selectAll();
    }
    else
        _m_ui->password->setFocus();

    _m_ui->port_number->setValidator(new QIntValidator(0, 65535, this));

    connect(_m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
}

Form_Logon::~Form_Logon()
{
    delete _m_ui;
}

/**
  Validation par le bouton OK
**/
void Form_Logon::accept()
{
    int             tmp_login_result = NOERR;
    long            tmp_return_result = 0;

    int				tmp_host_port = 0;

    QSettings       tmp_settings("rtmr", "options");

    if (_m_ui->port_number->text().isEmpty() == false)
        tmp_host_port = _m_ui->port_number->text().toInt();


    tmp_login_result = cl_connect(
    Session::instance()->getClientSessionPtr(),
    _m_ui->hostname->text().toStdString().c_str(),
    tmp_host_port, _m_ui->username->text().toStdString().c_str(),
    _m_ui->password->text().toStdString().c_str(),
    _m_ui->debug_mode_check_box->isChecked(),
    _m_ui->logfile_path->text().toStdString().c_str(),
    _m_ui->log_level->value(), PROTOCOL_VERSION,
    tmp_settings.value("server_recv_timeout", 30).toInt(),
    tmp_settings.value("server_send_timeout", 30).toInt());
    if (tmp_login_result == NOERR)
    {
        Record::init(Session::instance()->getClientSession());

        QList< QMap < QString, ClientModule*> >	tmp_modules_list = Session::instance()->externalsModules().values();
        QMap < QString, ClientModule*>	tmp_modules_map;

        foreach(tmp_modules_map, tmp_modules_list)
        {
            foreach(ClientModule *tmp_module, tmp_modules_map)
            {
                tmp_module->initModuleFromSession(Session::instance()->getClientSession());
            }
        }


        tmp_return_result = Session::instance()->loadUserInfos(_m_ui->username->text().toStdString().c_str());
        if ( tmp_return_result != NOERR )
        {
            QMessageBox::critical(this, tr("Erreur de chargement des données utilisateur"), Session::instance()->getErrorMessage(tmp_return_result));
        }
        else
        {
            writeSettings();
            QDialog::accept();
            return;
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Erreur d'authentification"), Session::instance()->getErrorMessage(tmp_login_result));
    }

    cl_disconnect(Session::instance()->getClientSessionPtr());
}


void Form_Logon::readSettings()
{
    QSettings settings("rtmr", "");
    QString tmp_username = settings.value("username", "").toString();
    QString tmp_host = settings.value("host", "localhost").toString();
    QString tmp_port = settings.value("port", QString::number(SERVER_PORT)).toString();
    QString tmp_logfile = settings.value("logfile", "").toString();
    int tmp_loglevel = settings.value("loglevel", 1).toInt();
    bool tmp_debug = settings.value("debug", false).toBool();


    _m_ui->username->setText(tmp_username);
    _m_ui->hostname->setText(tmp_host);
    _m_ui->port_number->setText(tmp_port);
    _m_ui->logfile_path->setText(tmp_logfile);
    _m_ui->log_level->setValue(tmp_loglevel);
    _m_ui->debug_mode_check_box->setChecked(tmp_debug);
}

void Form_Logon::writeSettings()
{
    QSettings settings("rtmr", "");

    settings.setValue("username", _m_ui->username->text());
    settings.setValue("host", _m_ui->hostname->text());
    settings.setValue("port", _m_ui->port_number->text());
    settings.setValue("logfile", _m_ui->logfile_path->text());
    settings.setValue("loglevel", _m_ui->log_level->value());
    settings.setValue("debug", _m_ui->debug_mode_check_box->isChecked());
}


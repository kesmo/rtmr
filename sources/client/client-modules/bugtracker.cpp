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

#include "bugtracker.h"
#include <QSslError>
#include <QStringList>


Bugtracker::Bugtracker()
{
}



Bugtracker::~Bugtracker()
{
}


void Bugtracker::setBaseUrl(QUrl in_base_url)
{
	_m_base_url = in_base_url;
}



void Bugtracker::setWebserviceUrl(QUrl in_url)
{
	_m_webservice_url = in_url;
}


void Bugtracker::setCredential(QString username, QString password)
{
	_m_username = username;
	_m_password = password;
}


void Bugtracker::getBug(QString in_bug_id)
{
    QStringList tmp_bugs_ids_list;

    tmp_bugs_ids_list << in_bug_id;

    getBugs(tmp_bugs_ids_list);
}


#ifndef QT_NO_OPENSSL

void Bugtracker::sslErrors(QNetworkReply *in_network_reply,QList<QSslError> /* in_ssl_errors_list */)
{
    in_network_reply->ignoreSslErrors();
}
#endif

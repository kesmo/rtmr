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

#ifndef CLIENT_H
#define CLIENT_H

#include <QApplication>
#include <QFile>
#include <QNetworkReply>
#include <QAuthenticator>
#include <QStringList>
#include <QDir>
#include <QFile>
#include <QProgressDialog>

class ClientLauncherUtils;

class LauncherApplication : public QApplication
{
    Q_OBJECT

public:
    LauncherApplication(int &argc, char **argv);

    bool launchClient();

    static int exec();
    static void exit(int retcode=0);

private:
    ClientLauncherUtils* _mClientLauncherUtils;

private slots:
    void bringClientAppToFront();

};


class ClientLauncherUtils : public QObject
{
    Q_OBJECT

public:
    ClientLauncherUtils(LauncherApplication* mainApp, bool checkNewVersion = false, bool downloadNewVersion = true, bool launchClientApp = true, QString url = QString());

    static void readAndSetApplicationProxySettings();

public slots:
    bool start();

    bool checkNewVersions();
    void slotReadyRead();
    void requestFinished();
    void slotProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *);

    void downloadNewVersion();
    void downloadReadyRead();
    void downloadFinished();
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);

protected:
    bool run();

    void loadLanguage(const QString& in_language);

private:
    LauncherApplication     *_m_main_app;
    QFile		    * _m_versions_file;
    QNetworkReply	    *_m_reply;
    QProgressDialog*        _m_download_progress_dialog;

    QFile		    *_m_new_version_archive_file;

    QString		    _m_current_version;
    QString		    _m_new_version;
    QString		    _m_new_version_notes;
    QStringList		    _m_new_version_files;
    QString		    _m_installer_file;

    bool		    _m_check_new_version;
    bool		    _m_download_new_version;
    bool		    _m_launch_client_app;
    QString		    _m_url;
};

#endif // CLIENT_H

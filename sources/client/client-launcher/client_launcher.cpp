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

#include "client_launcher.h"
#include "process_utils.h"
#include "gui/form_new_version_information.h"

#include <QSettings>
#include <QMessageBox>
#include <QProcess>
#include <QFileDialog>
#include <QFile>
#include <QTranslator>
#include <QTimer>
#include <QLocale>

LauncherApplication::LauncherApplication(int &argc, char **argv) : QApplication(argc, argv)
{
    bool tmp_check_new_version = false;
    bool tmp_download_new_version = false;
    bool tmp_launch_client_app = false;
    QString tmp_url;

    if (argc > 1)
    {
        for (int tmp_index = 1; tmp_index < argc; tmp_index++)
        {
            if (strcmp(argv[tmp_index], "-check") == 0)
            {
                tmp_check_new_version = true;
            }
            else if (strcmp(argv[tmp_index], "-download") == 0)
            {
                tmp_download_new_version = true;
            }
            else if (strcmp(argv[tmp_index], "-install") == 0)
            {
                if (tmp_index + 1 < argc)
                {

                }
            }
            else if (strcmp(argv[tmp_index], "-url") == 0)
            {
                if (tmp_index + 1 < argc)
                {
                    tmp_url = argv[++tmp_index];
                }
            }
            else if (strcmp(argv[tmp_index], "-launch"))
            {
                tmp_launch_client_app = true;
            }

        }

        _mClientLauncherUtils = new ClientLauncherUtils(this, tmp_check_new_version, tmp_download_new_version, tmp_launch_client_app, tmp_url);
    }
    else
    {
        _mClientLauncherUtils = new ClientLauncherUtils(this);
    }
}


int LauncherApplication::exec()
{
    if (static_cast<LauncherApplication*>(qApp)->_mClientLauncherUtils->start())
        return QApplication::exec();
    else
        exit();

    return 0;
}



void ClientLauncherUtils::loadLanguage(const QString& in_language)
{
    QLocale locale = QLocale(in_language);
    QLocale::setDefault(locale);

    QTranslator*	    tmp_qt_translator = new QTranslator(this);
    QTranslator*	    tmp_client_translator = new QTranslator(this);

    if (tmp_qt_translator->load(":/languages/qt_" + in_language))
        qApp->installTranslator(tmp_qt_translator);

    if (tmp_client_translator->load(QString(":/languages/client-launcher_") + in_language))
        qApp->installTranslator(tmp_client_translator);
    else if (in_language != "fr" && tmp_client_translator->load(QString(":/languages/client-launcher_en")))
        qApp->installTranslator(tmp_client_translator);
}


void LauncherApplication::exit(int retcode)
{
    static_cast<LauncherApplication*>(qApp)->launchClient();

    QApplication::exit(retcode);
}


ClientLauncherUtils::ClientLauncherUtils(LauncherApplication* mainApp, bool checkNewVersion, bool downloadNewVersion, bool launchClientApp, QString url)
{
    _m_main_app = mainApp;

    _m_check_new_version = checkNewVersion;
    _m_download_new_version = downloadNewVersion;
    _m_launch_client_app = launchClientApp;

    _m_url = url;
}

bool ClientLauncherUtils::start()
{
    return run();
}

bool ClientLauncherUtils::run()
{
    QSettings				tmp_options_settings("rtmr", "options");

    bool				tmp_check_new_versions = tmp_options_settings.value("versions_check_new", QVariant(false)).toBool();
    QString				tmp_versions_url = tmp_options_settings.value("versions_url", "http://rtmr.net").toString();

    QString                             tmp_prefered_language(tmp_options_settings.value("prefered_language", "").toString());

    // Chargement de la langue du système par défaut
    if (!tmp_prefered_language.isEmpty())
        loadLanguage(tmp_prefered_language);
    else
        loadLanguage(QLocale::system().name().section('_', 0, 0));

    // Vérifier s'il y a un nouvelle version de RTMR
    if (tmp_check_new_versions || _m_check_new_version)
    {
        ProcessUtils::readAndSetApplicationProxySettings();

        if (_m_url.isEmpty())
            _m_url = tmp_versions_url;

        return checkNewVersions();
    }

    return false;
}



bool ClientLauncherUtils::checkNewVersions()
{
    QNetworkAccessManager   *tmp_manager = new QNetworkAccessManager(this);
    QNetworkRequest	    tmp_request;
    QString		    tmp_url;

    if (!_m_url.isEmpty())
    {
        if (_m_url.endsWith('/'))
            tmp_url = _m_url + "versions.txt";
        else
            tmp_url = _m_url + "/versions.txt";

        _m_versions_file = new QFile(QDir::homePath()+"/versions.txt");
        if (!_m_versions_file->open(QIODevice::WriteOnly))
        {
            delete _m_versions_file;
            _m_versions_file = NULL;
        }
        else
        {
            tmp_request.setUrl(QUrl(tmp_url));

            _m_reply = tmp_manager->get(tmp_request);

            connect(_m_reply, SIGNAL(finished()), this, SLOT(requestFinished()));
            connect(_m_reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
            connect(tmp_manager, SIGNAL(proxyAuthenticationRequired ( const QNetworkProxy &, QAuthenticator *)),
                    this, SLOT(slotProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));

            return true;
        }
    }
    return false;
}


void ClientLauncherUtils::slotReadyRead()
{
    if (_m_versions_file && _m_reply->error() == QNetworkReply::NoError)
        _m_versions_file->write(_m_reply->readAll());
}

void ClientLauncherUtils::requestFinished()
{
    QSettings	    *tmp_versions_settings = NULL;
    QProcess	    tmp_ClientLauncher_process;

    bool		tmp_download_new_version = false;
    bool		tmp_has_new_version = false;

    bool		tmp_error = false;

    _m_versions_file->flush();
    _m_versions_file->close();

    if (_m_reply->error() == QNetworkReply::NoError)
    {
        tmp_versions_settings = new QSettings(_m_versions_file->fileName(), QSettings::IniFormat);
        if (tmp_versions_settings != NULL)
        {
            _m_new_version = tmp_versions_settings->value("rtmr", "").toString();
            _m_new_version_notes = tmp_versions_settings->value("notes", "").toString();
            QString tmp_files_str = tmp_versions_settings->value("files", "").toString();
            _m_new_version_files = tmp_files_str.split(' ', QString::SkipEmptyParts);
            _m_installer_file = tmp_versions_settings->value("installer", "").toString();
            if (!_m_new_version.isEmpty())
            {
                QStringList tmpArgs;
                QString tmp_app_path = QApplication::applicationDirPath() + "/";

                tmpArgs << "-version";
#if defined(__WINDOWS) ||  defined(WIN32)
                tmp_ClientLauncher_process.start("\""+tmp_app_path+"rtmrapp.exe\"", tmpArgs);
#else
#ifdef __APPLE__
                tmp_ClientLauncher_process.start("./rtmrapp", tmpArgs);
#else
#if defined __linux__

                tmp_ClientLauncher_process.start(QString(BINDIR) + "/rtmrapp", tmpArgs);
#endif
#endif
#endif
                tmp_ClientLauncher_process.waitForFinished();

                QByteArray tmp_array = tmp_ClientLauncher_process.readLine();
                if (!tmp_array.isEmpty())
                {
                    int tmp_cr = tmp_array.indexOf('\r');
                    if (tmp_cr < 0)
                        tmp_cr = tmp_array.indexOf('\n');

                    if (tmp_cr >= 0)
                        _m_current_version = QString(tmp_array.left(tmp_cr));
                    else
                        _m_current_version = QString(tmp_array) ;

                    QStringList tmp_new_version_digits = _m_new_version.split('.', QString::SkipEmptyParts);
                    QStringList tmp_current_version_digits = _m_current_version.split('.', QString::SkipEmptyParts);

                    for(int tmp_digit_index = 0; tmp_digit_index < tmp_new_version_digits.count(); tmp_digit_index++)
                    {
                        if (tmp_digit_index < tmp_current_version_digits.count())
                        {
                            QString tmp_new_version_digit = tmp_new_version_digits[tmp_digit_index];
                            QString tmp_current_version_digit = tmp_current_version_digits[tmp_digit_index];

                            int tmp_new_version_digit_int = tmp_new_version_digit.toInt();
                            int tmp_current_version_digit_int = tmp_current_version_digit.toInt();

                            if (tmp_new_version_digit_int > tmp_current_version_digit_int)
                            {
                                tmp_has_new_version = true;
                                break;
                            }
                            else if (tmp_new_version_digit_int < tmp_current_version_digit_int)
                            {
                                break;
                            }
                        }
                    }

                    if (tmp_has_new_version)
                    {
                        Form_New_Version_Information* tmp_form = new Form_New_Version_Information(_m_new_version, _m_new_version_notes);

                        if (tmp_form->exec() == QDialog::Accepted)
                        {
                            tmp_download_new_version = true;
                        }
                    }
                }

            }
            delete tmp_versions_settings;
        }
    }
    else
    {
        if (_m_check_new_version)
        {
            tmp_error = true;
            QMessageBox::critical(NULL, tr("Erreur"), _m_reply->errorString()+"<p><span style=\"color: red\"><b>"+tr("La requête a échouée.")+"</b></span></p>");
        }
    }

    _m_reply->deleteLater();
    _m_reply = NULL;

    delete _m_versions_file;
    _m_versions_file = NULL;

    if (tmp_download_new_version)
    {
        downloadNewVersion();
    }
    else
    {
        if (tmp_has_new_version)
        {
            _m_main_app->exit(0);
        }
        else
        {
            if (tmp_error)
            {
                _m_main_app->exit(2);
            }
            else
            {
                if (_m_check_new_version)
                    QMessageBox::information(NULL, tr("Nouvelle version"), tr("Vous disposez déjà de la dernière version."));
                _m_main_app->exit(1);
            }
        }
    }
}


void ClientLauncherUtils::slotProxyAuthenticationRequired(const QNetworkProxy & /* in_proxy */, QAuthenticator *in_authenticator)
{
    QSettings			tmp_options_settings("rtmr", "options");
    QString				tmp_proxy_login(tmp_options_settings.value("proxy_login", "").toString());
    QString				tmp_proxy_password(tmp_options_settings.value("proxy_password", "").toString());

    in_authenticator->setUser(tmp_proxy_login);
    in_authenticator->setPassword(tmp_proxy_password);
}



void ClientLauncherUtils::downloadNewVersion()
{
    QNetworkAccessManager *tmp_manager = new QNetworkAccessManager(this);
    QNetworkRequest tmp_request;
    QString tmp_url;
    QString tmp_file_name;

    if (_m_url.isEmpty())
    {
        QMessageBox::critical(NULL, tr("Paramètre manquant"), tr("L'URL de mise à jour des versions est nécessaire."));
        _m_main_app->exit();
    }
    else
    {
        if (_m_url.endsWith('/'))
            tmp_url = _m_url + "downloads/";
        else
            tmp_url = _m_url + "/downloads/";

#if defined(__WINDOWS) ||  defined(WIN32)
        if (!_m_installer_file.isEmpty())
            tmp_file_name = _m_installer_file;
        else
            tmp_file_name = "rtmr_" + _m_new_version.left(_m_new_version.lastIndexOf('.')) +".zip";

        tmp_url += "Windows/" + tmp_file_name;
#else
#ifdef __APPLE__
        //        tmp_file_name = "rtmr_" + _m_new_version.left(_m_new_version.lastIndexOf('.')) +".dmg";
        //	tmp_url += "MacOSX/" + tmp_file_name;
#else
#if defined __linux__ && defined DEBIAN
        tmp_file_name = "rtmr_" + _m_new_version.left(_m_new_version.lastIndexOf('.')) +"-1";
#ifdef __x86_64__
        tmp_file_name += "_amd64.deb";
#else
        tmp_file_name += "_i386.deb";
#endif //__x86_64__
        tmp_url += "Linux/debian/" + tmp_file_name;
#else

#endif // __linux__
#endif // __APPLE__
#endif //__WINDOWS

        if (tmp_file_name.isEmpty())
        {
            QMessageBox::information(NULL, tr("Information"), tr("La plateforme d'execution n'est pas prise en compte pour l'installation automatique.<br>Vous devrez télécharger la nouvelle version sur le site <a href=\"%1\">%1</a> et l'installer manuellement.").arg(tmp_url).arg(tmp_url));
            _m_main_app->exit();
        }
        else
        {
            QString tmp_download_path = QDir::homePath() + "/rtmr/downloads/";

            if (!QDir(tmp_download_path).exists())
            {
                if (!QDir(QDir::homePath()).mkpath("rtmr/downloads"))
                {
                    QMessageBox::critical(NULL, tr("Information"), tr("La création du dossier de téléchargement (%1) n'a pu être réalisée.").arg(tmp_download_path));
                    _m_main_app->exit();
                }
            }

            _m_new_version_archive_file = new QFile(tmp_download_path + tmp_file_name);
            if (!_m_new_version_archive_file->open(QIODevice::WriteOnly))
            {
                QMessageBox::critical(NULL, "HTTP", tr("Le fichier %1 n'a pu être enregistré (%2).").arg(tmp_file_name).arg(_m_versions_file->errorString()));
                delete _m_new_version_archive_file;
                _m_new_version_archive_file = NULL;
                _m_main_app->exit();
            }
            else
            {
                _m_download_progress_dialog = new QProgressDialog();
                _m_download_progress_dialog->setMinimum(0);
                _m_download_progress_dialog->setMaximum(100);
                _m_download_progress_dialog->setValue(0);
                _m_download_progress_dialog->setLabelText(tr("Téléchargement de l'archive %1").arg(tmp_file_name));
                _m_download_progress_dialog->show();

                tmp_request.setUrl(QUrl(tmp_url));

                _m_reply = tmp_manager->get(tmp_request);

                connect(_m_reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(downloadProgress(qint64,qint64)));
                connect(_m_reply, SIGNAL(finished()), this, SLOT(downloadFinished()));
                connect(_m_reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
                connect(tmp_manager, SIGNAL(proxyAuthenticationRequired ( const QNetworkProxy &, QAuthenticator *)), this,
                        SLOT(slotProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *)));
            }
        }
    }
}

void ClientLauncherUtils::downloadReadyRead()
{
    if (_m_new_version_archive_file && _m_reply->error() == QNetworkReply::NoError)
        _m_new_version_archive_file->write(_m_reply->readAll());
}


void ClientLauncherUtils::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (_m_download_progress_dialog != NULL)
    {
        if (bytesReceived >= 0 && bytesTotal > 0)
        {
            _m_download_progress_dialog->setValue(100 * bytesReceived / bytesTotal);
        }
    }
}

void ClientLauncherUtils::downloadFinished()
{
    bool tmp_cancel_install = false;

    _m_new_version_archive_file->flush();
    _m_new_version_archive_file->close();

    if (_m_reply->error())
    {
        QMessageBox::critical(NULL, "HTTP", "<b>" + tr("Erreur lors du téléchargement de la nouvelle version") + "</b> :<br>" + _m_reply->errorString());
    }
    else
    {
        QProcess tmp_gzip;

#if defined(__WINDOWS) ||  defined(WIN32)
        while (ProcessUtils::isRunning("rtmrapp.exe") && !tmp_cancel_install)
#else
#ifdef __APPLE__
        while (ProcessUtils::isRunning("rtmrapp") && !tmp_cancel_install)
#else
#if defined __linux__
        while (ProcessUtils::isRunning("rtmrapp") && !tmp_cancel_install)
#endif // __linux__
#endif // __APPLE__
#endif //__WINDOWS

        {
            tmp_cancel_install = QMessageBox::question(
                        NULL,
                        tr("Avertissement"),
                        tr("L'application est en cours d'exécution.<br>" \
                           "Vous devez fermer l'application avant l'installation de la nouvelle version, puis cliquer sur le bouton [Réessayer]"),
                        QMessageBox::Retry | QMessageBox::Cancel,
                        QMessageBox::Cancel) == QMessageBox::Cancel;
        }

        if (!tmp_cancel_install)
        {
#if defined(__WINDOWS) ||  defined(WIN32)

            // Fichier .zip
            QString tmp_app_path = QApplication::applicationDirPath() + "/";
            QString tmp_home_path = QDir::homePath() + "/";
            QString tmp_backup_path = "rtmr/backup/rtmr_"+_m_current_version;
            QString tmp_backup_fullpath = tmp_home_path + tmp_backup_path + "/";
            bool tmp_proceed_zip_extraction = false;
            // Backup fichiers courant
            if (!_m_new_version_files.isEmpty())
            {
                QDir tmp_backup_dir(tmp_home_path);
                if (tmp_backup_dir.exists(tmp_backup_path) || tmp_backup_dir.mkpath(tmp_backup_path))
                {
                    _m_download_progress_dialog->setLabelText(tr("Sauvegarde..."));
                    _m_download_progress_dialog->setValue(0);

                    foreach(QString tmp_filename, _m_new_version_files)
                    {
                        QFile tmp_file(tmp_app_path + tmp_filename);
                        if (tmp_file.exists())
                        {
                            tmp_proceed_zip_extraction = tmp_file.copy(tmp_backup_fullpath + tmp_filename);
                            if (!tmp_proceed_zip_extraction)
                            {
                                QMessageBox::critical(
                                            NULL,
                                            tr("Installation"),
                                            tr("Le fichier %1 n'a pu être sauvegardé vers %2 (%3).")
                                            .arg(tmp_file.fileName())
                                            .arg(tmp_backup_fullpath + tmp_filename)
                                            .arg(tmp_file.errorString()));
                                break;
                            }
                        }
                        else
                            tmp_proceed_zip_extraction = true;
                    }

                    if (tmp_proceed_zip_extraction)
                    {
                        tmp_gzip.start("\""+tmp_app_path+"7za.exe\" a \"" + tmp_home_path + "rtmr/backup/rtmr_"+_m_current_version + ".zip\" \"" + tmp_backup_fullpath+"\"");
                        while (!tmp_gzip.waitForFinished(500))
                        {
                            if (_m_download_progress_dialog->value() <= 100)
                                _m_download_progress_dialog->setValue(_m_download_progress_dialog->value()+5);
                        }

                        if (tmp_gzip.exitStatus() == QProcess::NormalExit)
                        {
                            tmp_proceed_zip_extraction = true;
                        }
                        else
                        {
                            tmp_proceed_zip_extraction = false;
                            QMessageBox::critical(NULL, tr("Installation"), tr("L'archivage du dossier %1 n'a pu être réalisé.").arg(tmp_backup_fullpath));
                        }
                    }

                    ProcessUtils::removeDir(tmp_backup_fullpath);
                }
            }
            else if (!_m_installer_file.isEmpty())
            {
                tmp_proceed_zip_extraction = false;

                QProcess::startDetached(_m_new_version_archive_file->fileName());
            }
            else
                tmp_proceed_zip_extraction = true;

            if (tmp_proceed_zip_extraction)
            {
                _m_download_progress_dialog->setLabelText(tr("Extraction..."));
                _m_download_progress_dialog->setValue(0);
                // Extraction nouvelle version
                tmp_gzip.start("\""+tmp_app_path+"7za.exe\" x \"" + _m_new_version_archive_file->fileName()+"\"");
                while (!tmp_gzip.waitForFinished(500))
                {
                    if (_m_download_progress_dialog->value() <= 100)
                        _m_download_progress_dialog->setValue(_m_download_progress_dialog->value()+5);
                }

                if (tmp_gzip.exitStatus() == QProcess::NormalExit)
                {
                    foreach(QString tmp_filename, _m_new_version_files)
                    {
                        QFile tmp_file(tmp_app_path + tmp_filename);
                        QFile tmp_new_file("rtmr/"+tmp_filename);
                        tmp_proceed_zip_extraction = tmp_new_file.exists();

                        if (tmp_proceed_zip_extraction)
                        {
                            if (tmp_file.exists())
                                tmp_proceed_zip_extraction = tmp_file.remove();

                            if (tmp_proceed_zip_extraction)
                            {
                                tmp_proceed_zip_extraction = tmp_new_file.copy(tmp_file.fileName());
                                if (!tmp_proceed_zip_extraction)
                                    QMessageBox::critical(NULL, tr("Installation"), tr("Le fichier %1 n'a pu être copié (%2).").arg(tmp_new_file.fileName()).arg(tmp_new_file.errorString()));
                            }
                            else
                                QMessageBox::critical(NULL, tr("Installation"), tr("Le fichier %1 n'a pu être supprimé (%2).").arg(tmp_file.fileName()).arg(tmp_file.errorString()));
                        }
                        else
                        {
                            QMessageBox::critical(NULL, tr("Installation"), tr("Le fichier %1 n'existe pas (%2).").arg(tmp_new_file.fileName()).arg(tmp_new_file.errorString()));
                        }

                        if (!tmp_proceed_zip_extraction)
                            break;
                    }

                    ProcessUtils::removeDir(QDir("rtmr"));
                }
                else
                {
                    QMessageBox::critical(NULL, tr("Installation"), tr("L'extraction de l'archive %1 n'a pu être réalisée.").arg(_m_new_version_archive_file->fileName()));
                }
            }
#else
#ifdef __APPLE__
            // Fichier .dmg
            tmp_gzip.start("hdiutil attach " + _m_new_version_archive_file->fileName());
            if (tmp_gzip.waitForFinished() && tmp_gzip.exitStatus() == QProcess::NormalExit)
            {

            }
#else
#if defined __linux__
            // Fichier .tar.gz
            tmp_gzip.start("gzip -d " + _m_new_version_archive_file->fileName());
            if (tmp_gzip.waitForFinished() && tmp_gzip.exitStatus() == QProcess::NormalExit)
            {

            }
#endif // __linux__
#endif // __APPLE__
#endif //__WINDOWS

        }
    }

    if (_m_download_progress_dialog != NULL)
        _m_download_progress_dialog->close();

    _m_reply->deleteLater();
    _m_reply = NULL;

    delete _m_new_version_archive_file;
    _m_new_version_archive_file = NULL;

    _m_main_app->exit();
}

bool LauncherApplication::launchClient()
{
    // Lancement de l'application cliente dans un processus independant
    QProcess	tmp_client_application_process;
    QString tmp_app_path = QApplication::applicationDirPath() + "/";

    bool tmp_rtmrapp_started = false;

#if defined(__WINDOWS) ||  defined(WIN32)
    if (!ProcessUtils::isRunning("rtmrapp.exe", true))
    {
        tmp_rtmrapp_started = tmp_client_application_process.startDetached("\""+tmp_app_path+"rtmrapp.exe\"");
        if (tmp_rtmrapp_started)
            bringClientAppToFront();
    }
#else
#ifdef __APPLE__
    if (!ProcessUtils::isRunning("rtmrapp", true))
    {
        tmp_rtmrapp_started = tmp_client_application_process.startDetached(tmp_app_path + "/rtmrapp");
        if (tmp_rtmrapp_started)
            bringClientAppToFront();
    }
#else
#if defined __linux__
    if (!ProcessUtils::isRunning("rtmrapp", true))
    {
        tmp_rtmrapp_started = tmp_client_application_process.startDetached(QString(BINDIR) + "/rtmrapp");
        if (tmp_rtmrapp_started)
            bringClientAppToFront();
    }
#endif // __linux__
#endif // __APPLE__
#endif //__WINDOWS

    return tmp_rtmrapp_started;
}


void LauncherApplication::bringClientAppToFront()
{
#if defined(__WINDOWS) ||  defined(WIN32)
    while (!ProcessUtils::isRunning("rtmrapp.exe", true));
#else
    while (!ProcessUtils::isRunning("rtmrapp", true));
#endif //__WINDOWS
}


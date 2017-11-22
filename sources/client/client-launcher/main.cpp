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

#include <QtCore/QCoreApplication>
#include <QSettings>
#include <QProcess>
#include <QTextCodec>
#include <QMessageBox>
#include <QMainWindow>

int main(int argc, char *argv[])
{
    QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

    LauncherApplication	tmp_launcher(argc, argv);
    QMainWindow mainWindow;
    tmp_launcher.setWindowIcon(QIcon(QPixmap(QString::fromUtf8(":/images/client.png"))));
    mainWindow.setGeometry(0, 0, 0, 0);
    mainWindow.show();

    return tmp_launcher.exec();
}

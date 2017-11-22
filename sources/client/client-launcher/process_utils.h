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

#ifndef PROCESS_UTILS_H
#define PROCESS_UTILS_H

#include <QString>
#include <QDir>
#include <QSettings>
#include <qtextstream.h>

#if defined __linux__
    #include <proc/readproc.h>
    #include <X11/Xlib.h>
    #include <X11/Xatom.h>
#endif // __linux__

class ProcessUtils
{
public:
    ProcessUtils();

    static void readAndSetApplicationProxySettings();
    static bool removeDir(QDir aDir);
    static bool isRunning(QString processName, bool bringToFront = false, long processId = -1);

#if defined __linux__
    static void searchWindowsWithPid(Display* dsp, Window wnd, unsigned long pid, Atom atomPid);
#endif // __linux__


};

#endif // PROCESS_UTILS_H

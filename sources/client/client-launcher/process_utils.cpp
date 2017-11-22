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

#include "process_utils.h"

#include <QNetworkProxy>

#if defined(__WINDOWS) ||  defined(WIN32)
    #include <windows.h>
    #include <psapi.h>

static HWND _m_process_window_handle = NULL;

#else
#ifdef __APPLE__
    #include <Carbon/Carbon.h>
#else
#if defined __linux__

#endif // __linux__
#endif // __APPLE__
#endif //__WINDOWS


ProcessUtils::ProcessUtils()
{
}


void ProcessUtils::readAndSetApplicationProxySettings()
{
    QSettings				tmp_options_settings("rtmr", "options");
    QNetworkProxy			tmp_proxy;
    QFlags<QNetworkProxy::Capability>	tmp_proxy_capabilities = 0;
    bool				tmp_proxy_use_system_config = tmp_options_settings.value("proxy_use_system_config", QVariant(false)).toBool();
    QString				tmp_proxy_address(tmp_options_settings.value("proxy_address", "").toString());
    QString				tmp_proxy_port(tmp_options_settings.value("proxy_port", "").toString());
    QString				tmp_proxy_login(tmp_options_settings.value("proxy_login", "").toString());
    QString				tmp_proxy_password(tmp_options_settings.value("proxy_password", "").toString());
    int					tmp_proxy_type = QNetworkProxy::NoProxy;

    if (tmp_proxy_use_system_config)
    {
	QNetworkProxyFactory::setUseSystemConfiguration(true);
    }
    else
    {
	QNetworkProxyFactory::setUseSystemConfiguration(false);

	tmp_proxy_type = tmp_options_settings.value("proxy_type", QVariant(QNetworkProxy::NoProxy)).toInt();
	tmp_proxy_capabilities = QFlags<QNetworkProxy::Capability>(tmp_options_settings.value("proxy_capabilities", QVariant(0)).toInt());

	if (tmp_proxy_address.isEmpty())
	{
	    tmp_proxy.setType(QNetworkProxy::NoProxy);
	}
	else
	{
	    tmp_proxy.setType((QNetworkProxy::ProxyType)tmp_proxy_type);
	    tmp_proxy.setCapabilities(tmp_proxy_capabilities);
	    tmp_proxy.setHostName(tmp_proxy_address);
	    tmp_proxy.setPort(tmp_proxy_port.toInt());
	    if (!tmp_proxy_login.isEmpty())
	    {
		tmp_proxy.setUser(tmp_proxy_login);
		tmp_proxy.setPassword(tmp_proxy_password);
	    }
	}

	QNetworkProxy::setApplicationProxy(tmp_proxy);
    }
}

bool ProcessUtils::removeDir(QDir aDir)
{
    bool has_err = false;
    if (aDir.exists())//QDir::NoDotAndDotDot
    {
	QFileInfoList entries = aDir.entryInfoList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
	int count = entries.size();
	for (int idx = 0; ((idx < count) && !has_err); idx++)
	{
	    QFileInfo entryInfo = entries[idx];
	    QString path = entryInfo.absoluteFilePath();
	    if (entryInfo.isDir())
	    {
		has_err = removeDir(QDir(path));
	    }
	    else
	    {
		QFile file(path);
		if (!file.remove())
		    has_err = true;
	    }
	}
	if (!aDir.rmdir(aDir.absolutePath()))
	    has_err = true;
    }
    return(has_err);
}


#if defined(__WINDOWS) ||  defined(WIN32)
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	DWORD dwProcessID;

	// Recuperer le numero de process attacher à la fenêtre
	GetWindowThreadProcessId(hwnd, &dwProcessID);
	if (dwProcessID == (*(DWORD*)lParam))
	{
	    // Si la fenêtre est "visible" : il s'agit de la fenêtre principale de l'application
	    if (IsWindowVisible(hwnd))
	    {
		// Restaurer la feneêtre si elle est minimisée, sinon la passer au premier plan
		if(IsIconic(hwnd))
		    ShowWindowAsync(hwnd, SW_RESTORE);
		else
		    SetForegroundWindow(hwnd);
	    }
	}

	return TRUE;
}

#endif

#ifdef __APPLE__

QString toQString(CFStringRef str)
{
    if (!str)
	return QString();

    CFIndex length = CFStringGetLength(str);
    if (length == 0)
	return QString();

    QString string(length, Qt::Uninitialized);
    CFStringGetCharacters(str, CFRangeMake(0, length), reinterpret_cast<UniChar *>
	(const_cast<QChar *>(string.unicode())));
    return string;
}

#endif


bool ProcessUtils::isRunning(QString processName, bool bringToFront, long processId)
{
#if defined(__WINDOWS) ||  defined(WIN32)
    Q_UNUSED(processId)

    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;

    if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
    {
	return false;
    }


    // Nombre de process
    cProcesses = cbNeeded / sizeof(DWORD);

    for ( i = 0; i < cProcesses; i++ )
    {
	if( aProcesses[i] != 0 )
	{
	    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

	    // Get a handle to the process.
	    HANDLE hProcess = OpenProcess( PROCESS_TERMINATE |
					   PROCESS_QUERY_INFORMATION |
					   PROCESS_VM_READ,
					   FALSE, aProcesses[i] );

	    // Get the process name.
	    if (NULL != hProcess )
	    {
		HMODULE hMod;
		DWORD cbNeeded;

		if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod),
		     &cbNeeded) )
		{
		    GetModuleBaseName( hProcess, hMod, szProcessName,
				       sizeof(szProcessName)/sizeof(TCHAR) );
		}
	    }

	    QString tmpProcessName = QString::fromWCharArray(szProcessName);

	    // Print the process name and identifier.
	    if (processName == tmpProcessName)
	    {
		// Rechercher la fenêtre associé au processus
		if (bringToFront)
		{
		    _m_process_window_handle = NULL;

		    EnumWindows(EnumWindowsProc, (LPARAM)&aProcesses[i]);

		    if (_m_process_window_handle != NULL)
		    {
//                        BringWindowToTop(_m_process_window_handle);
		    }
		}

		// Release the handle to the process.
		CloseHandle( hProcess );
		return true;
	    }
	    // Release the handle to the process.
	    CloseHandle( hProcess );

	}
    }

#else
#ifdef __APPLE__

    ProcessSerialNumber tmp_psn = {kNoProcess, kNoProcess};
    OSStatus status;

    /* Rechercher le processus processName */
    do
    {
	status = GetNextProcess(&tmp_psn);
	if (status == noErr)
	{
	    CFStringRef tmp_process_name;

	    if (CopyProcessName(&tmp_psn, &tmp_process_name) == noErr)
	    {
		QString pname = toQString(tmp_process_name) ;
		if (pname == processName || tmp_psn.highLongOfPSN == processId)
		{
		    if (bringToFront)
		    {
			SetFrontProcess(&tmp_psn);
		    }

		    return true;
		}
	    }
	}
    }
    while (status == noErr);

#else
#if defined __linux__

	/* Rechercher le processus processName */
	PROCTAB* proc = openproc(PROC_FILLSTAT | PROC_FILLSTATUS | PROC_FILLUSR);
	if (proc)
	{
	    proc_t proc_info;
	    memset(&proc_info, 0, sizeof(proc_info));
	    int tmp_process_index = 0;
	    while (readproc(proc, &proc_info) != NULL)
	    {
		if (proc_info.euid == (int)getuid())
		{
		    if (processName.compare(proc_info.cmd) == 0)
		    {
			if (bringToFront)
			{
			    /* Rechercher la fenetre du processus processName */
			    Display *dsp = XOpenDisplay(NULL);

			    if (dsp)
			    {
				Atom tmp_atom_pid = XInternAtom(dsp, "_NET_WM_PID", True);
				if(tmp_atom_pid != None)
				{
				    searchWindowsWithPid(dsp, XDefaultRootWindow(dsp), proc_info.tid, tmp_atom_pid);

				}
				XCloseDisplay ( dsp );
			    }
			}

			return true;
		    }
		}
		tmp_process_index++;
	    }

	    closeproc(proc);
	}

#endif // __linux__
#endif // __APPLE__
#endif //__WINDOWS

	return false;
}

#if defined __linux__

void ProcessUtils::searchWindowsWithPid(Display* dsp, Window wnd, unsigned long pid, Atom atomPid)
{
    Window    wRoot;
    Window    wParent;
    Window   *wChild;
    unsigned  nChildren;

    Atom           type;
    int            format;
    unsigned long  nItems;
    unsigned long  bytesAfter;
    unsigned char *propPID = 0;
    unsigned long  cpid;

    XWindowAttributes wattr;
    XEvent xev;

    if(0 != XQueryTree(dsp, wnd, &wRoot, &wParent, &wChild, &nChildren))
    {
	for(unsigned i = 0; i < nChildren; i++)
	{
	    // Get the PID for the current Window.
	    if(Success == XGetWindowProperty(dsp, *wChild, atomPid, 0, 1, False, XA_CARDINAL,
					     &type, &format, &nItems, &bytesAfter, &propPID))
	    {
		if(propPID != 0)
		{
		    cpid = *(unsigned long*)propPID;
		    if(pid == cpid)
		    {
			memset(&xev, 0, sizeof(xev));

			xev.type = ClientMessage;
			xev.xclient.display = dsp;
			xev.xclient.window = *wChild;
			xev.xclient.message_type = XInternAtom(dsp, "_NET_ACTIVE_WINDOW", False);
			xev.xclient.format = 32;
			xev.xclient.data.l[0] = 2L; /* 2 == Message from a window pager */
			xev.xclient.data.l[1] = CurrentTime;

			XGetWindowAttributes(dsp, *wChild, &wattr);
			XSendEvent(dsp, wattr.screen->root, False,
					 SubstructureNotifyMask | SubstructureRedirectMask,
					 &xev);
		    }

		    XFree(propPID);
		}

		searchWindowsWithPid(dsp, *wChild, pid, atomPid);
	    }
	    wChild++;
	}
    }
}

#endif // __linux__

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (C) 1993-1997  Microsoft Corporation.  All Rights Reserved.
//
//  MODULE:   simple.c
//
//  PURPOSE:  Implements the body of the service.
//            The default behavior is to open a
//            named pipe, \\.\pipe\simple, and read
//            from it.  It the modifies the data and
//            writes it back to the pipe.
//
//  FUNCTIONS:
//            ServiceStart(DWORD dwArgc, LPTSTR *lpszArgv);
//            ServiceStop( );
//
//  COMMENTS: The functions implemented in simple.c are
//            prototyped in service.h
//              
//
//  AUTHOR: Craig Link - Microsoft Developer Support
//


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#include "service.h"
#include "serveur.h"

// this event is signalled when the
// service should end
//
HANDLE  hServerStopEvent = NULL;

//
//  FUNCTION: ServiceStart
//
//  PURPOSE: Actual code of the service
//           that does the work.
//
//  PARAMETERS:
//    dwArgc   - number of command line arguments
//    lpszArgv - array of command line arguments
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//
VOID ServiceStart (DWORD dwArgc, char **lpszArgv)
{
    HANDLE                  hEvents[2] = {NULL, NULL};

    ///////////////////////////////////////////////////
    //
    // Service initialization
    //

    // report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NOERR,              // exit code
        3000))                 // wait hint
        goto cleanup;

    // create the event object. The control handler function signals
    // this event when it receives the "stop" control code.
    //
    hServerStopEvent = CreateEvent(
        NULL,    // no security attributes
        TRUE,    // manual reset event
        FALSE,   // not-signalled
        NULL);   // no name

    if ( hServerStopEvent == NULL)
        goto cleanup;

    hEvents[0] = hServerStopEvent;

    // report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NOERR,              // exit code
        3000))                 // wait hint
        goto cleanup;

    // create the event object object use in overlapped i/o
    //
    hEvents[1] = CreateEvent(
        NULL,    // no security attributes
        TRUE,    // manual reset event
        FALSE,   // not-signalled
        NULL);   // no name

    if ( hEvents[1] == NULL)
        goto cleanup;

    // report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NOERR,              // exit code
        3000))                 // wait hint
        goto cleanup;

    // report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_START_PENDING, // service state
        NOERR,              // exit code
        3000))                 // wait hint
        goto cleanup;


   // report the status to the service control manager.
    //
    if (!ReportStatusToSCMgr(
        SERVICE_RUNNING,       // service state
        NOERR,              // exit code
        0))                    // wait hint
        goto cleanup;

    //
    // End of initialization
    //
    ////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////
    //
    // Service is now running, perform work until shutdown
    //

	launch_srv(dwArgc, lpszArgv);

  cleanup:

    if (hServerStopEvent)
        CloseHandle(hServerStopEvent);

    if (hEvents[1]) // overlapped i/o event
        CloseHandle(hEvents[1]);

}


//
//  FUNCTION: ServiceStop
//
//  PURPOSE: Stops the service
//
//  PARAMETERS:
//    none
//
//  RETURN VALUE:
//    none
//
//  COMMENTS:
//    If a ServiceStop procedure is going to
//    take longer than 3 seconds to execute,
//    it should spawn a thread to execute the
//    stop code, and return.  Otherwise, the
//    ServiceControlManager will believe that
//    the service has stopped responding.
//    
VOID ServiceStop()
{
	stop_srv();

    if ( hServerStopEvent )
        SetEvent(hServerStopEvent);
}

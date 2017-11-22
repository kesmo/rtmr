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

#include "stdautomation.h"
#include "automatedactionvalidation.h"
#include "session.h"
#include <stdarg.h>

int CheckActiveWindowForAction(WId in_window_id, bool in_must_be_active_window, AutomatedActionValidation* in_automated_action, char* out_error_msg, void* in_ptr, ...)
{
    va_list tmp_args_list;
    void *ptr = in_ptr;

    va_start(tmp_args_list, in_ptr);
    while( ptr != NULL )
    {
        ptr = va_arg(tmp_args_list, void*);
    }
    va_end(tmp_args_list);

    if (in_window_id)
    {
#if (defined(_WINDOWS) || defined (WIN32))

        HWND tmp_active_window = GetActiveWindow();
        LOG_TRACE(Session::instance()->getClientSession(), "%s (module %s) : Active window id=%lu, current window id=%lu\n",
                  __FUNCTION__,
                  in_automated_action->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_NAME),
                  tmp_active_window,
                  in_window_id);
        if (in_must_be_active_window && tmp_active_window != in_window_id)
        {
            WINDOWINFO tmp_window_info;
            CHAR tmp_window_class_name[128];
            CHAR tmp_window_name[128];
            CHAR tmp_cwindow_class_name[128];
            CHAR tmp_cwindow_name[128];

            if (GetWindowInfo(tmp_active_window, &tmp_window_info) == TRUE){
                GetClassNameA(tmp_active_window, tmp_window_class_name, 128);
                GetWindowTextA(tmp_active_window, tmp_window_name, 128);

                if (GetWindowInfo(in_window_id, &tmp_window_info) == TRUE){
                    GetClassNameA(in_window_id, tmp_cwindow_class_name, 128);
                    GetWindowTextA(in_window_id, tmp_cwindow_name, 128);

                    sprintf(out_error_msg, "%s : target window %lu (class=%s name=%s) is not the active window %lu (class=%s name=%s).\n",
                            __FUNCTION__,
                            in_window_id,
                            tmp_cwindow_class_name,
                            tmp_cwindow_name,
                            tmp_active_window,
                            tmp_window_class_name,
                            tmp_window_name
                            );
                }else{
                    sprintf(out_error_msg, "%s : target window %lu (unknow class/name) is not the active window %lu (class=%s name=%s).\n",
                            __FUNCTION__,
                            in_window_id,
                            tmp_active_window,
                            tmp_window_class_name,
                            tmp_window_name
                            );
                }
            }else{
                sprintf(out_error_msg, "%s : target window %lu (unknow class/name) is not the active window %lu (unknow class/name).\n",
                        __FUNCTION__,
                        in_window_id,
                        tmp_active_window
                        );
            }
            LOG_ERROR(Session::instance()->getClientSession(), out_error_msg);

            return -1;
        }
#endif
    }

    return NOERR;
}



StdAutomation::StdAutomation()
{
}

QString StdAutomation::getModuleName()
{
    return "StdAutomation";
}


QString StdAutomation::getModuleVersion()
{
    return "1.0";
}


QString StdAutomation::getModuleInformationsUrl()
{
    return "http://rtmr.net";
}

QMap<QString, AutomationCallbackFunction*> StdAutomation::getFunctionsMap()
{
    QMap<QString, AutomationCallbackFunction*>	tmp_return_map;

    AutomationCallbackFunction* tmp_function = new AutomationCallbackFunction("CheckActiveWindow", "Check the active window", &CheckActiveWindowForAction, this);
    tmp_return_map[tmp_function->getName()] = tmp_function;

    return tmp_return_map;
}



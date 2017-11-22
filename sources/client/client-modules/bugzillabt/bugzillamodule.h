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

#ifndef BUGZILLAMODULE_H
#define BUGZILLAMODULE_H

#include "clientmodule.h"
#include "bugzillabt.h"

#ifdef  __cplusplus
extern "C" {
#endif

	/**
	\brief Bugzilla module implementation.
	*/
    class  BugzillaModule : public BugtrackerModule
    {
    public:
				/**
				Return the module instance.<br>
				This function is call by \a create_module_instance function (see ClientModule).
				*/
        static ClientModule* getModuleInstance()
        {
            return new BugzillaModule;
        }

				/**
				Destroy the module instance.<br>
				This function is call by \a destroy_module_instance function (see ClientModule).
				*/
        static void destroyModuleInstance(ClientModule *in_module_instance)
        {
            delete  in_module_instance;
        }

        virtual QString getModuleName();
	virtual QString getModuleVersion();
	virtual QString getModuleInformationsUrl();

	virtual Bugtracker* createBugtracker();

    private:
        BugzillaModule();

    };

    CLIENT_MODULE_EXPORT ClientModule* create_module_instance()
    {
        return BugzillaModule::getModuleInstance();
    }

    CLIENT_MODULE_EXPORT void destroy_module_instance(ClientModule *in_module_instance)
    {
        BugzillaModule::destroyModuleInstance(in_module_instance);
    }

#ifdef  __cplusplus
}
#endif

#endif // BUGZILLAMODULE_H

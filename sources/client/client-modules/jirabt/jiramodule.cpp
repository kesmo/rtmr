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

#include "jiramodule.h"

JiraModule::JiraModule()
{
}


QString JiraModule::getModuleName()
{
    return "Jira";
}


QString JiraModule::getModuleVersion()
{
    return "1.0";
}


QString JiraModule::getModuleInformationsUrl()
{
    return "http://rtmr.net";
}


Bugtracker* JiraModule::createBugtracker()
{
    return new Jirabt();
}

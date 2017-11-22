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

#include "projectparameter.h"
#include "utilities.h"

ProjectParameter::ProjectParameter() : Parameter(getEntityDef())
{
    _m_project = NULL;
}


ProjectParameter::ProjectParameter(Project *in_project) : Parameter(getEntityDef())
{
    _m_project = NULL;
    setProject(in_project);
}

ProjectParameter::~ProjectParameter()
{
    delete _m_project;
    destroy();
}

/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* ProjectParameter::getEntityDef() const
{
    return &projects_parameters_table_def;
}


void ProjectParameter::setProject(Project* in_project)
{
    if (in_project != NULL)
    {
        delete _m_project;
        _m_project = in_project->clone<Project>(false);
    }
}

int ProjectParameter::saveRecord()
{
	int	tmp_return = NOERR;

	if (is_empty_string(getIdentifier()))
	{
		if (_m_project != NULL)
            setValueForKey(_m_project->getIdentifier(), PROJECTS_PARAMETERS_TABLE_PROJECT_ID);

		tmp_return = Record::insertRecord();
	}
        else
            tmp_return = Record::saveRecord();

	return tmp_return;
}


const char* ProjectParameter::name()
{
    return getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME);
}


const char* ProjectParameter::value()
{
    return getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE);
}


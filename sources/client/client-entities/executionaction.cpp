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

#include "executionaction.h"
#include "netcommon.h"

ExecutionAction::ExecutionAction() : Record(getEntityDef())
{
        _m_action = NULL;
}


ExecutionAction::ExecutionAction(Action *in_action) : Record(getEntityDef())
{
  _m_action = NULL;
        setAction(in_action);
}

ExecutionAction::~ExecutionAction()
{
	delete _m_action;
	destroy();
}


/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* ExecutionAction::getEntityDef() const
{
    return &executions_actions_table_def;
}


void ExecutionAction::setAction(Action *in_action)
{
    if (in_action != NULL)
    {
        delete _m_action;
	_m_action = in_action->clone<Action>(false);
        setValueForKey(_m_action->getIdentifier(), EXECUTIONS_ACTIONS_TABLE_ACTION_ID);
    }
}


int ExecutionAction::saveRecord()
{
    int tmp_return = NOERR;

    if (is_empty_string(getIdentifier()))
    {
        if (_m_action != NULL)
            setValueForKey(_m_action->getIdentifier(), EXECUTIONS_ACTIONS_TABLE_ACTION_ID);

        tmp_return = Record::saveRecord();
    }
    else
        tmp_return = Record::saveRecord();

    return tmp_return;
}


QList<Bug*> ExecutionAction::loadBugs()
{
    Bug					**tmp_bugs = NULL;
    unsigned long			tmp_bugs_count = 0;
    unsigned long			tmp_index = 0;
    QList<Bug*>				tmp_bugs_list;

    sprintf(CLIENT_SESSION->m_last_query, "%s=%s", BUGS_TABLE_EXECUTION_ACTION_ID, getIdentifier());

    tmp_bugs = Record::loadRecords<Bug>(CLIENT_SESSION, &bugs_table_def, CLIENT_SESSION->m_last_query, BUGS_TABLE_CREATION_DATE, &tmp_bugs_count);
    if (tmp_bugs != NULL)
    {
	if (tmp_bugs_count > 0)
	{
	    for(tmp_index = 0; tmp_index < tmp_bugs_count; tmp_index++)
	    {
		tmp_bugs_list.append(tmp_bugs[tmp_index]);
	    }
	}

	Record::freeRecords<Bug>(tmp_bugs, 0);
    }

    return tmp_bugs_list;
}

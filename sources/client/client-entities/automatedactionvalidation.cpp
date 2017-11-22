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

#include "automatedactionvalidation.h"

AutomatedActionValidation::AutomatedActionValidation() : Entity<AutomatedActionValidation>()
{
    _m_automated_action = NULL;
}

/**
  Constructeur
**/
AutomatedActionValidation::AutomatedActionValidation(AutomatedAction *in_automated_action) : Entity<AutomatedActionValidation>()
{
  _m_automated_action = NULL;
    setAutomatedAction(in_automated_action);
}

AutomatedActionValidation::~AutomatedActionValidation()
{
    delete _m_automated_action;
    destroy();
}


int AutomatedActionValidation::saveRecord()
{
    int tmp_return = NOERR;

    if (is_empty_string(getIdentifier()))
    {
        if (_m_automated_action != NULL)
            setValueForKey(_m_automated_action->getIdentifier(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_AUTOMATED_ACTION_ID);

        tmp_return = Record::saveRecord();
    }
    else
        tmp_return = Record::saveRecord();

    return tmp_return;
}


void AutomatedActionValidation::setAutomatedAction(AutomatedAction *in_automated_action)
{
    if (in_automated_action != NULL)
    {
        delete _m_automated_action;
        _m_automated_action = in_automated_action->clone<AutomatedAction>(false);
        setValueForKey(_m_automated_action->getIdentifier(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_AUTOMATED_ACTION_ID);
    }
}


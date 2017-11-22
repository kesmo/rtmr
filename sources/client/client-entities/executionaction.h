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

#ifndef EXECUTIONACTION_H
#define EXECUTIONACTION_H

#include "record.h"
#include "action.h"
#include "executiontest.h"
#include "bug.h"

#include <QList>

class ExecutionAction : public Record
{
private:
    Action			*_m_action;

public:
    ExecutionAction();
    ExecutionAction(Action *in_action);
    ~ExecutionAction();

    const entity_def* getEntityDef() const;

    void setAction(Action *in_action);
    Action* action(){return _m_action;}

    int saveRecord();

    QList<Bug*> loadBugs();

};

class ActionResult : public Record
{
public:
    ActionResult() : Record(getEntityDef()){}
    ~ActionResult(){destroy();}

    const entity_def* getEntityDef() const {return &actions_results_table_def;}

};

#endif // EXECUTIONACTION_H

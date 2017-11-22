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

#ifndef ACTION_H
#define ACTION_H

#include "record.h"
#include "testcontent.h"
#include "testhierarchy.h"

class Action : public Record
{
private:
    TestContent  *_m_test_content;
    QList<Action*>  _m_associated_test_actions;

    QList<Action*> loadAssociatedTestActionsForVersion(const char *in_project_version);
    bool appendAssociatedTestActionsToAction(QList<Action*> & in_actions_list, const char *in_original_test_id, const char *in_project_version);

public:
    Action();
    Action(TestContent *in_test_content);
    ~Action();

    const entity_def* getEntityDef() const;

    Action* copy(TestContent *in_test_content);
    int saveRecord();

    void setTestContent(TestContent *in_test_content);

    bool loadAssociatedActionsForVersion(const char *in_project_version, const char *in_original_test_content_id = NULL);
    bool setAssociatedTest(TestHierarchy *in_test);
    QList<Action*> associatedTestActions();

};

#endif // ACTION_H

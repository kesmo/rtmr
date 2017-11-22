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

#ifndef AUTOMATEDACTION_H
#define AUTOMATEDACTION_H

#include "record.h"
#include "testcontent.h"
#include "testhierarchy.h"
#include "parent.h"

class AutomatedActionValidation;

class AutomatedAction :
    public Entity<AutomatedAction>,
    public Parent<AutomatedActionValidation>
{
    private:
        TestContent  *_m_test_content;

    public:

        static const char      FieldSeparator = ':';
        static const char      WindowSeparator = '/';

        AutomatedAction();
        AutomatedAction(TestContent *in_test_content);
        ~AutomatedAction();

        static const entity_def* getEntityDefinition() {return &automated_actions_table_def;}

        AutomatedAction* copy(TestContent *in_test_content);
        int saveRecord();

        void setTestContent(TestContent *in_test_content);
        TestContent* getTestContent() const {return _m_test_content;}

        QList<AutomatedActionValidation*> loadChilds();

};

#endif // AUTOMATEDACTION_H

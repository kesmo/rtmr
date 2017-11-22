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

#ifndef AUTOMATEDACTIONVALIDATION_H
#define AUTOMATEDACTIONVALIDATION_H

#include "automatedaction.h"

class AutomatedActionValidation : public Entity<AutomatedActionValidation>
{
    private:
        AutomatedAction  *_m_automated_action;

    public:
        AutomatedActionValidation();
        AutomatedActionValidation(AutomatedAction *in_automated_action);
        ~AutomatedActionValidation();

        static const entity_def* getEntityDefinition() {return &automated_actions_validations_table_def;}

        int saveRecord();

        void setAutomatedAction(AutomatedAction *in_automated_action);
        AutomatedAction* getAutomatedAction() const {return _m_automated_action;}
};


#endif // AUTOMATEDACTIONVALIDATION_H

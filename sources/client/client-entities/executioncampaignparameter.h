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

#ifndef EXECUTIONCAMPAIGNPARAMETER_H
#define EXECUTIONCAMPAIGNPARAMETER_H

#include "parameter.h"

class ExecutionCampaignParameter : public Parameter
{
public:
    ExecutionCampaignParameter();

    ~ExecutionCampaignParameter();

    const entity_def* getEntityDef() const;

    const char* name();
    const char* value();

};

#endif // EXECUTIONCAMPAIGNPARAMETER_H

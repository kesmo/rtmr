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

/** \file entities.h
* This file contains the definition for all entities.<br>
* Each entity corresponds to a database table or a database view, with their own columns definitions.
*/
#ifndef ENTITIES_H_
#define ENTITIES_H_

#include "entities-def.h"
#include "utilities.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
  Retrieve the structure definition of an enity
  \return NOERR if the entity identifier is known.
  */
    DLLEXPORT int DLLCALL get_table_def(int, entity_def **);

    int get_ldap_entry_def(int , char ***);

    char* value_for_key(entity_def *in_entity_def, char** in_columns_values, const char* in_key);
    int index_for_key(entity_def *in_entity_def, const char* in_key);

#ifdef __cplusplus
}
#endif

#endif /*ENTITIES_H_*/

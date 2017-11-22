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

#include "genericrecord.h"

GenericRecord::GenericRecord(const entity_def *in_entity_def) : Record(in_entity_def), _m_entity_def(in_entity_def)
{
}


GenericRecord::GenericRecord(const entity_def* in_entity_def, char **in_columns_record) : Record(in_columns_record), _m_entity_def(in_entity_def)
{
}


GenericRecord::~GenericRecord()
{
	destroy();
}


/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* GenericRecord::getEntityDef() const
{
    return _m_entity_def;
}


GenericRecord** GenericRecord::loadRecords(net_session *in_session, const entity_def *in_entity_def, char* in_where_clause, char* in_order_by_clause, unsigned long *in_out_records_count, net_callback_fct *in_callback)
{
    char***         	tmp_rows = NULL;
    GenericRecord**     tmp_records = NULL;

    unsigned long   tmp_records_count = 0;

    tmp_rows = cl_load_records(in_session, in_entity_def->m_entity_signature_id , in_where_clause, in_order_by_clause, &tmp_records_count, in_callback);
    if (tmp_rows != NULL)
    {
        tmp_records = new GenericRecord*[tmp_records_count];

        for (unsigned long tmp_index = 0; tmp_index < tmp_records_count; tmp_index++)
        {
            tmp_records[tmp_index] = new GenericRecord(in_entity_def,tmp_rows[tmp_index]);
        }

        cl_free_rows_columns_array(&tmp_rows, 0, 0);
    }

    if (in_out_records_count != NULL)
        *in_out_records_count = tmp_records_count;

    return tmp_records;
}


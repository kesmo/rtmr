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

#ifndef GENERICRECORD_H_
#define GENERICRECORD_H_

#include "record.h"

class GenericRecord: public Record {
private:
        const entity_def	*_m_entity_def;

	GenericRecord(const entity_def* in_entity_def, char **in_columns_record);

public:
	GenericRecord(const entity_def *in_entity_def);
	virtual ~GenericRecord();

    const entity_def* getEntityDef() const;

    static GenericRecord** loadRecords(net_session *in_session, const entity_def *in_entity_def, char* in_where_clause, char* in_order_by_clause, unsigned long *in_out_records_count = NULL, net_callback_fct *in_callback = NULL);

};

#endif /* GENERICRECORD_H_ */

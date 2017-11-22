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

#include "hierarchy.h"
#include <QList>

Hierarchy::Hierarchy(const entity_def* in_entity_def) : Record(in_entity_def)
{
}


Hierarchy::~Hierarchy()
{
}

bool Hierarchy::canInsert(int position, int in_child_type)
{
    Hierarchy	*tmp_previous_item = NULL;
    Hierarchy	*tmp_next_item = NULL;
    int			tmp_lock_previous_record_status = RECORD_STATUS_MODIFIABLE;
    int			tmp_lock_next_record_status = RECORD_STATUS_MODIFIABLE;
    int			tmp_lock_parent_record_status = RECORD_STATUS_MODIFIABLE;

    const char  *tmp_previous_parent_record_id = NULL;

    if (position < 0 || position > childCount(in_child_type))
        return false;

    if (isWritable(in_child_type) == false)
        return false;

    if (position > 0)
    {
        /* Dernier element de la liste */
        if (position == childCount(in_child_type))
        {
            tmp_previous_item = child(position - 1, in_child_type);
            if (tmp_previous_item != NULL && is_empty_string(tmp_previous_item->getIdentifier()) == false)
            {
                char			tmp_statement_str[256];
                char			***tmp_records = NULL;
                unsigned long	tmp_rows_count = 0;
                unsigned long	tmp_columns_count = 0;
                const char      *tmp_parent_key = tmp_previous_item->columnNameForParentItem();

                tmp_lock_previous_record_status = tmp_previous_item->lockRecordStatus();
                if (tmp_previous_item->lockRecord(true) != NOERR)
                    return false;

                /* Verifier qu'il n'existe pas un nouvel element a la suite */
                if (is_empty_string(tmp_parent_key) == FALSE)
                {
                    tmp_previous_parent_record_id = tmp_previous_item->getValueForKey(tmp_parent_key);
                    sprintf(tmp_statement_str, "SELECT %s FROM %s WHERE %s=%s AND %s=%s;"
                            , tmp_previous_item->getEntityDef()->m_entity_columns_names[0]
                            , tmp_previous_item->getEntityDef()->m_entity_name
                            , tmp_previous_item->columnNameForPreviousItem()
                            , tmp_previous_item->getIdentifier()
                            , tmp_parent_key
                            , is_empty_string(tmp_previous_parent_record_id) ? "NULL" : tmp_previous_parent_record_id);
                }
                else
                {
                    sprintf(tmp_statement_str, "SELECT %s FROM %s WHERE %s=%s;"
                            , tmp_previous_item->getEntityDef()->m_entity_columns_names[0]
                            , tmp_previous_item->getEntityDef()->m_entity_name
                            , tmp_previous_item->columnNameForPreviousItem()
                            , tmp_previous_item->getIdentifier());
                }

                tmp_records = cl_run_sql(CLIENT_SESSION, tmp_statement_str, &tmp_rows_count, &tmp_columns_count);
                if (tmp_records != NULL)
                {
                    cl_free_rows_columns_array(&tmp_records, tmp_rows_count, tmp_columns_count);
                    if (tmp_rows_count > 0)
                    {
                        if (tmp_lock_previous_record_status != RECORD_STATUS_OWN_LOCK)
                            tmp_previous_item->unlockRecord();

                        return false;
                    }
                }
            }
        }
    }

    /* Traitement de l'element suivant */
    if (position < childCount(in_child_type))
    {
        tmp_next_item = child(position, in_child_type);
        if (tmp_next_item != NULL)
        {
            tmp_lock_next_record_status = tmp_next_item->lockRecordStatus();
            if (tmp_next_item->lockRecord(true) != NOERR)
            {
                if (tmp_previous_item != NULL && tmp_lock_previous_record_status != RECORD_STATUS_OWN_LOCK)
                    tmp_previous_item->unlockRecord();

                return false;
            }
        }
    }

    if (childCount(in_child_type) == 0)
    {
        tmp_lock_parent_record_status = lockRecordStatus();
        if (lockRecord(true) != NOERR)
        {
            if (tmp_previous_item != NULL && tmp_lock_previous_record_status != RECORD_STATUS_OWN_LOCK)
                tmp_previous_item->unlockRecord();

            if (tmp_next_item != NULL && tmp_lock_next_record_status != RECORD_STATUS_OWN_LOCK)
                tmp_next_item->unlockRecord();

            return false;
        }

        if (dbChildCount(in_child_type) > 0)
        {
            if (tmp_previous_item != NULL && tmp_lock_previous_record_status != RECORD_STATUS_OWN_LOCK)
                tmp_previous_item->unlockRecord();

            if (tmp_next_item != NULL && tmp_lock_next_record_status != RECORD_STATUS_OWN_LOCK)
                tmp_next_item->unlockRecord();

            if (tmp_lock_parent_record_status != RECORD_STATUS_OWN_LOCK)
                unlockRecord();
        }
    }

    return true;
}

bool Hierarchy::canRemove(int position, int in_child_type)
{
    Hierarchy 	*tmp_item = NULL;
    Hierarchy	*tmp_previous_item = NULL;
    Hierarchy	*tmp_next_item = NULL;
    int			tmp_lock_record_status = RECORD_STATUS_MODIFIABLE;
    int			tmp_lock_previous_record_status = RECORD_STATUS_MODIFIABLE;

    if (position < 0 || position >= childCount(in_child_type))
        return false;

    if (isWritable(in_child_type) == false)
        return false;

    tmp_item = child(position, in_child_type);
    if (tmp_item != NULL)
    {
        tmp_lock_record_status = tmp_item->lockRecordStatus();
        if (tmp_item->lockRecord(true) != NOERR)
            return false;
    }


    /* Traitement du test precedent
    if (position > 0)
    {
        tmp_previous_item = child(position - 1, in_child_type);
        if (tmp_previous_item != NULL)
        {
            if (tmp_previous_item->lockRecord(true) != NOERR)
            {
                if (tmp_item != NULL && tmp_lock_record_status != RECORD_STATUS_OWN_LOCK)
                    tmp_item->unlockRecord();

                return false;
            }
        }
    }
    */

    /* Traitement du test suivant */
    if (position + 1 < childCount(in_child_type))
    {
        tmp_next_item = child(position + 1, in_child_type);
        if (tmp_next_item != NULL)
        {
            if (tmp_next_item->lockRecord(true) != NOERR)
            {
                if (tmp_item != NULL && tmp_lock_record_status != RECORD_STATUS_OWN_LOCK)
                    tmp_item->unlockRecord();

                if (tmp_previous_item != NULL && tmp_lock_previous_record_status != RECORD_STATUS_OWN_LOCK)
                    tmp_previous_item->unlockRecord();

                return false;
            }
        }
    }

    return true;
}


bool Hierarchy::isChildOf(Hierarchy *in_item)
{
    Hierarchy	*tmp_parent = parent();

    if (tmp_parent == NULL)
        return false;

    if (tmp_parent == in_item)
        return true;

    return tmp_parent->isChildOf(in_item);
}


bool Hierarchy::isParentOf(Hierarchy *in_item)
{
    Hierarchy	*tmp_child = NULL;
    int			tmp_index = 0;

    for(tmp_index = 0; tmp_index < childCount(); tmp_index++)
    {
        tmp_child = child(tmp_index);

        if (in_item == tmp_child)
            return true;

        if (tmp_child->isParentOf(in_item))
            return true;
    }

    return false;
}


int Hierarchy::dbChildCount(int in_child_type)
{
    return childCount(in_child_type);
}


Hierarchy* Hierarchy::findItemWithValueForKey(const char *in_value, const char *in_key, int in_child_type, bool in_recursive)
{
    Hierarchy	*tmp_child = NULL;
    int		tmp_index = 0;

    for(tmp_index = 0; tmp_index < childCount(in_child_type); tmp_index++)
    {
        tmp_child = child(tmp_index, in_child_type);

        if (compare_values(tmp_child->getValueForKey(in_key), in_value) == 0)
            return tmp_child;

        if (in_recursive)
        {
            tmp_child = tmp_child->findItemWithValueForKey(in_value, in_key, in_child_type, in_recursive);
            if (tmp_child != NULL)
                return tmp_child;
        }
    }

    return NULL;
}


int Hierarchy::indexForItemWithValueForKey(const char *in_value, const char *in_key, int in_child_type)
{
    Hierarchy	*tmp_child = NULL;
    int		tmp_index = 0;

    for(tmp_index = 0; tmp_index < childCount(in_child_type); tmp_index++)
    {
        tmp_child = child(tmp_index, in_child_type);

        if (compare_values(tmp_child->getValueForKey(in_key), in_value) == 0)
            return tmp_index;
    }

    return -1;
}


int Hierarchy::indexForItemWithValueForKeyAtPosition(const char *in_value, const char *in_key, int position,  int in_child_type)
{
    Hierarchy	*tmp_child = NULL;
    int		tmp_index = 0;
    int		tmp_count = 0;

    for(tmp_index = 0; tmp_index < childCount(in_child_type); tmp_index++)
    {
        tmp_child = child(tmp_index, in_child_type);

        if (compare_values(tmp_child->getValueForKey(in_key), in_value) == 0)
        {
            if (tmp_count == position)
                return tmp_index;

            tmp_count++;
        }
    }

    return -1;
}




bool Hierarchy::canMove(int /* in_child_type */)
{
    return true;
}


bool Hierarchy::mayHaveCyclicRedundancy(Hierarchy * /* in_dest_item */, bool /* in_check_link */)
{
    return false;
}



Hierarchy* Hierarchy::link()
{
    return NULL;
}


QList<Hierarchy*>* Hierarchy::orginalsRecordsForGenericRecord(GenericRecord * /* in_item */, int /* in_child_type */)
{
    return NULL;
}

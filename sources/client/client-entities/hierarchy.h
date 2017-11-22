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

#ifndef HIERARCHY_H
#define HIERARCHY_H

#include <QVariant>
#include <QList>
#include "record.h"
#include "genericrecord.h"
#include "utilities.h"

class Hierarchy : public Record
{
public:
    Hierarchy(const entity_def* in_entity_def);
    virtual ~Hierarchy() = 0;

    virtual const entity_def* getEntityDef() const = 0;

    virtual bool insertChildren(int position, int count, Hierarchy *in_item = NULL, int in_child_type = 0) = 0;
    virtual bool insertCopyOfChildren(int position, int count, GenericRecord *in_item, int in_child_type = 0) = 0;
    virtual bool removeChildren(int position, int count, bool in_move_indic = true, int in_child_type = 0) = 0;

    virtual bool isWritable(int in_child_type = 0) = 0;
    virtual bool canInsert(int position, int in_child_type = 0);
    virtual bool canRemove(int position, int in_child_type = 0);
    virtual bool canMove(int in_child_type = 0);

    virtual QVariant data(int column, int role = 0) = 0;

    virtual Hierarchy* copy() = 0;
    virtual Hierarchy* cloneForMove(bool in_recursivly = true) = 0;
    virtual bool mayHaveCyclicRedundancy(Hierarchy *in_dest_item, bool in_check_link);
    virtual Hierarchy* link();

    virtual Hierarchy* parent() = 0;

    virtual int row() const = 0;
    virtual int childCount(int in_child_type = 0) = 0;
    virtual Hierarchy* child(int number, int in_child_type = 0) = 0;

    virtual const char *columnNameForPreviousItem(){return NULL;}
    virtual const char *columnNameForParentItem(){return NULL;}

    virtual int dbChildCount(int in_child_type);

    template < class T >
    static int saveHierarchicalList(const QList < T * >& in_hierarchical_list, const char *in_previous_key)
    {
        int     tmp_result = NOERR;
        T       *tmp_previous_item = NULL;
        T       *tmp_item = NULL;

        for (int tmp_index = 0; tmp_index < in_hierarchical_list.count(); tmp_index++)
        {
            tmp_item = in_hierarchical_list[tmp_index];
            if (tmp_item != NULL)
            {
                if (is_empty_string(in_previous_key) == FALSE)
                {
                    if (tmp_previous_item != NULL)
                        tmp_item->setValueForKey(tmp_previous_item->getIdentifier(), in_previous_key);
                    else
                        tmp_item->setValueForKey(NULL, in_previous_key);
                }

		tmp_result = tmp_item->saveRecord();
                if (tmp_result != NOERR)
                    break;

                tmp_previous_item = tmp_item;
            }
        }

        return tmp_result;
    }

    static QList<Hierarchy*> parentRecordsFromRecordsList(const QList<Hierarchy*>& in_records_list)
    {
        QList<Hierarchy*>   tmp_records_list;
        bool                tmp_indic = false;
        QList<Hierarchy*>   tmp_remove_records_list;

        foreach (Hierarchy* tmp_record, in_records_list)
        {
            tmp_indic = false;

            foreach(Hierarchy* tmp_current_record, tmp_records_list)
            {
                if (tmp_record->isChildOf(tmp_current_record))
                {
                    tmp_indic = true;
                    break;
                }

                if(tmp_record->isParentOf(tmp_current_record))
                {
                    tmp_remove_records_list.append(tmp_current_record);
                }
            }

            if (!tmp_indic)
            {
                tmp_records_list.append(tmp_record);
            }
        }

        foreach(Hierarchy* tmp_record, tmp_records_list)
        {
            if (tmp_remove_records_list.indexOf(tmp_record) >= 0)
            {
                tmp_records_list.removeAll(tmp_record);
            }
        }

        return tmp_records_list;
    }

    bool isChildOf(Hierarchy *in_item);
    bool isParentOf(Hierarchy *in_item);

    virtual Hierarchy* findItemWithId(const char* in_item_id, int in_child_type = 0, bool in_recursive = true) = 0;

    virtual Hierarchy* findItemWithValueForKey(const char *in_value, const char *in_key, int in_child_type = 0, bool in_recursive = true);
    virtual int indexForItemWithValueForKey(const char *in_value, const char *in_key, int in_child_type);
    virtual int indexForItemWithValueForKeyAtPosition(const char *in_value, const char *in_key, int position,  int in_child_type);

    virtual QList<Hierarchy*>* orginalsRecordsForGenericRecord(GenericRecord *in_item = NULL, int in_child_type = 0);

};

template <class T> inline QList<T*> castHierarchyList(const QList<Hierarchy*>& in_records_list)
{
    QList<T*>   tmp_list;
    T*          tmp_cast_record = NULL;

    foreach (Hierarchy *tmp_record, in_records_list)
    {
        tmp_cast_record = dynamic_cast<T*>(tmp_record);
        if (tmp_cast_record != NULL)
            tmp_list << tmp_cast_record;
    }

    return tmp_list;
}


#endif // HIERARCHY_H

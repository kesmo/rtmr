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

#include "records_tree_model.h"
#include "utilities.h"
#include "session.h"
#include "genericrecord.h"
#include <QtGui>
#include <stdlib.h>

/**
  Constructeur
**/
RecordsTreeModel::RecordsTreeModel()
{
    _m_root_item = NULL;
    _m_child_type = 0;
    _m_drop_actions = Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
    _m_mime_type = DEFAULT_MIME_TYPE;
    _m_columns_count = 1;
}

/**
  Constructeur
**/
RecordsTreeModel::RecordsTreeModel(Hierarchy* in_root_item, int in_child_type)
{
    _m_root_item = in_root_item;
    _m_child_type = in_child_type;
    _m_drop_actions = Qt::CopyAction | Qt::MoveAction | Qt::LinkAction;
    _m_mime_type = DEFAULT_MIME_TYPE;
    _m_columns_count = 1;
}


/**
  Destructeur
**/
RecordsTreeModel::~RecordsTreeModel()
{
}


void RecordsTreeModel::setRootItem(Hierarchy* in_root_item, int in_child_type)
{
    _m_root_item = in_root_item;
    _m_child_type = in_child_type;
}


void RecordsTreeModel::setColumnsCount(int count)
{
    _m_columns_count = count;
}


/**
  Renvoie le nombre de colonnes
**/
int RecordsTreeModel::columnCount(const QModelIndex & /* parent */) const
{
    return _m_columns_count;
}


/**
  Renvoie les donnees en fonction de l'index passe en parametre
**/
QVariant RecordsTreeModel::data(const QModelIndex &index, int role) const
{
    Hierarchy *tmp_item = NULL;

    if (!index.isValid())
        return QVariant();

    tmp_item = getItem(index);
    if (tmp_item != NULL)
        return tmp_item->data(index.column(), role);

    return QVariant();
}


/**
  Renvoie les proprietes du item se trouvant a l'index passe en parametre
**/
Qt::ItemFlags RecordsTreeModel::flags(const QModelIndex &index) const
{
    Hierarchy *tmp_item = NULL;
    Qt::ItemFlags defaultFlags;

    defaultFlags = QAbstractItemModel::flags(index);

    tmp_item = getItem(index);
    if (tmp_item != NULL)
    {
        if (tmp_item->isWritable(_m_child_type))
            return Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;
        else if (tmp_item->canMove(_m_child_type))
            return Qt::ItemIsDragEnabled | defaultFlags;
    }

    return defaultFlags;
}


/**
    Renvoie le item se trouvant a l'index passe en parametre
**/
Hierarchy* RecordsTreeModel::getItem(const QModelIndex &index) const
{
    if (index.isValid())
    {
        Hierarchy *item = static_cast<Hierarchy*>(index.internalPointer());
        if (item != NULL)
            return item;
    }

    return _m_root_item;
}


QModelIndex RecordsTreeModel::firstModelIndexForItemWithValueForKey(const char *in_value, const char *in_key, const QModelIndex &in_parent)
{
    QModelIndex     tmp_model_index, tmp_current_model_index;
    Hierarchy       *tmp_item = NULL;
    Hierarchy       *tmp_parent_item = NULL;

    if (in_parent.isValid() == false)
        tmp_parent_item = _m_root_item;
    else
        tmp_parent_item = getItem(in_parent);

    for (int tmp_index = 0; tmp_index < tmp_parent_item->childCount(_m_child_type); tmp_index++)
    {
        tmp_item = tmp_parent_item->child(tmp_index, _m_child_type);
        if (tmp_item != NULL)
        {
            tmp_current_model_index = index(tmp_index, 0, in_parent);
            if (tmp_current_model_index.isValid())
            {
                if (compare_values(tmp_item->getValueForKey(in_key), in_value) == 0)
                {
                    return tmp_current_model_index;
                }
                else
                {
                    tmp_model_index = firstModelIndexForItemWithValueForKey(in_value, in_key, tmp_current_model_index);
                    if (tmp_model_index.isValid())
                        return tmp_model_index;
                }
            }
        }
    }

    return QModelIndex();
}


QModelIndexList RecordsTreeModel::modelIndexesForItemWithValueForKey(const char *in_value, const char *in_key, const QModelIndex &in_parent)
{
    QModelIndexList tmp_final_indexes_list, tmp_indexes_list;
    QModelIndex     tmp_current_model_index;
    Hierarchy       *tmp_item = NULL;
    Hierarchy       *tmp_parent_item = NULL;

    if (in_parent.isValid() == false)
        tmp_parent_item = _m_root_item;
    else
        tmp_parent_item = getItem(in_parent);

    for (int tmp_index = 0; tmp_index < tmp_parent_item->childCount(_m_child_type); tmp_index++)
    {
        tmp_item = tmp_parent_item->child(tmp_index, _m_child_type);
        if (tmp_item != NULL)
        {
            tmp_current_model_index = index(tmp_index, 0, in_parent);
            if (tmp_current_model_index.isValid())
            {
                if (compare_values(tmp_item->getValueForKey(in_key), in_value) == 0)
                {
                    tmp_final_indexes_list.append(tmp_current_model_index);
                }
                else
                {
                    tmp_indexes_list = modelIndexesForItemWithValueForKey(in_value, in_key, tmp_current_model_index);
                    tmp_final_indexes_list.append(tmp_indexes_list);
                }
            }
        }
    }

    return tmp_final_indexes_list;
}


QModelIndex RecordsTreeModel::modelIndexForItem(Hierarchy *in_item, const QModelIndex &in_parent)
{
    QModelIndex     tmp_model_index, tmp_current_model_index;
    Hierarchy       *tmp_item = NULL;
    Hierarchy       *tmp_parent_item = NULL;

    if (in_parent.isValid() == false)
        tmp_parent_item = _m_root_item;
    else
        tmp_parent_item = getItem(in_parent);

    for (int tmp_index = 0; tmp_index < tmp_parent_item->childCount(_m_child_type); tmp_index++)
    {
        tmp_item = tmp_parent_item->child(tmp_index, _m_child_type);
        if (tmp_item != NULL)
        {
            tmp_current_model_index = index(tmp_index, 0, in_parent);
            if (tmp_current_model_index.isValid())
            {
                if (in_item == tmp_item)
                {
                    return tmp_current_model_index;
                }
                else
                {
                    tmp_model_index = modelIndexForItem(in_item, tmp_current_model_index);
                    if (tmp_model_index.isValid())
                        return tmp_model_index;
                }
            }
        }
    }

    return QModelIndex();
}




QModelIndex RecordsTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    Hierarchy *parentItem = NULL;
    Hierarchy *childItem = NULL;

    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        parentItem = _m_root_item;
    else
        parentItem = static_cast<Hierarchy*>(parent.internalPointer());

    childItem = parentItem->child(row, _m_child_type);

    if (childItem != NULL)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}



/**
  Inserer un nouveau item
**/
bool RecordsTreeModel::insertRows(int position, int rows, const QModelIndex &parent)
{
    Hierarchy *parentItem = getItem(parent);
    bool success = false;

    if (parentItem->isWritable(_m_child_type)&& parentItem->canInsert(position, _m_child_type))
    {
        beginInsertRows(parent, position, position + rows - 1);
        success = parentItem->insertChildren(position, rows, NULL, _m_child_type);
        endInsertRows();
    }

    return success;
}


bool RecordsTreeModel::insertRow(int row, const QModelIndex &parent)
{
    return insertRows(row, 1, parent);
}


/**
  Inserer un nouveau item
**/
bool RecordsTreeModel::insertItem(int position, const QModelIndex &parent, Hierarchy *in_item)
{
    Hierarchy *parentItem = getItem(parent);
    bool success = false;

    if (parentItem->isWritable(_m_child_type)&& parentItem->canInsert(position, _m_child_type))
    {
        beginInsertRows(parent, position, position);
        success = parentItem->insertChildren(position, 1, in_item, _m_child_type);
        endInsertRows();
    }

    return success;
}


/**
  Inserer une copie de l'item
**/
bool RecordsTreeModel::insertCopyOfItem(int position, const QModelIndex &parent, GenericRecord *in_item)
{
    Hierarchy			*parentItem = getItem(parent);
    bool				success = false;
    QList<Hierarchy*>		*tmp_list = NULL;
    int					tmp_item_index = 0;

    if (parentItem->isWritable(_m_child_type)&& parentItem->canInsert(position, _m_child_type))
    {
        tmp_list = parentItem->orginalsRecordsForGenericRecord(in_item, _m_child_type);
        if(tmp_list != NULL)
        {
            if (tmp_list->isEmpty())
            {
                success = true;
            }
            else
            {
                beginInsertRows(parent, position, position + tmp_list->count() - 1);
                for(tmp_item_index = 0; tmp_item_index < tmp_list->count(); tmp_item_index++)
                {
                    success = parentItem->insertChildren(position + tmp_item_index, 1, tmp_list->at(tmp_item_index), _m_child_type);
                    if (!success)
                        break;
                }
                endInsertRows();
            }

            delete tmp_list;
        }
        else
        {
            beginInsertRows(parent, position, position);
            success = parentItem->insertCopyOfChildren(position, 1, in_item, _m_child_type);
            endInsertRows();
        }
    }

    return success;
}


/**
  Renvoie le item parent
**/
QModelIndex RecordsTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();

    Hierarchy *childItem = static_cast<Hierarchy*>(index.internalPointer());
    Hierarchy *parentItem = childItem->parent();

    if (parentItem == NULL || parentItem == _m_root_item)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}


/**
  Supprimer le item
**/
bool RecordsTreeModel::removeRows(int position, int rows, const QModelIndex &parent)
{
    Hierarchy *parentItem = getItem(parent);
    bool success = false;

    if (parentItem->isWritable(_m_child_type) && parentItem->canRemove(position, _m_child_type))
    {
        beginRemoveRows(parent, position, position + rows - 1);
        success = parentItem->removeChildren(position, rows, true, _m_child_type);
        endRemoveRows();
    }

    return success;
}


bool RecordsTreeModel::removeRow(int row, const QModelIndex &parent)
{
    return removeRows(row, 1, parent);
}


/**
  Supprimer le item
**/
bool RecordsTreeModel::removeItem(int position, const QModelIndex &parent)
{
    Hierarchy *parentItem = getItem(parent);
    bool success = false;

    if (parentItem->isWritable(_m_child_type)&& parentItem->canRemove(position, _m_child_type))
    {
        beginRemoveRows(parent, position, position);
        success = parentItem->removeChildren(position, 1, false, _m_child_type);
        endRemoveRows();
    }

    return success;
}
/**
  Renvoie le nombre de items enfants du item passe en parametre
**/
int RecordsTreeModel::rowCount(const QModelIndex &parent) const
{
    Hierarchy *parentItem = NULL;

    if (!parent.isValid())
        parentItem = _m_root_item;
    else
        parentItem = static_cast<Hierarchy*>(parent.internalPointer());

    if (parentItem)
        return parentItem->childCount(_m_child_type);

    return 0;
}

void RecordsTreeModel::setSupportedDropActions(Qt::DropActions in_drop_actions)
{
    _m_drop_actions = in_drop_actions;
}


Qt::DropActions RecordsTreeModel::supportedDropActions() const
{
    return _m_drop_actions;
}


bool RecordsTreeModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int /* column */, const QModelIndex &parent)
{
    QByteArray          tmp_bytes_array = data->data(_m_mime_type);
    QList<QString>      tmp_records_list;
    int                 tmp_index = 0;

    entity_def		*tmp_entity_def = NULL;
    int                 tmp_start_index = 0;

    GenericRecord       *tmp_generic_record = NULL;
    Hierarchy		*tmp_record = NULL;
    int			tmp_lock_record_status = RECORD_STATUS_MODIFIABLE;
    Hierarchy           *tmp_drop_parent_item = NULL;
    Hierarchy		*tmp_drag_parent_item = NULL;

    Hierarchy		*tmp_link_item = NULL;

    bool                tmp_return = false;
    int                 beginRow = 0;

    if (row != -1)
        beginRow = row;
    else if (parent.isValid())
        beginRow = 0;
    else
        beginRow = rowCount(QModelIndex());

    // Verifier les donnees deposees
    if (tmp_bytes_array.isEmpty() == false)
    {
        tmp_records_list = QString(tmp_bytes_array).split('\n');
        for (tmp_index = 0; tmp_index < tmp_records_list.count(); tmp_index++)
        {
            tmp_start_index = tmp_records_list[tmp_index].indexOf(SEPARATOR_CHAR);
            if (tmp_start_index >= 0)
            {
                std::string tmp_std_string  = tmp_records_list[tmp_index].section(SEPARATOR_CHAR, 0, 0).toStdString();
                if (get_table_def(atoi(tmp_std_string.c_str()), &tmp_entity_def) == NOERR)
                {
                    tmp_generic_record = new GenericRecord(tmp_entity_def);
                    if (tmp_generic_record != NULL)
                    {
                        tmp_generic_record->deserialize(tmp_records_list[tmp_index]);

                        switch (action)
                        {
                        case Qt::IgnoreAction:
                            return true;

                        case Qt::CopyAction:
                            tmp_return = insertCopyOfItem(beginRow + tmp_index, parent, tmp_generic_record);
                            break;

                        case Qt::MoveAction:
                            // Autoriser le deplacement pour des entites de même type uniquement
                            if (tmp_entity_def->m_entity_signature_id == _m_child_type)
                            {
                                tmp_record = _m_root_item->findItemWithId(tmp_generic_record->getIdentifier(), tmp_generic_record->getEntityDefSignatureId());
                                if (tmp_record != NULL)
                                {
                                    tmp_drag_parent_item = tmp_record->parent();
                                    if (tmp_drag_parent_item != NULL)
                                    {
                                        tmp_drop_parent_item = getItem(parent);
                                        if (tmp_record->mayHaveCyclicRedundancy(tmp_drop_parent_item, false))
                                        {
                                            emit cyclicRedundancy();
                                        }
                                        else
                                        {
                                            tmp_lock_record_status = tmp_record->lockRecordStatus();
                                            if(tmp_drag_parent_item->canRemove(tmp_record->row(), tmp_record->getEntityDefSignatureId()))
                                            {
                                                if (tmp_drop_parent_item->canInsert(beginRow + tmp_index, _m_child_type))
                                                {
                                                    tmp_return = insertItem(beginRow + tmp_index, parent, tmp_record->cloneForMove());
                                                }
                                                else
                                                {
                                                    // Annuler le verrou
                                                    if (tmp_lock_record_status != RECORD_STATUS_OWN_LOCK)
                                                        tmp_record->unlockRecord();
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                            break;

                        case Qt::LinkAction:
                            // Autoriser les liens pour des entites de même type uniquement
                            if (tmp_entity_def->m_entity_signature_id == _m_child_type)
                            {
                                tmp_record = _m_root_item->findItemWithId(tmp_generic_record->getIdentifier(), tmp_generic_record->getEntityDefSignatureId());
                                if (tmp_record != NULL)
                                {
                                    if (tmp_record->mayHaveCyclicRedundancy(getItem(parent), true))
                                    {
                                        emit cyclicRedundancy();
                                    }
                                    else
                                    {
                                        tmp_link_item = tmp_record->link();
                                        if (tmp_link_item != NULL)
                                            tmp_return = insertItem(beginRow, parent, tmp_link_item);
                                    }
                                }
                            }
                            break;

                        default:
                            break;
                        }

                        delete tmp_generic_record;
                    }
                }
            }
        }
    }

    return tmp_return;
}



QStringList RecordsTreeModel::mimeTypes() const
{
    QStringList types;
    types << _m_mime_type;
    return types;
}


QMimeData* RecordsTreeModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData       	*tmp_mime_data = new QMimeData();
    QByteArray      	tmp_byte_array;
    Hierarchy       	*tmp_record = NULL;
    Hierarchy		*tmp_current_record = NULL;
    QList<Hierarchy*>	tmp_records_list;
    QList<Hierarchy*>	tmp_remove_records_list;
    bool		tmp_indic = false;

    foreach (QModelIndex tmp_current_index, indexes)
    {
        if (tmp_current_index.isValid())
        {
            tmp_record = getItem(tmp_current_index);
            if (tmp_record != NULL)
            {
                tmp_indic = false;

                foreach(tmp_current_record, tmp_records_list)
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
        }
    }

    foreach(tmp_record, tmp_records_list)
    {
        if (tmp_remove_records_list.indexOf(tmp_record) < 0)
        {
            tmp_byte_array.append(tmp_record->serialize());
            tmp_byte_array.append('\n');
        }
    }

    tmp_mime_data->setData(_m_mime_type, tmp_byte_array);

    return tmp_mime_data;
}


void RecordsTreeModel::setMimeType(QString in_mime_type)
{
    _m_mime_type = in_mime_type;
}

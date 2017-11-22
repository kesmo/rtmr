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

#ifndef RECORDTREEMODEL_H
#define RECORDTREEMODEL_H

#include <QList>
#include <QKeyEvent>
#include <QAbstractItemModel>
#include "hierarchy.h"
#include "genericrecord.h"

#define DEFAULT_MIME_TYPE "application/records.list"

class RecordsTreeModel : public QAbstractItemModel
{
    Q_OBJECT

private:
    Hierarchy              *_m_root_item;
    int                    _m_child_type;

    Qt::DropActions        _m_drop_actions;

    QString					_m_mime_type;

    int                    _m_columns_count;

public:
    RecordsTreeModel();
    RecordsTreeModel(Hierarchy* in_root_item, int in_child_type = 0);
    ~RecordsTreeModel();

    void setRootItem(Hierarchy* in_root_item, int in_child_type = 0);

    QVariant data(const QModelIndex &index, int role) const;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void setSupportedDropActions(Qt::DropActions in_drop_actions);
    Qt::DropActions supportedDropActions() const;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QModelIndexList &indexes) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool insertItem(int position, const QModelIndex &parent, Hierarchy *in_item);
    bool insertCopyOfItem(int position, const QModelIndex &parent, GenericRecord *in_item);
    bool insertRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    bool insertRow(int row, const QModelIndex &parent = QModelIndex());
    bool removeItem(int position, const QModelIndex &parent = QModelIndex());
    bool removeRows(int position, int rows, const QModelIndex &parent = QModelIndex());
    bool removeRow(int row, const QModelIndex &parent = QModelIndex());

    Hierarchy* getItem(const QModelIndex &index) const;

    void setMimeType(QString in_mime_type);

    QModelIndexList modelIndexesForItemWithValueForKey(const char *in_value, const char *in_key, const QModelIndex &in_parent = QModelIndex());
    QModelIndex firstModelIndexForItemWithValueForKey(const char *in_value, const char *in_key, const QModelIndex &in_parent = QModelIndex());
    QModelIndex modelIndexForItem(Hierarchy *in_item, const QModelIndex &in_parent = QModelIndex());

    void startReset(){beginResetModel();}
    void endReset(){endResetModel();}

    void setColumnsCount(int count);

Q_SIGNALS:
    void cyclicRedundancy();
};

#endif // RECORDTREEMODEL_H

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
#ifndef RECORDSTABLEMODEL_H
#define RECORDSTABLEMODEL_H

#include "record.h"
#include "session.h"

#include <QAbstractTableModel>

typedef QVariant (ColumnDataFunction)(Record* record, int role);

template <class T>
class RecordsTableModel : public QAbstractTableModel
{

public:
  RecordsTableModel(QObject *parent = 0)
    : QAbstractTableModel(parent)
  {
    for(unsigned int columnIndex = 0; columnIndex < T::getEntityDefinition()->m_entity_columns_count; ++columnIndex) {
        _m_headers.append(QPair<QString, int>(T::getEntityDefinition()->m_entity_columns_names[columnIndex], columnIndex));
      }
  }


  ~RecordsTableModel()
  {
    revertCache();

    qDeleteAll(_m_original_records_list);
    _m_original_records_list.clear();
  }


  RecordsTableModel(const QList<QString>& in_headers, const QList<T*>& in_record_list = QList<T*>(), QObject *parent = 0)
    : QAbstractTableModel(parent),
      _m_records_list(in_record_list),
      _m_original_records_list(in_record_list),
      _m_headers()
  {
    int index = 0;
    foreach (QString header, in_headers) {
        _m_headers.append(QPair<QString, int>(header, index));
        ++index;
      }
  }


  RecordsTableModel(const QList< QPair<QString, QString> >& in_headers, const QList<T*>& in_record_list = QList<T*>(), QObject *parent = 0)
    : QAbstractTableModel(parent),
      _m_records_list(in_record_list),
      _m_original_records_list(in_record_list),
      _m_headers()
  {
    QPair<QString, QString> header;
    foreach (header, in_headers) {
        _m_headers.append(QPair<QString, int>(header.first, T::indexForKey(header.second.toStdString().c_str())));
      }
  }

  void setColumnDataFunctionForColumn(int columnIndex, ColumnDataFunction* in_function_ptr){
    _m_column_data_functions_map[columnIndex] = in_function_ptr;
  }

  void setRecordsList(const QList<T*>& in_record_list, bool in_add_to_remove_list_current_records = false)
  {
    if (in_add_to_remove_list_current_records){
        removeAllRecords();
      }
    else {
        revertCache();
    }

    _m_original_records_list = in_record_list;
    _m_records_list = in_record_list;
    reset();
  }


  const QList<T*>& getRecordsList() const
  {
    return _m_records_list;

  }

  QVariant data(const QModelIndex &index, int role) const
  {
    if (index.column() < _m_headers.count())
      {
        if (_m_column_data_functions_map.contains(index.column()))
          return _m_column_data_functions_map[index.column()](_m_records_list[index.row()], role);

        switch(role)
          {
          case Qt::DisplayRole:
          case Qt::EditRole:
          case Qt::ToolTipRole:
            T* record = _m_records_list[index.row()];
            if (record)
              {
                return record->getValueForKeyAtIndex(_m_headers[index.column()].second);
              }
            break;
          }
      }

    return QVariant();
  }


  int rowCount(const QModelIndex & /*parent*/ = QModelIndex()) const
  {
    return _m_records_list.count();
  }


  int columnCount(const QModelIndex & /*parent*/ = QModelIndex()) const
  {
    return _m_headers.count();
  }


  QVariant headerData(int section, Qt::Orientation orientation, int role) const
  {
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
      return _m_headers[section].first;

    return QVariant();
  }


  bool insertRows (int row, int count, const QModelIndex & parent)
  {
    beginInsertRows(parent, row, row + count - 1);
    _m_records_list.insert(row, NULL);
    endInsertRows();

    return true;
  }


  bool removeRows(int row, int count, const QModelIndex & parent)
  {
    beginRemoveRows(parent, row, row + count - 1);
    for(int tmp_index = 0; tmp_index < count; ++tmp_index)
      {
        _m_removed_records_list.append(_m_records_list.takeAt(row));
      }
    endRemoveRows();

    return true;
  }

  void removeAllRecords(){
    if (_m_records_list.count() > 0){
        beginRemoveRows(QModelIndex(), 0, _m_records_list.count() - 1);
        _m_removed_records_list.append(_m_records_list);
        _m_records_list.clear();
        endRemoveRows();
      }
  }

  bool moveRows(int row, int count, int newRow, const QModelIndex & parent)
  {
    int delta = newRow - row;
    int total = _m_records_list.count();
    QList<T*> tmp_moved_record_list;
    beginMoveRows(parent, row, row + count -1, parent, newRow);
    for(int tmp_index = 0; tmp_index < count; ++tmp_index)
      {
        tmp_moved_record_list.append(_m_records_list.takeAt(row));
      }

    if (delta > 0)
      {
        if (delta < count)
          {
            for(int tmp_index = 0; tmp_index < count; ++tmp_index)
              {
                _m_records_list.insert(row + delta + tmp_index, tmp_moved_record_list[tmp_index]);
              }
          }
        else
          {
            for(int tmp_index = 0; tmp_index < count; ++tmp_index)
              {
                _m_records_list.insert(newRow - count + tmp_index, tmp_moved_record_list[tmp_index]);
              }
          }
      }
    else
      {
        for(int tmp_index = 0; tmp_index < count; ++tmp_index)
          {
            _m_records_list.insert(newRow + tmp_index, tmp_moved_record_list[tmp_index]);
          }
      }

    endMoveRows();

    return true;
  }


  QModelIndex index(int row, int column, const QModelIndex &/*parent*/) const
  {
    if (row < _m_records_list.count())
      return createIndex(row, column, _m_records_list[row]);

    return QModelIndex();
  }

  void addRecord(T* record)
  {
    beginInsertRows(QModelIndex(), _m_records_list.count(), _m_records_list.count());
    _m_records_list.append(record);
    endInsertRows();
  }


  void removeRecord(T* record)
  {
    int index = _m_records_list.indexOf(record);
    if (index >= 0)
      {
        beginRemoveRows(QModelIndex(), index, index);
        _m_records_list.removeAt(index);
        _m_removed_records_list.append(record);
        endRemoveRows();
      }
  }


  T* recordAtIndex(const QModelIndex &index) const
  {
    if (index.isValid())
      return static_cast<T*>(index.internalPointer());

    return NULL;
  }

  T* recordAtIndex(const int index) const
  {
    if (index < _m_records_list.count())
      return _m_records_list[index];

    return NULL;
  }


public Q_SLOTS:
  bool submit()
  {
    int result = NOERR;

    beginResetModel();
    for(int index = 0; index < _m_records_list.count() && result == NOERR; ++index)
      {
        T* record = _m_records_list[index];
        if (record)
          {
            result = record->saveRecord();
          }
      }

    for(int index = 0; index < _m_removed_records_list.count() && result == NOERR; ++index)
      {
        T* record = _m_removed_records_list[index];
        if (record)
          {
            result = record->deleteRecord();
          }
      }

    if (result == NOERR)
      {
        qDeleteAll(_m_removed_records_list);
        _m_removed_records_list.clear();
        _m_original_records_list = _m_records_list;
      }
    else{
        Session::instance()->setLastErrorMessage(result);
    }
    endResetModel();

    return result == NOERR;
  }

  void revert()
  {
    beginResetModel();

    revertCache();
    _m_records_list = _m_original_records_list;
    endResetModel();

    QAbstractItemModel::revert();
  }

private:
  QList<T*> _m_records_list;
  QList<T*> _m_original_records_list;
  QList<T*> _m_removed_records_list;
  QList< QPair<QString, int > > _m_headers;

  QMap<int, ColumnDataFunction*> _m_column_data_functions_map;

  void revertCache(){
    int result = NOERR;
    for(int index = 0; index < _m_records_list.count() && result == NOERR; ++index)
      {
        T* record = _m_records_list[index];
        if (record){
            if (!_m_original_records_list.contains(record)){
                _m_records_list.removeAll(record);
                delete record;
              }
          }
      }

    for(int index = 0; index < _m_removed_records_list.count() && result == NOERR; ++index)
      {
        T* record = _m_removed_records_list[index];
        if (record){
            if (!_m_original_records_list.contains(record)){
                _m_removed_records_list.removeAll(record);
                delete record;
              }
          }
      }

    _m_records_list.clear();
    _m_removed_records_list.clear();

  }
};

#endif // RECORDSTABLEMODEL_H

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

#ifndef PARENT_H
#define PARENT_H

#include "constants.h"
#include "record.h"
#include <QList>

template <class T>
class Parent
{
public:
  Parent() :
      _m_childs_loaded(false)
  {
  }


  ~Parent()
  {
      qDeleteAll(_m_childs);
      qDeleteAll(_m_removed_childs);
  }


  const QList<T*>& getChilds(bool in_force_reload = false, bool in_force_not_loaded = false)
  {
      if ((in_force_reload || !_m_childs_loaded) && !in_force_not_loaded)
      {
          _m_childs = loadChilds();
          _m_childs_loaded = true;
      }

    return _m_childs;
  }


  void addChild(T* in_child){
      _m_childs.append(in_child);
  }


  void setChilds(QList<T*> in_childs){
      _m_childs = in_childs;
  }


  void removeChild(T* in_child){
      _m_childs.removeAll(in_child);
      _m_removed_childs.append(in_child);
  }


  void removeAllChilds(){
      _m_removed_childs.append(_m_childs);
      _m_childs.clear();
  }


  void moveChild(int from, int to){
      _m_childs.move(from, to);
  }



  int saveChilds(const char* in_previous_identifier_key){
      int tmp_return_result = NOERR;
      const char* tmp_previous_identifier = NULL;

      for(int tmp_index = 0; tmp_index < _m_removed_childs.count() && tmp_return_result == NOERR; ++tmp_index){
          tmp_return_result = _m_removed_childs[tmp_index]->deleteRecord();
      }

      for(int tmp_index = 0; tmp_index < _m_childs.count() && tmp_return_result == NOERR; ++tmp_index){
          if (in_previous_identifier_key)
              _m_childs[tmp_index]->setValueForKey(tmp_previous_identifier, in_previous_identifier_key);

          tmp_return_result = _m_childs[tmp_index]->saveRecord();
          if (tmp_return_result == NOERR)
              tmp_previous_identifier = _m_childs[tmp_index]->getIdentifier();
      }

      return tmp_return_result;
  }

protected:
  mutable QList<T*> _m_childs;
  QList<T*> _m_removed_childs;
  bool _m_childs_loaded;

  virtual QList<T*> loadChilds() = 0;
};

#endif // PARENT_H

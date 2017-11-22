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

#ifndef PARAMETER_H
#define PARAMETER_H

#include "record.h"

class Parameter : public Record
{
public:
    Parameter(const entity_def* in_entity_def);
    virtual ~Parameter() = 0;

    virtual const entity_def* getEntityDef() const = 0;

    virtual const char* name() = 0;
    virtual const char* value() = 0;

    template <class T>
    static QList<QString> parametersNamesList(QList<T*> in_params_list)
    {
	QList<QString> tmp_list;

	foreach(T *tmp_param, in_params_list)
	{
	    tmp_list << tmp_param->name();
	}

	return tmp_list;
    }

    template <class T>
    static const char* paramValueForParamName(QList<T*> in_params_list,  std::string in_param_name)
    {
	foreach(T *tmp_param, in_params_list)
	{
	    if (compare_values(tmp_param->name(), in_param_name.c_str()) == 0)
		return tmp_param->value();
	}

	return NULL;
    }

    template <class T>
    static T* parameterForParamName(QList<T*> in_params_list, const char *in_param_name)
    {
	foreach(T *tmp_param, in_params_list)
	{
	    if (compare_values(tmp_param->name(), in_param_name) == 0)
		return tmp_param;
	}

	return NULL;
    }


};

#endif // PARAMETER_H

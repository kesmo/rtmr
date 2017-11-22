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

#include "Trinome.h"


/**
 * Constructeur
 */
Trinome::Trinome()
{
        m_x_value = NULL;
        m_y_value = NULL;
        m_z_value = NULL;
}

/**
 * Constructeur
 */
Trinome::Trinome(DataObject *in_x_value)
{
	m_x_value = in_x_value;
        m_y_value = NULL;
        m_z_value = NULL;
}

/**
 * Constructeur
 */
Trinome::Trinome(DataObject *in_x_value, DataObject *in_y_value)
{
	m_x_value = in_x_value;
	m_y_value = in_y_value;
        m_z_value = NULL;
}

/**
 * Constructeur
 * @param in_values_array	tableau de valeurs
 */
Trinome::Trinome(DataObject *in_x_value, DataObject *in_y_value, DataObject *in_z_value)
{
	m_x_value = in_x_value;
	m_y_value = in_y_value;
	m_z_value = in_z_value;
}


Trinome& Trinome::operator=(const Trinome& other)
{
    m_x_value = other.m_x_value;
    m_y_value = other.m_y_value;
    m_z_value = other.m_z_value;

    return *this;
}


Trinome::~Trinome()
{
    delete m_x_value;
    delete m_y_value;
    delete m_z_value;
}


/**
 * Comparer le trinome à un autre trinome
 * @param in_tuple	un tuple
 * @return
 */
int Trinome::compareTo(const Trinome &in_tuple) const
{
	int	result = 0;
	
	/* Comparer les valeurs X en premier, puis Z si necessaire */
	if (m_x_value != NULL)
	{
		if (in_tuple.m_x_value != NULL)
		{
			if ((result = m_x_value->compareTo(*in_tuple.m_x_value)) == 0)
			{
			    if (m_z_value != NULL)
			    {
					if (in_tuple.m_z_value != NULL)
						result = m_z_value->compareTo(*in_tuple.m_z_value);
					else
						return 1;
				}
				else if (in_tuple.m_z_value != NULL)
					return -1;
			}
		}
		else
		{
			return 1;
		}
	}
	else
	{
		if (in_tuple.m_x_value != NULL)
		{
			return -1;
		}
		else
		{
		    if (m_z_value != NULL)
		    {
				if (in_tuple.m_z_value != NULL)
					result = m_z_value->compareTo(*in_tuple.m_z_value);
				else
					return 1;
			}
			else if (in_tuple.m_z_value != NULL)
				return -1;
		}
	}
	
	return result;
}



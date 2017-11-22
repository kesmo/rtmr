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

#ifndef TRINOME_H
#define TRINOME_H

#include "objects/Comparable.h"
#include "objects/DataObject.h"


/**
 * Classe de définition d'un trinome<br>
 * Un trinome est un groupe de trois valeurs X, Y et Z.
 */
class Trinome
{
public:
	/**
	* Valeur X
	*/
        DataObject	*m_x_value;

	/**
	 * Valeur Y
	 */
	DataObject	*m_y_value;

	/**
	 * Valeur Z
	 */
	DataObject	*m_z_value;

        /**
         * Constructeur
         */
        Trinome();

        /**
	 * Constructeur
	 */
	Trinome(DataObject *in_x_value);

	/**
	 * Constructeur
	 */
	Trinome(DataObject *in_x_value, DataObject *in_y_value);

	/**
	 * Constructeur
	 */
	Trinome(DataObject *in_x_value, DataObject *in_y_value, DataObject *in_z_value);

    /**
     * Constructeur par copie
     */
    Trinome& operator=(const Trinome& other);

    /**
     * Desstructeur
     */
    ~Trinome();

	/**
	 * Comparer le trinome à un autre trinome
	 * @param in_tuple	un tuple
	 * @return
	 */
	int compareTo(const Trinome &in_tuple) const;

	/**
	 * Comparer deux trinomes
	 * @param in_first_trinome
	 * @param in_sec_trinome
	 * @return
	 */
	static int compare(Trinome *in_first_trinome, Trinome *in_sec_trinome)
	{
		if (in_first_trinome != NULL)
		{
			if (in_sec_trinome != NULL)
				return in_first_trinome->compareTo(*in_sec_trinome);
			else
				return -1;
		}
		else if (in_sec_trinome != NULL)
			return 1;
		else
			return 0;
	}
};


#endif // TRINOME_H

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


#ifndef TRINOMEARRAY_H_
#define TRINOMEARRAY_H_

#include "Trinome.h"
#include "DataObject.h"


#include <vector>

using namespace std;

class TrinomeArray
{
public:
        /**
        * Liste des objets X
        */
        vector<DataObject*>	m_x_objects;

	/**
	 * Liste des objets Y
	 */
        vector<DataObject*>	m_y_objects;

	/**
	 * Liste des objets Z
	 */
        vector<DataObject*>	m_z_objects;

	/**
	 * Valeur la plus grande parmi les X
	 **/
	DataObject	*m_max_x;

	/**
	 * Valeur la plus grande parmi les Y
	 **/
	DataObject	*m_max_y;

	/**
	 * Valeur la plus grande parmi les Z
	 **/
	DataObject	*m_max_z;

	/**
	 * Valeur la plus petite parmi les X
	 **/
	DataObject	*m_min_x;

	/**
	 * Valeur la plus petite parmi les Y
	 **/
	DataObject	*m_min_y;

	/**
	 * Valeur la plus petite parmi les Z
	 **/
	DataObject	*m_min_z;

	/**
	 * Somme des X (si les X sont des nombres)
	 */
	double		m_total_x;

	/**
	 * Somme des Y (si les Y sont des nombres)
	 */
	double		m_total_y;

	/**
	 * Tableau des sommes des valeurs Y par catégories X
	 */
        vector<double>		m_total_y_by_x_class;

	/**
	 * Valeur la plus grande des sommes des valeurs Y par catégories X
	 */
	double		m_max_total_y_by_x_class;

	/**
	 * Somme des Z (si les Z sont des nombres)
	 */
	double		m_total_z;

	/**
	 * Indicateur de présence de nombres décimaux parmi les X
	 */
	bool		m_has_decimal_x_indic;

	/**
	 * Indicateur de présence de nombres décimaux parmi les Y
	 */
	bool		m_has_decimal_y_indic;

	/**
	 * Nombre de trinomes
	 **/
        unsigned int			length;

	/**
	 * Tableau des valeurs distinctes X
	 **/
        vector<DataObject*>		m_x_categories;

	/**
	 * Tableau des valeurs distinctes Y
	 **/
        vector<DataObject*>		m_y_categories;

	/**
	 * Tableau des valeurs distinctes Z
	 **/
        vector<DataObject*>		m_z_categories;

        /**
         * Constructeur
         */
        TrinomeArray();

        /**
         * Destructeur
         */
        ~TrinomeArray();

	/**
	 * Constructeur
	 */
    TrinomeArray(const QList<Trinome*> &in_array);


	/**
	 * Renvoie le trinome à l'index spécifié
	 */
    Trinome *tupleAtIndex(int in_index);

	/**
	 * Ordonner le tableau.<br>
	 * Comparaison trinome par trinome
	 */
	void orderArray();

	template <class T>
        static int indexOfObjectInArray(T *in_object, vector<T*> in_array)
        {
            for (unsigned int tmp_index = 0; tmp_index < in_array.size(); tmp_index++)
            {
                if (in_object == in_array[tmp_index] || (in_object != NULL && in_array[tmp_index] != NULL && (*in_object) == (*in_array[tmp_index])))
                    return tmp_index;
            }
            return -1;
        }


	template <class T>
        static int indexOfIdenticalObjectInArray(T in_object, vector<T> in_array)
        {
            for (unsigned int tmp_index = 0; tmp_index < in_array.size(); tmp_index++)
            {
                if (in_object == in_array[tmp_index])
                    return tmp_index;
            }
            return -1;
        }


private:

	/* Tableau des trinomes */
    QList<Trinome*>	_m_trinomes_array;


	/**
	 * Creation des tableaux d'objets x, y et z.
	 */
	void _createObjectsArrays();


        void _init();
};

#endif /* TRINOMEARRAY_H_ */

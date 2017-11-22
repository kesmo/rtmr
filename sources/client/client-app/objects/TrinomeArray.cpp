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

#include "TrinomeArray.h"
#include <stdio.h>

/**
 * Constructeur
 * @param in_array	tableau de trinomes
 */
TrinomeArray::TrinomeArray()
{
    _init();
}


/**
 * Constructeur
 * @param in_array	tableau de trinomes
 */
TrinomeArray::TrinomeArray(const QList<Trinome *> &in_array)
{
    int tmp_dest_index = 0;

    _init();

    if (in_array.size() > 0)
    {
        // Insérer les données en triant
        foreach(Trinome* tmp_trinome, in_array)
        {
            tmp_dest_index = 0;
            foreach(Trinome* tmp_included_trinome, _m_trinomes_array)
            {
                if (tmp_trinome != tmp_included_trinome && tmp_trinome->compareTo(*tmp_included_trinome) == -1)
                    break;
                tmp_dest_index++;
            }

            _m_trinomes_array.insert(tmp_dest_index, tmp_trinome);
        }

        length = in_array.size();

        /* Ordonner le tableau avant traitement */
        orderArray();

    }
}


/**
 * Destructeur
 */
TrinomeArray::~TrinomeArray()
{
}


void TrinomeArray::_init()
{
    /**
         * Valeur la plus grande parmi les X
         **/
    m_max_x = NULL;

    /**
         * Valeur la plus grande parmi les Y
         **/
    m_max_y = NULL;

    /**
         * Valeur la plus grande parmi les Z
         **/
    m_max_z = NULL;

    /**
         * Valeur la plus petite parmi les X
         **/
    m_min_x = NULL;

    /**
         * Valeur la plus petite parmi les Y
         **/
    m_min_y = NULL;

    /**
         * Valeur la plus petite parmi les Z
         **/
    m_min_z = NULL;

    /**
         * Valeur la plus grande des sommes des valeurs Y par catégories X
         */
    m_max_total_y_by_x_class = 0.0;

    /**
         * Indicateur de présence de nombres décimaux parmi les X
         */
    m_has_decimal_x_indic = false;

    /**
         * Indicateur de présence de nombres décimaux parmi les Y
         */
    m_has_decimal_y_indic = false;

    /**
         * Nombre de trinomes
         **/
    length = 0;
}


/**
 * Renvoie le trinome à l'index spécifié
 * @param in_index
 * @return
 */
Trinome* TrinomeArray::tupleAtIndex(int in_index)
{
    return _m_trinomes_array[in_index];
}


/**
 * Ordonner le tableau.<br>
 * Comparaison trinome par trinome @see Trinome.TupleComparator.
 */
void TrinomeArray::orderArray()
{
    _createObjectsArrays();
}


/**
 * Creation des tableaux d'objets x, y et z.
 */
void TrinomeArray::_createObjectsArrays()
{
    unsigned int         tmp_index;
    int         tmp_x_class_index, tmp_y_class_index, tmp_z_class_index;

    m_min_x = _m_trinomes_array[0]->m_x_value;
    m_min_y = _m_trinomes_array[0]->m_y_value;
    m_min_z = _m_trinomes_array[0]->m_z_value;
    m_max_x = m_min_x;
    m_max_y = m_min_y;
    m_max_z = m_min_z;

    m_total_x = 0;
    m_total_y = 0;
    m_total_z = 0;

    m_x_objects = vector<DataObject*>(length, NULL);
    m_y_objects = vector<DataObject*>(length, NULL);
    m_z_objects = vector<DataObject*>(length, NULL);

    m_total_y_by_x_class = vector<double>(length, 0.0);

    for (tmp_index = 0; tmp_index < length; tmp_index++)
    {
        m_x_objects[tmp_index] = _m_trinomes_array[tmp_index]->m_x_value;
        m_y_objects[tmp_index] = _m_trinomes_array[tmp_index]->m_y_value;
        m_z_objects[tmp_index] = _m_trinomes_array[tmp_index]->m_z_value;

        tmp_x_class_index = indexOfObjectInArray<DataObject>(m_x_objects[tmp_index], m_x_categories);

        /* Vérifier si X est une catégorie distincte */
        if (m_x_objects[tmp_index] != NULL && tmp_x_class_index < 0)
        {
            for(tmp_x_class_index = 0; (uint)tmp_x_class_index < m_x_categories.size(); tmp_x_class_index++)
            {
                if ((*m_x_objects[tmp_index]) < (*m_x_categories[tmp_x_class_index]))
                    break;
            }
            m_x_categories.insert(m_x_categories.begin()+tmp_x_class_index, m_x_objects[tmp_index]);
        }

        tmp_y_class_index = indexOfObjectInArray<DataObject>(m_y_objects[tmp_index], m_y_categories);
        /* Vérifier si Y est une catégorie distincte */
        if (m_y_objects[tmp_index] != NULL && tmp_y_class_index < 0)
        {
            for(tmp_y_class_index = 0; (uint)tmp_y_class_index < m_y_categories.size(); tmp_y_class_index++)
            {
                if ((*m_y_objects[tmp_index]) < (*m_y_categories[tmp_y_class_index]))
                    break;
            }
            m_y_categories.insert(m_y_categories.begin()+tmp_y_class_index, m_y_objects[tmp_index]);
        }

        tmp_z_class_index = indexOfObjectInArray<DataObject>(m_z_objects[tmp_index], m_z_categories);
        /* Vérifier si Z est une catégorie distincte */
        if (m_z_objects[tmp_index] != NULL && tmp_z_class_index < 0)
        {
            for(tmp_z_class_index = 0; (uint)tmp_z_class_index < m_z_categories.size(); tmp_z_class_index++)
            {
                if ((*m_z_objects[tmp_index]) < (*m_z_categories[tmp_z_class_index]))
                    break;
            }
            m_z_categories.insert(m_z_categories.begin()+tmp_z_class_index, m_z_objects[tmp_index]);
        }

        /* Calculer le plus grand X */
        if (DataObject::compare(m_max_x, m_x_objects[tmp_index]) == 1)
            m_max_x = m_x_objects[tmp_index];

        /* Calculer le plus grand Y si les valeurs Y sont comparables */
        if (DataObject::compare(m_max_y, m_y_objects[tmp_index]) == -1)
            m_max_y = m_y_objects[tmp_index];

        /* Calculer le plus grand Z */
        if (DataObject::compare(m_max_z, m_z_objects[tmp_index]) == -1)
            m_max_z = m_z_objects[tmp_index];

        /* Calculer le plus petit X */
        if (DataObject::compare(m_x_objects[tmp_index], m_min_x) == -1)
            m_min_x = m_x_objects[tmp_index];

        /* Calculer le plus petit Y si les valeurs Y sont comparables */
        if (DataObject::compare(m_y_objects[tmp_index], m_min_y) == -1)
            m_min_y = m_y_objects[tmp_index];

        /* Calculer le plus petit Z */
        if (DataObject::compare(m_z_objects[tmp_index], m_min_z) == -1)
            m_min_z = m_z_objects[tmp_index];

        /* Calculer la somme de la série X si les X sont des nombres */
        if (m_x_objects[tmp_index] != NULL && m_x_objects[tmp_index]->isNumber())
        {
            m_total_x += m_x_objects[tmp_index]->doubleValue();
            if (!m_has_decimal_x_indic && (int)m_total_x != m_total_x)
                m_has_decimal_x_indic = true;
        }

        /* Calculer la somme de la série Y si les Y sont des nombres */
        if (m_y_objects[tmp_index] != NULL && m_y_objects[tmp_index]->isNumber())
        {
            if (tmp_x_class_index >= 0)
            {
                m_total_y_by_x_class[tmp_x_class_index] += m_y_objects[tmp_index]->doubleValue();
                if (m_max_total_y_by_x_class < m_total_y_by_x_class[tmp_x_class_index])
                    m_max_total_y_by_x_class = m_total_y_by_x_class[tmp_x_class_index];
            }
            m_total_y += m_y_objects[tmp_index]->doubleValue();
            if (!m_has_decimal_y_indic && (int)m_total_y != m_total_y)
                m_has_decimal_y_indic = true;
        }

        /* Calculer la somme de la série Z si les Z sont des nombres */
        if (m_z_objects[tmp_index] != NULL && m_z_objects[tmp_index]->isNumber())
        {
            m_total_z += m_z_objects[tmp_index]->doubleValue();
        }
    }

    /* Vérification d'intégrité */
    if (m_z_categories.size() == 0 && m_x_categories.size() != length)
    {
        fprintf(stdout, "WARNING : La série de données X comporte des éléments redondants !!\n");
        fflush(stdout);
    }
}


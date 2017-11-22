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

#ifndef CHAIN_H
#define CHAIN_H

#include "constants.h"

/**
 * Cette classe permet de gérer une chaéne d'objets.
 */
template <class T>
class Chain
{
private:
    /**
     * Données
     */
    T		*_m_data;

    /**
     * Maillon précédent
     */
    Chain		*_m_previous;

    /**
     * Maillon suivant
     */
    Chain		*_m_next;

public:
    /**
     * Constructeur
     */
    Chain()
    {
            _m_data = NULL;
            _m_next = NULL;
            _m_previous = NULL;
    }


    /**
     * Constructeur
     */
    Chain(T *in_data)
    {
    	_m_data = in_data;
            _m_next = NULL;
            _m_previous = NULL;
    }


    /**
     * Renvoyer l'objet é la position <code>in_index</code> dans la chaine
     * @param in_index	position de l'objet cherché
     * @return
     */

    T* objectAtIndex(int in_index)
    {
    	int		tmp_index = 0;
            Chain	*tmp_chain = this;

            if (in_index < 0) return NULL;

            while (tmp_chain != NULL)
    	{
                    if (in_index == tmp_index)	return tmp_chain->_m_data;
                    tmp_chain = tmp_chain->_m_next;
    		tmp_index++;
    	}

            return NULL;
    }


    /**
     * Renvoyer le maillon é la position <code>in_index</code>
     * @param in_index	position du maillon cherché
     * @return
     */
    Chain<T>* chainAtIndex(int in_index)
    {
    	int		tmp_index = 0;
            Chain	*tmp_chain = this;

            if (in_index < 0) return NULL;

            while (tmp_chain != NULL)
    	{
    		if (in_index == tmp_index)	return tmp_chain;
                    tmp_chain = tmp_chain->_m_next;
    		tmp_index++;
    	}

            return NULL;
    }


    /**
     * Insérer un objet dans la chaéne
     * @param in_new_object	nouvel objet
     * @param in_index	position d'insertion
     */
    void insertObjectAtIndex(T *in_new_object, int in_index)
    {
    	int		tmp_index = 0;
            Chain	*tmp_chain = this;
            Chain	*tmp_new_chain = NULL;

    	if (in_index < 0) return;

            while (tmp_chain != NULL)
    	{
    		if (in_index == tmp_index)
    		{
                            tmp_new_chain = new Chain(tmp_chain->_m_data);
                            tmp_new_chain->_m_previous = tmp_chain;
                            tmp_new_chain->_m_next = tmp_chain->_m_next;
                            tmp_chain->_m_next = tmp_new_chain;
                            tmp_chain->_m_data = in_new_object;
    			return;
    		}
                    tmp_chain = tmp_chain->_m_next;
    		tmp_index++;
    	}
    }


    /**
     * Supprimer un obet de la chaéne
     * @param in_index	position de l'objet é supprimer
     */
    void removeObjectAtIndex(int in_index)
    {
    	int		tmp_index = 0;
            Chain	*tmp_chain = this;

    	if (in_index < 0) return;

            while (tmp_chain != NULL)
    	{
    		if (in_index == tmp_index)
    		{
                            tmp_chain->_m_data = tmp_chain->_m_next->_m_data;
                            tmp_chain->_m_next = tmp_chain->_m_next->_m_next;
                            tmp_chain->_m_next->_m_previous = tmp_chain;
    		}
                    tmp_chain = tmp_chain->_m_next;
    		tmp_index++;
    	}
    }


    /**
     * Chercher un objet dans une chaéne
     * @param in_object	objet é chercher
     * @return	index de l'objet trouvé ou -1 si l'objet n'a pas été trouvé
     */
    int indexOfObject(T *in_object)
    {
    	int		tmp_index = 0;
            Chain	*tmp_chain = this;

            while (tmp_chain != NULL)
    	{
                    if (tmp_chain->_m_data->equals(*in_object))	return tmp_index;
                    tmp_chain = tmp_chain->_m_next;
    		tmp_index++;
    	}

    	return -1;
    }


    /**
     * Longueur de la chaéne
     * @return
     */
    int length()
    {
    	int		tmp_index = 1;
            Chain	*tmp_chain = _m_next;

            while (tmp_chain != NULL)
    	{
                    tmp_chain = tmp_chain->_m_next;
    		tmp_index++;
    	}

    	return tmp_index;
    }


    /**
     * Ajouter un maillon
     * @param in_new_object contenu du nouveau maillon
     */
    void add(T *in_new_object)
    {
    	int		tmp_index = 0;
            Chain	*tmp_chain = this;

            while (tmp_chain->_m_next != NULL)
    	{
                    tmp_chain = tmp_chain->_m_next;
    		tmp_index++;
    	}

            tmp_chain->_m_next = new Chain(in_new_object);
            tmp_chain->_m_next->_m_previous = tmp_chain;
    }


    /**
     * Ajouter un maillon
     */
    void add()
    {
    	int		tmp_index = 0;
            Chain	*tmp_chain = this;

            while (tmp_chain->_m_next != NULL)
    	{
                    tmp_chain = tmp_chain->_m_next;
    		tmp_index++;
    	}

            tmp_chain->_m_next = new Chain();
            tmp_chain->_m_next->_m_previous = tmp_chain;
    }


    /**
     * Définir le contenu du maillon courant
     * @param in_new_object
     */
    void setData(T *in_new_object)
    {
    	_m_data = in_new_object;
    }


    /**
     * Définir le contenu du maillon é une certaine position
     * @param in_new_object
     */
    void setDataAtIndex(T *in_new_object, int in_index)
    {
            Chain	*tmp_chain = chainAtIndex(in_index);

            if (tmp_chain != NULL)	tmp_chain->setData(in_new_object);
    }


    /**
     * Récupérer le contenu du maillon courant
     * @return
     */
    T* getData()
    {
    	return _m_data;
    }


    /**
     * Récupérer le contenu du maillon é une certaine position
     * @return
     */
    T* getDataAtIndex(int in_index)
    {
            Chain	*tmp_chain = chainAtIndex(in_index);

            if (tmp_chain != NULL)	return tmp_chain->getData();

            return NULL;
    }


    /**
     * Maillon suivant
     * @return
     */
    Chain<T>* next()
    {
    	return _m_next;
    }


    /**
     * Maillon précédent
     * @return
     */
    Chain<T>* previous()
    {
    	return _m_previous;
    }

};


#endif //CHAIN_H

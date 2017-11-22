/*****************************************************************************
Copyright (C) 2010 Emmanuel Jorge ejorge@free.fr

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

#ifndef RECTANGLE3DITERATOR_H
#define RECTANGLE3DITERATOR_H

/**
 * Classe de gestion du chemin d'un Rectangle3D.
 */
class Rectangle3DIterator : public PathIterator
{
public:
    /**
     * Le rectangle3D
     */
    Rectangle3D 	m_losange;

    /**
     * Transformation affine
     */
    AffineTransform m_affine;

    /**
     * Index du segment courant du chemin
     */
    int 			m_index;


    /**
     * Constructeur
     * @param in_losange	le Rectangle3D
     * @param in_at		transformation affine
     */
    Rectangle3DIterator(Rectangle3D in_losange, AffineTransform in_at);


    /**
     * Retourn� la r�gle de pliage � l'int�rieur du chemin
     * @see #WIND_EVEN_ODD
     * @see #WIND_NON_ZERO
     */
    int getWindingRule();

    /**
     * Est-ce la fin du chemin ?
     * @return true si il y a d'autres points dans le chemin
     */
    bool isDone();
    /**
     * Segment suivant du chemin
     */

    void next();
    /**
     * Renvoie les coordonn�es et type du segment courant
     * @param Un tableaux de 6 coordonn�es doit �tre pass� en param�tres<br>
     * pour enregistrer les coordonn�es des points.<br>
     * Chaque point est enregistr� sous forme de paire de coordonn�es :<br>
     * <ul><li>les types SEG_MOVETO et SEG_LINETO retournent un seul point</li>
     * <li>SEG_QUADTO retourne deux points</li>
     * <li>SEG_CUBICTO retourne 3 points</li>
     * <li>SEG_CLOSE ne retourne aucun point</li></ul>
     * @return le type renvoy� :<br>
     * SEG_MOVETO, SEG_LINETO, SEG_QUADTO, SEG_CUBICTO, ou SEG_CLOSE.
     */
    int currentSegment(float in_coords[]);


    /**
     * Renvoie les coordonn�es et type du segment courant
     * @param Un tableaux de 6 coordonn�es doit �tre pass� en param�tres<br>
     * pour enregistrer les coordonn�es des points.<br>
     * Chaque point est enregistr� sous forme de paire de coordonn�es :<br>
     * <ul><li>les types SEG_MOVETO et SEG_LINETO retournent un seul point</li>
     * <li>SEG_QUADTO retourne deux points</li>
     * <li>SEG_CUBICTO retourne 3 points</li>
     * <li>SEG_CLOSE ne retourne aucun point</li></ul>
     * @return le type renvoy� :<br>
     * SEG_MOVETO, SEG_LINETO, SEG_QUADTO, SEG_CUBICTO, ou SEG_CLOSE.
     */
    int currentSegment(double in_coords[]);

};

#endif //RECTANGLE3DITERATOR_H

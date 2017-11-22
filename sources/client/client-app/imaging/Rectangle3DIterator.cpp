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

#include "Rectangle3DIterator.h"

/**
 * Constructeur
 * @param in_losange	le Rectangle3D
 * @param in_at		transformation affine
 */
Rectangle3DIterator::Rectangle3DIterator(Rectangle3D in_losange, AffineTransform in_at)
{
	m_losange = in_losange;
	m_affine = in_at;
}


/**
 * Retourn� la r�gle de pliage � l'int�rieur du chemin
 * @see #WIND_EVEN_ODD
 * @see #WIND_NON_ZERO
 */
int Rectangle3DIterator::getWindingRule()
{
	return WIND_NON_ZERO;
}


/**
 * Est-ce la fin du chemin ?
 * @return true si il y a d'autres points dans le chemin
 */
bool Rectangle3DIterator::isDone()
{
	return m_index > 4;
}

/**
 * Segment suivant du chemin
 */
void Rectangle3DIterator::next()
{
	m_index++;
}

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
int Rectangle3DIterator::currentSegment(float in_coords[])
{
	if (isDone())
	{
		throw new NoSuchElementException("quadrilater iterator out of bounds");
	}
	if (m_index == 4)
	{
		return SEG_CLOSE;
	}

	in_coords[0] = (float)m_losange.m_coords[m_index][0];
	in_coords[1] = (float)m_losange.m_coords[m_index][1];

	if (m_affine != null)
	{
		m_affine.transform(in_coords, 0, in_coords, 0, 1);
	}

	return (m_index == 0 ? SEG_MOVETO : SEG_LINETO);
}


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
int Rectangle3DIterator::currentSegment(double in_coords[])
{
	if (isDone())
	{
		throw new NoSuchElementException("Rectangle3D iterator out of bounds");
	}

	if (m_index == 4)
	{
		return SEG_CLOSE;
	}

	in_coords[0] = m_losange.m_coords[m_index][0];
	in_coords[1] = m_losange.m_coords[m_index][1];

	if (m_affine != null) {
		m_affine.transform(in_coords, 0, in_coords, 0, 1);
	}
	return (m_index == 0 ? SEG_MOVETO : SEG_LINETO);
}


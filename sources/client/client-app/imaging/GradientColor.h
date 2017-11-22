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

#ifndef GRADIENTCOLOR_H
#define GRADIENTCOLOR_H

#include <QColor>
#include <vector>

using namespace std;

/**
 * Gestion d'un dégradé de couleurs
 */
class GradientColor : public QColor
{
private:
	/* Pas(step) de saturation */
	int			m_step_red;
	int			m_step_blue;
	int			m_step_green;

	/* Index du dégradé courant */
	int			m_current_gradation;

	vector<QColor>		_m_colors_array;

	void _stepsForColor(QColor in_color);

	void _init();

protected:
	int		m_total_gradation;

public:


	/**
	 * Constructeur
	 * @param in_start_color : couleur de départ
	 */
	GradientColor(QColor in_start_color);


	/**
	 * Constructeur
	 * @param in_colors_array : tableau de couleurs
	 */
	GradientColor(vector<QColor> in_colors_array);


	/**
	 * Constructeur
	 * @param in_start_color : couleur de départ
	 * @param in_gradation : nombre de couleurs
	 */
	GradientColor(QColor in_start_color, int in_gradation);

	/**
	 * Constructeur
	 * @param in_colors_array : tableau de couleurs
	 * @param in_gradation : nombre de couleurs
	 */
	GradientColor(vector<QColor> in_colors_array, int in_gradation);

	/**
	 * Couleur dégradée suivante
	 * @return
	 */
	QColor nextColor();


	/**
	 * Couleur dégradée précédente
	 * @return
	 */
	QColor previousColor();


	/**
	 * Couleur dégradée é la niméme position
	 * @return
	 */
	QColor colorAtIndex(int in_color_index);


	/**
	 * Définir le nombre de couleurs dégradées
	 * @param in_gradation : nombre de couleurs
	 */
	void setGradation(int in_gradation);

};

#endif //GRADIENTCOLOR_H

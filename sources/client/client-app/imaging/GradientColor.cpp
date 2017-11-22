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

#include "GradientColor.h"

/**
 * Constructeur
 * @param in_start_color : couleur de départ
 */
GradientColor::GradientColor(QColor in_start_color) : QColor(in_start_color.rgba())
{
	_init();
}


/**
 * Constructeur
 * @param in_colors_array : tableau de couleurs
 */
GradientColor::GradientColor(vector<QColor> in_colors_array) : QColor(in_colors_array[0].rgba())
{
	_init();
	_m_colors_array = in_colors_array;
}


/**
 * Constructeur
 * @param in_start_color : couleur de départ
 * @param in_gradation : nombre de couleurs
 */
GradientColor::GradientColor(QColor in_start_color, int in_gradation) : QColor(in_start_color.rgba())
{
	_init();
	setGradation(in_gradation);
}


/**
 * Constructeur
 * @param in_colors_array : tableau de couleurs
 * @param in_gradation : nombre de couleurs
 */
GradientColor::GradientColor(vector<QColor> in_colors_array, int in_gradation) : QColor(in_colors_array[0].rgba())
{
	_init();
	_m_colors_array = in_colors_array;
	
	setGradation(in_gradation);
}

void GradientColor::_init()
{
	m_step_red = 0;
	m_step_blue = 0;
	m_step_green = 0;

	/* Index du dégradé courant */
	m_current_gradation = 0;

	m_total_gradation = 0;
}


/**
 * Couleur dégradée suivante
 * @return
 */
QColor GradientColor::nextColor()
{
	return colorAtIndex(++m_current_gradation);
}


/**
 * Couleur dégradée précédente
 * @return
 */
QColor GradientColor::previousColor()
{
	return colorAtIndex(--m_current_gradation);
}


/**
 * Couleur dégradée é la niméme position
 * @return
 */
QColor GradientColor::colorAtIndex(int in_color_index)
{
	/* Composantes de la couleur */
	int tmp_red;
	int tmp_green;
	int tmp_blue;
	int	tmp_index;
	
	if (_m_colors_array.size() > 0 && in_color_index > 0)
	{
		if (_m_colors_array.size() > (uint)in_color_index)
		{
			return _m_colors_array[in_color_index];
		}
		else
		{
			m_total_gradation = in_color_index / _m_colors_array.size() + 1;
			tmp_index = in_color_index % _m_colors_array.size();
			tmp_red = _m_colors_array[tmp_index].red();
			tmp_green = _m_colors_array[tmp_index].green();
			tmp_blue = _m_colors_array[tmp_index].blue();

			_stepsForColor(_m_colors_array[tmp_index]);
			
			return QColor(tmp_red + m_step_red * (m_total_gradation-1),
					tmp_green + m_step_green * (m_total_gradation-1),
					tmp_blue + m_step_blue * (m_total_gradation-1));
		}

	}
	else
	{
		tmp_red = red();
		tmp_green = green();
		tmp_blue = blue();

		return QColor(tmp_red + m_step_red * in_color_index,
				tmp_green + m_step_green * in_color_index,
				tmp_blue + m_step_blue * in_color_index);
	}

}


/**
 * Définir le nombre de couleurs dégradées
 * @param in_gradation : nombre de couleurs
 */
void GradientColor::setGradation(int in_gradation)
{
	/* Composantes de la couleur */
	int tmp_red = red();
	int tmp_green = green();
	int tmp_blue = blue();

	if (in_gradation > 0)
	{
		m_total_gradation = in_gradation;
		/* 
		 * Calcul des pas rouge, vert, bleu pour la saturation.
		 * Cela permet de donner un effet de dégradé de couleurs.
		 * */
		if (tmp_red >= tmp_green)
		{
			if (tmp_red >= tmp_blue)
			{
				m_step_red = 0;
				m_step_green = (tmp_red - tmp_green) / m_total_gradation;
				m_step_blue = (tmp_red - tmp_blue) / m_total_gradation;
			}
			else
			{
				m_step_blue = 0;
				m_step_green = (tmp_blue - tmp_green) / m_total_gradation;
				m_step_red = (tmp_blue - tmp_red) / m_total_gradation;
			}
		}
		else if (tmp_green >= tmp_blue)
		{
			m_step_green = 0;
			m_step_red = (tmp_green - tmp_red) / m_total_gradation;
			m_step_blue = (tmp_green - tmp_blue) / m_total_gradation;
		}
		else
		{
			m_step_blue = 0;
			m_step_green = (tmp_blue - tmp_green) / m_total_gradation;
			m_step_red = (tmp_blue - tmp_red) / m_total_gradation;
		}
	}
}


void GradientColor::_stepsForColor(QColor in_color)
{
	/* Composantes de la couleur */
	int tmp_red = in_color.red();
	int tmp_green = in_color.green();
	int tmp_blue = in_color.blue();

	/*
	 * Calcul des pas rouge, vert, bleu pour la saturation.
	 * Cela permet de donner un effet de dégradé de couleurs.
	 * */
	if (tmp_red >= tmp_green)
	{
		if (tmp_red >= tmp_blue)
		{
			m_step_red = 0;
			m_step_green = (tmp_red - tmp_green) / m_total_gradation;
			m_step_blue = (tmp_red - tmp_blue) / m_total_gradation;
		}
		else
		{
			m_step_blue = 0;
			m_step_green = (tmp_blue - tmp_green) / m_total_gradation;
			m_step_red = (tmp_blue - tmp_red) / m_total_gradation;
		}
	}
	else if (tmp_green >= tmp_blue)
	{
		m_step_green = 0;
		m_step_red = (tmp_green - tmp_red) / m_total_gradation;
		m_step_blue = (tmp_green - tmp_blue) / m_total_gradation;
	}
	else
	{
		m_step_blue = 0;
		m_step_green = (tmp_blue - tmp_green) / m_total_gradation;
		m_step_red = (tmp_blue - tmp_red) / m_total_gradation;
	}
}


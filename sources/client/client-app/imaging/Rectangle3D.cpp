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

#include "Rectangle3D.h"
#include <cmath>

const double PI = 3.14159265358979;
const double DEG = 180.0/PI;
const double RAD = PI/180.0;

RoundedRect::RoundedRect(double in_x, double in_y, double in_width, double in_height, double in_arc_width, double in_arc_height)
{
	_init(in_x, in_y, in_width, in_height, in_arc_width, in_arc_height);
}

void RoundedRect::_init(double in_x, double in_y, double in_width, double in_height, double /* in_arc_width */, double /* in_arc_height */)
{

	m_coords[0][0] = in_x;
	m_coords[0][1] = in_y;
	m_coords[1][0] = in_x + in_height;
	m_coords[1][1] = in_y + in_height;
	m_coords[2][0] = m_coords[1][0];
	m_coords[2][1] = m_coords[1][1];
	m_coords[3][0] = in_x;
	m_coords[3][1] = in_y;

	_m_width = in_width;
	_m_height = in_height;
}

QPainterPath RoundedRect::path ()
{
	QPainterPath	tmp_path = QPainterPath(QPointF(m_coords[0][0], m_coords[0][1]));

	tmp_path.lineTo(m_coords[1][0], m_coords[1][1]);
	tmp_path.lineTo(m_coords[2][0], m_coords[2][1]);
	tmp_path.lineTo(m_coords[3][0], m_coords[3][1]);

	return tmp_path;
}

/**
 * Constructeur
 * @param in_x : abcisse du coin gauche supérieur
 * @param in_y : ordonnée du coin gauche supérieur
 * @param in_width : largeur
 * @param in_height : profondeur
 * @param in_x_incline : inclinaison horizontale
 * @param in_y_incline : inclinaison verticale
 * @param in_z_incline : inclinaison azimutale
 */
Rectangle3D::Rectangle3D(double in_x, double in_y, double in_width, double in_height, double in_x_incline, double in_y_incline, double in_z_incline)
{
	_init(in_x, in_y, in_width, in_height, in_x_incline, in_y_incline, in_z_incline);
}


void Rectangle3D::_init(double in_x, double in_y, double in_width, double in_height, double in_x_incline, double in_y_incline, double in_z_incline)
{
	double tmp_cos_z = in_width * cos(in_z_incline*RAD);
	double tmp_sin_z = in_width * sin(in_z_incline*RAD);

	m_coords[0][0] = in_x;
	m_coords[0][1] = in_y;
	m_coords[1][0] = in_x + in_height * sin(in_y_incline*RAD);
	m_coords[1][1] = in_y + in_height * sin(in_x_incline*RAD);
	m_coords[2][0] = m_coords[1][0] + tmp_cos_z;
	m_coords[2][1] = m_coords[1][1] + tmp_sin_z;
	m_coords[3][0] = in_x + tmp_cos_z;
	m_coords[3][1] = in_y + tmp_sin_z;

	_m_width = in_width;
	_m_height = in_height;
}


/**
 * Constructeur
 * @param in_rect : rectangle
 * @param in_x_incline : inclinaison horizontale
 * @param in_y_incline : inclinaison verticale
 * @param in_z_incline : inclinaison azimutale
 */
Rectangle3D::Rectangle3D(QRect in_rect, double in_x_incline, double in_y_incline, double in_z_incline)
{
	_init(in_rect.x(), in_rect.y(), in_rect.width(), in_rect.height(), in_x_incline, in_y_incline, in_z_incline);
}


/**
 * Le losange contient-il le point ?
 * @param in_x	abcisse du point
 * @param in_y	ordonnée du point
 */
bool Rectangle3D::contains(double in_x, double in_y)
{
	double tmp_m = 0;
	double tmp_p = 0;

	if (in_y < m_coords[0][1] || in_y > m_coords[1][1]) return false;

	tmp_m = (m_coords[0][1] - m_coords[1][1]) / (m_coords[0][0] - m_coords[1][0]);
	tmp_p = m_coords[0][1] - tmp_m * m_coords[0][0];
	if (in_y < tmp_m * in_x + tmp_p) return false;

	tmp_m = (m_coords[2][1] - m_coords[3][1]) / (m_coords[2][0] - m_coords[3][0]);
	tmp_p = m_coords[2][1] - tmp_m * m_coords[2][0];
	if (in_y > tmp_m * in_x + tmp_p) return false;

	return true;
}


/**
 * Le losange contient-il le segment ?
 * @param in_x	abcisse du point origine
 * @param in_y	ordonnée du point origine
 * @param in_w	largeur relative
 * @param in_h	hauteur relative
 */
bool Rectangle3D::contains(double in_x, double in_y, double in_w, double in_h)
{
	return contains(in_x, in_y)
		&& contains(in_x + in_w, in_y)
		&& contains(in_x, in_y + in_h)
		&& contains(in_x + in_w, in_y + in_h);
}


/**
 * Le segment coupe-t'il le losange ?
 * @param in_x	abcisse du point origine
 * @param in_y	ordonnée du point origine
 * @param in_w	largeur relative
 * @param in_h	hauteur relative
 */
bool Rectangle3D::intersects(double in_x, double in_y, double in_w, double in_h)
{
	return contains(in_x, in_y)
		|| contains(in_x + in_w, in_y)
		|| contains(in_x, in_y + in_h)
		|| contains(in_x + in_w, in_y + in_h);
}


/**
 * Le losange contient-il le point ?
 * @param in_p	le point
 */
bool Rectangle3D::contains(QPointF in_p)
{
	return contains(in_p.x(), in_p.y());
}


QPainterPath Rectangle3D::path ()
{
	QPainterPath	tmp_path = QPainterPath(QPointF(m_coords[0][0], m_coords[0][1]));

	tmp_path.lineTo(m_coords[1][0], m_coords[1][1]);
	tmp_path.lineTo(m_coords[2][0], m_coords[2][1]);
	tmp_path.lineTo(m_coords[3][0], m_coords[3][1]);

	return tmp_path;
}

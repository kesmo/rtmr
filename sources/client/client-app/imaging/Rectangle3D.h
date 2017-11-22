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

#ifndef RECTANGLE_H
#define RECTANGLE_H

#include <QGraphicsItem>
#include <QRect>
#include <QPoint>
#include <QPainter>
#include <QPainterPath>

class RoundedRect
{
public:
    /**
     * Coordonnées des 4 points du losange
     */
    double m_coords[4][2];


	RoundedRect(double in_x, double in_y, double in_width, double in_height, double in_arc_width, double in_arc_height);

    void _init(double in_x, double in_y, double in_width, double in_height, double in_arc_width, double in_arc_height);

    QPainterPath path ();

private:
    /* Largeur et hauteur */
    double _m_width;
    double _m_height;
};


/**
 * Classe de gestion d'un Rectangle3D.
 * Un rectangle3D est un plan rectangulaire fermé dans un systéme é 3 dimensions.<br>
 * Sa représentation graphique en 2D n'est rien d'autre qu'un losange :<br>
 *              /|<br>
 *             / |<br>
 *            /  |<br>
 *           |   |<br>
 *           |  /<br>
 *           | /<br>
 *           |/<br>
 * De plus, elle dépend de sa position et sa taille (longueur et largeur)<br>
 * dans le systéme 3D des axes x,y et z ainsi que de l'inclinaison de ces axes.
 */
class Rectangle3D
{
public:
    /**
     * Coordonnées des 4 points du losange
     */
    double m_coords[4][2];

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
    Rectangle3D(double in_x, double in_y, double in_width, double in_height, double in_x_incline, double in_y_incline, double in_z_incline);

    void _init(double in_x, double in_y, double in_width, double in_height, double in_x_incline, double in_y_incline, double in_z_incline);

    /**
     * Constructeur
     * @param in_rect : rectangle
     * @param in_x_incline : inclinaison horizontale
     * @param in_y_incline : inclinaison verticale
     * @param in_z_incline : inclinaison azimutale
     */
    Rectangle3D(QRect in_rect, double in_x_incline, double in_y_incline, double in_z_incline);

    /**
     * Le losange contient-il le point ?
     * @param in_x	abcisse du point
     * @param in_y	ordonnée du point
     */
    bool contains(double in_x, double in_y);


    /**
     * Le losange contient-il le segment ?
     * @param in_x	abcisse du point origine
     * @param in_y	ordonnée du point origine
     * @param in_w	largeur relative
     * @param in_h	hauteur relative
     */
    bool contains(double in_x, double in_y, double in_w, double in_h);


    /**
     * Le segment coupe-t'il le losange ?
     * @param in_x	abcisse du point origine
     * @param in_y	ordonnée du point origine
     * @param in_w	largeur relative
     * @param in_h	hauteur relative
     */
    bool intersects(double in_x, double in_y, double in_w, double in_h);


    /**
     * Le losange contient-il le point ?
     * @param in_p	le point
     */
    bool contains(QPointF in_p);


    /**
     * Le rectangle passé en paramétre intersecte-t'il le Rectangle3D
     * @param in_rect	rectangle
     */
    bool intersects(QRect in_rect);

    QPainterPath path ();

private:
    /* Largeur et hauteur */
    double _m_width;
    double _m_height;

};

#endif //RECTANGLE3D_H

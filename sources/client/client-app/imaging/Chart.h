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

#ifndef CHART_H
#define CHART_H

#include <QColor>
#include <QFont>
#include <QString>
#include <QtGui>

#include "GraphicImage.h"
#include "objects/DataObject.h"
#include "objects/TrinomeArray.h"
#include "Chain.h"


#include <vector>
#include <cmath>

using namespace std;

/**
 * Classe de gestion de graphiques statistiques.<br>
 * Différents types de graphiques sont disponibles :
 * <ul><li>histogrammes</li>
 * <li>lignes polygonales</li>
 * <li>camemberts</li></ul><br>
 * Les fonctions fournies permettent la définition (setter) et la récupération (getter) des paramétres du graphique.<br>
 * Elles doivent étre utilisées afin de définir les différents aspects du graphique dans un WOComponent.<br>
 * ----------------------------------------------------------------<br>
 * example.java (définir un attribut de classe de type Chart): <br>
 * ----------------------------------------------------------------<br>
 * public class example extends WOComponent<br>
 * {<br>
 * 	[...]<br>
 * 	public Chart m_graphique = new Chart();<br>
 * 	[...]<br>
 * }<br>
 * ---------------------------------------------------------------<br>
 * example.wod (associer des bindings aux propriétés du graphique): <br>
 * ---------------------------------------------------------------<br>
 * [...]<br>
 * Image1: WOImage {<br>
 *		width = m_graphique.imageWidth;	//Appels implicites de setImageWidth et getImageWidth<br>
 *		height = m_graphique.imageHeight; // Appels implicites de setImageHeight et getImageHeight<br>
 *		mimeType = m_graphique.mimeType; // Appels implicites de setMimeType et getMimeType<br>
 *		data = m_graphique.imageData; // Appels implicites de getImageData<br>
 * }<br>
 * [...]<br>
 * <hr>
 * Pour définir les données é dessiner, utiliser la fonction {@link #setData},<br>
 * oé le paramétre TrinomeArray n'est rien d'autres qu'un tableau de données.<br>
 * Chaque élément du tableau (nommé Trinome) est constitué de 2 ou 3 valeurs qui vont définir les suites de valeurs X, Y et Z du graphique.<br>
 * Exemple d'une série de binome : {{"janvier", 10.5}, {"Février", 12}, {"Mars", 8}, {"Mai", 9.5}}<br>
 * Exemple d'une série de trinome : {{"janvier", 15.5, "David"}, {"Février", 14, "David"}, {"janvier", 11, "Eric"}, {"Février", 12, "Eric"}}<br>
 * Le traitement des valeurs consiste en l'ordonnancement des données par catégorie X, Y et Z.<br>
 * Dans les exemples ci-dessus les catégories définies sont :<br>
 * Catégorie X d'une série de binome : {"janvier", "Février", "Mars", "Mai"}<br>
 * Catégorie Y d'une série de binome : {10.5, 12, 8, 9.5}<br>
 * Catégorie X d'une série de trinome : {"janvier", "Février"}<br>
 * Catégorie Y d'une série de trinome : {15.5, 14, 11, 12}<br>
 * Catégorie Z d'une série de trinome : {"David", "Eric"}<br>
 * Le tableau passé en paramétre ne doit pas forcément étre ordonné. Par contre les valeurs d'une méme catégorie,<br>
 * doivent étre de méme type de données (Integer ou String ou Double...etc).<br>
 * Pour chaque catégorie on peut définir la légende et la couleur de dessin :
 * <ul><li>pour la légende utiliser les fonctions {@link #setXLegend}, {@link #setYLegend} et {@link #setZLegend}</li>
 * <li>pour les couleurs utiliser les fonctions {@link #setGraphColor}, {@link #setGraphColors}</li></ul><br>
 * D'autres fonctions et propriétés permettent de modifier le rendu en fonction du type de graphique :
 * <ul><li>{@link #m_raised_histogram_indic permet de dessiner un histogramme en 3D</li>
 * <li>{@link #setGraphOpacity} pour un aspect transparent des graphiques 3D (histogramme 3D et fromage)</li>
 * <li>{@link #setCheesePortionSpace} pour espacer les différentes portion d'un fromage</li>
 * <li>{@link #setInclineX} et {@link #setInclineY} pour modifier la perspective (histogramme 3D et fromage)</li>
 * <li>{@link #setTitle} pour ajouter un titre au graphique</li>
 * <li>...etc</li></ul>
 * */

class Chart : public GraphicImage
{
public:
/*-----------------------------------------------------------------
  				CONSTANTES ET VARIABLES DE CLASSE
------------------------------------------------------------------*/
	/**
	 * Largeur du graphique par defaut
	 */
	static const int MAX_WIDTH = 800;

	/**
	 * Hauteur du graphique par defaut
	 */
	static const int MAX_HEIGHT = 600;

	/**
	 * Largeur du graphique par defaut
	 */
	static const int DEFAULT_WIDTH = 480;

	/**
	 * Hauteur du graphique par defaut
	 */
	static const int DEFAULT_HEIGHT = 360;

	/**
	 * Largeur de la bordure gauche par defaut
	 */
	static const int DEFAULT_BORDER_LEFT = 40;

	/**
	 * Largeur de la bordure droite par defaut
	 */
	static const int DEFAULT_BORDER_RIGHT = 40;

	/**
	 * Largeur de la bordure haute par defaut
	 */
	static const int DEFAULT_BORDER_TOP = 40;

	/**
	 * Largeur de la bordure basse par defaut
	 */
	static const int DEFAULT_BORDER_BOTTOM = 40;

	/**
	 * Taille de la graduation horizontale par defaut
	 */
	static const int DEFAULT_X_STEP = 10;

	/**
	 * Taille de la graduation verticale par defaut
	 */
	static const int DEFAULT_Y_STEP = 10;

	/**
	 * Graphique en histogramme (par defaut)
	 */
	static const int GRAPH_TYPE_HISTOGRAM = 1;

	/**
	 * Graphique en polygone
	 */
	static const int GRAPH_TYPE_POLYGON = 2;

	/**
	 * Graphique en fromage
	 */
	static const int GRAPH_TYPE_CHEESE = 3;

	enum GraphType {
		Histogramme = GRAPH_TYPE_HISTOGRAM,
		Polygone = GRAPH_TYPE_POLYGON,
		Camembert = GRAPH_TYPE_CHEESE
	};


	/**
	 * Alignement du texte au centre (hor. et ver.)
	 */
	static const int TITLE_ALIGN_CENTER = 0;

	/**
	 * Alignement du texte é gauche (hor.)
	 */
	static const int TITLE_ALIGN_LEFT = 1;

	/**
	 * Alignement du texte é droite (hor.)
	 */
	static const int TITLE_ALIGN_RIGTH = 2;

	/**
	 * Alignement du texte en haut (ver.)
	 */
	static const int TITLE_ALIGN_TOP = 3;

	/**
	 * Alignement du texte en bas (ver.)
	 */
	static const int TITLE_ALIGN_BOTTOM = 4;

	/**
	 * Position de la légende à gauche
	 */
	static const int LEGEND_LEFT = 1;

	/**
	 * Position de la légende à droite
	 */
	static const int LEGEND_RIGTH = 2;

	/**
	 * Position de la légende en haut
	 */
	static const int LEGEND_TOP = 3;

	/**
	 * Position de la légende en bas
	 */
	static const int LEGEND_BOTTOM = 4;


	/**
	 * Pas de graduations en seconde
	 */
        static const unsigned long SECOND_STEP = 1000;

	/**
	 * Pas de graduations en minute
	 */
        static const unsigned long MINUTE_STEP = SECOND_STEP * 60;

	/**
	 * Pas de graduations en heure
	 */
        static const unsigned long HOUR_STEP = MINUTE_STEP * 60;

	/**
	 * Pas de graduations en jour
	 */
        static const unsigned long DAY_STEP = HOUR_STEP * 24;

	/**
	 * Pas de graduations en semaine
	 */
	static const unsigned long WEEK_STEP = DAY_STEP * 7;

	/**
	 * Pas de graduations en mois
	 */
	static const unsigned long MONTH_STEP = DAY_STEP * 30;

	/**
	 * Pas de graduations en année
	 */
        static const unsigned long YEAR_STEP = DAY_STEP / SECOND_STEP * 365;

/*-----------------------------------------------------------------
						PROPRIETES PUBLIQUES
------------------------------------------------------------------*/
	/**
	 * Indicateur d'affichage de l'histogramme en relief
	 **/
	bool m_raised_histogram_indic;

	/**
	 * Indicateur d'affichage d'un histogramme empilé
	 */
	bool m_stack_histogram_indic;

	/**
	 * Indicateur d'affichage de la légende
	 **/
	bool m_show_legend;

	/**
	 * Indicateur de calcul automatique des tailles des bordures.<br>
	 * Si il est égal é <code>true</code> les tailles des bordures autours du graphique sont calculées automatiquement<br>
	 * en fonction de la largeur du dessin et de l'affichage ou non des légendes.
	 **/
	bool m_auto_borders;

private:
/*-----------------------------------------------------------------
						PROPRIETES PRIVEES
------------------------------------------------------------------*/

	/* Type de graphique */
	int	_m_type;

	/* Tableau des données des séries X, Y et Z */
	TrinomeArray _m_trinomes_array;

	/* Listes des données distinctes (classes, catégories) des séries X et Z */
	vector<DataObject*> _m_x_categories;
	vector<DataObject*> _m_y_categories;
	vector<DataObject*> _m_z_categories;
	/* Tableaux des libellés é dessiner sur les axes pour les séries X, Y et Z */
	vector<QString> _m_x_labels;
	vector<QString> _m_y_labels;
	vector<QString> _m_z_labels;
	vector<DataObject*>	_m_legend_categories;

	/* Valeurs maximales des séries X, Y et Z */
        DataObject *_m_max_x_categories;
        DataObject *_m_max_y_categories;
        DataObject *_m_max_z_categories;

	/* Valeurs minimales des séries X, Y et Z */
        DataObject *_m_min_x_categories;
        DataObject *_m_min_y_categories;
        DataObject *_m_min_z_categories;

	/* Nombres de séries X, Y et Z */
	int _m_x_categories_count;
	int _m_y_categories_count;
	int _m_z_categories_count;

	/* Espace entre chaque graduation verticale et horizontale */
	double _m_x_step;
	double _m_y_step;

	/* Coordonnées des points correspondants aux données des séries X et Y */
	vector<double> _m_x_array;
	vector<double> _m_y_array;

	/* Tableau des mesures d'angles des portions dans un fromage */
	vector<double> _m_angles_array;

	/* Décalage des portions du fromage par rapport au centre */
	double _m_delta_cheese_portion;

	/* Origine du graphique dans l'image */
	double _m_graph_origin_x;
	double _m_graph_origin_y;

	/* Largeur et hauteur du graphique */
	double _m_graph_width;
	double _m_graph_height;

	/* Angle d'inclinaison vertical */
	double _m_y_incline;

	/* Angle d'inclinaison horizontal */
	double _m_x_incline;

	/* Indicateur de dessin des axes */
	bool _m_drawn_axes;

	/* Police d'affichage du graphique */
	QFont _m_graph_font;

	/* Taille de la police d'affichage du graphique */
	float _m_graph_font_size;

	/* Police d'affichage du titre */
	QFont _m_title_font;

	/* Titre du graphique */
	QString _m_title;

	/* Alignement horizontal du titre (centré par défaut) */
	int _m_graph_title_halign;

	/* Alignement vertical du titre (en haut par défaut) */
	int _m_graph_title_valign;

	/* Alignement de la légende (en bas par défaut) */
	int _m_graph_legend_position;

	/* Couleur du titre (orange par défaut) */
	QColor _m_graph_title_color;

	/* Tableau des couleurs utilisées */
	vector<QColor> _m_colors;

	/* Niveau de transparence du graphique (é 80% opaque par défaut) */
	int _m_graph_alpha_paint_color;

	/* Indique si le tracé du graphique dépend des abcisses
	 * lorsqu'il s'agit de données de type numérique ou date */
	bool _m_abciss_proportionnal_indic;

public:
/*-----------------------------------------------------------------
							METHODES
------------------------------------------------------------------*/

	/**
	* Constructeur par défaut.
	*/
	Chart();


    /**
    * Destructeur.
    */
    ~Chart();


    /**
	* Constructeur secondaire.
	*/
	Chart(int in_width, int in_height);


	/**
	 * Définir la légende de la série de données X
	 * @param in_x_objects tableau de libellés
	 */
	void setXLegend(vector<QString> in_x_objects);


	/**
	 * Définir la légende de la série de données Y
	 * @param in_y_objects tableau de libellés
	 */
	void setYLegend(vector<QString> in_y_objects);


	/**
	 * Définir la légende de la série de données Z
	 * @param in_z_objects tableau de libellés
	 */
	void setZLegend(vector<QString> in_z_objects);


	/**
	 * Définir les bornes et le nombre de graduations sur l'axe des abcisses
	 * @param in_min_x	valeur minimale en abcisse
	 * @param in_max_x	valeur maximale en abcisse
	 * @param in_x_count pas de graduations horizontales
	 */
        void setHorizontalGraduations(DataObject *in_min_x, DataObject *in_max_x, double in_x_step);


	/**
	 * Définir les bornes et le nombre de graduations sur l'axe des ordonnées
	 * @param in_min_y valeur minimale en ordonnée
	 * @param in_max_y valeur maximale en ordonnée
	 * @param in_y_count pas de graduations verticales
	 */
        void setVerticalGraduations(DataObject *in_min_y, DataObject *in_max_y, int in_y_step);

	/**
	 * Définir les bornes et le nombre de graduations sur l'axe des azimuts
	 * @param in_min_z valeur minimale en azimut
	 * @param in_max_z valeur maximale en azimut
	 * @param in_z_count pas de graduations azimutales
	 */
        void setAzimutalGraduations(DataObject *in_min_z, DataObject *in_max_z, int in_z_step);

	/**
	 * Définir le tableau des données
	 * @param in_trinomes_array
	 */
	void setData(const TrinomeArray& in_trinomes_array);

	void setType(Chart::GraphType in_graph_type);

protected:
        void _init();

	/**
	 * Initialiser les paramétres du graphique (coordonnées, bordures, légendes...etc) avant le dessin
	 */
	void initDrawing();

	/**
	 * Initialiser le contexte graphique
	 */
	void _initGraphicContext();

	/**
	 * Dessiner le cadre
	 */
	void drawAxis();

	/**
	 * Dessiner la légende
	 */
	void drawLegend();

	/**
	 * Déssiner le titre du graphique.
	 */
	void drawTitle();

	/**
	 * Dessiner l'histogramme
	 */
	void drawHistogram();

	/**
	 * Dessiner le polygone
	 */
	void drawPolygon();

	/**
	 * Dessiner une portion de fromage
	 * @param in_index
	 * @param in_start
	 * @param in_end
	 */
	void drawCheesePortion(int in_index, double in_start, double in_end);

	/**
	 * Dessiner la légende d'une portion du fromage
	 * @param in_index	index de la portion
	 */
	void drawCheesePortionLegend(int in_index);

	/**
	 * Trier les portions de fromage dans l'ordre oé elles doivent étre dessinées, afin d'éviter les chevauchements de dessins.
	 * @return chaine triée dont chaque maillon contient l'index de portion
	 */
    QList<Integer> orderedDrawingCheesePortions();

	/**
	 * Dessiner le fromage portion par portion.
	 */
	void drawCheese();


private:
	/**
	 * Calculer la taille des graduations horizontales et verticales
	 */
	void _calculateGraduations();

	/**
	 * Définir la taille des bords
	 * @param in_border_left taille du bord gauche
	 * @param in_border_right taille du bord droit
	 * @param in_border_top taille du bord supérieur
	 * @param in_border_bottom taille du bord inférieur
	 */
	void _setBorderSize(double in_border_left, double in_border_right, double in_border_top, double in_border_bottom);

	/**
	 * Définir la position et la taille du graphique dans l'image
	 * @param in_graph_x position horizontale
	 * @param in_graph_y position verticale
	 * @param in_graph_width largeur
	 * @param in_graph_height hauteur
	 */
	void _setGraphRect(double in_graph_x, double in_graph_y, double in_graph_width, double in_graph_height);

	/**
	 * Calcul des coordonnées des points du graphique<br>
	 * Utilisé pour un histogramme et un polygone
	 */
	void _calculateGraphPoints();

	/**
	 * Dessiner un losange
	 * @param in_x0
	 * @param in_y0
	 * @param in_x1
	 * @param in_y1
	 * @param in_thickness
	 */
	void _drawPortionSide(double in_x0, double in_y0, double in_x1, double in_y1, double in_thickness);


	/**
	 * Tracer les losanges de cété de la portion
	 * @param in_index
	 * @param in_start
	 * @param in_end
	 * @param in_draw_left
	 * @param in_draw_right
	 * @param in_center_x
	 * @param in_center_y
	 * @param in_cheese_thickness
	 * @param in_paint_color
	 */
	void _drawPortionSides(int in_index, double in_start, double in_end,
	        bool in_draw_left, bool in_draw_right,
	        double in_center_x, double in_center_y, double in_cheese_thickness,
                QBrush in_paint_color);

	/**
	 * Tracer un losange de cété de la portion
	 * @param in_x
	 * @param in_y
	 * @param in_center_x
	 * @param in_center_y
	 * @param in_cheese_thickness
	 * @param in_paint_color
	 */
        void _drawPortionSide(double in_x, double in_y, double in_center_x, double in_center_y, double in_cheese_thickness, QBrush in_paint_color);


public:
	/**
	 * Définir la taille des bords seulement si <code>m_auto_borders</code> est positionné é <code>false</code>
	 * @param in_border_left taille du bord gauche
	 * @param in_border_right taille du bord droit
	 * @param in_border_top taille du bord supérieur
	 * @param in_border_bottom taille du bord inférieur
	 */
	void setBorderSize(double in_border_left, double in_border_right, double in_border_top, double in_border_bottom);

	/**
	 * Effacer le contenu de l'image
	 */
	void clear ();


	/**
	 * Dessiner le graphe
	 */
	void draw();


	/**
	 * Dessiner le graphe d'aprés le type passé en paramétre
	 * @param in_graph_type : une des valeurs GRAPH_TYPE_HISTOGRAM, GRAPH_TYPE_POLYGON...etc
	 */
	void drawGraph (Chart::GraphType in_graph_type);


/*-----------------------------------------------------------------
						GETTERS ET SETTERS
------------------------------------------------------------------*/

    /**
     * Renvoie la couleur de dessin du graphique au format html/javascript<br>
     * ex : #FF0000
     */
	QString getGraphColor();


	/**
     * Renvoie la couleur du titre au format html/javascript<br>
     * ex : #FF0000
     */
	QString getTitleColor();


    /**
     * Définir la couleur de dessin du graphique
     * @param	in_js_color	couleur au format html/javascript (ex : #FF0000)
     */
	void setGraphColor(QString in_js_color);


    /**
     * Définir la couleur du titre
     * @param	in_js_color	couleur au format html/javascript (ex : #FF0000)
     */
	void setTitleColor(QString in_js_color);


    /**
     * Définir les couleurs de dessin des catégories.
     * @param	in_colors	tableau d'objets de type <code>Color</code> pour les couleurs java,<br>
     * oé de type <code>String</code> pour les couleurs au format html/javascript
     */
	void setGraphColors(vector<QColor> in_colors);


	/**
     * Renvoie le taux d'opacité ( != transparence) du graphique.
     */
	int getGraphOpacity();


	/**
     * Définir le taux d'opacité ( != transparence) du graphique.
     * @param	in_alpha	taux d'opacité
     */
	void setGraphOpacity(int in_alpha);


    /**
     * Renvoie la largeur du graphique en nombre de pixels.
     */
	double getGraphWidth();


    /**
     * Renvoie la hauteur du graphique en nombre de pixels.
     */
	double getGraphHeight();


    /**
     * Renvoie l'inclinaison vertical du graphique en degrés celcius.
     */
	double getInclineY();


    /**
     * Définir l'inclinaison vertical du graphique en degrés celcius.
     * @param	in_incline	angle d'inclinaison vertical
     */
	void setInclineY(int in_incline) ;


    /**
     * Renvoie l'inclinaison horizontal du graphique en degrés celcius.
     */
	double getInclineX();


    /**
     * Définir l'inclinaison horizontal du graphique en degrés celcius.
     * @param	in_incline	angle d'inclinaison vertical
     */
	void setInclineX(int in_incline);


    /**
     * Renvoie la taille de la bordure é gauche du graphique exprimée en nombre de pixels.
     */
	double getBorderLeft();


    /**
     * Renvoie la taille de la bordure é droite du graphique exprimée en nombre de pixels.
     */
	double getBorderRight();


    /**
     * Renvoie la taille de la bordure en haut du graphique exprimée en nombre de pixels.
     */
	double getBorderTop();


    /**
     * Renvoie la taille de la bordure en bas du graphique exprimée en nombre de pixels.
     */
	double getBorderBottom();


    /**
     * Définir la taille de la bordure é gauche du graphique.
     * @param	in_size	taille exprimée en nombre de pixels
     */
	void setBorderLeft(double in_size);


    /**
     * Définir la taille de la bordure é droite du graphique.
     * @param	in_size	taille exprimée en nombre de pixels
     */
	void setBorderRight(double in_size);


    /**
     * Définir la taille de la bordure en haut du graphique.
     * @param	in_size	taille exprimée en nombre de pixels
     */
	void setBorderTop(double in_size);


    /**
     * Définir la taille de la bordure en bas du graphique.
     * @param	in_size	taille exprimée en nombre de pixels
     */
	void setBorderBottom(double in_size);


	/**
	 * Définir le titre.
	 * @param in_title	chaéne de caractéres
	 */
	void setTitle(QString in_title);


	/**
	 * Renvoie le titre du graphique.
	 */
	QString getTitle();


	/**
	 * Renvoie la police d'affichage du titre.
	 * @return	un objet de type
	 * @see java.awt.Font
	 */
	QFont getFontTitle();


	/**
	 * Définir la police d'affichage du titre.
	 * @param in_font	un objet de type
	 * @see java.awt.Font
	 */
	void setFontTitle(QFont in_font);


	/**
	 * Définir l'espacement entre les portions d'un fromage.
	 * @param in_delta	exprimée en nombre de pixels
	 */
	void setCheesePortionSpace(int in_delta);


	/**
	 * Renvoie l'espacement entre les portions d'un fromage.
	 */
	int getCheesePortionSpace();


	/**
	 * Renvoie le tableau des abcisses des points du graphique
	 * @return
	 */
	vector<double> getHorizontalCoords();


	/**
	 * Renvoie le tableau des ordonnées des points du graphique
	 * @return
	 */
	vector<double> getVerticalCoords();


	/**
	 * Définir la position de la légende
	 */
	void setLegendPosition(int in_legend_position);

};


#endif //CHART_H

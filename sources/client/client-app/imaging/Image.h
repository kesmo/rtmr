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

#ifndef IMAGE_H
#define IMAGE_H

#include <QPixmap>
#include <QPainter>
#include <QPaintDevice>
#include <QString>
#include <QColor>
#include <QFont>
#include <QFontDatabase>
#include <QImageWriter>
#include <QImageReader>
#include <QByteArray>
#include <QTextLayout>
#include <QRect>
#include <QBuffer>
#include "utilities.h"

const double PI = 3.14159265358979;
const double DEG = 180.0/PI;
const double RAD = PI/180.0;
const double QT_ANGLE_RATIO = 16;


/**
 * Sous-classer cette classe pour réaliser des traitements d'images.
 * Image regroupe des propriétés redondantes des images :
 * 		<ul><li>largeur et hauteur</li>
 * 		<li>couleur de fond</li>
 * 		<li>couleur de crayon</li>
 * 		<li>transparence</li>
 * 		<li>format d'image</li><ul>
 * Image offre les fonctionnalités de bases suivantes :
 * 		<ul><li>sauvegarder l'image sur disque</li>
 * 		<li>mettre en mémoire l'image dans le cache de l'application</li>
 * 		<li>optimiser le rendu soit pour la qualité soit pour la vitesse de traitement</li>
 */
class Image : public QObject
{
public:

	// --------------------------------------------------------------
	// VARIABLES
	// --------------------------------------------------------------


    
	/**
	* Constructeur secondaire.
	*/
	Image(double in_width, double in_height);
	
    ~Image();
	
	/**
	 * Dessiner l'image
	 */
	virtual void draw() = 0;
	

	/**
	 * Effacer le contenu de l'image
	 */
        virtual void clear ();
	
	/**
	 * Liste des polices systèmes disponibles
	 * @return
	 */
        QList<QString> availablesFonts();
	

	/**
	 * Renvoie l'objet image
	 * @return
	 */
        QPixmap* getImage ();
	
	
	/**
	 * Renvoie les données de l'image
	 * @return
	 */
        QByteArray getImageData();

	/**
	 * Sauvegarder l'image sur le disque
	 * @param in_image_filename : nom du fichier destination
	 * @throws Exception
	 */
        void saveAs(QString in_image_filename);

	

	/**
	 * Définir les dimensions de l'image
	 * @param in_width : largeur
	 * @param in_height : hauteur
	 */
	void setImageSize(double in_width, double in_height);
	
	
	/**
	 * Définir les couleurs de crayon et du fond
	 * @param in_pencil_color	couleur de dessin du graphique
	 * @param in_bg_color	couleur de fond
	 * @return <code>true</code> si les couleurs définies ne rentrent pas en conflit
	 */
        bool setColors(QColor in_pencil_color, QColor in_bg_color);
	
	/**
	 * Générer le dessin d'un texte selon une longueur maxi de pixels.<br>
	 * La longueur est calculée de gauche à droite horizontalement.
	 * @param in_str : texte à dessiner
	 * @param in_max_length : nb de pixels à ne pas dépasser (si négatif pas de limite)
	 * @return
	 */
        static QTextLayout* layoutFor(QPaintDevice *in_paint_device, QString in_str, QFont in_font, float in_max_length);
	
	/**
	 * Crée un objet de classe <code>Color</code> à partir de sa valeur javascript
	 * @param in_js_color : couleur javascript (i.e : #FF0000 => rouge)
	 * @return un objet de classe <code>Color</code>
	 */
        static QColor jsColorToJavaColor(QString in_js_color);

	
	/**
	 * Renvoie une couleur javascript à partir d'une couleur java
	 * @param in_color : couleur java (i.e : <code>Color.RED</code>)
	 * @return chaine representant la couleur au format "#xxxxxx"
	 */
        static QString javaColorToJsColor(QColor in_color);
	
	/**
	 * Calculer les coordonnées X et Y du centre d'un triangle de sommets A, B, C
	 * @param x0 : abcisse du point A
	 * @param y0 : ordonnée du point A
	 * @param x1 : abcisse du point B
	 * @param y1 : ordonnée du point B
	 * @param x2 : abcisse du point C
	 * @param y2 : ordonnée du point C
	 * @return
	 */
        static QPoint centerOf(double x0, double y0, double x1, double y1, double x2, double y2);
	
	
	/**
	 * Calculer les coordonnées du centre entre deux points A et B
	 * @param x0 : abcisse du point A
	 * @param y0 : ordonnée du point A
	 * @param x1 : abcisse du point B
	 * @param y1 : ordonnée du point B
	 * @return
	 */
        static QPoint centerOf(double x0, double y0, double x1, double y1);
	
	
	/**
	 * Calculer les coordonnées du point résultant de la translation du point A vers le point B,<br>
	 * et dépendant de <code>in_ratio</code>.
	 * @param x0 : abcisse du point A
	 * @param y0 : ordonnée du point A
	 * @param x1 : abcisse du point B
	 * @param y1 : ordonnée du point B
	 * @param in_ratio : le ratio de translation de A->B(ex : 0.5 pour obtenir les coordonnées du centre de A et B)
	 * @return
	 */
        static QPoint translateOf(double x0, double y0, double x1, double y1, double in_ratio);
	
	

		
	/**
	 * Récupérer la couleur de fond
	 * @return	chaîne définissant la couleur au format javascript (ex:#000000)
	 */
        QString getBackgroundColor();
	
	
	/**
	 * Définir la couleur de fond
	 * @param in_js_color	chaîne définissant la couleur au format javascript (ex:#000000)
	 */
        void setBackgroundColor(QString in_js_color);

	
	/**
	 * Récupérer la largeur de l'image
	 * @return
	 */
	double getImageWidth();
	
	
	/**
	 * Récupérer la hauteur de l'image
	 * @return
	 */
	double getImageHeight();
	
	
	/**
	 * Définir la largeur de l'image
	 * @return
	 */
	void setImageWidth(double in_width);
	
	
	/**
	 * Définir la hauteur de l'image
	 * @return
	 */
	void setImageHeight(double in_height);

	
	/**
	 * Indiquer si l'image NE peut PAS être transparente
	 * @return <code>true</code> ou <code>false</code>
	 */
	bool cantBeTransparent();
	
	
	/**
	 * L'image est-elle transparente ?
	 * @return	<code>true</code> ou <code>false</code>
	 */
	bool getTransparency();


	/**
	 * Définir la transparence de l'image
	 * @param in_transparency	indicateur <code>true</code> ou <code>false</code>
	 */
	void setTransparency(bool in_transparency);

	
	/**
	 * Améliorer la qualité du rendu de l'image (au détriment de la vitesse)
	 */
	void setRenderingForQuality();

	
	/**
	 * Améliorer la vitesse de rendu de l'image (au détriment de la qualité)
	 */
	void setRenderingForSpeed();
	
	
	/**
	 * Récupérer le format de l'image
	 * @return	chaîne de la forme "PNG"
	 */
        QString getFileFormat();
	
	
	/**
	 * Définir le format de l'image basé sur les formats disponibles
	 * @param in_file_format	chîne de la forme "PNG"
	 */
        void setFileFormat(QString in_file_format);


	/**
	 * Chercher un caractère dans un tableau
	 * @param in_search_str	caractère à chercher
	 * @param in_str_array	tableau de caractères
	 * @param in_start_index index de départ de la recherche
	 * @return index du caractère danbs le tableau ou -1 si le caractère n'a pas été trouvé.
	 */
        static int indexOfChar(char in_search_str, QByteArray in_str_array, int in_start_index = 0);


	/**
	 * Créer un sous-tableau
	 * @param in_char_array	tableau d'origine
	 * @param in_start_index	position de début de copie
	 * @param in_end_index	position de fin de copie
	 * @return	le sous-tableau
	 */
        static QByteArray subCharArray(QByteArray in_char_array, int in_start_index, int in_end_index);

protected:


	/* Objet image */
        QPixmap                          *_m_BufferedImage;
	
	/* Contexte graphique */
        QPainter			*_m_Graphics2D;

	/* Largeur et hauteur de l'image */
	double				_m_width;
	double				_m_height;

	/* Format de l'image */
        QString				_m_image_format_str;

	/* Couleur de fond */
        QColor					_m_background_Color;
	
	/* Couleur du crayon */
        QColor					_m_pencil_Color;
	
	/* Indicateur de transparence */
	bool				_m_transparency_indic;	
	
	/* Indicateur de rendu optimisé pour la vitesse */
	bool				_m_rendering_speed_indic;
	
	
        /**
         * Constructeur
         */
        Image();

        virtual void _init();

	/**
	 * Créer l'objet image
	 */
        virtual void _createImage ();

	/**
	 * Dessiner un rectangle 3D
	 * @param x
	 * @param y
	 * @param width
	 * @param height
	 * @param raised
	 */
	void draw3DRect(double x, double y, double width, double height, bool raised);


	/**
	 * 
	 * @param x
	 * @param y
	 * @param width
	 * @param height
	 */
	void fill3DRect(double x, double y, double width, double height, double in_x_incline, double in_y_incline);
	
	/**
	 * Initialiser le contexte graphique
	 */
	virtual void _initGraphicContext ();


};

#endif //IMAGE_H

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

#include "Image.h"


/**
* Constructeur
*/
Image::Image()
{
    _init();
}


/**
* Constructeur secondaire.
*/
Image::Image(double in_width, double in_height)
{
    _init();

    _m_width = in_width;
    _m_height = in_height;
}

Image::~Image()
{
    delete _m_Graphics2D;
    delete _m_BufferedImage;
}


void Image::_init()
{
    _m_image_format_str = "PNG";
    _m_background_Color = Qt::white;
    _m_pencil_Color = Qt::black;
    _m_transparency_indic = false;
    _m_BufferedImage = NULL;
    _m_Graphics2D = NULL;
}


/**
 * Créer l'objet image
 */
void Image::_createImage ()
{
    delete _m_Graphics2D;
    _m_Graphics2D = NULL;
    delete _m_BufferedImage;

    /* Créer l'image */
    _m_BufferedImage = new QPixmap(QSize((int)_m_width, (int)_m_height));

	/* Initialiser le contexte graphique */
	_initGraphicContext();
}


/**
 * Initialiser le contexte graphique
 */
void Image::_initGraphicContext ()
{	
    delete _m_Graphics2D;

    /* Créer un nouveau contexte graphique de travail */
        _m_Graphics2D = new QPainter(_m_BufferedImage);

        /* Optimisation du rendu */
	if (_m_rendering_speed_indic)
	    setRenderingForSpeed();
	else
	    setRenderingForQuality();
	
	/* Initialiser la couleur de fond */
        _m_background_Color = QColor(_m_background_Color.red(),
                        _m_background_Color.green(),
                        _m_background_Color.blue(),
			_m_transparency_indic ? 0x00 : 0xFF);
	
	/* Effacer avec la couleur de fond */
	clear();
}


/**
 * Effacer le contenu de l'image
 */
void Image::clear ()
{
    if (_m_transparency_indic)
        _m_Graphics2D->fillRect(0, 0, _m_width, _m_height, Qt::transparent);
    else
        _m_Graphics2D->fillRect(0, 0, _m_width, _m_height, _m_background_Color);
}


/**
 * Liste des polices systémes disponibles
 * @return
 */
QList<QString> Image::availablesFonts()
{
    return QFontDatabase().families();
}


/**
 * Renvoie l'objet image
 * @return
 */
QPixmap* Image::getImage ()
{
	return _m_BufferedImage;
}


/**
 * Renvoie les données de l'image
 * @return
 */
QByteArray Image::getImageData()
{
     QByteArray tmp_bytes_array;
     QBuffer    tmp_buffer(&tmp_bytes_array);

     tmp_buffer.open(QIODevice::WriteOnly);
     _m_BufferedImage->save(&tmp_buffer, _m_image_format_str.toStdString().c_str());
     tmp_buffer.close();

    return tmp_bytes_array;
}


/**
 * Sauvegarder l'image sur le disque
 * @param in_image_filename : nom du fichier destination
 * @throws Exception
 */
void Image::saveAs(QString in_image_filename)
{
    if (in_image_filename != NULL && in_image_filename.length() > 0)
    {
                if (in_image_filename.toLower().endsWith("." + _m_image_format_str.toLower()))
                        _m_BufferedImage->save(in_image_filename);
		    else
                        _m_BufferedImage->save(in_image_filename + "." + _m_image_format_str.toLower());

    }
}


	

/**
 * Définir les dimensions de l'image
 * @param in_width : largeur
 * @param in_height : hauteur
 */
void Image::setImageSize(double in_width, double in_height)
{
    _m_width = in_width;
    _m_height = in_height;

    _createImage();
}


/**
 * Définir les couleurs de crayon et du fond
 * @param in_pencil_color	couleur de dessin du graphique
 * @param in_bg_color	couleur de fond
 * @return <code>true</code> si les couleurs définies ne rentrent pas en conflit
 */
bool Image::setColors(QColor in_pencil_color, QColor in_bg_color)
{
        QColor tmp_pencil_color;
        QColor tmp_bg_color;
	
	/* Couleur du graphique */
        tmp_pencil_color = in_pencil_color;

	/* Couleur du fond */
        tmp_bg_color = in_bg_color;
	
        /* Vérification */
        if (!(tmp_pencil_color == tmp_bg_color))
	{
		/* Changer le fond */
                if (!(_m_background_Color == tmp_bg_color))
		{
		    _m_background_Color = tmp_bg_color;
			clear();
		}
		
		_m_pencil_Color = tmp_pencil_color;
		
		return true;
	}
	
	return false;
}


/**
 * Générer le dessin d'un texte selon une longueur maxi de pixels.<br>
 * La longueur est calculée de gauche é droite horizontalement.
 * @param in_str : texte é dessiner
 * @param in_max_length : nb de pixels é ne pas dépasser (si négatif pas de limite)
 * @return
 */
QTextLayout* Image::layoutFor(QPaintDevice *in_paint_device, QString in_str, QFont in_font, float in_max_length)
{
    QString			tmp_result_buff = in_str;
    QTextLayout		*tmp_str_layout = NULL;
    QRectF			tmp_text_rect;
    QByteArray			tmp_chars;
    	
        /* Créer un premier dessin du texte */
	tmp_str_layout = new QTextLayout(in_str, in_font, in_paint_device);
	if (in_max_length < 0) return tmp_str_layout;
	
	/* Calculer à quelle position du texte la longueur maxi tombe */
	tmp_text_rect = tmp_str_layout->boundingRect();

	while (tmp_text_rect.width() > in_max_length && tmp_result_buff.length() > 0)
	{
		tmp_result_buff = in_str.mid(0, tmp_result_buff.length() - 1);
		tmp_str_layout = new QTextLayout(tmp_result_buff, in_font, in_paint_device);
		tmp_text_rect = tmp_str_layout->boundingRect();
	}
	
	if (tmp_result_buff.length() > 0)
	{
		tmp_str_layout->beginLayout();
		tmp_str_layout->createLine();
		tmp_str_layout->endLayout();

		return tmp_str_layout;
	}
	else
		return NULL;
}


/**
 * Crée un objet de classe <code>Color</code> é partir de sa valeur javascript
 * @param in_js_color : couleur javascript (i.e : #FF0000 => rouge)
 * @return un objet de classe <code>Color</code>
 */
QColor Image::jsColorToJavaColor(QString in_js_color)
{
	int tmp_red_index = 0;
	int tmp_green_index = 0;
	int tmp_blue_index = 0;
	int	tmp_str_index = 0;
	bool tmp_ok;
	
	if (in_js_color.length() > 5)
	{
                if (in_js_color[0] == '#') tmp_str_index = 1;
		
                tmp_red_index = in_js_color.mid(tmp_str_index, 2).toInt(&tmp_ok, 16);
                if (tmp_ok)
                {
                    tmp_str_index+=2;
                    tmp_green_index = in_js_color.mid(tmp_str_index, 2).toInt(&tmp_ok, 16);
                    if (tmp_ok)
                    {
                        tmp_str_index+=2;
                        tmp_blue_index = in_js_color.mid(tmp_str_index, 2).toInt(&tmp_ok, 16);
                        if (tmp_ok)
                            return QColor(tmp_red_index, tmp_green_index, tmp_blue_index);
                    }
                }
	}
	
	return QColor();
}


/**
 * Renvoie une couleur javascript é partir d'une couleur java
 * @param in_color : couleur java (i.e : <code>Color.RED</code>)
 * @return chaine representant la couleur au format "#xxxxxx"
 */
QString Image::javaColorToJsColor(QColor in_color)
{
	QString		tmp_color_str;

        QString	tmp_red;
        QString	tmp_green;
        QString	tmp_blue;
	
        tmp_red.setNum(in_color.red(), 16);
        tmp_green.setNum(in_color.green(), 16);
        tmp_blue.setNum(in_color.blue(), 16);

        tmp_color_str = "#" + (tmp_red.length() == 1 ? "0" + tmp_red : tmp_red)
				+ (tmp_green.length() == 1 ? "0" + tmp_green : tmp_green)
				+ (tmp_blue.length() == 1 ? "0" + tmp_blue : tmp_blue);

	return tmp_color_str;
}


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
QPoint Image::centerOf(double x0, double y0, double x1, double y1, double x2, double y2)
{
    double	X = 0, Y = 0;
    double	x01 = x0 - (x0 -x1)/2, y01 = y0 - (y0 - y1)/2; // Centre du segment AB
    double	x12 = x1 - (x1 -x2)/2, y12 = y1 - (y1 - y2)/2; // Centre du segment BC
    
    /* Trajectoire de la bissectrice de AB de la forme y = m0 * x + p0 */
    double	m0 = (y12 - y0)/(x12 - x0);
    double	p0 = y0 - m0 * x0;
    
    /* Trajectoire de la bissectrice de AC de la forme y = m1 * x + p1 */
    double	m1 = (y01 - y2)/(x01 - x2);
    double	p1 = y2 - m1 * x2;
    
    /* L'intersection des deux bissectrices est le centre du triangle */
    X = (p1 - p0)/(m0 - m1);
    Y = m0 * X + p0;
        
    return QPoint(X, Y);
}


/**
 * Calculer les coordonnées du centre entre deux points A et B
 * @param x0 : abcisse du point A
 * @param y0 : ordonnée du point A
 * @param x1 : abcisse du point B
 * @param y1 : ordonnée du point B
 * @return
 */
QPoint Image::centerOf(double x0, double y0, double x1, double y1)
{
    return translateOf(x0, y0, x1, y1, 0.5);
}


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
QPoint Image::translateOf(double x0, double y0, double x1, double y1, double in_ratio)
{
    return QPoint(x0 - (x0 - x1) * in_ratio, y0 - (y0 - y1) * in_ratio);
}


/**
 * Dessiner un rectangle 3D
 * @param x
 * @param y
 * @param width
 * @param height
 * @param raised
 */
void Image::draw3DRect(double x, double y, double width, double height, bool raised)
{
	QColor tmp_color;

	if (raised)
		tmp_color = _m_Graphics2D->pen().color().lighter();
	else
		tmp_color = _m_Graphics2D->pen().color().darker();

	//drawLine(x, y, x, y + height);
	_m_Graphics2D->fillRect(QRect(x, y, 1, height + 1), tmp_color);
	//drawLine(x + 1, y, x + width - 1, y);
	_m_Graphics2D->fillRect(QRect(x + 1, y, width - 1, 1), tmp_color);

	if (raised)
		tmp_color = _m_Graphics2D->pen().color().lighter();
	else
		tmp_color = _m_Graphics2D->pen().color().darker();

	//drawLine(x + 1, y + height, x + width, y + height);
	_m_Graphics2D->fillRect(QRect(x + 1, y + height, width, 1), tmp_color);
	//drawLine(x + width, y, x + width, y + height - 1);
	_m_Graphics2D->fillRect(QRect(x + width, y, 1, height), tmp_color);
}    


/**
 * 
 * @param x
 * @param y
 * @param width
 * @param height
 */
void Image::fill3DRect(double x, double y, double width, double height, double /* in_x_incline */, double /* in_y_incline */)
{
	QColor brighter = _m_Graphics2D->pen().color().lighter();
	QColor darker = _m_Graphics2D->pen().color().darker();

	double arcw = (width <= 10 ? 0 : 10), arch = (height <= 10 ? 0 : 10);
	
	/* Desssiner un rectangle arrondi */
	//_m_Graphics2D->fillRect(QRect(x+1, y+1, width-2, height-2, arcw, arch));
	//_m_Graphics2D->fillRect(QRect(x+1, y+height-arch/2, width-2, arch/2));

	/* Dessiner les ombres claires */
	_m_Graphics2D->setPen(brighter);
	/* Ligne claire de gauche */
	_m_Graphics2D->drawLine(QLineF(x + 1, y + arch/2, x + 1, y + height - 1));
	/* Arc clair gauche supérieur */
	//_m_Graphics2D->drawArc(QRect(x + 1, y + 1, arcw, arch, 90, 90, Arc2D.OPEN));
	_m_Graphics2D->drawArc(QRectF(x + 1, y + 1, arcw, arch), 90*QT_ANGLE_RATIO, 90*QT_ANGLE_RATIO);
	/* Ligne claire supérieure */
	_m_Graphics2D->drawLine(QLineF(x + arcw/2, y, x + width - arcw/2, y));
	/* Arc clair droit supérieur*/
	//_m_Graphics2D->drawArc(QRect(x + width - arcw - 1, y + 1, arcw, arch, 0, 90, Arc2D.OPEN));
	_m_Graphics2D->drawArc(QRectF(x + width - arcw - 1, y + 1, arcw, arch), 0*QT_ANGLE_RATIO, 90*QT_ANGLE_RATIO);
	/* Dessiner les ombres sombres */
	_m_Graphics2D->setPen(darker);
	/* Ligne sombre inférieure */
	_m_Graphics2D->drawLine(QLineF(x + 1, y + height - 1, x + width - 1, y + height - 1));
	/* Ligne sombre de droite */
	_m_Graphics2D->drawLine(QLineF(x + width - 1, y + arch/2, x + width - 1, y + height - 1));
	_m_Graphics2D->setPen(_m_pencil_Color);
}

	
/**
 * Récupérer la couleur de fond
 * @return	chaéne définissant la couleur au format javascript (ex:#000000)
 */
QString Image::getBackgroundColor()
{
	return javaColorToJsColor(_m_background_Color);
}


/**
 * Définir la couleur de fond
 * @param in_js_color	chaéne définissant la couleur au format javascript (ex:#000000)
 */
void Image::setBackgroundColor(QString in_js_color)
{
	setColors(_m_pencil_Color, jsColorToJavaColor(in_js_color));
}


/**
 * Récupérer la largeur de l'image
 * @return
 */
double Image::getImageWidth()
{
	return _m_width;
}


/**
 * Récupérer la hauteur de l'image
 * @return
 */
double Image::getImageHeight()
{
	return _m_height;
}


/**
 * Définir la largeur de l'image
 * @return
 */
void Image::setImageWidth(double in_width) {setImageSize(in_width, _m_height);}


/**
 * Définir la hauteur de l'image
 * @return
 */
void Image::setImageHeight(double in_height)
{
	setImageSize(_m_width, in_height);
}


/**
 * Indiquer si l'image NE peut PAS étre transparente
 * @return <code>true</code> ou <code>false</code>
 */
bool Image::cantBeTransparent()
{
        return _m_BufferedImage->hasAlphaChannel();
}


/**
 * L'image est-elle transparente ?
 * @return	<code>true</code> ou <code>false</code>
 */
bool Image::getTransparency()
{
	return _m_transparency_indic;
}


/**
 * Définir la transparence de l'image
 * @param in_transparency	indicateur <code>true</code> ou <code>false</code>
 */
void Image::setTransparency(bool in_transparency)
{
    bool tmp_transparency_indic = in_transparency;
	
	if (cantBeTransparent()) {
	    tmp_transparency_indic = false;
	}

	if (tmp_transparency_indic != _m_transparency_indic) {
		_m_transparency_indic = tmp_transparency_indic;
		clear();			
	}
}


/**
 * Améliorer la qualité du rendu de l'image (au détriment de la vitesse)
 */
void Image::setRenderingForQuality()
{
    _m_rendering_speed_indic = false;
        _m_Graphics2D->setRenderHint(QPainter::Antialiasing, true);
        _m_Graphics2D->setRenderHint(QPainter::TextAntialiasing, true);
        _m_Graphics2D->setRenderHint(QPainter::SmoothPixmapTransform, true);
        _m_Graphics2D->setRenderHint(QPainter::HighQualityAntialiasing, true);
        _m_Graphics2D->setRenderHint(QPainter::NonCosmeticDefaultPen, true);

}


/**
 * Améliorer la vitesse de rendu de l'image (au détriment de la qualité)
 */
void Image::setRenderingForSpeed()
{
    _m_rendering_speed_indic = true;
        _m_Graphics2D->setRenderHint(QPainter::Antialiasing, false);
        _m_Graphics2D->setRenderHint(QPainter::TextAntialiasing, false);
        _m_Graphics2D->setRenderHint(QPainter::SmoothPixmapTransform, false);
        _m_Graphics2D->setRenderHint(QPainter::HighQualityAntialiasing, false);
        _m_Graphics2D->setRenderHint(QPainter::NonCosmeticDefaultPen, false);
}


/**
 * Récupérer le format de l'image
 * @return	chaéne de la forme "PNG"
 */
QString Image::getFileFormat()
{
	return _m_image_format_str;
}


/**
 * Définir le format de l'image basé sur les formats disponibles
 * @param in_file_format	chéne de la forme "PNG"
 */
void Image::setFileFormat(QString in_file_format)
{
	QList<QByteArray>	tmp_available_file_formats = QImageWriter::supportedImageFormats();

    if (_m_image_format_str != in_file_format)
    {
		for (int tmpIndex = 0; tmpIndex < tmp_available_file_formats.count(); tmpIndex++)
		{
			if (tmp_available_file_formats[tmpIndex] == in_file_format)
			{
				_m_image_format_str = in_file_format;
				if (cantBeTransparent()) _m_transparency_indic = false;

				_createImage();
				break;
	        }
	    }
    }
}


/**
 * Chercher un caractére dans un tableau
 * @param in_search_str	caractére é chercher
 * @param in_str_array	tableau de caractéres
 * @param in_start_index index de départ de la recherche
 * @return index du caractére danbs le tableau ou -1 si le caractére n'a pas été trouvé.
 */
int Image::indexOfChar(char in_search_str, QByteArray in_str_array, int in_start_index)
{
    int ii;
    
    for (ii = in_start_index; ii < in_str_array.length(); ii++)
    	if (in_str_array[ii] == in_search_str)
			return ii;

    return -1;
}


/**
 * Créer un sous-tableau
 * @param in_char_array	tableau d'origine
 * @param in_start_index	position de début de copie
 * @param in_end_index	position de fin de copie
 * @return	le sous-tableau
 */
QByteArray Image::subCharArray(QByteArray in_char_array, int in_start_index, int in_end_index)
{
    int				ii;
    QByteArray	tmp_char_array;
    
    tmp_char_array.reserve(in_end_index - in_start_index);
    for (ii = 0; ii < tmp_char_array.length(); ii++)
    	tmp_char_array[ii] = in_char_array[ii + in_start_index];
    
    return tmp_char_array;
}


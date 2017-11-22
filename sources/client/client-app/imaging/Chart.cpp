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

#include "session.h"
#include "Chart.h"
#include "GradientColor.h"
#include "Rectangle3D.h"

/**
 * Tableau de couleurs par défaut
 */
static const QColor STD_COLORS_ARRAY[] = {QColor(102, 204, 255), QColor(255, 128, 0), QColor(204, 255, 102), Qt::cyan, Qt::yellow, Qt::red, Qt::green};
static const vector<QColor> COLORS(STD_COLORS_ARRAY, STD_COLORS_ARRAY + sizeof(STD_COLORS_ARRAY) / sizeof(QColor));


/**
* Constructeur par défaut.
*/
Chart::Chart()
    :GraphicImage(DEFAULT_WIDTH, DEFAULT_HEIGHT)
{
    _init();
    _createImage();
}


/**
* Constructeur secondaire.
*/
Chart::Chart(int in_width, int in_height)
    : GraphicImage(in_width, in_height)
{
    _init();
    if ((in_width > 0 && in_width <= MAX_WIDTH) && (in_height > 0 && in_height <= MAX_HEIGHT))
    {
	_m_width = in_width;
	_m_height = in_height;
    }
    _createImage();
}


Chart::~Chart()
{

}


void Chart::_init()
{
    LOG_TRACE(Session::instance()->getClientSession(), "[Chart] init\n");

    Image::_init();

    m_raised_histogram_indic = true;

    /**
     * Indicateur d'affichage d'un histogramme empilé
     */
    m_stack_histogram_indic = true;

    /**
     * Indicateur d'affichage de la légende
     **/
    m_show_legend = true;

    /**
     * Indicateur de calcul automatique des tailles des bordures.<br>
     * Si il est égal é <code>true</code> les tailles des bordures autours du graphique sont calculées automatiquement<br>
     * en fonction de la largeur du dessin et de l'affichage ou non des légendes.
     **/
    m_auto_borders = true;

    /* Type de graphique */
    _m_type = GRAPH_TYPE_HISTOGRAM;

    /* Nombres de séries X, Y et Z */
    _m_x_categories_count = 1;
    _m_y_categories_count = 1;
    _m_z_categories_count = 1;

    /* Espace entre chaque graduation verticale et horizontale */
    _m_x_step = DEFAULT_X_STEP;
    _m_y_step = DEFAULT_Y_STEP;

    /* Décalage des portions du fromage par rapport au centre */
    _m_delta_cheese_portion = 0;

    /* Origine du graphique dans l'image */
    _m_graph_origin_x = DEFAULT_BORDER_RIGHT;
    _m_graph_origin_y = DEFAULT_HEIGHT - DEFAULT_BORDER_BOTTOM;

    /* Largeur et hauteur du graphique */
    _m_graph_width = DEFAULT_WIDTH - DEFAULT_BORDER_LEFT - DEFAULT_BORDER_RIGHT;
    _m_graph_height = DEFAULT_HEIGHT - DEFAULT_BORDER_TOP - DEFAULT_BORDER_BOTTOM;

    /* Angle d'inclinaison vertical */
    _m_y_incline = 60.0;

    /* Angle d'inclinaison horizontal */
    _m_x_incline = 30.0;

    /* Indicateur de dessin des axes */
    _m_drawn_axes = false;

    /* Alignement horizontal du titre (centré par défaut) */
    _m_graph_title_halign = TITLE_ALIGN_CENTER;

    /* Alignement vertical du titre (en haut par défaut) */
    _m_graph_title_valign = TITLE_ALIGN_TOP;

    /* Alignement de la légende (en bas par défaut) */
    _m_graph_legend_position = LEGEND_BOTTOM;

    /* Couleur du titre (orange par défaut) */
    _m_graph_title_color = Qt::darkRed;

    /* Tableau des couleurs utilisées */
    _m_colors = COLORS;

    /* Niveau de transparence du graphique (à 80% opaque par défaut) */
    _m_graph_alpha_paint_color = 0xCC;

    /* Indique si le tracé du graphique dépend des abcisses
     * lorsqu'il s'agit de données de type numérique ou date */
    _m_abciss_proportionnal_indic = false;

    _m_graph_font_size = 10;
}


/**
 * Définir la légende de la série de données X
 * @param in_x_objects tableau de libellés
 */
void Chart::setXLegend(vector<QString> in_x_objects)
{
    if (in_x_objects.size() > 0)
    {
        _m_x_labels = in_x_objects;
        _m_x_categories_count = in_x_objects.size();
    }
}


/**
 * Définir la légende de la série de données Y
 * @param in_y_objects tableau de libellés
 */
void Chart::setYLegend(vector<QString> in_y_objects)
{
    if (in_y_objects.size() > 0)
    {
        _m_y_labels = in_y_objects;
        _m_y_categories_count = in_y_objects.size();
    }
}


/**
 * Définir la légende de la série de données Z
 * @param in_z_objects tableau de libellés
 */
void Chart::setZLegend(vector<QString> in_z_objects)
{
    if (in_z_objects.size() > 0)
    {
        _m_z_labels = in_z_objects;
    }
}


/**
 * Définir les bornes et le nombre de graduations sur l'axe des abcisses
 * @param in_min_x	valeur minimale en abcisse
 * @param in_max_x	valeur maximale en abcisse
 * @param in_x_count pas de graduations horizontales
 */
void Chart::setHorizontalGraduations(DataObject *in_min_x, DataObject *in_max_x, double in_x_step)
{
    vector<QString>			tmp_x_labels;
    Double				tmp_min_x(0);
    Double				tmp_max_x(0);
    Date				tmp_date;
    DataObject              *tmp_min_x_categories = in_min_x;
    DataObject              *tmp_max_x_categories = in_max_x;
    int                     tmp_length = 0;

    if (tmp_min_x_categories != NULL && tmp_max_x_categories != NULL && tmp_min_x_categories->compareTo(*tmp_max_x_categories) == -1)
    {
        tmp_min_x = Double(tmp_min_x_categories->doubleValue());
        tmp_max_x = Double(tmp_max_x_categories->doubleValue());
        tmp_length =(int)((tmp_max_x.doubleValue() - tmp_min_x.doubleValue()) / in_x_step) + 1;
        for (int tmp_index = 0; tmp_index < tmp_length; tmp_index++)
        {
            if (tmp_max_x_categories->isNumber())
            {
                tmp_x_labels.push_back(Double(tmp_min_x.doubleValue() + tmp_index * in_x_step).toString());
            }
            else if (tmp_max_x_categories->isDate())
            {
                tmp_date = Date(tmp_min_x.doubleValue() + tmp_index * in_x_step);

                if (in_x_step <= SECOND_STEP)
                    tmp_x_labels.push_back(QString::number(tmp_date.dateTime().time().second()));
                else if (in_x_step <= MINUTE_STEP)
                    tmp_x_labels.push_back(QString::number(tmp_date.dateTime().time().minute()));
                else if (in_x_step <= HOUR_STEP)
                    tmp_x_labels.push_back(QString::number(tmp_date.dateTime().time().hour()) + ":" + QString::number(tmp_date.dateTime().time().minute()));
                else if (in_x_step <= DAY_STEP)
                    tmp_x_labels.push_back(tmp_date.toString());
                else if (in_x_step <= WEEK_STEP)
                    tmp_x_labels.push_back(tmp_date.toString());
                else if (in_x_step <= MONTH_STEP)
                    tmp_x_labels.push_back(tmp_date.toString());
                else if (in_x_step <= YEAR_STEP)
                    tmp_x_labels.push_back(tmp_date.toString());
            }
            else
                tmp_x_labels.push_back("");
        }

        setXLegend(tmp_x_labels);

        _m_min_x_categories = tmp_min_x_categories;
        _m_max_x_categories = tmp_max_x_categories;

        _m_abciss_proportionnal_indic = true;
    }
}


/**
 * Définir les bornes et le nombre de graduations sur l'axe des ordonnées
 * @param in_min_y valeur minimale en ordonnée
 * @param in_max_y valeur maximale en ordonnée
 * @param in_y_count pas de graduations verticales
 */
void Chart::setVerticalGraduations(DataObject *in_min_y, DataObject *in_max_y, int in_y_step)
{
    vector<QString>	tmp_y_labels;
    Double	tmp_min_y(0);
    Double	tmp_max_y(0);

    DataObject *tmp_min_y_categories = in_min_y;
    DataObject *tmp_max_y_categories = in_max_y;

    int                     tmp_length = 0;

    if (tmp_min_y_categories != NULL && tmp_max_y_categories != NULL && tmp_min_y_categories->compareTo(*tmp_max_y_categories) == -1)
    {
        tmp_min_y = Double(tmp_min_y_categories->doubleValue());
        tmp_max_y = Double(tmp_max_y_categories->doubleValue());

        tmp_length = (int)((tmp_max_y.doubleValue() - tmp_min_y.doubleValue()) / in_y_step);
        for (int tmp_index = 0; tmp_index < tmp_length; tmp_index++)
        {
            if (_m_max_y_categories->isNumber())
                tmp_y_labels.push_back(QString::number(tmp_min_y.doubleValue() + (tmp_index + 1) * in_y_step));
            else if (_m_max_y_categories->isDate())
                tmp_y_labels.push_back(Date(tmp_min_y.doubleValue() + tmp_index * in_y_step).toString());
            else
                tmp_y_labels.push_back("");
        }

        setYLegend(tmp_y_labels);

        _m_min_y_categories = tmp_min_y_categories;
        _m_max_y_categories = tmp_max_y_categories;
    }
}


/**
 * Définir les bornes et le nombre de graduations sur l'axe des azimuts
 * @param in_min_z valeur minimale en azimut
 * @param in_max_z valeur maximale en azimut
 * @param in_z_count pas de graduations azimutales
 */
void Chart::setAzimutalGraduations(DataObject *in_min_z, DataObject *in_max_z, int in_z_step)
{
    vector<QString>	tmp_z_labels;
    Double	tmp_min_z(0);
    Double	tmp_max_z(0);

    DataObject *tmp_min_z_categories = in_min_z;
    DataObject *tmp_max_z_categories = in_max_z;

    int     tmp_length = 0;

    if (tmp_min_z_categories != NULL && tmp_max_z_categories != NULL && tmp_min_z_categories->compareTo(*tmp_max_z_categories) == -1)
    {
        tmp_min_z = Double(tmp_min_z_categories->doubleValue());
        tmp_max_z = Double(tmp_max_z_categories->doubleValue());

        tmp_length = (int)((tmp_max_z.doubleValue() - tmp_min_z.doubleValue()) / in_z_step);
        for (int tmp_index = 0; tmp_index < tmp_length; tmp_index++)
        {
            if (_m_max_z_categories->isNumber())
                tmp_z_labels.push_back(QString::number(tmp_min_z.doubleValue() + tmp_index * in_z_step));
            else if (_m_max_z_categories->isDate())
                tmp_z_labels.push_back(Date(tmp_min_z.doubleValue() + tmp_index * in_z_step).toString());
            else
                tmp_z_labels.push_back("");
        }

        setZLegend(tmp_z_labels);

        _m_min_z_categories = tmp_min_z_categories;
        _m_max_z_categories = tmp_max_z_categories;
    }
}


/**
 * Définir le tableau des données
 * @param in_trinomes_array
 */
void Chart::setData(const TrinomeArray& in_trinomes_array)
{
    uint			tmp_coords_count = 0;

    _m_trinomes_array = in_trinomes_array;

    /* Traitement des données */
    /* Initialiser les tableaux des catégories */
    _m_x_categories = _m_trinomes_array.m_x_categories;
    _m_y_categories = _m_trinomes_array.m_y_categories;
    _m_z_categories = _m_trinomes_array.m_z_categories;

    /* Initialiser les bornes inférieures */
    _m_min_x_categories = _m_trinomes_array.m_min_x;
    _m_min_y_categories = _m_trinomes_array.m_min_y;
    _m_min_z_categories = _m_trinomes_array.m_min_z;

    /* Initialiser les bornes supérieures */
    _m_max_x_categories = _m_trinomes_array.m_max_x;
    _m_max_y_categories = _m_trinomes_array.m_max_y;
    _m_max_z_categories = _m_trinomes_array.m_max_z;

    _m_legend_categories.clear();

    _m_abciss_proportionnal_indic = (_m_min_x_categories != NULL && _m_max_x_categories != NULL &&
            (_m_min_x_categories->isNumber() || _m_min_x_categories->isDate())
            && (_m_max_x_categories->isNumber() || _m_max_x_categories->isDate()));

    if (_m_x_labels.size() > 0)	_m_x_categories_count = _m_x_labels.size();
    else if (_m_x_categories.size() > 0)	_m_x_categories_count = _m_x_categories.size();

    if (_m_y_labels.size() > 0)	_m_y_categories_count = _m_y_labels.size();
    else if (_m_y_categories.size() > 0)	_m_y_categories_count = _m_y_categories.size();

    if (_m_z_categories.size() > 1)
    {
        /* Définir la série des légendes */
        _m_legend_categories = _m_z_categories;

        /* Définir le nombre de valeurs de la catégorie Z */
        _m_z_categories_count = _m_z_categories.size();

        /* Définir le nombre de coordonnées nécessaires */
        tmp_coords_count = _m_z_categories_count * _m_x_categories_count;
    }
    else
    {
        /* Définir la série des légendes */
        if (_m_type == GRAPH_TYPE_CHEESE)	_m_legend_categories = _m_x_categories;
        //else	_m_legend_categories = NULL;

        /* Définir le nombre de valeurs de la catégorie Z */
        _m_z_categories_count = 1;

        /* Définir le nombre de coordonnées nécessaires */
        tmp_coords_count = _m_trinomes_array.length;
    }

    /* Initialiser les tableaux de coordonnées */
    // if (_m_x_array.size() == 0 || _m_x_array.size() != tmp_coords_count)
    _m_x_array = vector<double>(tmp_coords_count);
    //if (_m_y_array.size() == 0 || _m_y_array.size() != tmp_coords_count)
    _m_y_array = vector<double>(tmp_coords_count);

}


/**
 * Initialiser les paramètres du graphique (coordonnées, bordures, légendes...etc) avant le dessin
 */
void Chart::initDrawing()
{

    if (_m_type != GRAPH_TYPE_POLYGON)
        _m_abciss_proportionnal_indic = false;

    if (_m_z_categories_count == 1)
        m_stack_histogram_indic = false;

    /* Calculer la taille des graduations sur les axes */
    _calculateGraduations();

    /* Si la taille des bordures est automatique...*/
    if (m_auto_borders)
    {
        /* ...mettre à jour la taille de la bordure du bas
         * en fonction de l'affichage ou non de la légende */
        if (_m_legend_categories.size() > 0 && m_show_legend)
        {
            setBorderSize(-1, -1, -1, -1);
        }
        else
            _setBorderSize(-1, -1, -1, DEFAULT_BORDER_BOTTOM * _m_width / DEFAULT_WIDTH);
    }
}


/**
 * Calculer la taille des graduations horizontales et verticales
 */
void Chart::_calculateGraduations()
{
    double	tmp_max_y = 0.0;
    double	tmp_factor = 0.0;
    double	tmp_step_count = 0.0;
    int		tmp_factors[] = {1, 2, 4, 5};

    int		tmp_index;

    /* Modifier la taille des graduations horizontales en fonction de la largeur du graphique */
    if (_m_x_categories_count != 0)
    {
        if (_m_abciss_proportionnal_indic)
            _m_x_step = _m_graph_width / (_m_x_categories_count - 1);
        else
            _m_x_step = _m_graph_width / _m_x_categories_count;
    }
    else
    {
        _m_x_step = DEFAULT_X_STEP * _m_graph_width / (DEFAULT_WIDTH - DEFAULT_BORDER_LEFT - DEFAULT_BORDER_RIGHT);
    }

    /* Modifier la taille des graduations verticales en fonction de la hauteur du graphique */
    if (_m_y_labels.size() == 0)
    {
        /* Vérifier l'existance de données */
        if(_m_trinomes_array.length > 0)
        {
            /* Graduation maximale */
            if (_m_type == GRAPH_TYPE_HISTOGRAM && m_stack_histogram_indic)
                tmp_max_y = _m_trinomes_array.m_max_total_y_by_x_class;
            else if(_m_trinomes_array.m_max_y != NULL && _m_trinomes_array.m_max_y->isNumber())
                tmp_max_y = _m_trinomes_array.m_max_y->doubleValue();

            if(tmp_max_y > 0.0)
            {
                /* Valeur par défaut */
                _m_y_step = _m_graph_height / 10 ;

                /* Calcul du facteur de graduation maximal */
                /* Attention : la fonction log calcule le logarithme népérien basé sur la constante d'(e)uler
                 * et non pas le logarithme décimal basé sur le nombre dix. Dans notre cas, nous voulons calculer le
                                 * logarithme décimal. Celui-ci peut être calculé pour un nombre X, à partir de la formule :
                                 * log(X) = ln(X)/ln(10) où 'ln' signifie logarithme népérien
                 * car si X = 10^y alors ln(X) = ln(10) * y et donc y = ln(X)/ln(10)
                 */
                tmp_factor = (int)(log(tmp_max_y) / log(10.0)) - 1;
                while (tmp_max_y / pow(10.0, tmp_factor + 1) > 1.0) tmp_factor++;
                tmp_factor = tmp_max_y / pow(10.0, tmp_factor);

                for(tmp_index = 0; tmp_index < 4; tmp_index++)
                {
                    tmp_step_count = tmp_factor * tmp_factors[tmp_index];
                    /* Il doit y avoir de 1 à 10 graduations pour des valeurs entières
                     * et de 4 à 10 pour des valeurs décimales */
                    if (tmp_step_count >= (_m_trinomes_array.m_has_decimal_y_indic ? 4.0 : 1.0)
                            && tmp_step_count <= 10.0)
                    {
                        _m_y_step = _m_graph_height / tmp_step_count;
                        break;
                    }
                }
            }
            else
            {
                _m_y_step = _m_graph_height / (_m_y_categories.size() != 0 ? _m_y_categories.size() : 1);
            }
        }
        else
        {
            _m_y_step = _m_graph_height / 10 ;
        }
    }
    else
    {
        _m_y_step = _m_graph_height / (_m_y_labels.size() != 0 ? _m_y_labels.size() : 1);
    }
}


/**
 * Définir la taille des bords seulement si <code>m_auto_borders</code> est positionné à <code>false</code>
 * @param in_border_left taille du bord gauche
 * @param in_border_right taille du bord droit
 * @param in_border_top taille du bord supérieur
 * @param in_border_bottom taille du bord inférieur
 */
void Chart::setBorderSize(double in_border_left, double in_border_right, double in_border_top, double in_border_bottom)
{
    double  tmp_max_width = 0;
    double  tmp_current_width = 0;
    uint    tmpIndex = 0;
    QString tmp_str;

    QRectF  tmp_str_rect;

    if (!m_auto_borders)
    {
        _setBorderSize(in_border_left, in_border_right, in_border_top, in_border_bottom);
    }
    else
    {
        /* Calculer la largeur du libellé le plus large */
        for(tmpIndex = 0; tmpIndex < _m_legend_categories.size(); tmpIndex++)
        {
            /* Définir le libellé */
            if (_m_legend_categories == _m_x_categories && _m_x_labels.size() > tmpIndex)
                tmp_str = _m_x_labels[tmpIndex];
            if (_m_legend_categories == _m_z_categories && _m_z_labels.size() > tmpIndex)
                tmp_str = _m_z_labels[tmpIndex];
            else if (_m_legend_categories[tmpIndex] != NULL)
                tmp_str = _m_legend_categories[tmpIndex]->toString();
            else
                tmp_str = "(vide)";

            tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);
            tmp_current_width = _m_graph_font_size * 2 + 10 + tmp_str_rect.width();

            if (tmp_current_width > tmp_max_width)
                tmp_max_width = tmp_current_width;
        }

        switch (_m_graph_legend_position)
        {
            case LEGEND_TOP:
                _setBorderSize(-1, -1, _m_graph_font_size + 10 + _m_legend_categories.size() * (_m_graph_font_size + 10), -1);
                break;
            case LEGEND_LEFT:
                _setBorderSize(tmp_max_width + 10, -1, -1, -1);
                break;
            case LEGEND_RIGTH:
                _setBorderSize(-1, tmp_max_width + 10, -1, -1);
                break;
            default:
                _setBorderSize(-1, -1, -1, _m_graph_font_size + 10 + _m_legend_categories.size() * (_m_graph_font_size + 10));
                break;
        }
    }
}


/**
 * Définir la taille des bords
 * @param in_border_left taille du bord gauche
 * @param in_border_right taille du bord droit
 * @param in_border_top taille du bord supérieur
 * @param in_border_bottom taille du bord inférieur
 */
void Chart::_setBorderSize(double in_border_left, double in_border_right, double in_border_top, double in_border_bottom)
{

    double	tmp_left = (in_border_left >= 0 ? in_border_left : _m_graph_origin_x);
    double	tmp_right = (in_border_right >= 0 ? in_border_right : _m_width - _m_graph_width - _m_graph_origin_x);
    double	tmp_top = (in_border_top >= 0 ? in_border_top : _m_graph_origin_y - _m_graph_height);
    double	tmp_bottom = (in_border_bottom >= 0 ? in_border_bottom : _m_height - _m_graph_origin_y);

    /* Ne conserver que des bords suffisemment larges pour afficher le graphe
     * et ne rien modifier si les nouvelles valeurs sont identiques aux anciennes */
    if (_m_width / 2 <= tmp_left + tmp_right
            || 2 * _m_height / 3 <= tmp_bottom + tmp_top
            ||(tmp_left == _m_graph_origin_x
               && tmp_bottom == _m_height - _m_graph_origin_y
               && tmp_top == _m_graph_origin_y - _m_graph_height
               && tmp_right ==  _m_width - _m_graph_width - _m_graph_origin_x))
    {
        return ;
    }

    /* Mettre à jour la taille et la position du graphique */
    _setGraphRect(tmp_left, _m_height - tmp_bottom, _m_width - tmp_left - tmp_right, _m_height - tmp_top - tmp_bottom);
}


/**
 * Initialiser le contexte graphique
 */
void Chart::_initGraphicContext()
{
    double	tmp_graph_width = (DEFAULT_WIDTH - DEFAULT_BORDER_LEFT - DEFAULT_BORDER_RIGHT) * _m_width / DEFAULT_WIDTH;
    double	tmp_graph_height = (DEFAULT_HEIGHT - DEFAULT_BORDER_TOP - DEFAULT_BORDER_BOTTOM) * _m_height / DEFAULT_HEIGHT;

    LOG_TRACE(Session::instance()->getClientSession(), "[Chart] Initialisaton...\n");

    _m_rendering_speed_indic = false;

    Image::_initGraphicContext();

    /* Positionner l'indicateur de dessin des axes */
    _m_drawn_axes = false;

    /* Initialiser la position et la taille du graphique */
    _setGraphRect(
                (_m_width - tmp_graph_width) / 2,
                _m_height - (_m_height - tmp_graph_height) / 2,
                tmp_graph_width,
                tmp_graph_height);

    /* Initialiser les polices par défaut */
    _m_graph_font_size = (float)(_m_width / DEFAULT_WIDTH < _m_height / DEFAULT_HEIGHT ? 6 * _m_width / DEFAULT_WIDTH : 6 * _m_height / DEFAULT_HEIGHT);
    _m_graph_font = QFont(QFont().family());
    _m_graph_font.setPixelSize(_m_graph_font_size);
    _m_Graphics2D->setFont(_m_graph_font);
    _m_title_font = QFont();

    LOG_TRACE(Session::instance()->getClientSession(), "[Chart] POLICE = %s\n", _m_graph_font.toString().toStdString().c_str());
}


/**
 * Dessiner le cadre
 */
void Chart::drawAxis()
{
    uint			tmp_index = 0;
    QString		tmp_str;
    QPen		tmp_current_stroke = _m_Graphics2D->pen();
    double		tmp_x_step = 0.0;
    QTransform	tmp_originTr, tmp_newTr;
    QList<QString>	tmp_text_array;
    QRectF          tmp_text_rect;
    bool		tmp_oblic_text_layout_indic = false;

    if (_m_type == GRAPH_TYPE_HISTOGRAM && m_stack_histogram_indic)
        tmp_x_step = _m_x_step / 2;
    else
        tmp_x_step = (_m_x_step / 2) / _m_z_categories_count;

    /* Dessiner le cadre sauf pour le camembert */
    if (_m_type != GRAPH_TYPE_CHEESE && !_m_drawn_axes)
    {
        _m_drawn_axes = true;

        _m_Graphics2D->setPen(Qt::black);

        if (_m_type == GRAPH_TYPE_HISTOGRAM && m_raised_histogram_indic)
        {
            _m_Graphics2D->drawLine(QLineF(
                                        _m_graph_origin_x,
                                        _m_graph_origin_y,
                                        _m_graph_origin_x + tmp_x_step * sin(RAD*_m_y_incline),
                                        _m_graph_origin_y - tmp_x_step * sin(RAD*_m_x_incline)));

            /* Tracer une ligne en pointillés */
            _m_Graphics2D->setPen(QPen(Qt::lightGray, 1, Qt::DotLine, Qt::SquareCap, Qt::BevelJoin));
            _m_Graphics2D->drawLine(QLineF(
                                        _m_graph_origin_x + tmp_x_step * sin(RAD*(_m_y_incline)),
                                        _m_graph_origin_y - tmp_x_step * sin(RAD*(_m_x_incline)),
                                        _m_graph_origin_x + tmp_x_step * sin(RAD*(_m_y_incline)),
                                        _m_graph_origin_y - tmp_x_step * sin(RAD*(_m_x_incline)) - _m_graph_height));

            _m_Graphics2D->setPen(Qt::black);
            _m_Graphics2D->setPen(tmp_current_stroke);
        }


        /* Tracer les graduations horizontales */
        if (_m_x_step != 0)
        {
            for (tmp_index = 0; (int)tmp_index < _m_x_categories_count; tmp_index++)
            {
                /* Tracer une ligne en pointillés */
                _m_Graphics2D->setPen(QPen(Qt::lightGray, 1, Qt::DotLine, Qt::SquareCap, Qt::BevelJoin));
                if(_m_type == GRAPH_TYPE_HISTOGRAM && m_raised_histogram_indic)
                {
                    _m_Graphics2D->drawLine(QLineF(
                                                _m_graph_origin_x + _m_x_step * tmp_index,
                                                _m_graph_origin_y,
                                                _m_graph_origin_x + _m_x_step * tmp_index + tmp_x_step * sin(RAD*(_m_y_incline)),
                                                _m_graph_origin_y - tmp_x_step * sin(RAD*(_m_x_incline))));

                    _m_Graphics2D->drawLine(QLineF(
                                                _m_graph_origin_x + _m_x_step * tmp_index + tmp_x_step * sin(RAD*(_m_y_incline)),
                                                _m_graph_origin_y - tmp_x_step * sin(RAD*(_m_x_incline)),
                                                _m_graph_origin_x + _m_x_step * tmp_index + tmp_x_step * sin(RAD*(_m_y_incline)),
                                                _m_graph_origin_y - tmp_x_step * sin(RAD*(_m_x_incline)) - _m_graph_height));
                }
                else
                {
                    _m_Graphics2D->drawLine(QLineF(
                                                _m_graph_origin_x + _m_x_step * tmp_index,
                                                _m_graph_origin_y,
                                                _m_graph_origin_x + _m_x_step * tmp_index,
                                                _m_graph_origin_y - _m_graph_height));
                }

                _m_Graphics2D->setPen(Qt::black);
                _m_Graphics2D->setPen(tmp_current_stroke);

                /* Tracer une graduation */
                _m_Graphics2D->drawLine(QLineF(_m_graph_origin_x + tmp_index * _m_x_step, _m_graph_origin_y - 1, _m_graph_origin_x + tmp_index * _m_x_step, _m_graph_origin_y - 4));

                /* Préparer le libellé */
                if (_m_x_labels.size() > tmp_index)
                {
                    tmp_str = _m_x_labels[tmp_index];
                }
                else if (_m_x_categories.size() > tmp_index)
                    tmp_str = _m_x_categories[tmp_index]->toString();

                tmp_text_array.append(tmp_str);
            }

            _m_Graphics2D->drawLine(QLine(_m_graph_origin_x + _m_graph_width, _m_graph_origin_y - 1, _m_graph_origin_x + _m_graph_width, _m_graph_origin_y - 4));

            /* Dessiner les libellés */
            for (tmp_index = 0; (int)tmp_index  < tmp_text_array.size(); tmp_index++)
            {
                tmp_text_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_text_array[tmp_index]);
                if (tmp_text_rect.width() > _m_x_step)
                {
                    tmp_oblic_text_layout_indic = true;
                }

                /* Dessiner le libellé en oblique (45°)*/
                if (tmp_oblic_text_layout_indic)
                {
                    tmp_originTr =_m_Graphics2D->transform();

                    tmp_newTr = QTransform();
                    tmp_newTr.translate(_m_graph_origin_x + tmp_index * _m_x_step + (_m_abciss_proportionnal_indic ? 0 : _m_x_step / 2)- tmp_text_rect.width() * cos(RAD*(45)),
                                        _m_graph_origin_y + 10 + tmp_text_rect.width() * sin(RAD*(45)));
                    tmp_newTr.rotate(-45);
                    _m_Graphics2D->setTransform(tmp_newTr);

                    _m_Graphics2D->drawText(QPointF(0, 0), tmp_text_array[tmp_index]);
                    _m_Graphics2D->setTransform(tmp_originTr);
                }
                /* Dessiner le libellé horizontalement */
                else
                {
                    _m_Graphics2D->drawText(
                                QPointF(
                                    (_m_graph_origin_x + tmp_index * _m_x_step + (_m_abciss_proportionnal_indic ? - tmp_text_rect.width()/2 : (_m_x_step - tmp_text_rect.width())/2)),
                                    (_m_graph_origin_y + _m_graph_font_size + 5)),
                                tmp_text_array[tmp_index]);
                }
            }
        }

        /* Tracer les graduations verticales */
        if (_m_y_step != 0)
        {
            for (tmp_index = 0; tmp_index <= _m_graph_height / _m_y_step; tmp_index++)
            {
                /* Tracer une ligne en pointillés */
                _m_Graphics2D->setPen(Qt::lightGray);
                _m_Graphics2D->setPen(QPen(Qt::lightGray, tmp_current_stroke.width(), Qt::DotLine, Qt::SquareCap, Qt::BevelJoin));
                if(_m_type == GRAPH_TYPE_HISTOGRAM && m_raised_histogram_indic)
                {
                    _m_Graphics2D->drawLine(QLineF(_m_graph_origin_x,
                                                   _m_graph_origin_y - tmp_index * _m_y_step,
                                                   _m_graph_origin_x + tmp_x_step * sin(RAD*(_m_y_incline)),
                                                   _m_graph_origin_y - tmp_index * _m_y_step - tmp_x_step * sin(RAD*(_m_x_incline))));
                    _m_Graphics2D->drawLine(QLineF(_m_graph_origin_x + tmp_x_step * sin(RAD*(_m_y_incline)),
                                                   _m_graph_origin_y - tmp_index * _m_y_step - tmp_x_step * sin(RAD*(_m_x_incline)),
                                                   _m_graph_origin_x + tmp_x_step * sin(RAD*(_m_y_incline)) + _m_graph_width,
                                                   _m_graph_origin_y - tmp_index * _m_y_step - tmp_x_step * sin(RAD*(_m_x_incline))));
                }
                else
                {
                    _m_Graphics2D->drawLine(QLineF(_m_graph_origin_x,
                                                   _m_graph_origin_y - tmp_index * _m_y_step,
                                                   _m_graph_origin_x + _m_graph_width,
                                                   _m_graph_origin_y - tmp_index * _m_y_step));
                }

                _m_Graphics2D->setPen(tmp_current_stroke);

                /* Tracer une graduation */
                _m_Graphics2D->setPen(Qt::black);
                _m_Graphics2D->drawLine(QLineF(_m_graph_origin_x, _m_graph_origin_y - tmp_index * _m_y_step, _m_graph_origin_x + 3, _m_graph_origin_y - tmp_index * _m_y_step));

                /* Définir le libellé de la légende */
                if (_m_y_labels.size() > tmp_index - 1)
                {
                    tmp_str = _m_y_labels[tmp_index - 1];
                }
                else if (_m_type == GRAPH_TYPE_HISTOGRAM && m_stack_histogram_indic)
                {
                    tmp_str = QString::number(_m_trinomes_array.m_max_total_y_by_x_class * tmp_index * _m_y_step / _m_graph_height);
                }
                else if (_m_trinomes_array.m_max_y != NULL && _m_trinomes_array.m_max_y->isNumber())
                {
                    tmp_str = QString::number(_m_trinomes_array.m_max_y->doubleValue() * tmp_index * _m_y_step / _m_graph_height);
                }
                else if (_m_y_categories.size() > tmp_index - 1)
                {
                    tmp_str = _m_y_categories[tmp_index - 1]->toString();
                }

                /* Dessiner le libellé */
                tmp_text_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);

                _m_Graphics2D->drawText(QPointF(
                                            _m_graph_origin_x - tmp_text_rect.width() - 5,
                                            _m_graph_origin_y - tmp_index * _m_y_step + tmp_text_rect.height() / 2),
                                        tmp_str);
            }

            _m_Graphics2D->drawLine(QLineF(_m_graph_origin_x, _m_graph_origin_y - _m_graph_height, _m_graph_origin_x + 3, _m_graph_origin_y - _m_graph_height));
        }

        /* Tracer l'axe des abscisses */
        _m_Graphics2D->drawLine(QLineF(_m_graph_origin_x, _m_graph_origin_y, _m_graph_origin_x + _m_graph_width, _m_graph_origin_y));
        /* Tracer l'axe des ordonnées */
        _m_Graphics2D->drawLine(QLineF(_m_graph_origin_x, _m_graph_origin_y, _m_graph_origin_x, _m_graph_origin_y - _m_graph_height));
    }
}


/**
 * Dessiner la légende
 */
void Chart::drawLegend()
{
    uint		tmpIndex = 0;
    QString		tmp_str;
    QRectF          tmp_str_rect;
    GradientColor	tmp_grad_color(_m_colors);
    double		   tmp_start_x_position = _m_graph_origin_x;
    double		   tmp_start_y_position = _m_graph_origin_y;

    /* Dessiner uniquement si il y a une série de valeurs et
     * si l'indicateur d'affichage des légendes est positionné à vrai */
    if (_m_legend_categories.size() > 0 && m_show_legend)
    {
        switch (_m_graph_legend_position)
        {
            case LEGEND_TOP:
                tmp_start_x_position = _m_graph_origin_x;
                tmp_start_y_position = 5;
                break;
            case LEGEND_LEFT:
                tmp_start_x_position = 5;
                tmp_start_y_position = 5;
                break;
            case LEGEND_RIGTH:
                tmp_start_x_position = _m_graph_origin_x + _m_graph_width + 5;
                tmp_start_y_position = 5;
                break;
            default:
                tmp_start_x_position = _m_graph_origin_x;
                tmp_start_y_position = _m_graph_origin_y;
                break;
        }

        for(tmpIndex = 0; tmpIndex < _m_legend_categories.size(); tmpIndex++)
        {
            /* Dessiner le carré de couleur de la catégorie courante */
            _m_Graphics2D->setPen(tmp_grad_color.colorAtIndex(tmpIndex));
            fill3DRect(tmp_start_x_position, tmp_start_y_position + _m_graph_font_size + 10 + tmpIndex * (_m_graph_font_size + 5), _m_graph_font_size, _m_graph_font_size, true, _m_x_incline, _m_y_incline);

            /* Définir le libellé */
            if (_m_legend_categories == _m_x_categories && _m_x_labels.size() > tmpIndex)
                tmp_str = _m_x_labels[tmpIndex];
            if (_m_legend_categories == _m_z_categories && _m_z_labels.size() > tmpIndex)
                tmp_str = _m_z_labels[tmpIndex];
            else if (_m_legend_categories[tmpIndex] != NULL)
                tmp_str = _m_legend_categories[tmpIndex]->toString();
            else
                tmp_str = "(vide)";

            /* Dessiner le libellé */
            tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);
            _m_Graphics2D->setPen(Qt::black);
            _m_Graphics2D->drawText(QPointF(
                                        (float)(tmp_start_x_position + _m_graph_font_size + 10),
                                        (float)(tmp_start_y_position + _m_graph_font_size * 2 + 5 + tmpIndex * (_m_graph_font_size + 5))),
                                    tmp_str);
        }
    }
}


/**
 * Déssiner le titre du graphique.
 */
void Chart::drawTitle()
{
    float		tmp_font_size = (float)(_m_width / DEFAULT_WIDTH < _m_height / DEFAULT_HEIGHT ? 24 * _m_width / DEFAULT_WIDTH : 24 * _m_height / DEFAULT_HEIGHT);
    QRectF      tmp_title_rect;
    float		tmp_title_pos_x = 0, tmp_title_pos_y = 0;

    if (_m_title != NULL && _m_title.length() > 0)
    {
        _m_Graphics2D->setFont(QFont(_m_title_font.family(), tmp_font_size, _m_title_font.weight()));
        tmp_title_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, _m_title);

        /* Calcul de la position horizontale du titre dans l'image */
        switch (_m_graph_title_halign)
        {
            case TITLE_ALIGN_CENTER :
                tmp_title_pos_x = (float)( (_m_width - tmp_title_rect.width()) / 2);
                break;
            case TITLE_ALIGN_LEFT :
                tmp_title_pos_x = 5;
                break;
            case TITLE_ALIGN_RIGTH :
                tmp_title_pos_x = (float)(_m_graph_width - tmp_title_rect.width() - 5);
                break;
        }

        /* Calcul de la position verticale du titre dans l'image */
        switch (_m_graph_title_valign)
        {
            case TITLE_ALIGN_CENTER :
                tmp_title_pos_y = (float)((_m_height + tmp_title_rect.height()) / 2);
                break;
            case TITLE_ALIGN_TOP :
                tmp_title_pos_y = (float)tmp_title_rect.height();
                break;
            case TITLE_ALIGN_BOTTOM :
                tmp_title_pos_y = (float)_m_height - 5;
                break;
        }

        /* Dessiner l'ombre du titre */
        _m_Graphics2D->setPen(Qt::black);
        _m_Graphics2D->drawText(QPointF(tmp_title_pos_x + 1, tmp_title_pos_y - 1), _m_title);
        _m_Graphics2D->drawText(QPointF(tmp_title_pos_x + 2, tmp_title_pos_y - 2), _m_title);

        /* Dessiner le titre */
        _m_Graphics2D->setPen(_m_graph_title_color);
        _m_Graphics2D->drawText(QPointF(tmp_title_pos_x, tmp_title_pos_y), _m_title);

        _m_Graphics2D->setFont(_m_graph_font);
    }
}


/**
 * Effacer le contenu de l'image
 */
void Chart::clear ()
{
    _m_drawn_axes = false;
    Image::clear();
}


/**
 * Définir la position et la taille du graphique dans l'image
 * @param in_graph_x position horizontale
 * @param in_graph_y position verticale
 * @param in_graph_width largeur
 * @param in_graph_height hauteur
 */
void Chart::_setGraphRect(double in_graph_x, double in_graph_y, double in_graph_width, double in_graph_height)
{
    /* Modifier la position du graphique */
    _m_graph_origin_x = in_graph_x;
    _m_graph_origin_y = in_graph_y;

    /* Modifier les dimensions du graphique */
    _m_graph_width = in_graph_width;
    _m_graph_height = in_graph_height;

    /* Mettre à jour la taille des graduations */
    _calculateGraduations();
}


/**
 * Calcul des coordonnées des points du graphique<br>
 * Utilisé pour un histogramme et un polygone
 */
void Chart::_calculateGraphPoints()
{
    int			tmpIndex = 0;
    uint			tmpIndexX = 0;
    int			tmpIndexZ = 0;
    int			tmp_z_index = 0;
    uint			tmpTupleIndex = 0;
    DataObject *tmp_x_object = NULL;

    /* Parcourrir les catégories X */
    for(tmpIndexX = 0; tmpIndexX < _m_x_categories.size(); tmpIndexX++)
    {
        /* Parcourrir les catégories Z */
        for(tmpIndexZ = 0; tmpIndexZ < _m_z_categories_count; tmpIndexZ++)
        {
            tmpIndex = tmpIndexX * _m_z_categories_count + tmpIndexZ;
            tmp_x_object = _m_x_categories[tmpIndexX];

            /* Vérifier si la valeur X et Y courantes sont des catégories valides */
            if (tmpTupleIndex < _m_trinomes_array.length
                    &&(_m_z_categories_count == 1 ||
                       (DataObject::areEquals(_m_trinomes_array.m_x_objects[tmpTupleIndex], tmp_x_object)
                        && DataObject::areEquals(_m_trinomes_array.m_z_objects[tmpTupleIndex], _m_z_categories[tmpIndexZ]))))
            {
                /* Calcul des abscisses */
                if (_m_abciss_proportionnal_indic && (_m_trinomes_array.m_x_objects[tmpTupleIndex]->isNumber() || _m_trinomes_array.m_x_objects[tmpTupleIndex]->isDate()))
                {
                    _m_x_array[tmpIndex] = _m_trinomes_array.m_x_objects[tmpTupleIndex]->doubleValue();
                }
                else
                {
                    _m_x_array[tmpIndex] = (tmpIndexX + 1) * _m_x_step;
                }

                /* Calcul des ordonnées */
                if (_m_trinomes_array.m_y_objects[tmpTupleIndex]->isNumber() || _m_trinomes_array.m_y_objects[tmpTupleIndex]->isDate())
                {
                    _m_y_array[tmpIndex] = _m_trinomes_array.m_y_objects[tmpTupleIndex]->doubleValue();
                }
                else
                {
                    if (_m_y_labels.size() > 0)
                    {
                        tmp_z_index = TrinomeArray::indexOfIdenticalObjectInArray<QString>(_m_trinomes_array.m_y_objects[tmpTupleIndex]->toString(), _m_y_labels);
                        _m_y_array[tmpIndex] = tmp_z_index * _m_y_step;
                    }
                    else
                    {
                        _m_y_array[tmpIndex] = (tmpTupleIndex + 1)* _m_y_step;
                    }
                }
            }
            else
            {
                /* Calcul des abscisses */
                if (tmp_x_object != NULL && (tmp_x_object->isNumber() || tmp_x_object->isDate()))
                {
                    _m_x_array[tmpIndex] = tmp_x_object->doubleValue();
                }
                else
                {
                    _m_x_array[tmpIndex] = (tmpIndexX + 1) * _m_x_step;
                }

                _m_y_array[tmpIndex] = 0;
            }
            tmpTupleIndex++;

        }
    }
}


/**
 * Dessiner l'histogramme
 */
void Chart::drawHistogram()
{
    uint		tmpIndex = 0;
    uint		tmpIndexX = 0;
    int		tmpIndexZ = 0;

    double	tmp_x_step = m_stack_histogram_indic ? _m_x_step - 2 : (_m_x_step - 2) / _m_z_categories_count;

    double	tmp_max_x = _m_graph_width;
    double	tmp_max_y = _m_graph_height;

    double	tmp_bar_height = 0;
    double	tmp_prev_y = 0;

    GradientColor	tmp_grad_color = GradientColor(_m_colors);

    QColor	tmp_paint_color;
    QRectF	tmp_str_rect;

    QString	tmp_str;

    tmp_max_x = _m_max_x_categories->doubleValue();
    tmp_max_y = _m_max_y_categories->doubleValue();

    /* Définir les coordonnées des points du graphique */
    _calculateGraphPoints();

    /* Dessiner les barres de l'histogramme */
    if (_m_z_categories_count > 1)
    {
        /* Parcourrir les catégories X */
        for(tmpIndexX = 0; tmpIndexX < _m_x_categories.size(); tmpIndexX++)
        {
            tmp_prev_y = _m_graph_origin_y;

            /* Parcourrir les catégories Z */
            for(tmpIndexZ = 0; tmpIndexZ < _m_z_categories_count; tmpIndexZ++)
            {
                tmpIndex = tmpIndexX * _m_z_categories_count + tmpIndexZ;

                /* Définir la couleur selon la série courante */
                tmp_paint_color = tmp_grad_color.colorAtIndex(tmpIndexZ);
                _m_Graphics2D->setPen(QColor(tmp_paint_color.red(),
                                             tmp_paint_color.green(),
                                             tmp_paint_color.blue(),
                                             _m_graph_alpha_paint_color));

                /* Dessiner um histogramme empilé */
                if (m_stack_histogram_indic)
                {
                    /* Ne pas dessiner si l'ordonnée est nulle---- (y=0) */
                    if (_m_y_array[tmpIndex] != 0)
                    {
                        /* Placer le point selon la taille et la position du graphe dans l'image */
                        _m_x_array[tmpIndex] = _m_graph_origin_x + _m_x_array[tmpIndex];
                        tmp_bar_height = _m_y_array[tmpIndex] * _m_graph_height / _m_trinomes_array.m_max_total_y_by_x_class;
                        _m_y_array[tmpIndex] = tmp_prev_y - tmp_bar_height;
                        tmp_prev_y = _m_y_array[tmpIndex];

                        fill3DRect(_m_x_array[tmpIndex] - _m_x_step + 1,
                                   _m_y_array[tmpIndex] + 1,
                                   tmp_x_step,
                                   tmp_bar_height - 2,
                                   m_raised_histogram_indic, _m_x_incline, _m_y_incline);

                        // Dessiner le libellé de la valeur Y
                        _m_Graphics2D->setPen(Qt::black);
                        tmp_str = _m_trinomes_array.m_y_objects[tmpIndex]->toString();
                        tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);
                        _m_Graphics2D->drawText(QPointF(
                                                    (float)(_m_x_array[tmpIndex] - _m_x_step + 1 + (_m_x_step - tmp_str_rect.width()) / 2),
                                                    (float)(_m_y_array[tmpIndex] + 1 + tmp_str_rect.height())),
                                                tmp_str);
                    }
                }
                else
                {
                    /* Placer le point selon la taille et la position du graphe dans l'image */
                    _m_x_array[tmpIndex] = _m_graph_origin_x + _m_x_array[tmpIndex];
                    _m_y_array[tmpIndex] = _m_graph_origin_y - _m_y_array[tmpIndex] * _m_graph_height / tmp_max_y;

                    LOG_TRACE(Session::instance()->getClientSession(), "X=%s,Y=%s,Z=%s\n", _m_trinomes_array.m_x_objects[tmpIndex]->toString().toStdString().c_str(),
                              _m_trinomes_array.m_y_objects[tmpIndex]->toString().toStdString().c_str(),
                              _m_trinomes_array.m_z_objects[tmpIndex]->toString().toStdString().c_str());

                    /* Ne pas dessiner si l'ordonnée est nulle---- (y=0) */
                    if (_m_graph_origin_y > _m_y_array[tmpIndex])
                    {
                        fill3DRect(_m_x_array[tmpIndex] - _m_x_step +  tmpIndexZ * tmp_x_step + 1,
                                   _m_y_array[tmpIndex] + 1,
                                   tmp_x_step,
                                   _m_graph_origin_y - _m_y_array[tmpIndex] - 2,
                                   m_raised_histogram_indic, _m_x_incline, _m_y_incline);

                        // Dessiner le libellé de la valeur Y
                        _m_Graphics2D->setPen(Qt::black);
                        tmp_str = _m_trinomes_array.m_y_objects[tmpIndex]->toString();
                        tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);
                        _m_Graphics2D->drawText(QPointF(
                                                    (float)(_m_x_array[tmpIndex] - _m_x_step +  tmpIndexZ * tmp_x_step + 1 + (tmp_x_step - tmp_str_rect.width()) / 2),
                                                    (float)(_m_y_array[tmpIndex] + tmp_str_rect.height())),
                                                tmp_str);
                    }
                    else if (m_raised_histogram_indic)
                    {
                        LOG_TRACE(Session::instance()->getClientSession(), "Null X=>%f, Y=%f\n", _m_x_array[tmpIndex], _m_y_array[tmpIndex]);
                        _m_Graphics2D->fillPath(Rectangle3D(_m_x_array[tmpIndex] - _m_x_step + 2 + tmp_x_step * tmpIndexZ,
                                                            _m_graph_origin_y - 2,
                                                            tmp_x_step - 2,
                                                            tmp_x_step / 2,
                                                            -_m_x_incline,
                                                            _m_y_incline,
                                                            0).path(), tmp_grad_color);
                    }
                }
            }
        }
    }
    else
    {
        /* Parcourrir le tableau de coordonnées */
        for(tmpIndex = 0; tmpIndex < _m_x_array.size(); tmpIndex++)
        {
            /* Placer le point selon la taille et la position du graphe dans l'image */
            _m_x_array[tmpIndex] = _m_graph_origin_x + _m_x_array[tmpIndex];
            _m_y_array[tmpIndex] = _m_graph_origin_y - _m_y_array[tmpIndex] * _m_graph_height / tmp_max_y;

            tmp_paint_color = tmp_grad_color.colorAtIndex(tmpIndex);
            _m_Graphics2D->setPen(QColor(tmp_paint_color.red(),
                                         tmp_paint_color.green(),
                                         tmp_paint_color.blue(),
                                         _m_graph_alpha_paint_color));

            /* Ne pas dessiner si l'ordonnée est nulle---- (y=0) */
            if (_m_graph_origin_y > _m_y_array[tmpIndex])
            {
                fill3DRect(_m_x_array[tmpIndex] - _m_x_step + 1,
                           _m_y_array[tmpIndex] + 1,
                           tmp_x_step,
                           _m_graph_origin_y - _m_y_array[tmpIndex] - 2,
                           m_raised_histogram_indic, _m_x_incline, _m_y_incline);

                // Dessiner le libellé de la valeur Y
                _m_Graphics2D->setPen(Qt::black);
                tmp_str = _m_trinomes_array.m_y_objects[tmpIndex]->toString();
                tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);
                _m_Graphics2D->drawText(QPointF(
                                            (float)(_m_x_array[tmpIndex] - _m_x_step +  1 + (_m_x_step - tmp_str_rect.width()) / 2),
                                            (float)(_m_y_array[tmpIndex] + tmp_str_rect.height())),
                                        tmp_str);
            }
            else if (m_raised_histogram_indic)
            {
                _m_Graphics2D->fillPath(Rectangle3D(_m_x_array[tmpIndex] - _m_x_step + 2,
                                                    _m_graph_origin_y - 2,
                                                    tmp_x_step - 2,
                                                    tmp_x_step / 2,
                                                    -_m_x_incline,
                                                    _m_y_incline,
                                                    0).path(), tmp_grad_color);
            }
        }
    }
}


/**
 * Dessiner le polygone
 */
void Chart::drawPolygon()
{
    double	tmp_min_x = 0.0;
    double	tmp_min_y = 0.0;
    double	tmp_max_x = _m_graph_width;
    double	tmp_max_y = _m_graph_height;

    double	tmp_prev_point_x = 0.0;
    double	tmp_prev_point_y = 0.0;

    double	tmp_x_step = 0.0;
    double	tmp_y_step = 0.0;

    double	tmp_delta_x = _m_x_step / 2;
    double	tmp_delta_y = 0.0;

    GradientColor	tmp_grad_color = GradientColor(_m_colors);
    QColor	tmp_paint_color;

    QString	tmp_str;
    QRectF	tmp_str_rect;

    uint		tmpIndex = 0;
    uint		tmpIndexX = 0;
    int		tmpIndexZ = 0;

    if (_m_abciss_proportionnal_indic)
    {
        tmp_min_x = _m_min_x_categories->doubleValue();
        tmp_max_x = _m_max_x_categories->doubleValue();
        tmp_x_step = 0.0;
        tmp_delta_x = 0.0;
    }
    if (_m_max_y_categories->isNumber() || _m_max_y_categories->isDate())
    {
        //if (_m_y_array.length > _m_trinomes_array.length)
        tmp_min_y = 0.0;
        //else
        //	tmp_min_y = _m_min_y_categories->doubleValue();
        tmp_max_y = _m_max_y_categories->doubleValue();
        tmp_y_step = 0.0;
        tmp_delta_y = 0.0;
    }

    /* Définir les coordonnées des points du graphique */
    _calculateGraphPoints();

    /* Dessiner la ligne polygonale */
    if (_m_z_categories_count > 1)
    {
        /* Parcourrir les catégories Z */
        for(tmpIndexZ = 0; tmpIndexZ < _m_z_categories_count; tmpIndexZ++)
        {
            /* Tracer le premier point de la catégorie X courante */
            _m_x_array[tmpIndexZ] = _m_graph_origin_x + (_m_x_array[tmpIndexZ] - tmp_min_x) * (_m_graph_width - tmp_x_step) / (tmp_max_x - tmp_min_x) - tmp_delta_x;
            _m_y_array[tmpIndexZ] = _m_graph_origin_y - (_m_y_array[tmpIndexZ] - tmp_min_y) * (_m_graph_height - tmp_y_step) / (tmp_max_y - tmp_min_y) - tmp_delta_y;
            tmp_prev_point_x = _m_x_array[tmpIndexZ];
            tmp_prev_point_y = _m_y_array[tmpIndexZ];

            tmp_paint_color = tmp_grad_color.colorAtIndex(tmpIndexZ);
            _m_Graphics2D->setPen(QColor(tmp_paint_color.red(),
                                         tmp_paint_color.green(),
                                         tmp_paint_color.blue(),
                                         _m_graph_alpha_paint_color));

            _m_Graphics2D->drawEllipse(QRectF(_m_x_array[tmpIndexZ] - 2, _m_y_array[tmpIndexZ] - 2, 4, 4));

            // Dessiner le libellé de l'ordonnée
            _m_Graphics2D->setPen(Qt::black);
            tmp_str = _m_trinomes_array.m_y_objects[tmpIndexZ]->toString();
            tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);
            _m_Graphics2D->drawText(QPointF(
                                        (float)(_m_x_array[tmpIndexZ] - tmp_str_rect.width() / 2),
                                        (float)(_m_y_array[tmpIndexZ] - 4)),
                                    tmp_str);

            /* Parcourrir les catégories Y */
            for(tmpIndexX = 1; tmpIndexX < _m_x_categories.size(); tmpIndexX++)
            {
                tmpIndex = tmpIndexX * _m_z_categories_count + tmpIndexZ;

                /* Mettre à l'échelle de la taille de l'image */
                _m_x_array[tmpIndex] = _m_graph_origin_x + (_m_x_array[tmpIndex] - tmp_min_x) * (_m_graph_width - tmp_x_step) / (tmp_max_x - tmp_min_x) - tmp_delta_x;
                _m_y_array[tmpIndex] = _m_graph_origin_y - (_m_y_array[tmpIndex] - tmp_min_y) * (_m_graph_height - tmp_y_step) / (tmp_max_y - tmp_min_y) - tmp_delta_y;

                _m_Graphics2D->setPen(QColor(tmp_paint_color.red(),
                                             tmp_paint_color.green(),
                                             tmp_paint_color.blue(),
                                             _m_graph_alpha_paint_color));
                _m_Graphics2D->drawEllipse(QRectF(_m_x_array[tmpIndex] - 2, _m_y_array[tmpIndex] - 2, 4, 4));
                _m_Graphics2D->drawLine(QLineF(tmp_prev_point_x, tmp_prev_point_y, _m_x_array[tmpIndex], _m_y_array[tmpIndex]));

                // Dessiner le libellé de l'ordonnée
                _m_Graphics2D->setPen(Qt::black);
                tmp_str = _m_trinomes_array.m_y_objects[tmpIndex]->toString();
                tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);
                _m_Graphics2D->drawText(QPointF(
                                            (float)(_m_x_array[tmpIndex] - tmp_str_rect.width() / 2),
                                            (float)(_m_y_array[tmpIndex] - 4)),
                                        tmp_str);

                tmp_prev_point_x = _m_x_array[tmpIndex];
                tmp_prev_point_y = _m_y_array[tmpIndex];
            }
        }
    }
    else
    {
        /* Tracer le premier point */
        _m_x_array[0] = _m_graph_origin_x + (_m_x_array[0] - tmp_min_x) * (_m_graph_width - tmp_x_step) / (tmp_max_x - tmp_min_x) - tmp_delta_x;
        _m_y_array[0] = _m_graph_origin_y - (_m_y_array[0] - tmp_min_y) * (_m_graph_height - tmp_y_step) / (tmp_max_y - tmp_min_y) - tmp_delta_y;
        tmp_prev_point_x = _m_x_array[0];
        tmp_prev_point_y = _m_y_array[0];
        _m_Graphics2D->setPen(_m_pencil_Color);
        _m_Graphics2D->drawEllipse(QRectF(_m_x_array[0] - 2, _m_y_array[0] - 2, 4, 4));

        // Dessiner le libellé de l'ordonnée
        _m_Graphics2D->setPen(Qt::black);
        tmp_str = _m_trinomes_array.m_y_objects[0]->toString();
        tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);
        _m_Graphics2D->drawText(QPointF(
                                    (float)(_m_x_array[0] - tmp_str_rect.width() / 2),
                                    (float)(_m_y_array[0] - 4)),
                                tmp_str);

        for(tmpIndex = 1; tmpIndex < _m_x_array.size(); tmpIndex++)
        {
            /* Mettre à l'échelle de la taille de l'image */
            _m_x_array[tmpIndex] = _m_graph_origin_x + (_m_x_array[tmpIndex] - tmp_min_x) * (_m_graph_width  - tmp_x_step) / (tmp_max_x - tmp_min_x) - tmp_delta_x;
            _m_y_array[tmpIndex] = _m_graph_origin_y - (_m_y_array[tmpIndex] - tmp_min_y) * (_m_graph_height - tmp_y_step) / (tmp_max_y - tmp_min_y) - tmp_delta_y;

            _m_Graphics2D->setPen(_m_pencil_Color);
            _m_Graphics2D->drawEllipse(QRectF(_m_x_array[tmpIndex] - 2, _m_y_array[tmpIndex] - 2, 4, 4));
            _m_Graphics2D->drawLine(QLineF(tmp_prev_point_x, tmp_prev_point_y, _m_x_array[tmpIndex], _m_y_array[tmpIndex]));

            // Dessiner le libellé de l'ordonnée
            _m_Graphics2D->setPen(Qt::black);
            tmp_str = _m_trinomes_array.m_y_objects[tmpIndex]->toString();
            tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);
            _m_Graphics2D->drawText(QPointF(
                                        (float)(_m_x_array[tmpIndex] - tmp_str_rect.width() / 2),
                                        (float)(_m_y_array[tmpIndex] - 4)),
                                    tmp_str);

            tmp_prev_point_x = _m_x_array[tmpIndex];
            tmp_prev_point_y = _m_y_array[tmpIndex];
        }
    }
}


/**
 * Dessiner un losange
 * @param in_x0
 * @param in_y0
 * @param in_x1
 * @param in_y1
 * @param in_thickness
 */
void Chart::_drawPortionSide(double in_x0, double in_y0, double in_x1, double in_y1, double in_thickness)
{
    QPointF         tmp_points[4];

    tmp_points[0] = QPointF(in_x0, in_y0);
    tmp_points[1] = QPointF(in_x0, in_y0 + in_thickness);
    tmp_points[2] = QPointF(in_x1, in_y1 + in_thickness);
    tmp_points[3] = QPointF(in_x1, in_y1);

    _m_Graphics2D->drawPolygon(tmp_points, 4);
}


/**
 * Tracer les losanges de cÃ´té de la portion
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
void Chart::_drawPortionSides(int in_index, double in_start, double in_end,
                              bool in_draw_left, bool in_draw_right,
                              double in_center_x, double in_center_y, double in_cheese_thickness,
                              QBrush in_paint_color)
{
    double 			tmp_cheese_width = _m_graph_width - _m_delta_cheese_portion * 2;
    double 			tmp_upper_left_x = in_center_x - tmp_cheese_width / 2;

    /* Dessiner le cÃ´té droit avant le gauche en fonction des angles */
    if (in_start < 90 || in_end > 270)
    {
        if (in_draw_right)
        {
            _drawPortionSide( _m_x_array[in_index], _m_y_array[in_index], in_center_x, in_center_y, in_cheese_thickness, in_paint_color);
        }

        if (in_draw_left)
        {
            if (in_index > 0)
                _drawPortionSide( _m_x_array[in_index - 1], _m_y_array[in_index - 1], in_center_x, in_center_y, in_cheese_thickness, in_paint_color);
            else
                _drawPortionSide( tmp_upper_left_x + tmp_cheese_width, in_center_y, in_center_x, in_center_y, in_cheese_thickness, in_paint_color);
        }
    }
    else
    {
        if (in_draw_left)
        {
            if (in_index > 0)
                _drawPortionSide( _m_x_array[in_index - 1], _m_y_array[in_index - 1], in_center_x, in_center_y, in_cheese_thickness, in_paint_color);
            else
                _drawPortionSide( tmp_upper_left_x + tmp_cheese_width, in_center_y, in_center_x, in_center_y, in_cheese_thickness, in_paint_color);
        }

        if (in_draw_right)
        {
            _drawPortionSide( _m_x_array[in_index], _m_y_array[in_index], in_center_x, in_center_y, in_cheese_thickness, in_paint_color);
        }
    }

    /* Tracer le segment central */
    //if (in_draw_left || in_draw_right)
    //	_m_Graphics2D->drawLine(QLineF(in_center_x, in_center_y, in_center_x, in_center_y + in_cheese_thickness));

}


/**
 * Tracer un losange de cÃ´té de la portion
 * @param in_x
 * @param in_y
 * @param in_center_x
 * @param in_center_y
 * @param in_cheese_thickness
 * @param in_paint_color
 */
void Chart::_drawPortionSide(double in_x, double in_y, double in_center_x, double in_center_y, double in_cheese_thickness, QBrush in_paint_color)
{
    _m_Graphics2D->setPen(Qt::black);
    _m_Graphics2D->setBrush(in_paint_color);
    _drawPortionSide(in_center_x, in_center_y, in_x, in_y, in_cheese_thickness);
    //_m_Graphics2D->setPen(Qt::black);
    /* Tracer le segment du centre à la périphérie */
    //_m_Graphics2D->drawLine(QLineF(in_x, in_y + in_cheese_thickness, in_center_x, in_center_y + in_cheese_thickness));
    /* Tracer le segment de fin de portion */
    //_m_Graphics2D->drawLine(QLineF(in_x, in_y, in_x, in_y + in_cheese_thickness));
}


/**
 * Dessiner une portion de fromage
 * @param in_index
 * @param in_start
 * @param in_end
 */
void Chart::drawCheesePortion(int in_index, double in_start, double in_end)
{
    QRectF          	tmp_upper_portion;
    QRectF          	tmp_lower_portion;

    double 			tmp_cheese_width = _m_graph_width - _m_delta_cheese_portion * 2;
    double 			tmp_cheese_height = (_m_graph_height - _m_delta_cheese_portion * 2) * sin(RAD*(_m_y_incline));
    double 			tmp_cheese_thickness = _m_graph_height - tmp_cheese_height;

    double 			tmp_center_x = _m_graph_origin_x + _m_graph_width / 2;
    double 			tmp_center_y = _m_graph_origin_y - _m_graph_height + _m_graph_height * sin(RAD*(_m_y_incline)) / 2;

    double 			tmp_upper_left_x = tmp_center_x - tmp_cheese_width / 2;
    double 			tmp_upper_left_y = tmp_center_y - tmp_cheese_height / 2;

    double			tmp_extend = in_end - in_start;

    double			tmp_start = (in_index > 0 ? _m_angles_array[in_index - 1] : 0);

    double			tmp_first_x;
    double			tmp_last_x;

    double			tmp_first_y;
    double			tmp_last_y;

    GradientColor           tmp_grad_color = GradientColor(_m_colors);
    QLinearGradient         tmp_grad_paint;
    QLinearGradient         tmp_grad_portion_paint;
    QColor			tmp_paint_color;

    QPainterPath            tmp_path;

    bool			tmp_draw_left = (in_start == (in_index > 0 ? _m_angles_array[in_index - 1] : 0));
    bool			tmp_draw_right = (in_end == _m_angles_array[in_index]);

    QTransform		tmp_origin_aft;
    QTransform		tmp_aft;

    double			tmp_delta_x;
    double			tmp_delta_y;

    int				tmp_z_index = 0;

    LOG_TRACE(Session::instance()->getClientSession(), "[Chart] \t\t- portion n°%i(de %f° à %f° - l=%s, r=%s)\n", in_index, in_start, in_end, (tmp_draw_left?"O":"N"), (tmp_draw_right?"O":"N"));

    if (tmp_extend > 0)
    {
        /* Dessiner la portion de fromage décalée de delta pixels par rapport au centre */
        if (_m_delta_cheese_portion > 0)
        {
            tmp_origin_aft = _m_Graphics2D->transform();

            /* Définir le décalage horizontal et vertical */
            if (tmp_draw_left == false || tmp_draw_right == false)
            {
                tmp_delta_x = cos(RAD*((_m_angles_array[in_index] + (in_index > 0 ? _m_angles_array[in_index - 1] : 0))/2)) * _m_delta_cheese_portion;
                tmp_delta_y = - sin(RAD*((_m_angles_array[in_index] + (in_index > 0 ? _m_angles_array[in_index - 1] : 0))/2)) * _m_delta_cheese_portion * sin(RAD*(_m_y_incline));
            }
            else
            {
                tmp_delta_x = cos(RAD*(in_end - tmp_extend/2)) * _m_delta_cheese_portion;
                tmp_delta_y = - sin(RAD*(in_end - tmp_extend/2)) * _m_delta_cheese_portion * sin(RAD*(_m_y_incline));
            }

            tmp_aft.translate(tmp_delta_x, tmp_delta_y);

            /* Appliquer la transformation affine au contexte graphique */
            _m_Graphics2D->setTransform(tmp_aft);
        }

        /* Définir la couleur de la portion */
        if (_m_z_categories_count > 1)
        {
            tmp_z_index = TrinomeArray::indexOfObjectInArray<DataObject>(_m_trinomes_array.m_z_objects[in_index], _m_z_categories);
            tmp_paint_color = tmp_grad_color.colorAtIndex(tmp_z_index);
        }
        else
            tmp_paint_color = tmp_grad_color.colorAtIndex(in_index);

        tmp_paint_color = QColor(tmp_paint_color.red(),
                                 tmp_paint_color.green(),
                                 tmp_paint_color.blue(),
                                 _m_graph_alpha_paint_color);

        tmp_grad_paint = QLinearGradient(QPointF(_m_graph_origin_x, tmp_center_y),
                                         QPoint(_m_graph_origin_x + _m_graph_width, tmp_center_y));

        tmp_grad_paint.setColorAt(0, tmp_paint_color);
        tmp_grad_paint.setColorAt(1, tmp_paint_color.darker());

        /* Définir les parties haute et basse de la portion */
        tmp_lower_portion = QRectF(tmp_upper_left_x, tmp_upper_left_y + tmp_cheese_thickness, tmp_cheese_width, tmp_cheese_height);
        tmp_upper_portion = QRectF(tmp_upper_left_x, tmp_upper_left_y, tmp_cheese_width, tmp_cheese_height);

        if (tmp_cheese_thickness > 0)
        {
            if (tmp_draw_left && tmp_draw_right)
                _m_Graphics2D->setPen(Qt::black);
            else
                _m_Graphics2D->setPen(Qt::transparent);

            /* Dessiner la partie basse de la portion en premier */
            _m_Graphics2D->setBrush(tmp_grad_paint);
            _m_Graphics2D->drawPie(tmp_lower_portion, in_start*QT_ANGLE_RATIO, tmp_extend*QT_ANGLE_RATIO);

            if (tmp_draw_left == false && tmp_draw_right)
            {
                _m_Graphics2D->setPen(Qt::black);
                _m_Graphics2D->setBrush(Qt::transparent);
                _m_Graphics2D->drawPie(tmp_upper_portion, tmp_start*QT_ANGLE_RATIO, (in_end-tmp_start)*QT_ANGLE_RATIO);
            }

            /* Tracer les losanges de cÃ´tés */
            _drawPortionSides(in_index, in_start, in_end,
                              tmp_draw_left && in_start >= 90 && in_start < 270, tmp_draw_right && (in_end < 90 || in_end >= 270),
                              tmp_center_x, tmp_center_y, tmp_cheese_thickness,
                              tmp_grad_paint);

            /* Dessiner le bord de portion du fromage */
            _m_Graphics2D->setPen(Qt::transparent);
            // Dessiner le bord en deux fois
            if (in_start < 180 && in_end > 180)
            {
                _m_Graphics2D->setPen(Qt::black);
                /* Tracer une ligne à gauche et à droite */
                _m_Graphics2D->drawLine(QLineF(tmp_upper_left_x, tmp_center_y, tmp_upper_left_x, tmp_center_y + tmp_cheese_thickness));
                //_m_Graphics2D->drawLine(QLineF(tmp_upper_left_x + tmp_cheese_width, tmp_center_y, tmp_upper_left_x + tmp_cheese_width, tmp_center_y + tmp_cheese_thickness));
                _m_Graphics2D->setPen(Qt::transparent);

                // 1ere partie jusqu'à 180°
                tmp_first_x = tmp_center_x + cos(RAD*(in_start)) * tmp_cheese_width / 2;
                tmp_last_x = tmp_center_x + cos(RAD*(180)) * tmp_cheese_width / 2;

                tmp_first_y = tmp_center_y + sin(RAD*(in_start)) * tmp_cheese_height / 2;
                tmp_last_y = tmp_center_y + sin(RAD*(180)) * tmp_cheese_height / 2;

                tmp_path = QPainterPath(QPointF(tmp_first_x, tmp_first_y));
                tmp_path.lineTo(tmp_first_x, tmp_first_y + tmp_cheese_thickness);
                tmp_path.arcTo(tmp_lower_portion, in_start, 180 - in_start);
                tmp_path.lineTo(tmp_last_x, tmp_last_y);
                tmp_path.arcTo(tmp_upper_portion, 180, in_start - 180);
                _m_Graphics2D->drawPath(tmp_path);

                // 2eme partie au delÃ  de 180°
                tmp_first_x = tmp_center_x + cos(RAD*(180)) * tmp_cheese_width / 2;
                tmp_last_x = tmp_center_x + cos(RAD*(in_end)) * tmp_cheese_width / 2;

                tmp_first_y = tmp_center_y + sin(RAD*(180)) * tmp_cheese_height / 2;
                tmp_last_y = tmp_center_y + sin(RAD*(in_end)) * tmp_cheese_height / 2;

                tmp_path = QPainterPath(QPointF(tmp_first_x, tmp_first_y));
                tmp_path.lineTo(tmp_first_x, tmp_first_y + tmp_cheese_thickness);
                tmp_path.arcTo(tmp_lower_portion, 180, in_end - 180);
                tmp_path.lineTo(tmp_last_x, tmp_last_y);
                tmp_path.arcTo(tmp_upper_portion, in_end, 180 - in_end);
                _m_Graphics2D->drawPath(tmp_path);
            }
            else
            {
                tmp_first_x = tmp_center_x + cos(RAD*(in_start)) * tmp_cheese_width / 2;
                tmp_last_x = tmp_center_x + cos(RAD*(in_end)) * tmp_cheese_width / 2;

                tmp_first_y = tmp_center_y + sin(RAD*(in_start)) * tmp_cheese_height / 2;
                tmp_last_y = tmp_center_y + sin(RAD*(in_end)) * tmp_cheese_height / 2;

                tmp_path = QPainterPath(QPointF(tmp_first_x, tmp_first_y));
                tmp_path.lineTo(tmp_first_x, tmp_first_y + tmp_cheese_thickness);
                tmp_path.arcTo(tmp_lower_portion, in_start, in_end - in_start);
                tmp_path.lineTo(tmp_last_x, tmp_last_y);
                tmp_path.arcTo(tmp_upper_portion, in_end, in_start - in_end);
                _m_Graphics2D->drawPath(tmp_path);
            }


            /* Tracer les losanges de cotés */
            _drawPortionSides(in_index, in_start, in_end,
                              tmp_draw_left && (in_start < 90 || in_start >= 270), tmp_draw_right && in_end >= 90 && in_end < 270,
                              tmp_center_x, tmp_center_y, tmp_cheese_thickness,
                              tmp_grad_paint);

        }

        /* Dessiner la partie haute de la portion en dernier */
        if (tmp_draw_left && tmp_draw_right)
            _m_Graphics2D->setPen(Qt::black);
        else
            _m_Graphics2D->setPen(Qt::transparent);

        _m_Graphics2D->setBrush(tmp_grad_paint);
        _m_Graphics2D->drawPie(tmp_upper_portion, in_start*QT_ANGLE_RATIO, tmp_extend*QT_ANGLE_RATIO);

        _m_Graphics2D->setPen(Qt::black);
        if (tmp_draw_left == false && tmp_draw_right)
        {
            _m_Graphics2D->setBrush(Qt::transparent);
            _m_Graphics2D->drawPie(tmp_upper_portion, tmp_start*QT_ANGLE_RATIO, (in_end-tmp_start)*QT_ANGLE_RATIO);
        }

        _m_Graphics2D->setTransform(tmp_origin_aft);
    }
}


/**
 * Trier les portions de fromage dans l'ordre où elles doivent être dessinées, afin d'éviter les chevauchements de dessins.
 * @return chaine triée dont chaque maillon contient l'index de portion
 */
QList<Integer> Chart::orderedDrawingCheesePortions()
{
    uint				tmpIndex = 0;
    int				tmp_portion_index = 0;
    int				tmp_last_portion_index = 0;
    double			tmp_total_value = _m_trinomes_array.m_total_y > 0.0 ? _m_trinomes_array.m_total_y : _m_trinomes_array.length;

    double 			tmp_current_angle = 0;
    double 			tmp_previous_angle = 0;

    double 			tmp_cheese_width = _m_graph_width - _m_delta_cheese_portion * 2;
    double 			tmp_cheese_height = (_m_graph_height - _m_delta_cheese_portion * 2) * sin(RAD*(_m_y_incline));

    double 			tmp_center_x = _m_graph_origin_x + _m_graph_width / 2;
    double 			tmp_center_y = _m_graph_origin_y - _m_graph_height + _m_graph_height * sin(RAD*(_m_y_incline)) / 2;

    QList<Integer>			tmp_ordered_portions;

    _m_angles_array = vector<double>(_m_x_array.size());

    /* Parcourrir la liste des valeurs */
    for(tmpIndex = 0; tmpIndex < _m_angles_array.size(); tmpIndex++)
    {
        if (_m_trinomes_array.m_y_objects[tmpIndex]->isNumber())
        {
            /* Calcul de l'angle de la portion courante pris sur les 360° */
            tmp_current_angle = 360.0 * _m_trinomes_array.m_y_objects[tmpIndex]->doubleValue() / tmp_total_value;
            _m_angles_array[tmpIndex] = tmp_previous_angle + tmp_current_angle;

            /* Coordonnées du dernier point de la portion courante sur le cercle */
            _m_x_array[tmpIndex] = tmp_center_x + cos(RAD*(_m_angles_array[tmpIndex])) * tmp_cheese_width / 2;
            _m_y_array[tmpIndex] = tmp_center_y - sin(RAD*(_m_angles_array[tmpIndex])) * tmp_cheese_height /2;

            if (tmp_current_angle > 0)
            {
                /* Placer la portion selon l'ordre de son dessin */
                if (tmp_previous_angle < 90 || _m_angles_array[tmpIndex] <= 90)
                {
                    /* Mettre en permier les portions à cheval sur l'angle des 90 degrés du fromage */
                    tmp_ordered_portions.insert(0, Integer(tmpIndex));
                    tmp_last_portion_index = tmp_portion_index + 1;
                }
                else
                {
                    /* Mettre les portions placées au delÃ  des 270 degrés du fromage
                     * avant la dernière placée avant les 270 degrès */
                    if (tmp_previous_angle >= 270 && _m_angles_array[tmpIndex] > 270)
                    {
                        tmp_ordered_portions.insert(tmp_last_portion_index, Integer(tmpIndex));
                    }
                    else
                    {
                        /* Mettre la portion courante à la suite des autres */
                        tmp_ordered_portions.append(Integer(tmpIndex));
                        tmp_last_portion_index = tmp_portion_index;
                    }
                }

                tmp_previous_angle = tmp_previous_angle + tmp_current_angle;
                tmp_portion_index++;

            }
        }
    }

    return tmp_ordered_portions;
}


/**
 * Dessiner le fromage portion par portion.
 */
void Chart::drawCheese()
{
    uint			tmpIndex = 0;
    int			tmpNextIndex = 0;

    double		tmp_previous_angle = 0;

    Integer		tmpObject(0);

    /* Trier les portions par ordre de dessin */
    QList<Integer>	tmp_chain_portions = orderedDrawingCheesePortions();
    QList<Integer>::iterator tmp_chain_portions_iter = tmp_chain_portions.begin();

    /* Parcourrir la chaine de portions triées et les dessiner */
    while (tmp_chain_portions_iter != tmp_chain_portions.end())
    {
        tmpObject = (*tmp_chain_portions_iter);
        //    if (tmpObject)
        //	{
        tmpIndex = tmpObject.intValue();

        tmp_previous_angle = tmpIndex > 0 ? _m_angles_array[tmpIndex - 1] : 0;

        /* Traiter le chevauchement pour les portions dont le début est inférieur à 90°
             * et la fin supérieur à 270° */
        if (tmp_chain_portions_iter + 1 != tmp_chain_portions.end() && tmp_previous_angle < 90 && _m_angles_array[tmpIndex] > 270)
        {
            tmp_chain_portions_iter++;

            /* Dessiner la premiere tranche de portion inférieure à 90° jusqu'à  180° */
            drawCheesePortion(tmpIndex, tmp_previous_angle, 180);

            /* Dessiner les portions suivantes dans la liste */
            while (tmp_chain_portions_iter != tmp_chain_portions.end())
            {
                tmpObject = (*tmp_chain_portions_iter);
                tmpNextIndex = tmpObject.intValue();
                drawCheesePortion(tmpNextIndex, tmpNextIndex > 0 ?_m_angles_array[tmpNextIndex - 1] : 0, _m_angles_array[tmpNextIndex]);
                tmp_chain_portions_iter++;
            }

            /* Dessiner la seconde tranche de portion de 180° et supérieure à 270° */
            drawCheesePortion(tmpIndex, 180, _m_angles_array[tmpIndex]);
        }
        /* Traitement normal du dessin d'une portion à la suite des autres */
        else
        {
            drawCheesePortion(tmpIndex, tmp_previous_angle, _m_angles_array[tmpIndex]);
            tmp_chain_portions_iter++;
        }
        //	}
        //	else
        //	{
        //	    tmp_chain_portions = tmp_chain_portions->next();
        //	}
    }


    /* Dessiner les libellés de portions */
    for (tmpIndex = 0; tmpIndex < _m_angles_array.size(); tmpIndex++)
    {
        drawCheesePortionLegend(tmpIndex);
    }
}


/**
 * Dessiner la légende d'une portion du fromage
 * @param in_index	index de la portion
 */
void Chart::drawCheesePortionLegend(int in_index)
{
    double 			tmp_cheese_height = _m_graph_height * sin(RAD*(_m_y_incline));

    double 			tmp_center_x = _m_graph_origin_x + _m_graph_width / 2;
    double 			tmp_center_y = _m_graph_origin_y - _m_graph_height + tmp_cheese_height / 2;

    QPoint			tmp_portion_center;

    QString                 tmp_str;
    QRectF                  tmp_str_rect;

    double			tmp_extend = _m_angles_array[in_index] - (in_index > 0 ? _m_angles_array[in_index - 1] : 0);
    double			tmp_center_angle = _m_angles_array[in_index] - tmp_extend / 2;

    QFont		    tmp_font = _m_graph_font;

    if (tmp_extend > 0)
    {
        /* Calcul de la position où écrire le pourcentage de la portion */
        tmp_portion_center = translateOf(tmp_center_x + cos(RAD*(tmp_center_angle)) * _m_graph_width / 2,
                                         tmp_center_y - sin(RAD*(tmp_center_angle)) * tmp_cheese_height /2,
                                         tmp_center_x,
                                         tmp_center_y,
                                         0.3);

        tmp_font.setPixelSize(_m_graph_font.pixelSize() + 3);
        _m_Graphics2D->setFont(tmp_font);

        tmp_str = QString::number(100 * tmp_extend / 360.0, 'f', 0) + "% (" + _m_trinomes_array.m_y_objects[in_index]->toString() + ")";
        tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);

        /* Dessiner le pourcentage de la valeur Y */
        _m_Graphics2D->drawText(QPointF(
                                    (float)(tmp_portion_center.x() - tmp_str_rect.width() / 2),
                                    (float)(tmp_portion_center.y() + tmp_str_rect.height() / 2)),
                                tmp_str);

        /* Dessiner la valeur de la série X courante */
        if (_m_z_categories_count > 1)
        {
            tmp_str = _m_trinomes_array.m_x_objects[in_index]->toString();
            tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);
            _m_Graphics2D->drawText(QPointF(
                                        (float)(tmp_portion_center.x() - tmp_str_rect.width() / 2),
                                        (float)(tmp_portion_center.y() + tmp_str_rect.height() / 2 + _m_graph_font_size + 1)),
                                    tmp_str);
        }

        _m_Graphics2D->setFont(_m_graph_font);
    }
}


/**
 * Dessiner le graphe
 */
void Chart::draw()
{
    QString	tmp_str;
    QRectF  tmp_str_rect;

    if (_m_trinomes_array.length > 0)
    {
        LOG_TRACE(Session::instance()->getClientSession(), "[Chart] dessiner :\n");

        /* Initialiser les paramètres du graphique avant le dessin */
        initDrawing();

        /* Dessiner les axes */
        LOG_TRACE(Session::instance()->getClientSession(), "[Chart] \t- les axes\n");

        drawAxis();

        /* Dessiner le graphiqe selon le type */
        LOG_TRACE(Session::instance()->getClientSession(), "[Chart] \t- le graphique\n");

        switch (_m_type)
        {
            case GRAPH_TYPE_HISTOGRAM :
                drawHistogram();
                break;
            case GRAPH_TYPE_POLYGON :
                drawPolygon();
                break;
            case GRAPH_TYPE_CHEESE :
                drawCheese();
                break;
        }

        /* Dessiner la légende */
        LOG_TRACE(Session::instance()->getClientSession(), "[Chart] \t- la legende\n");
        drawLegend();

        /* Dessiner le titre */
        LOG_TRACE(Session::instance()->getClientSession(), "[Chart] \t- le titre\n");
        drawTitle();

        /* Dessiner le copyright */
        LOG_TRACE(Session::instance()->getClientSession(), "[Chart] \t- le copyright\n");
        _m_Graphics2D->setPen(Qt::black);
        _m_Graphics2D->setFont(QFont(_m_graph_font.family(), _m_graph_font.pointSize(), _m_graph_font.weight(), true));

        tmp_str = "Copyright R.T.M.R 2012";
        tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);

        _m_Graphics2D->drawText(QPointF(
                                    (float)(_m_graph_origin_x + _m_graph_width - tmp_str_rect.width() - 5),
                                    (float)(_m_graph_origin_y - 5 )),
                                tmp_str);

        _m_Graphics2D->setFont(_m_graph_font);

    }
    else
    {
        _m_Graphics2D->setPen(Qt::red);
        _m_Graphics2D->setFont(QFont(_m_graph_font.family(), _m_graph_font.pointSize(), _m_graph_font.weight(), true));

        tmp_str = "Aucune donnée à afficher.";
        tmp_str_rect = _m_Graphics2D->boundingRect(QRectF(), Qt::AlignLeft | Qt::AlignTop, tmp_str);

        _m_Graphics2D->drawText(QPointF(
                                    (float)(_m_width - tmp_str_rect.width()) / 2,
                                    (float)(_m_height - tmp_str_rect.height()) / 2),
                                tmp_str);

        _m_Graphics2D->setFont(_m_graph_font);
    }
}


void Chart::setType(Chart::GraphType in_graph_type)
{
    _m_type = in_graph_type;
}


/**
 * Dessiner le graphe d'après le type passé en paramètre
 * @param in_graph_type : une des valeurs GRAPH_TYPE_HISTOGRAM, GRAPH_TYPE_POLYGON...etc
 */
void Chart::drawGraph (Chart::GraphType in_graph_type)
{
    _m_type = in_graph_type;

    draw();
}

/*-----------------------------------------------------------------
                    GETTERS ET SETTERS
------------------------------------------------------------------*/

/**
 * Renvoie la couleur de dessin du graphique au format html/javascript<br>
 * ex : #FF0000
 */
QString Chart::getGraphColor(){return javaColorToJsColor(_m_pencil_Color);}


/**
 * Renvoie la couleur du titre au format html/javascript<br>
 * ex : #FF0000
 */
QString Chart::getTitleColor(){return javaColorToJsColor(_m_graph_title_color);}


/**
 * Définir la couleur de dessin du graphique
 * @param	in_js_color	couleur au format html/javascript (ex : #FF0000)
 */
void Chart::setGraphColor(QString in_js_color){setColors(jsColorToJavaColor(in_js_color), _m_background_Color);}


/**
 * Définir la couleur du titre
 * @param	in_js_color	couleur au format html/javascript (ex : #FF0000)
 */
void Chart::setTitleColor(QString in_js_color){_m_graph_title_color = jsColorToJavaColor(in_js_color);}


/**
 * Définir les couleurs de dessin des catégories.
 * @param	in_colors	tableau d'objets de type <code>QColor</code> pour les couleurs Qt,<br>
 * ou de type <code>QString</code> pour les couleurs au format html/javascript
 */
void Chart::setGraphColors(vector<QColor> in_colors)
{
    if (in_colors.size() > 0)
    {
        _m_colors = in_colors;

        _m_pencil_Color = _m_colors[0];
    }
    else
    {
        _m_colors = COLORS;
    }
}


/**
 * Renvoie le taux d'opacité ( != transparence) du graphique.
 */
int Chart::getGraphOpacity(){return 100 * _m_graph_alpha_paint_color / 0xFF;}


/**
 * Définir le taux d'opacité ( != transparence) du graphique.
 * @param	in_alpha	taux d'opacité
 */
void Chart::setGraphOpacity(int in_alpha)
{
    if (in_alpha >= 0 && in_alpha <= 100)
    {
        _m_graph_alpha_paint_color = in_alpha * 0xFF / 100;
    }
}


/**
 * Renvoie la largeur du graphique en nombre de pixels.
 */
double Chart::getGraphWidth() {return _m_graph_width;}


/**
 * Renvoie la hauteur du graphique en nombre de pixels.
 */
double Chart::getGraphHeight() {return _m_graph_height;}


/**
 * Renvoie l'inclinaison vertical du graphique en degrés celcius.
 */
double Chart::getInclineY() {return _m_y_incline;}


/**
 * Définir l'inclinaison vertical du graphique en degrés celcius.
 * @param	in_incline	angle d'inclinaison vertical
 */
void Chart::setInclineY(int in_incline)
{
    if (in_incline >= 0.0 && in_incline <= 90.0)
    {
        _m_y_incline = in_incline;
    }
}


/**
 * Renvoie l'inclinaison horizontal du graphique en degrés celcius.
 */
double Chart::getInclineX() {return _m_x_incline;}


/**
 * Définir l'inclinaison horizontal du graphique en degrés celcius.
 * @param	in_incline	angle d'inclinaison vertical
 */
void Chart::setInclineX(int in_incline)
{
    if (in_incline >= 0.0 && in_incline <= 90.0)
    {
        _m_x_incline = in_incline;
    }
}


/**
 * Renvoie la taille de la bordure à gauche du graphique exprimée en nombre de pixels.
 */
double Chart::getBorderLeft() {return _m_graph_origin_x;}


/**
 * Renvoie la taille de la bordure à droite du graphique exprimée en nombre de pixels.
 */
double Chart::getBorderRight() {return _m_width - _m_graph_width - _m_graph_origin_x;}


/**
 * Renvoie la taille de la bordure en haut du graphique exprimée en nombre de pixels.
 */
double Chart::getBorderTop() {return _m_graph_origin_y - _m_graph_height;}


/**
 * Renvoie la taille de la bordure en bas du graphique exprimée en nombre de pixels.
 */
double Chart::getBorderBottom() {return _m_height - _m_graph_origin_y;}


/**
 * Définir la taille de la bordure à gauche du graphique.
 * @param	in_size	taille exprimée en nombre de pixels
 */
void Chart::setBorderLeft(double in_size){setBorderSize(in_size, -1, -1, -1);}


/**
 * Définir la taille de la bordure à droite du graphique.
 * @param	in_size	taille exprimée en nombre de pixels
 */
void Chart::setBorderRight(double in_size){setBorderSize(-1, in_size, -1, -1);}


/**
 * Définir la taille de la bordure en haut du graphique.
 * @param	in_size	taille exprimée en nombre de pixels
 */
void Chart::setBorderTop(double in_size){setBorderSize(-1, -1, in_size, -1);}


/**
 * Définir la taille de la bordure en bas du graphique.
 * @param	in_size	taille exprimée en nombre de pixels
 */
void Chart::setBorderBottom(double in_size){setBorderSize(-1, -1, -1, in_size);}


/**
 * Définir le titre.
 * @param in_title	chaÃ®ne de caractères
 */
void Chart::setTitle(QString in_title){_m_title = in_title;}


/**
 * Renvoie le titre du graphique.
 */
QString Chart::getTitle(){return _m_title;}


/**
 * Renvoie la police d'affichage du titre.
 * @return	un objet de type
 * @see java.awt.Font
 */
QFont Chart::getFontTitle(){return _m_title_font;}


/**
 * Définir la police d'affichage du titre.
 * @param in_font	un objet de type
 * @see java.awt.Font
 */
void Chart::setFontTitle(QFont in_font)
{
    if (in_font != _m_title_font)
    {
        _m_title_font = in_font;
    }
}


/**
 * Définir l'espacement entre les portions d'un fromage.
 * @param in_delta	exprimée en nombre de pixels
 */
void Chart::setCheesePortionSpace(int in_delta){_m_delta_cheese_portion = in_delta;}


/**
 * Renvoie l'espacement entre les portions d'un fromage.
 */
int Chart::getCheesePortionSpace(){return (int)_m_delta_cheese_portion;}


/**
 * Renvoie le tableau des abcisses des points du graphique
 * @return
 */
vector<double> Chart::getHorizontalCoords()
{
    return _m_x_array;
}


/**
 * Renvoie le tableau des ordonnées des points du graphique
 * @return
 */
vector<double> Chart::getVerticalCoords()
{
    return _m_y_array;
}


/**
 * Définir la position de la légende
 */
void Chart::setLegendPosition(int in_legend_position)
{
    _m_graph_legend_position = in_legend_position;
}

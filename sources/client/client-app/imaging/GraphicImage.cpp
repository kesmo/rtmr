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

#include "GraphicImage.h"
#include "Rectangle3D.h"
#include <cmath>


static const char		_OPERATORS[] = {'+', '-', '*', '/', '^', 'V'};

Result::Result()
{
    _m_value = NULL;
}

void Result::setValue(Double *in_value)
{
	_m_value = in_value;
}

Double* Result::getValue()
{
	return _m_value;
}


/**
 * Constructeur
 */
Op::Op()
{
	m_operator = -1;
}


/**
 * Constructeur
 */
Op::Op(DataObject *in_value, int in_operator)
{
	m_value = in_value;
	m_operator = in_operator;
}

/**
 * Calculer le résultat d'une équation
 * @param in_expression	chaéne définissant l'équation (ex : "((x+5)*2)^3")
 * @param in_value	valeur de l'inconnue 'x' dans l'équation
 * @return
 */
double Op::compute(QString *in_expression, double in_value)
{
    Result				*tmp_result = new Result();

    Chain<Op> *eq_chain = _stringToChainEquation(in_expression);


    _operate(NULL, eq_chain, tmp_result, in_value);
    if (tmp_result->getValue() == NULL)	return 0.0;

    return tmp_result->getValue()->doubleValue();
}


/**
 * Transfomer une équation en liste (Chain) d'opérations
 * @param in_expression	chaéne é transformer
 **/
Chain<Op>* Op::_stringToChainEquation(QString *in_expression)
{
	int					tmp_cursor_index = 0;
	Chain<Op>			*tmp_operators_list = new Chain<Op>();
	Chain<Op>			*tmp_current_chain = tmp_operators_list;
	QByteArray			tmp_chars;

	if (in_expression == NULL || in_expression->length() == 0)	return NULL;

	tmp_chars = in_expression->replace("\\s", "").toAscii();

	/* Générer la liste des opérations */
	while (tmp_cursor_index < tmp_chars.length())
	{
		tmp_cursor_index = _getOperation(tmp_current_chain, tmp_chars, tmp_cursor_index);
                if (tmp_cursor_index < 0 || tmp_cursor_index >= tmp_chars.length()) break;

                tmp_current_chain->add();
                tmp_current_chain = tmp_current_chain->next();
	}

	if (tmp_cursor_index < 0)	return NULL;

	return tmp_operators_list;
}


/**
 * Renvoie la position de la parenthése droite correspondante é la position de la premiére parenthése gauche<br>
 * Exemples d'appels de la fonction :<br>
 * <ul><li>_rigthParenthesisIndex("((2*X)^3)", 0) renvoie la valeur 8.</li>
 * <li>_rigthParenthesisIndex("((2*X)^3)", 1) renvoie la valeur 5.</li></ul>
 * @param in_chars_array	chaéne ou chercher la parenthése droite
 * @param in_start_index	position de la premiére parenthése gauche
 * @return
 */
int Op::_rigthParenthesisIndex(QByteArray in_chars_array, int in_start_index)
{
	int		tmp_lp_index, tmp_rp_index;

	tmp_rp_index = Image::indexOfChar(')', in_chars_array, in_start_index + 1);
	if (tmp_rp_index < 0)	return -1;

	tmp_lp_index = Image::indexOfChar('(', in_chars_array, in_start_index + 1);
	if (tmp_lp_index < 0 || tmp_rp_index < tmp_lp_index)	return tmp_rp_index;

	return _rigthParenthesisIndex(in_chars_array, tmp_rp_index + 1);
}


/**
 * Récupérer l'opération courante
 * @param in_operator_list	Liste des opérations
 * @param chars_array	équation sous forme de chaéne
 * @param in_index	position courante du curseur dans la chaéne
 * @return
 */
int Op::_getOperation(Chain<Op>	*in_operator_list, QByteArray in_chars_array, int in_index)
{
	bool	tmp_operator_indic = false;
	int		out_index = in_index;
	int		tmp_index = -1;
	Op		*tmp_operande = new Op();
	QString	*tmp_str = NULL;

	if (in_index >= in_chars_array.length()) return in_index;

	/* Chercher l'opérande */
	switch (in_chars_array[out_index])
	{
		case '-':
			in_operator_list->setData(new Op(new Double(-1), Op::_OPERATOR_MULT));
			return ++out_index;
		case '+':
			return _getOperation(in_operator_list, in_chars_array, ++out_index);
		case 'x':
		case 'X':
			tmp_operande->m_value = new String("x");
			out_index++;
			break;
		case '(':

			tmp_index = _rigthParenthesisIndex(in_chars_array, ++out_index);
			if (tmp_index < 0) return -1;

			tmp_operande->m_sub_expression = _stringToChainEquation(new QString(Image::subCharArray(in_chars_array, out_index, tmp_index)));

			out_index = tmp_index + 1;
			break;
		default:
			while (out_index < in_chars_array.length() && !tmp_operator_indic)
			{
				switch (in_chars_array[out_index]){
					case '.':
					case '0':
					case '1':
					case '2':
					case '3':
					case '4':
					case '5':
					case '6':
					case '7':
					case '8':
					case '9':
						out_index++;
						break;
					default :
						tmp_operator_indic = true;
				}
			}

			if (out_index == in_index)	return -1;

			tmp_str = new QString(in_chars_array);
			tmp_operande->m_value = new Double(tmp_str->mid(in_index, out_index - in_index).toDouble());
			break;
	}


	in_operator_list->setData(tmp_operande);

	/* Chercher l'opérateur */
	tmp_index = _getOperator(in_operator_list, in_chars_array, out_index);
	if (tmp_index >= 0)	out_index = tmp_index;

	return out_index;
}


/**
 * Récupérer l'opérateur courant
 * @param in_operator_list	Liste des opérations
 * @param chars_array	équation sous forme de chaéne
 * @param in_index	position courante du curseur dans la chaéne
 * @return
 */
int Op::_getOperator(
        Chain<Op>	*in_operator_list,
        QByteArray	in_chars_array,
	int		in_index)
{
	int 	out_index;
	Op		*tmp_operande = NULL;

	if (in_index >= in_chars_array.length()) return in_index;

	out_index = in_index;
	tmp_operande = (Op*)in_operator_list->getData();

	if ((tmp_operande->m_operator = Image::indexOfChar(in_chars_array[out_index], QByteArray(_OPERATORS, 0))) < 0)
	{
		switch (in_chars_array[out_index]){
			case 'x':
			case 'X':
			case '(':
				tmp_operande->m_operator = Op::_OPERATOR_MULT;
				return in_index;
			default :
				return -1;
		}
	}

	return in_index + 1;
}


/**
 * Calculer le résultat d'une équation
 * @param in_operations_list (doit étre <code>NULL</code>)
 * @param in_next_list	la liste d'opérations (calculée é partir de {@link #_stringToChainEquation(QString)})
 * @param une variable résultat non initialisée
 * @param la valeur de l'inconnue 'x' dans l'équation
 */
Chain<Op>* Op::_operate(Chain<Op> *in_operations_list, Chain<Op> *in_next_list, Result *in_result, double in_x_value)
{
	Op			*tmp_current_operation = NULL, *tmp_next_operation  = NULL;
	Double		*tmp_current_value = NULL, *tmp_next_value = NULL;
	Double		*tmp_result = NULL;
	Result		*tmp_sub_result = NULL;
	Chain<Op>	*tmp_chain = in_next_list;

	if (in_next_list == NULL)	return NULL;

	/* Seconde opération */
	tmp_next_operation = (Op*)in_next_list->getData();

	if (in_operations_list == NULL)
	{
		in_result->setValue(_operationValue(tmp_next_operation, in_x_value));
		return _operate(in_next_list, in_next_list->next(), in_result, in_x_value);
	}

	/* Premiere opération */
	tmp_current_operation = (Op*)in_operations_list->getData();

	if (in_result->getValue() == NULL)
	{
		tmp_current_value = _operationValue(tmp_current_operation, in_x_value);
		if (tmp_current_value == NULL)	return NULL;
	}
	else
	{
		tmp_current_value = in_result->getValue();
	}


	if (tmp_current_operation->m_operator >= tmp_next_operation->m_operator)
	{
		/* Valeur du premier opérande */
		tmp_next_value = _operationValue(tmp_next_operation, in_x_value);
		if (tmp_next_value == NULL)	return NULL;

		tmp_result = _calculate(tmp_current_value, tmp_next_value, tmp_current_operation->m_operator);
		if (in_result->getValue() != NULL)
		{
			in_result->setValue(tmp_result);
			return _operate(in_next_list, in_next_list->next(), in_result, in_x_value);
		}
		else
		{
			in_result->setValue(tmp_result);
		}
	}
	else
	{
		tmp_sub_result = new Result();
		tmp_chain = _operate(in_next_list, in_next_list->next(), tmp_sub_result, in_x_value);
		if (tmp_sub_result->getValue() == NULL)	return NULL;

		in_result->setValue(_calculate(tmp_current_value, tmp_sub_result->getValue(), tmp_current_operation->m_operator));
		if (tmp_chain != NULL)
			return _operate(tmp_chain, tmp_chain->next(), in_result, in_x_value);
	}

	return in_next_list;
}


/**
 * Valeur d'une opération
 * @param in_operation
 * @param in_x_value
 * @return
 */
Double* Op::_operationValue(Op *in_operation, double in_x_value)
{
	Result *tmp_result = NULL;

	if (in_operation->m_value != NULL)
	{
		return new Double(in_operation->m_value->doubleValue());
	}
	else if (in_operation->m_sub_expression != NULL)
	{
		tmp_result = new Result();
		_operate(NULL, in_operation->m_sub_expression, tmp_result, in_x_value);
		return tmp_result->getValue();
	}

	return NULL;
}


/**
 * Calculer l'opération de deux Double
 * @param in_first_number premier opérande
 * @param in_second_number second opérande
 * @param in_operator opérateur
 */
Double* Op::_calculate(Double *in_first_number, Double *in_second_number, int in_operator)
{
	double tmp_result;

        if (in_first_number == NULL || in_second_number == NULL)	return NULL;

	switch (in_operator)
	{
		/* Division */
		case Op::_OPERATOR_DIV :
			tmp_result = in_first_number->doubleValue() / in_second_number->doubleValue();
			break;
		/* Puissance */
		case Op::_OPERATOR_POW :
			tmp_result = pow(in_first_number->doubleValue(), in_second_number->doubleValue());
			break;
		/* Soustraction */
		case Op::_OPERATOR_LESS :
			tmp_result = in_first_number->doubleValue() - in_second_number->doubleValue();
			break;
		/* Racine (carré, cubique...etc.) */
		case Op::_OPERATOR_ROOT :
			tmp_result = pow(in_second_number->doubleValue(), 1/in_first_number->doubleValue());
			break;
		/* Addition */
		case Op::_OPERATOR_PLUS :
			tmp_result = in_first_number->doubleValue() + in_second_number->doubleValue();
			break;
		/* Multiplication par defaut */
		default :
			tmp_result = in_first_number->doubleValue() * in_second_number->doubleValue();
			break;
	}

	return new Double(tmp_result);
}


/**
* Constructeur
*/
GraphicImage::GraphicImage() : Image()
{
}


/**
* Constructeur secondaire.
*/
GraphicImage::GraphicImage(double in_width, double in_height) : Image(in_width, in_height)
{
}

/**
* Renvoie une couleur plus foncée que la couleur passée paramétre.<br>
* Modification de la méthode <code>darker()</code> de la classe <code>java.awt.Color</code><br>
* pour prise en charge de la valeur alpha de la couleur d'origine
* @param in_color	couleur d'origine
* @return
*/
QColor GraphicImage::darker(QColor in_color)
{
	double	tmp_factor = 0.5;

	return QColor(
			MAX((int)(in_color.red() * tmp_factor), 0),
			MAX((int)(in_color.green() * tmp_factor), 0),
			MAX((int)(in_color.blue() * tmp_factor), 0),
			in_color.alpha());
}



/**
* Dessiner un parallepipéde rectangle.
* @param x position horizontale
* @param y position verticale
* @param width largeur
* @param height hauteur
* @param raised en 3D
* @param in_x_incline inclinaison horizontale
* @param in_y_incline inclinaison verticale
*/
void GraphicImage::fill3DRect(double x, double y, double width, double height,
           bool raised, double in_x_incline, double in_y_incline)
{
	QBrush p = _m_Graphics2D->brush().color();
	QColor c = _m_Graphics2D->pen().color();
	QColor brighter = c.lighter();
	QColor darker = GraphicImage::darker(c);
	double arcw = (width <= 10 ? 0 : 10), arch = (height <= 10 ? 0 : 10);
	QLinearGradient	tmp_linear_gradiant(QPointF(x+1, y+1), QPointF(x+1+width-2, y+1+height-2));

	tmp_linear_gradiant.setColorAt(0, c);
	tmp_linear_gradiant.setColorAt(1, darker);

	/* Dessiner en relief ? */
	if (raised)
	{
		/* Dessiner un parallepipede rectangle */
		_m_Graphics2D->setPen(GraphicImage::darker(darker));
		/* Plan de gauche */
		_m_Graphics2D->fillPath(Rectangle3D(x, y+1, height-2, width/2, -in_x_incline, in_y_incline, 90).path(), GraphicImage::darker(darker));
		/* Plan arriére */
		_m_Graphics2D->fillRect(QRectF((int)(x + 1 + width/2 * sin(in_y_incline*RAD)), (int)(y + 1 - width/2 * sin(in_x_incline*RAD)), width-2, height-2), GraphicImage::darker(darker));
		/* Dégradé de couleurs sur le plan de face */
		_m_Graphics2D->setBrush(tmp_linear_gradiant);
		_m_Graphics2D->fillRect(QRectF(x+1, y+1, width-2, height-2), tmp_linear_gradiant);
		/* Plan de droite */
		_m_Graphics2D->setPen(darker);
		_m_Graphics2D->fillPath(Rectangle3D(x + 1, y+1, width-2, width/2, -in_x_incline, in_y_incline, 0).path(), darker);
		/* Plan de dessus */
		_m_Graphics2D->setPen(GraphicImage::darker(darker));
		_m_Graphics2D->fillPath(Rectangle3D(x + width - 1, y+1, height-2, width/2, -in_x_incline, in_y_incline, 90).path(), GraphicImage::darker(darker));
	}
	else
	{
		/* Desssiner un rectangle arrondi */
		//_m_Graphics2D->fillRect(QRect(x+1, y+1, width-2, height-2, arcw, arch));
		_m_Graphics2D->fillRect(QRectF(x+1, y+height-arch/2, width-2, arch/2), GraphicImage::darker(darker));

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
		//_m_Graphics2D->draw(new Arc2D.Double(x + width - arcw - 1, y + 1, arcw, arch, 0, 90, Arc2D.OPEN));
		_m_Graphics2D->drawArc(QRectF(x + width - arcw - 1, y + 1, arcw, arch), 0*QT_ANGLE_RATIO, 90*QT_ANGLE_RATIO);
		/* Dessiner les ombres sombres */
		_m_Graphics2D->setPen(darker);
		/* Ligne sombre inférieure */
		_m_Graphics2D->drawLine(QLineF(x + 1, y + height - 1, x + width - 1, y + height - 1));
		/* Ligne sombre de droite */
		_m_Graphics2D->drawLine(QLineF(x + width - 1, y + arch/2, x + width - 1, y + height - 1));
		_m_Graphics2D->setBrush(p);
	}
}



/**
* Dessiner la représentation graphique d'une equation d'inconnue 'x'.
* @param in_equation	une equation (i.e : 2 * x + 5)
*/
void GraphicImage::drawXdependantEquation(QString *in_equation)
{
	double			tmp_x = 0.0;
	double			tmp_prev_x = 0.0;
	double			tmp_y = 0.0;
	double			tmp_prev_y = 0.0;
	Chain<Op>		*tmp_eq_chain = Op::_stringToChainEquation(in_equation);
	Result			*tmp_result = new Result();


	Op::_operate(NULL, tmp_eq_chain, tmp_result, tmp_prev_x);
	if (tmp_result->getValue() != NULL)
	{
		tmp_prev_y = _m_height - tmp_result->getValue()->doubleValue();

		_m_Graphics2D->setPen(_m_pencil_Color);

		for (tmp_x = 1; tmp_x < _m_width; tmp_x++)
		{
			Op::_operate(NULL, tmp_eq_chain, tmp_result, tmp_x);
			if (tmp_result->getValue() != NULL)
			{
				tmp_y = _m_height - tmp_result->getValue()->doubleValue();

				_m_Graphics2D->drawLine(QLine( tmp_prev_x, tmp_prev_y, tmp_x, tmp_y));
			}
			tmp_prev_x = tmp_x;
			tmp_prev_y = tmp_y;
		}
	}
}


/**
* Dessiner la représentation graphique d'une equation d'inconnue 'y'.
* @param in_equation	une equation (i.e : y + 5)
*/
void GraphicImage::drawYdependantEquation(QString *in_equation)
{
	double		tmp_x = 0.0;
	double		tmp_prev_x = 0.0;
	double		tmp_y = 0.0;
	double		tmp_prev_y = 0.0;
	Chain<Op>		*tmp_eq_chain = NULL;
	Result		*tmp_result = new Result();
	QString		*tmp_equation_str = new QString(in_equation->replace('y', 'x'));

	tmp_equation_str = new QString(tmp_equation_str->replace('Y', 'x'));
	
	tmp_eq_chain = Op::_stringToChainEquation(tmp_equation_str);
	
        Op::_operate(NULL, tmp_eq_chain, tmp_result, tmp_prev_y);
        if (tmp_result->getValue() != NULL)
	{
		tmp_prev_x = tmp_result->getValue()->doubleValue();
		
		_m_Graphics2D->setPen(_m_pencil_Color);

		for (tmp_y = 1; tmp_y < _m_height; tmp_y++)
		{

			Op::_operate(NULL, tmp_eq_chain, tmp_result, tmp_y);
			if (tmp_result->getValue() != NULL)
			{
				tmp_x = tmp_result->getValue()->doubleValue();

				_m_Graphics2D->drawLine(QLine( tmp_prev_x, tmp_prev_y, tmp_x, tmp_y));
			}
			tmp_prev_x = tmp_x;
			tmp_prev_y = tmp_y;
		}
	}
}

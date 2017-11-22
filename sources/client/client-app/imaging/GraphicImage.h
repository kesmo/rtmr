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

#ifndef GRAPHICIMAGE_H
#define GRAPHICIMAGE_H

#include <QColor>
#include "objects/DataObject.h"
#include "Chain.h"
#include "Image.h"

/**
 * Classe de gestion d'un résultat
 */
class Result
{
private:
    Double	*_m_value;

public:
	Result();

    void setValue(Double* in_value);

    Double* getValue();
};


/**
 * Classe de gestion d'une opération
 */
class Op
{
public:
	static const int		_OPERATOR_PLUS = 0;
	static const int		_OPERATOR_LESS = 1;
	static const int		_OPERATOR_MULT = 2;
	static const int		_OPERATOR_DIV = 3;
	static const int		_OPERATOR_POW = 4;
	static const int		_OPERATOR_ROOT = 5;

	/**
	 * Opérande
	 */
        DataObject		*m_value;

	/**
	 * Opérateur
	 */
        int			m_operator;

	/**
	 * Listes de sous-opérations
	 */
        Chain<Op>		*m_sub_expression;


	/**
	 * Constructeur
	 */
	Op();


	/**
	 * Constructeur
	 */
	Op(DataObject *in_value, int in_operator);

	/**
	 * Calculer le résultat d'une équation
	 * @param in_expression	chaîne définissant l'équation (ex : "((x+5)*2)^3")
	 * @param in_value	valeur de l'inconnue 'x' dans l'équation
	 * @return
	 */
	static double compute(QString *in_expression, double in_value);

	/**
	 * Transfomer une équation en liste (Chain) d'opérations
	 * @param in_expression	chaîne à transformer
	 **/
	static Chain<Op>* _stringToChainEquation(QString *in_expression);

	/**
	 * Calculer le résultat d'une équation
	 * @param in_operations_list (doit être <code>null</code>)
	 * @param in_next_list	la liste d'opérations (calculée à partir de {@link #_stringToChainEquation(QString)})
	 * @param une variable résultat non initialisée
	 * @param la valeur de l'inconnue 'x' dans l'équation
	 */
	static Chain<Op>* _operate(Chain<Op> *in_operations_list, Chain<Op> *in_next_list, Result *in_result, double in_x_value);

	/**
	 * Renvoie la position de la parenthèse droite correspondante à la position de la première parenthèse gauche<br>
	 * Exemples d'appels de la fonction :<br>
	 * <ul><li>_rigthParenthesisIndex("((2*X)^3)", 0) renvoie la valeur 8.</li>
	 * <li>_rigthParenthesisIndex("((2*X)^3)", 1) renvoie la valeur 5.</li></ul>
	 * @param in_chars_array	chaîne ou chercher la parenthèse droite
	 * @param in_start_index	position de la première parenthèse gauche
	 * @return
	 */
	static int _rigthParenthesisIndex(QByteArray in_chars_array, int in_start_index);


	/**
	 * Récupérer l'opération courante
	 * @param in_operator_list	Liste des opérations
	 * @param chars_array	équation sous forme de chaîne
	 * @param in_index	position courante du curseur dans la chaîne
	 * @return
	 */
	static int _getOperation(
                        Chain<Op>	*in_operator_list,
                        QByteArray	in_chars_array,
			int		in_index
			);


	/**
	 * Récupérer l'opérateur courant
	 * @param in_operator_list	Liste des opérations
	 * @param chars_array	équation sous forme de chaîne
	 * @param in_index	position courante du curseur dans la chaîne
	 * @return
	 */
	static int _getOperator(
                Chain<Op>	*in_operator_list,
                QByteArray	in_chars_array,
		int		in_index);


	/**
	 * Valeur d'une opération
	 * @param in_operation
	 * @param in_x_value
	 * @return
	 */
	static Double* _operationValue(Op *in_operation, double in_x_value);


	/**
	 * Calculer l'opération de deux Double
	 * @param in_first_number premier opérande
	 * @param in_second_number second opérande
	 * @param in_operator opérateur
	 */
	static Double* _calculate(Double* in_first_number, Double* in_second_number, int in_operator);


};

/**
 * Sous-classer cette classe pour réaliser des traitements d'images.<br>
 * Image regroupe des propriétés redondantes des images :
 * 		<ul><li>largeur et hauteur</li>
 * 		<li>couleur de fond</li>
 * 		<li>couleur de crayon</li>
 * 		<li>transparence</li>
 * 		<li>format d'image</li><ul><br>
 * Image offre les fonctionnalités de bases suivantes :
 * 		<ul><li>sauvegarder l'image sur disque</li>
 * 		<li>mettre en mémoire l'image dans le cache de l'application</li>
 * 		<li>optimiser le rendu soit pour la qualité soit pour la vitesse de traitement</li>
 */
class GraphicImage : public Image
{
private:
	/**
     * Constructeur
     */
    GraphicImage();

protected:
	/**
	 * Dessiner un parallepipède rectangle.
	 * @param x position horizontale
	 * @param y position verticale
	 * @param width largeur
	 * @param height hauteur
	 * @param raised en 3D
	 * @param in_x_incline inclinaison horizontale
	 * @param in_y_incline inclinaison verticale
	 */
	void fill3DRect(double x, double y, double width, double height,
                           bool raised, double in_x_incline, double in_y_incline);

public:
	/**
	* Constructeur secondaire.
	*/
	GraphicImage(double in_width, double in_height);

	/**
	 * Renvoie une couleur plus foncée que la couleur passée paramètre.<br>
	 * Modification de la méthode <code>darker()</code> de la classe <code>java.awt.Color</code><br>
	 * pour prise en charge de la valeur alpha de la couleur d'origine
	 * @param in_color	couleur d'origine
	 * @return
	 */
        static QColor darker(QColor in_color);
	
	

	/**
	 * Dessiner la représentation graphique d'une equation d'inconnue 'x'.
	 * @param in_equation	une equation (i.e : 2 * x + 5)
	 */
	void drawXdependantEquation(QString *in_equation);
	
	/**
	 * Dessiner la représentation graphique d'une equation d'inconnue 'y'.
	 * @param in_equation	une equation (i.e : y + 5)
	 */
	void drawYdependantEquation(QString *in_equation);
};


#endif //GRAPHICIMAGE_H

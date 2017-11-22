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

#include "errors.h"

#if defined (__APPLE__) 
	#include <malloc/malloc.h>
#else
		#include <malloc.h>
#endif
#include <stdio.h>
#include <string.h>

/***********************************************************************************	*
*	get_error_message																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Renvoie la description du message d'erreur																		. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|	in_error_id : code de l'erreur														*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|																															*
*************************************************************************************/
const char	*get_error_message(int in_error_id)
{
	int		tmp_errors_count = sizeof(errors_list) / sizeof(error);
	int		tmp_error_index = 0;

	for (tmp_error_index = 0; tmp_error_index < tmp_errors_count; tmp_error_index++)
	{
		if (errors_list[tmp_error_index].id == in_error_id)
		{
			return errors_list[tmp_error_index].description;
		}
	}

	return NULL;
}


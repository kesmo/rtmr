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

#ifndef EJLDAPLIBRARY_H_
#define EJLDAPLIBRARY_H_


#ifdef __cplusplus
extern "C" {
#endif

	/***********************************************************************************	*
	*	ldap_query																																		*
	*----------------------------------------------------------------------------------------------------------------------------	*
	*	Exécuter une requête LDAP											. 																				*
	************************************************************************************/
	char***   ldap_query(const char *in_ldap_url, const char *in_distinguish_name, const char *in_credencial, const char *in_base_query, const char *in_filter_query, char **in_entry_attributes, unsigned long *out_rows_count, unsigned long *out_columns_count, int *out_return_code);

#ifdef __cplusplus
}
#endif

#endif /*EJLDAPLIBRARY_H_*/

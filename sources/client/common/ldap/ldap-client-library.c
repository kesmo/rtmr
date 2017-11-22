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

#if defined(_WINDOWS) || defined(WIN32)
#include <winsock2.h>
#include <winldap.h>
#else
#include <ldap.h>
#endif

#include <stdio.h>

#if defined (__APPLE__)
	#include <malloc/malloc.h>
#else
		#include <malloc.h>
#endif

#include "EJLdapLibrary.h"

#include "../../common/constants.h"
#include "../../common/utilities.h"

/***********************************************************************************	*
*	ldap_query																																		*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Exécuter une requête LDAP											. 																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_db																											*
*											|		in_distinguish_name																						*
*											|		in_credencial																	*
*											|		in_query_string																	*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		NOERR si traitement correct															*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20090618	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
char***   ldap_query(const char *in_ldap_url, const char *in_distinguish_name, const char *in_credencial, const char *in_base_query, const char *in_filter_query, char **in_entry_attributes, unsigned long *out_rows_count, unsigned long *out_columns_count, int *out_return_code)
{
	char						***tmp_rows_columns_array= NULL;

	char						*tmp_attrs_names_ptr = NULL;

	unsigned long	tmp_row_index = 0;
	unsigned long	tmp_column_index = 0;

	LDAP						*tmp_ldap = NULL;
	int							tmp_ldap_return_code = 0;
	int							tmp_ldap_msg_id = 0;
	int							tmp_ldap_error_code = 0;
	LDAPMessage		*tmp_ldap_results = NULL;
	LDAPMessage		*tmp_ldap_entrie = NULL;
	BerElement			*tmp_ldap_attribute = NULL;
	char						*tmp_ldap_attribute_name = NULL;
	struct berval		**tmp_ldap_attribute_values = NULL;

#if !defined(_WINDOWS) && !defined(WIN32)
	struct berval		tmp_cred;
#endif

	if (out_return_code != NULL)
		*out_return_code = NOERR;

	if (is_empty_string(in_ldap_url) == FALSE && in_distinguish_name != NULL && in_credencial != NULL)
	{
		/* Initialisation LDAP */
#if defined(_WINDOWS) || defined(WIN32)
		tmp_ldap = ldap_initA((PCHAR)in_ldap_url, 0);
		if (tmp_ldap != NULL)
			tmp_ldap_return_code = LDAP_SUCCESS;
		else
			tmp_ldap_return_code = LdapGetLastError();
#else
		tmp_ldap_return_code =ldap_initialize(&tmp_ldap, in_ldap_url);
#endif
		if (tmp_ldap_return_code == LDAP_SUCCESS)
		{
			/* Connexion LDAP */
			fprintf(stdout, "JNI  (ldap_query) ldap_sasl_bind.\n");
#if defined(_WINDOWS) || defined(WIN32)
			tmp_ldap_return_code = ldap_bind(tmp_ldap, (const PCHAR)in_distinguish_name, (const PCHAR)in_credencial, LDAP_AUTH_SIMPLE);
#else
			tmp_cred.bv_val = (char*)malloc(strlen(in_credencial) + 1);
			strcpy(tmp_cred.bv_val, in_credencial);
			tmp_cred.bv_len = strlen(in_credencial);

			tmp_ldap_return_code = ldap_sasl_bind(tmp_ldap, in_distinguish_name, LDAP_SASL_AUTOMATIC, &tmp_cred, NULL, NULL, &tmp_ldap_msg_id);
#endif
			if (tmp_ldap_return_code == LDAP_SUCCESS)
			{
				/* Récupération du résultat */
				fprintf(stdout, "JNI  (ldap_query) ldap_result.\n");
				tmp_ldap_return_code = ldap_result(tmp_ldap, tmp_ldap_msg_id, LDAP_MSG_ALL, 0, &tmp_ldap_results);
				if (tmp_ldap_return_code == LDAP_RES_BIND)
				{
					/* Vérification du résultat */
					fprintf(stdout, "JNI  (ldap_query) ldap_parse_result.\n");
#if defined(_WINDOWS) || defined(WIN32)
					tmp_ldap_return_code = ldap_parse_result(tmp_ldap, tmp_ldap_results, (ULONG*)&tmp_ldap_error_code, NULL, NULL, NULL, NULL, 1);
#else
					tmp_ldap_return_code = ldap_parse_result(tmp_ldap, tmp_ldap_results, &tmp_ldap_error_code, NULL, NULL, NULL, NULL, 1);
#endif
					if (tmp_ldap_return_code == LDAP_SUCCESS)
					{
						if (tmp_ldap_error_code == 0)
						{
							/* Exécution de la requête LDAP */
							fprintf(stdout, "JNI  (ldap_query) ldap_search_ext.\n");
#if defined(_WINDOWS) || defined(WIN32)
							tmp_ldap_return_code = ldap_search_extA(tmp_ldap, (PCHAR)in_base_query, LDAP_SCOPE_BASE, (PCHAR)in_filter_query, in_entry_attributes, 0, NULL, NULL, 0, LDAP_NO_LIMIT, (ULONG*)&tmp_ldap_msg_id);
#else
							tmp_ldap_return_code = ldap_search_ext(tmp_ldap, (char*)in_base_query, LDAP_SCOPE_BASE, (char*)in_filter_query, in_entry_attributes, 0, NULL, NULL, 0, LDAP_NO_LIMIT, &tmp_ldap_msg_id);
#endif
							if (tmp_ldap_return_code == LDAP_SUCCESS)
							{
								/* Vérification du résultat */
								fprintf(stdout, "JNI  (ldap_query) ldap_result.\n");
								tmp_ldap_return_code = ldap_result(tmp_ldap, tmp_ldap_msg_id, LDAP_MSG_ALL, 0, &tmp_ldap_results);
								if (tmp_ldap_results != NULL)
								{
									/* Compter le nombre de lignes de résultat */
									fprintf(stdout, "JNI  (ldap_query) ldap_count_entries.\n");
									*out_rows_count = ldap_count_entries(tmp_ldap, tmp_ldap_results);
									tmp_rows_columns_array = (char***)malloc(sizeof(char**) * (*out_rows_count));

									/* Calculer le nombre de d'attributs de l'entrée */
									(*out_columns_count) = 0;
									while ((tmp_attrs_names_ptr = in_entry_attributes[*out_columns_count]) != NULL)	(*out_columns_count)++;

									/* Parcourir chaque entrée trouvée */
									fprintf(stdout, "JNI  (ldap_query) ldap_first_entry.\n");
									for(tmp_ldap_entrie = ldap_first_entry(tmp_ldap, tmp_ldap_results); tmp_ldap_entrie != NULL; tmp_ldap_entrie = ldap_next_entry(tmp_ldap, tmp_ldap_entrie))
									{
										if (tmp_row_index < MAX_ROWS_BY_QUERY)
										{
											tmp_rows_columns_array[tmp_row_index] = (char**)malloc(sizeof(char*) * (*out_columns_count));
											for (tmp_column_index = 0; tmp_column_index < (*out_columns_count); tmp_column_index++)		tmp_rows_columns_array[tmp_row_index][tmp_column_index] = NULL;

											/* Parcourir chaque attribut de l'entrée courante */
#if defined(_WINDOWS) || defined(WIN32)
											 for ( tmp_ldap_attribute_name = ldap_first_attributeA(tmp_ldap, tmp_ldap_entrie, &tmp_ldap_attribute ); tmp_ldap_attribute_name != NULL; tmp_ldap_attribute_name = ldap_next_attributeA(tmp_ldap, tmp_ldap_entrie, tmp_ldap_attribute ) )
#else
											 for ( tmp_ldap_attribute_name = ldap_first_attribute(tmp_ldap, tmp_ldap_entrie, &tmp_ldap_attribute ); tmp_ldap_attribute_name != NULL; tmp_ldap_attribute_name = ldap_next_attribute(tmp_ldap, tmp_ldap_entrie, tmp_ldap_attribute ) )
#endif
											 {
											 		for (tmp_column_index = 0; tmp_column_index < (*out_columns_count); tmp_column_index++)
											 		{
													 	if (tmp_ldap_attribute_name != NULL && strcmp(tmp_ldap_attribute_name, in_entry_attributes[tmp_column_index]) == 0)
													 	{
#if defined(_WINDOWS) || defined(WIN32)
														 	tmp_ldap_attribute_values = ldap_get_values_lenA(tmp_ldap, tmp_ldap_entrie, tmp_ldap_attribute_name);
#else
														 	tmp_ldap_attribute_values = ldap_get_values_len(tmp_ldap, tmp_ldap_entrie, tmp_ldap_attribute_name);
#endif
														 	if ( tmp_ldap_attribute_values && tmp_ldap_attribute_values[0] )
														 	{
																tmp_rows_columns_array[tmp_row_index][tmp_column_index] = (char*)malloc(tmp_ldap_attribute_values[0]->bv_len + 1);
																strcpy(tmp_rows_columns_array[tmp_row_index][tmp_column_index] , tmp_ldap_attribute_values[0]->bv_val );
																fprintf(stdout, "JNI  (ldap_query) Attr %s = %s.\n", tmp_ldap_attribute_name, tmp_rows_columns_array[tmp_row_index][tmp_column_index]);
														 	}
														 	ldap_value_free_len( tmp_ldap_attribute_values );
													 	}
											 		}
											 }
										}
										tmp_row_index++;
									}

									/* Libération de la mémoire utilisée par les structures LDAP */
									fprintf(stdout, "JNI  (ldap_query) ldap_msgfree.\n");
									ldap_msgfree(tmp_ldap_results);
								}
								else
								{
									fprintf(stderr, "*** JNI error (EJMysqlLibrary.ldap_query) : LDAP result error : %s\n", ldap_err2string(tmp_ldap_return_code));
									if (out_return_code != NULL)
										*out_return_code = LDAP_QUERY_ERROR;
								}
							}
							else
							{
								fprintf(stderr, "*** JNI error (EJMysqlLibrary.ldap_query) : LDAP search error : %s\n", ldap_err2string(tmp_ldap_return_code));
								if (out_return_code != NULL)
									*out_return_code = LDAP_QUERY_ERROR;
							}

							/* Déonnexion LDAP */
#if defined(_WINDOWS) || defined(WIN32)
							tmp_ldap_return_code = ldap_unbind(tmp_ldap);
#else
							tmp_ldap_return_code = ldap_unbind_ext(tmp_ldap, NULL, NULL);
#endif
						}
						else
						{
							fprintf(stderr, "*** JNI error (EJMysqlLibrary.ldap_query) : ldap_parse_result error returned : %s\n", ldap_err2string(tmp_ldap_error_code));
							if (out_return_code != NULL)
								*out_return_code = LDAP_QUERY_ERROR;
						}
					}
					else
					{
						fprintf(stderr, "*** JNI error (EJMysqlLibrary.ldap_query) : ldap_parse_result error : %s\n", ldap_err2string(tmp_ldap_return_code));
						if (out_return_code != NULL)
							*out_return_code = LDAP_INVALID_CREDENTIAL;
					}
				}
				else
				{
					fprintf(stderr, "*** JNI error (EJMysqlLibrary.ldap_query) : ldap_result error : %s\n", ldap_err2string(tmp_ldap_return_code));
					if (out_return_code != NULL)
						*out_return_code = LDAP_INVALID_CREDENTIAL;
				}
			}
			else
			{
				fprintf(stderr, "*** JNI error (EJMysqlLibrary.ldap_query) : bind error : %s\n", ldap_err2string(tmp_ldap_return_code));
				if (out_return_code != NULL)
					*out_return_code = LDAP_CONNEXION_ERROR;
			}
		}
		else
		{
			fprintf(stderr, "*** JNI error (EJMysqlLibrary.ldap_query) : init error\n");
			if (out_return_code != NULL)
				*out_return_code = LDAP_CONNEXION_ERROR;
		}
	}
	else
	{
		fprintf(stderr, "*** JNI error (EJMysqlLibrary.ldap_query) : ldap url, distinguish name or credencial is empty\n");
		if (out_return_code != NULL)
			*out_return_code = EMPTY_OBJECT;
	}

	fprintf(stdout, "JNI  (ldap_query) Result =>Rows = %li, Columns = %li.\n", (*out_rows_count), (*out_columns_count));

	fflush(stdout);
	fflush(stderr);

	return tmp_rows_columns_array;
}

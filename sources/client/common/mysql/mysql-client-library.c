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

#include <my_global.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#if defined (__APPLE__)
	#include <malloc/malloc.h>
#else
		#include <malloc.h>
#endif
#include <string.h>

#include "../org_ej_EJNativeLibrary.h"
#include "EJMysqlLibrary.h"
#include "../constants.h"
#include "../utilities.h"
#include "../entities.h"



static const char *SERVER_GROUPS[] = {"embedded", NULL };

/***********************************************************************************	*
*	db_start																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Initialisation du serveur mysql et de la connexion à la base de données. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_server_args																							*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		MYSQL pointeur																						*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
void *db_start(const char *hostname, unsigned int port, const char *dbname, const char *username, const char *passwd, char** in_server_args)
{
	MYSQL	*tmp_mysql= NULL;
	char		**tmp_server_groups = in_server_args;

	if (tmp_server_groups == NULL)
		tmp_server_groups = (char**)SERVER_GROUPS;

	if (mysql_server_init(0, NULL, (char**)tmp_server_groups) == 0)
		tmp_mysql = db_connect(hostname, port, dbname, username, passwd);
	else
		fprintf(stderr, "*** JNI error (EJMysqlLibrary.start) : mysql_server_init\n");

	return tmp_mysql;
}

/***********************************************************************************	*
*	db_die																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Déconnexion de la base de donnée et arrêt du serveur mysql					. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		db																												*
*											|		fmt																												*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|																															*
* ************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
void db_die(void *db, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	(void)putc('\n', stderr);
	if (db)
		db_disconnect(db);
	exit(EXIT_FAILURE);
}

/***********************************************************************************	*
*	db_connect																																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Connection à une base de donnée					. 																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		dbname																										*
*											|		username																									*
*											|		passwd																										*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		MYSQL pointeur																						*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
void * db_connect(const char *hostname, unsigned int port, const char *dbname, const char *username, const char *passwd)
{
	MYSQL *db = mysql_init(NULL);
	if (!db)
		db_die(db, (char*)"mysql_init a échoué : pas de mémoire");
	/*
	 * Notez que le client et le serveur utilisent des noms de groupes séparés.
	 * Ceci est critique, car le serveur n'acceptera pas les options du client
	 * et vice versa.
	 */
	mysql_options(db, MYSQL_READ_DEFAULT_GROUP, "test_libmysqld_CLIENT");
	if (mysql_real_connect(db, hostname, username, passwd, dbname, port, NULL, 0) == NULL)
		db_die(db, (char*)"mysql_real_connect a échoué : %s", mysql_error(db));

	return db;
}

/***********************************************************************************	*
*	db_disconnect																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Déconnexion d'une base de donnée					. 																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		db																												*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|																															*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
void db_disconnect(void *db)
{
	mysql_close(db);
}

/***********************************************************************************	*
*	db_do_query																																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Exécuter une requête												. 																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_db																											*
*											|		in_query																										*
*											|		out_rows_count																						*
*											|		out_columns_count																					*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		char***																										*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
char*** db_do_query(void *in_db, const char *in_query, unsigned long *out_rows_count, unsigned long *out_columns_count)
{
	MYSQL_RES				*tmp_res;
	MYSQL_ROW				tmp_row;
	char							***tmp_rows_columns_array= NULL;
	unsigned long		tmp_row_index = 0;
	unsigned long		tmp_column_index = 0;
	unsigned long		*tmp_columns_lengths = NULL;

	*out_rows_count = 0;
	*out_columns_count = 0;

	/* Vérification des paramètres */
	if (in_db != NULL && is_empty_string(in_query) == FALSE)
	{
		fprintf(stdout, "JNI info (EJMysqlLibrary.db_do_query) requête : %s\n", in_query);

		// Allouer MAX_ROWS_BY_QUERY lignes
		tmp_rows_columns_array = (char***)malloc(sizeof(char*) * MAX_ROWS_BY_QUERY);

		// Lancer la requete
		if (mysql_query(in_db, in_query) == 0)
		{
			// Recuperer le resultat
			tmp_res = mysql_use_result(in_db);
			if (tmp_res)
			{
				// Nombre de colonnes pour chaque ligne
				*out_columns_count = mysql_field_count(in_db);

				// Parcourrir les lignes de resultat
				while ((tmp_row = mysql_fetch_row(tmp_res)))
				{
					if (tmp_row_index < MAX_ROWS_BY_QUERY)
					{
						tmp_rows_columns_array[tmp_row_index] = (char**)malloc(sizeof(char*) * (*out_columns_count));
						tmp_columns_lengths = mysql_fetch_lengths(tmp_res);

						for (tmp_column_index = 0; tmp_column_index < (*out_columns_count); tmp_column_index++)
						{
							if (tmp_row[tmp_column_index])
							{
								tmp_rows_columns_array[tmp_row_index][tmp_column_index] = (char*)malloc(tmp_columns_lengths[tmp_column_index] + 1);
								strcpy(tmp_rows_columns_array[tmp_row_index][tmp_column_index] , tmp_row[tmp_column_index]);
							}
							else
								tmp_rows_columns_array[tmp_row_index][tmp_column_index] = NULL;
						}
					}
					tmp_row_index++;
				}

				*out_rows_count = (unsigned long)mysql_num_rows(tmp_res);
				mysql_free_result(tmp_res);
			}
			else
				fprintf(stderr, "*** JNI error (EJMysqlLibrary.db_do_query->mysql_use_result) : %s\n", mysql_error(in_db));
		}
		else
			fprintf(stderr, "*** JNI error (EJMysqlLibrary.db_do_query->mysql_query) :%s\n", mysql_error(in_db));
	}
	else
		fprintf(stderr, "*** JNI error (EJMysqlLibrary.db_do_query) : statement query is empty\n");

	fflush(stdout);
	fflush(stderr);

	return tmp_rows_columns_array;
}


/***********************************************************************************	*
*	db_call_procedure																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Exécuter une procedure											. 																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_db																											*
*											|		in_param_count																						*
*											|		in_procedure_name																						*
*											|		in_out_parameters																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|																	*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20090625	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
void db_call_procedure(void *in_db, const char *in_procedure_name, int in_param_count, char **in_out_parameters)
{
	unsigned long	tmp_rows_count;
	unsigned long	tmp_columns_count;

	char						tmp_procedure_call[MAX_SQL_STATEMENT_LENGTH];
	char						*tmp_ptr = tmp_procedure_call;

	int							tmp_param_index = 0;

	char						tmp_query_set[24];
	char						tmp_query_result[MAX_SQL_STATEMENT_LENGTH];
	char						*tmp_result_ptr = tmp_query_result;

	char						***tmp_returned_rows = NULL;
	int							tmp_row_index = 0;
	int							tmp_columns_index = 0;

	// Initialiser la requete
	if (in_param_count > 0)
	{
		if (in_out_parameters[tmp_param_index])
			sprintf(tmp_query_set, "SET @param%i='%s'", tmp_param_index, in_out_parameters[tmp_param_index]);
		else
			sprintf(tmp_query_set, "SET @param%i=null", tmp_param_index);

		if (mysql_query(in_db, tmp_query_set) != 0)
			fprintf(stderr, "*** JNI error (EJMysqlLibrary.db_call_procedure->mysql_query) :%s\n", mysql_error(in_db));

		tmp_ptr += sprintf(tmp_procedure_call, "CALL %s(@param%i", in_procedure_name, tmp_param_index);
		tmp_result_ptr += sprintf(tmp_query_result, "SELECT @param%i", tmp_param_index);
	}
	else
		tmp_ptr += sprintf(tmp_procedure_call, "CALL %s(", in_procedure_name);

	for (tmp_param_index = 1; tmp_param_index < in_param_count; tmp_param_index++)
	{
		if (in_out_parameters[tmp_param_index])
			sprintf(tmp_query_set, "SET @param%i='%s'", tmp_param_index, in_out_parameters[tmp_param_index]);
		else
			sprintf(tmp_query_set, "SET @param%i=null", tmp_param_index);

		if (mysql_query(in_db, tmp_query_set) != 0)
			fprintf(stderr, "*** JNI error (EJMysqlLibrary.db_call_procedure->mysql_query) :%s\n", mysql_error(in_db));

		tmp_ptr += sprintf(tmp_ptr, ",@param%i", tmp_param_index);
		tmp_result_ptr += sprintf(tmp_result_ptr, ",@param%i", tmp_param_index);
	}

	tmp_ptr += sprintf(tmp_ptr, ")");

	if (mysql_query(in_db, tmp_procedure_call) == 0)
	{
		tmp_returned_rows = db_do_query(in_db, tmp_query_result, &tmp_rows_count, &tmp_columns_count);
		if (tmp_returned_rows)
		{
			for(tmp_row_index = 0; tmp_row_index < tmp_rows_count; tmp_row_index++)
			{
				for(tmp_columns_index = 0; tmp_columns_index < tmp_columns_count; tmp_columns_index++)
				{
					if (tmp_columns_index < in_param_count)
					{
						if (tmp_returned_rows[tmp_row_index][tmp_columns_index])
						{
							if (in_out_parameters[tmp_columns_index])
								free(in_out_parameters[tmp_columns_index]);

							in_out_parameters[tmp_columns_index] = tmp_returned_rows[tmp_row_index][tmp_columns_index];
						}
						else
						{
							free(in_out_parameters[tmp_columns_index]);
							in_out_parameters[tmp_columns_index] = NULL;
						}
					}
				}
				free(tmp_returned_rows[tmp_row_index]);
			}
			free(tmp_returned_rows);
		}
	}
	else
		fprintf(stderr, "*** JNI error (EJMysqlLibrary.db_call_procedure->mysql_query) :%s\n", mysql_error(in_db));

	fflush(stdout);
	fflush(stderr);
}


/***********************************************************************************	*
*	db_insert_rows																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Insérertion de données											. 																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_db																											*
*											|		in_insert_query																						*
*											|		in_out_affected_rows_count																	*
*											|		in_out_last_pk_value																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		NOERR si traitement correct															*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_insert_rows(void *in_db, entity_def *in_entity_def, const char *in_insert_query, unsigned long *in_out_affected_rows_count, unsigned long *in_out_last_pk_value)
{
	int								tmp_return_result = NOERR;
	unsigned long		tmp_columns_count = 0;
	MYSQL_RES				*tmp_res;

	/* Vérification des paramètres */
	if (in_db == NULL || is_empty_string(in_insert_query) == TRUE || in_out_affected_rows_count == NULL || in_out_last_pk_value == NULL)		return EMPTY_OBJECT;

	*in_out_affected_rows_count = 0;
	*in_out_last_pk_value = 0;

	fprintf(stdout, "JNI info (EJMysqlLibrary.db_insert_rows) requête : %s\n", in_insert_query);

	/* Lancer la requete */
	if (mysql_query(in_db, in_insert_query) == 0)
	{
		tmp_res = mysql_store_result(in_db);
		if (tmp_res)
		{
			/* La requête ne doit pas retourner de résultat */
			mysql_free_result(tmp_res);
			fprintf(stderr, "*** JNI error (EJMysqlLibrary.db_insert_rows->mysql_store_result) : la requête est correcte mais elle ne doit pas renvoyer de résultat.\n");
			tmp_return_result = DB_SQL_ILLEGAL_STATEMENT;
		}
		else
		{
			/* Nombre de colonnes pour chaque ligne (doit être égal à zéro, car aucune ligne ne doit être retournée lors d'un INSERT) */
			tmp_columns_count = mysql_field_count(in_db);
			if (tmp_columns_count == 0)
			{
				/* Nombre de lignes affectées */
				*in_out_affected_rows_count = (unsigned long)mysql_affected_rows(in_db);
				if (*in_out_affected_rows_count >= 1)
				{
					/* Dernier identifiant créé (via AUTO_INCREMENT) */
					*in_out_last_pk_value = (unsigned long)mysql_insert_id(in_db);
					tmp_return_result = NOERR;
				}
				else
				{
					fprintf(stderr, "*** JNI error (EJMysqlLibrary.db_insert_rows->mysql_affected_rows) : Aucune ligne n'a été insérée.\n");
					tmp_return_result = DB_SQL_ERROR;
				}
			}
			else
			{
				fprintf(stderr, "JNI error (EJMysqlLibrary.db_insert_rows->mysql_field_count) : la requête est incorrecte. De plus elle ne doit pas renvoyer de résultat.\n");
				tmp_return_result = DB_SQL_ILLEGAL_STATEMENT;
			}
		}
	}
	else
	{
		fprintf(stderr, "JNI error (EJMysqlLibrary.db_insert_row->mysql_query) : %s\n", mysql_error(in_db));
		tmp_return_result = DB_SQL_ERROR;
	}

	return tmp_return_result;
}

/***********************************************************************************	*
*	db_delete_rows																																		*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Suppression de données											. 																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_db																											*
*											|		in_delete_query																						*
*											|		in_out_affected_rows_count																	*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		NOERR si traitement correct															*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_delete_rows(void *in_db, const char *in_delete_query, unsigned long *in_out_affected_rows_count)
{
	int								tmp_return_result = NOERR;
	unsigned long		tmp_columns_count = 0;
	MYSQL_RES				*tmp_res;

	/* Vérification des paramètres */
	if (in_db == NULL || is_empty_string(in_delete_query) == TRUE || in_out_affected_rows_count == NULL)		return EMPTY_OBJECT;

	*in_out_affected_rows_count = 0;

	fprintf(stdout, "JNI info (EJMysqlLibrary.db_delete_rows) requête : %s\n", in_delete_query);

	/* Lancer la requete */
	if (mysql_query(in_db, in_delete_query) == 0)
	{
		tmp_res = mysql_store_result(in_db);
		if (tmp_res)
		{
			/* La requête ne doit pas retourner de résultat */
			mysql_free_result(tmp_res);
			fprintf(stderr, "*** JNI error (EJMysqlLibrary.db_delete_rows->mysql_store_result) : la requête est correcte mais elle ne doit pas renvoyer de résultat.\n");
			tmp_return_result = DB_SQL_ILLEGAL_STATEMENT;
		}
		else
		{
			/* Nombre de colonnes pour chaque ligne (doit être égal à zéro, car aucune ligne ne doit être retournée lors d'un INSERT) */
			tmp_columns_count = mysql_field_count(in_db);
			if (tmp_columns_count == 0)
			{
				/* Nombre de lignes affectées */
				*in_out_affected_rows_count = (unsigned long)mysql_affected_rows(in_db);
				tmp_return_result = NOERR;
				fprintf(stdout, "*** JNI info (EJMysqlLibrary.db_delete_rows) requête : %s\n-->affected rows = %li", in_delete_query, *in_out_affected_rows_count);
			}
			else
			{
				fprintf(stderr, "*** JNI error (EJMysqlLibrary.db_delete_rows->mysql_field_count) : la requête est incorrecte. De plus elle ne doit pas renvoyer de résultat.\n");
				tmp_return_result = DB_SQL_ILLEGAL_STATEMENT;
			}
		}
	}
	else
	{
		fprintf(stdout, "*** JNI error (EJMysqlLibrary.db_delete_rows->mysql_query) : %s\n", mysql_error(in_db));
		tmp_return_result = DB_SQL_ERROR;
	}

	return tmp_return_result;
}

/***********************************************************************************	*
*	db_update_rows																																		*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Mise à jour de données											. 																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_db																											*
*											|		in_update_query																						*
*											|		in_out_affected_rows_count																	*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		NOERR si traitement correct															*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_update_rows(void *in_db, const char *in_update_query, unsigned long *in_out_affected_rows_count)
{
	int								tmp_return_result = NOERR;
	unsigned long		tmp_columns_count = 0;
	MYSQL_RES				*tmp_res;

	/* Vérification des paramètres */
	if (in_db == NULL || is_empty_string(in_update_query) == TRUE || in_out_affected_rows_count == NULL)		return EMPTY_OBJECT;

	*in_out_affected_rows_count = 0;

	fprintf(stdout, "JNI info (EJMysqlLibrary.db_update_rows) requête : %s\n", in_update_query);

	/* Lancer la requete */
	if (mysql_query(in_db, in_update_query) == 0)
	{
		tmp_res = mysql_store_result(in_db);
		if (tmp_res)
		{
			/* La requête ne doit pas retourner de résultat */
			mysql_free_result(tmp_res);
			fprintf(stdout, "*** JNI error (EJMysqlLibrary.db_update_rows->mysql_store_result) : la requête est correcte mais elle ne doit pas renvoyer de résultat.\n");
			tmp_return_result = DB_SQL_ILLEGAL_STATEMENT;
		}
		else
		{
			/* Nombre de colonnes pour chaque ligne (doit être égal à zéro, car aucune ligne ne doit être retournée lors d'un INSERT) */
			tmp_columns_count = mysql_field_count(in_db);
			if (tmp_columns_count == 0)
			{
				/* Nombre de lignes affectées */
				*in_out_affected_rows_count = (unsigned long)mysql_affected_rows(in_db);
				tmp_return_result = NOERR;
			}
			else
			{
				fprintf(stdout, "*** JNI error (EJMysqlLibrary.db_update_rows->mysql_field_count) : la requête est incorrecte. De plus elle ne doit pas renvoyer de résultat.\n");
				tmp_return_result = DB_SQL_ILLEGAL_STATEMENT;
			}
		}
	}
	else
	{
		fprintf(stdout, "*** JNI error (EJMysqlLibrary.db_update_rows->mysql_query) : %s\n", mysql_error(in_db));
		tmp_return_result = DB_SQL_ERROR;
	}

	return tmp_return_result;
}

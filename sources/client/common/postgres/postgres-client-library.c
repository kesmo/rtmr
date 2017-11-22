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

#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#if defined (__APPLE__)
	#include <malloc/malloc.h>
#else
		#include <malloc.h>
#endif
#include <string.h>
#include <time.h>

#include "postgres-client-library.h"
#include <libpq-fe.h>
#include <libpq/libpq-fs.h>

#include "../../common/constants.h"
#include "../../common/utilities.h"
#include "../../common/entities.h"
#include "../../common/errors.h"

/***********************************************************************************	*
*	db_start																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Connection   la base de donnees. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_server_args																							*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		void pointeur																						*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
#if (defined(_WINDOWS) || defined(WIN32))
	int db_start(net_session *in_session, const char *hostname, unsigned int port, const char *dbname, const char *username, const char *passwd, char** in_server_args )
#else
	int db_start(net_session *in_session, const char *hostname, unsigned int port, const char *dbname, const char *username, const char *passwd, char** in_server_args __attribute__((unused)))
#endif
{
	int	tmp_result = NOERR;

	if (in_session == NULL)	return EMPTY_OBJECT;

	in_session->m_db_connexion = (PGconn*)db_connect(hostname, port, dbname, username, passwd);
	if (in_session->m_db_connexion == NULL)	return DB_CONNEXION_ERROR;

	switch(PQstatus((PGconn*)in_session->m_db_connexion))
	{
	    case CONNECTION_OK:
			LOG_TRACE(in_session, "db_start : Connexion %s OK\n", username);
	        break;

	    case CONNECTION_BAD:
	    	LOG_TRACE( in_session, "db_start : Connexion %s KO\n", username);
			tmp_result = DB_CONNEXION_ERROR;
			db_disconnect(in_session);
	        break;

	    case CONNECTION_STARTED:
	    	LOG_TRACE( in_session, "db_start : Attente de la connexion à réaliser\n");
	        break;

	    case CONNECTION_MADE:
			LOG_TRACE(in_session, "db_start : Connexion OK ; attente d'un envoi\n");
	        break;

	    case CONNECTION_AWAITING_RESPONSE:
			LOG_TRACE(in_session, "db_start : Attente d'une réponse du serveur\n");
	        break;

	    case CONNECTION_AUTH_OK:
			LOG_TRACE(in_session, "db_start : Authentification reçue ; attente de la fin du lancement du moteur\n");
	        break;

	    case CONNECTION_SSL_STARTUP:
			LOG_TRACE(in_session, "db_start : Négociation du cryptage SSL\n");
	        break;

	    case CONNECTION_SETENV:
			LOG_TRACE(in_session, "db_start : Négociation des paramétrages de l'environnement\n");
	        break;

	    default:
			LOG_TRACE(in_session, "db_start : Connexion\n");
	}

	return tmp_result;
}

/***********************************************************************************	*
*	db_die																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Déconnexion de la base de donnée					. 							*
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
void db_die(net_session *in_session)
{
	db_disconnect(in_session);
}

/***********************************************************************************	*
*	db_connect																																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Connection   une base de donnees					. 																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		dbname																										*
*											|		username																									*
*											|		passwd																										*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		void pointeur																						*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
void * db_connect(const char *hostname, unsigned int port, const char *dbname, const char *username, const char *passwd)
{
	char		tmp_connexion_info[512];
	char		*tmp_ptr = tmp_connexion_info;
	PGconn	*tmp_pg_cnx = NULL;

	memset(tmp_connexion_info, 0, sizeof(tmp_connexion_info));

	if (is_empty_string(hostname) == FALSE)
		tmp_ptr += sprintf(tmp_ptr, "host=%s ", hostname);

	if (port != 0)
		tmp_ptr += sprintf(tmp_ptr, "port=%u ", port);

	if (is_empty_string(dbname) == FALSE)
		tmp_ptr += sprintf(tmp_ptr, "dbname=%s ", dbname);

	tmp_ptr += sprintf(tmp_ptr, "user=%s ", username);

	tmp_ptr += sprintf(tmp_ptr, "password=%s ", passwd);

	tmp_ptr += sprintf(tmp_ptr, "connect_timeout=10 ");

	tmp_pg_cnx = PQconnectdb(tmp_connexion_info);

	return tmp_pg_cnx;
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
void db_disconnect(net_session *in_session)
{
	db_clear_last_results_set(in_session);

	if (in_session != NULL && in_session->m_db_connexion != NULL)
	{
		LOG_TRACE(in_session, "db_disconnect : Déconnexion %s\n", PQuser((PGconn*)in_session->m_db_connexion));
		PQfinish((PGconn*)in_session->m_db_connexion);
		in_session->m_db_connexion = NULL;
	}
}

/***********************************************************************************	*
*	db_do_query																																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Exécuter une requête												. 																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_session																											*
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
char*** db_do_query(net_session *in_session, const char *in_query, unsigned long *out_rows_count, unsigned long *out_columns_count)
{
	char							***tmp_rows_columns_array= NULL;
	unsigned long		tmp_row_index = 0;
	unsigned long		tmp_column_index = 0;

	PGresult						*tmp_results = NULL;
	ExecStatusType		tmp_res_status;

	int								tmp_value_length = 0;

	*out_rows_count = 0;
	*out_columns_count = 0;

	/* Vérification des paramètres */
	if (in_session != NULL && in_session->m_db_connexion != NULL && is_empty_string(in_query) == FALSE)
	{
		LOG_TRACE(in_session, "db_do_query : %s\n", in_query);

		// Lancer la requete
		tmp_results = PQexec((PGconn*)in_session->m_db_connexion, in_query);
		if (tmp_results != NULL)
		{
			tmp_res_status = PQresultStatus(tmp_results);
			if (tmp_res_status == PGRES_TUPLES_OK)
			{
				*out_rows_count = PQntuples(tmp_results);
				*out_columns_count = PQnfields(tmp_results);

				// Allouer n lignes
				tmp_rows_columns_array = (char***)malloc(sizeof(char*) * (*out_rows_count));
				memset(tmp_rows_columns_array, 0, sizeof(char*) * (*out_rows_count));

				for (tmp_row_index = 0; tmp_row_index < (*out_rows_count); tmp_row_index++)
				{
					tmp_rows_columns_array[tmp_row_index] = (char**)malloc(sizeof(char*) * (*out_columns_count));
					memset(tmp_rows_columns_array[tmp_row_index], 0, sizeof(char*) * (*out_columns_count));

					for (tmp_column_index = 0; tmp_column_index < (*out_columns_count); tmp_column_index++)
					{
						if (PQgetisnull(tmp_results, tmp_row_index, tmp_column_index) == FALSE)
						{
							tmp_value_length = PQgetlength(tmp_results, tmp_row_index, tmp_column_index);
							tmp_rows_columns_array[tmp_row_index][tmp_column_index] = (char*)malloc(tmp_value_length + 1);
							memcpy(tmp_rows_columns_array[tmp_row_index][tmp_column_index] , PQgetvalue(tmp_results, tmp_row_index, tmp_column_index), tmp_value_length);
							tmp_rows_columns_array[tmp_row_index][tmp_column_index][tmp_value_length] = '\0';
						}
						else
							tmp_rows_columns_array[tmp_row_index][tmp_column_index] = NULL;
					}
				}
			}
			else
			{
				sprintf(in_session->m_last_error_msg, "%s", PQresultErrorMessage (tmp_results));
				LOG_ERROR(in_session, "db_do_query : %s\n", in_query);
				LOG_ERROR(in_session, "db_do_query : result status is %s\n", in_session->m_last_error_msg);
			}

			PQclear(tmp_results);
		}
		else
		{
			sprintf(in_session->m_last_error_msg, "%s", PQerrorMessage((PGconn*)in_session->m_db_connexion));
			LOG_ERROR(in_session, "db_do_query : %s\n", in_query);
			LOG_ERROR(in_session, "db_do_query : %s\n", in_session->m_last_error_msg);
		}
	}
	else
		LOG_ERROR(in_session, "db_do_query : statement query is empty\n");

	return tmp_rows_columns_array;
}


static long long int db_calculate_results_length(net_session *in_session, PGresult *in_results)
{
	unsigned long		tmp_row_index = 0;
	unsigned long		tmp_column_index = 0;

	unsigned long		tmp_rows_count = PQntuples(in_results);
	unsigned long		tmp_columns_count = PQnfields(in_results);

	long long int		tmp_results_length = 0;

#if (defined(_WINDOWS) || defined(WIN32))
	tmp_results_length = sprintf(in_session->m_response, "%c%c%li%c%lu%c%lu%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_rows_count, SEPARATOR_CHAR, tmp_columns_count, SEPARATOR_CHAR);
#else
	tmp_results_length = sprintf(in_session->m_response, "%c%c%i%c%lu%c%lu%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_rows_count, SEPARATOR_CHAR, tmp_columns_count, SEPARATOR_CHAR);
#endif

	for (; tmp_row_index < tmp_rows_count; tmp_row_index++)
	{
		for (tmp_column_index = 0; tmp_column_index < tmp_columns_count; tmp_column_index++)
		{
			if (PQgetisnull(in_results, tmp_row_index, tmp_column_index) == FALSE)
				tmp_results_length += PQgetlength(in_results, tmp_row_index, tmp_column_index);

			tmp_results_length++;
		}
	}
	tmp_results_length++;

	return tmp_results_length;
}


int net_db_do_query(net_session *in_session, const char *in_query, int in_prepare_results_set, unsigned long in_start_result_set_index, unsigned long in_results_set_count)
{
	unsigned long		tmp_row_index = 0;
	unsigned long		tmp_column_index = 0;

	PGresult			*tmp_results = NULL;
	ExecStatusType		tmp_res_status;

	unsigned long		tmp_rows_count = 0;
	unsigned long		tmp_columns_count = 0;

	unsigned long		tmp_max_row_index = 0;

	char				*tmp_ptr = NULL;

	int					tmp_nb_bytes = 0;
	int					tmp_value_length = 0;

	int					tmp_get_last_results_set = is_empty_string(in_query) && in_session != NULL && in_session->m_last_results_set != NULL;

	/* Vérification des paramètres */
	if (in_session != NULL && in_session->m_db_connexion != NULL && in_session->m_response != NULL && (is_empty_string(in_query) == FALSE || tmp_get_last_results_set))
	{
		// Lancer la requete
		if (tmp_get_last_results_set)
		{
			tmp_results = (PGresult*)in_session->m_last_results_set;
			tmp_row_index = in_start_result_set_index;
		}
		else
		{
			LOG_TRACE(in_session, "net_db_do_query : %s\n", in_query);
			db_clear_last_results_set(in_session);
			tmp_results = PQexec((PGconn*)in_session->m_db_connexion, in_query);
			tmp_row_index = 0;
		}

		if (tmp_results != NULL)
		{
			tmp_res_status = PQresultStatus(tmp_results);
			if (tmp_res_status == PGRES_TUPLES_OK)
			{
				tmp_ptr = (char*)in_session->m_response;

				tmp_rows_count = PQntuples(tmp_results);
				tmp_columns_count = PQnfields(tmp_results);
				tmp_max_row_index = tmp_rows_count;
				if (tmp_get_last_results_set)
				{
					if (in_start_result_set_index >= tmp_rows_count)
					{
						LOG_ERROR(in_session, "net_db_do_query : results set index exceed results count\n");
						return net_send_simple_packet(in_session->m_socket, in_session->m_response, NET_RESULTS_SET_INDEX_OUT_OF_BOUND);
					}

					if (tmp_row_index + in_results_set_count < tmp_rows_count)
						tmp_max_row_index = tmp_row_index + in_results_set_count;

#if (defined(_WINDOWS) || defined(WIN32))
					tmp_nb_bytes = sprintf(tmp_ptr, "%c%c%li%c%lu%c%lu%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_max_row_index - tmp_row_index, SEPARATOR_CHAR, tmp_columns_count, SEPARATOR_CHAR);
#else
					tmp_nb_bytes = sprintf(tmp_ptr, "%c%c%i%c%lu%c%lu%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_max_row_index - tmp_row_index, SEPARATOR_CHAR, tmp_columns_count, SEPARATOR_CHAR);
#endif
				}
				else
				{
					if (in_prepare_results_set)
					{
						in_session->m_last_results_set = tmp_results;
#if (defined(_WINDOWS) || defined(WIN32))
						tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%lu%c%lu%c%I64i%c%c",
#else
						tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%lu%c%lu%c%lli%c%c",
#endif
								PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR,
								SEPARATOR_CHAR, tmp_rows_count, SEPARATOR_CHAR, tmp_columns_count, SEPARATOR_CHAR,
								db_calculate_results_length(in_session, tmp_results), SEPARATOR_CHAR, PAQUET_STOP_CHAR);
						return net_send_packet(in_session->m_socket, in_session->m_response, tmp_nb_bytes);
					}
					else
					{
#if (defined(_WINDOWS) || defined(WIN32))
						tmp_nb_bytes = sprintf(tmp_ptr, "%c%c%li%c%lu%c%lu%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_rows_count, SEPARATOR_CHAR, tmp_columns_count, SEPARATOR_CHAR);
#else
						tmp_nb_bytes = sprintf(tmp_ptr, "%c%c%i%c%lu%c%lu%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_rows_count, SEPARATOR_CHAR, tmp_columns_count, SEPARATOR_CHAR);
#endif
					}
				}

				tmp_ptr += tmp_nb_bytes;

				for (; tmp_row_index < tmp_max_row_index; tmp_row_index++)
				{
					for (tmp_column_index = 0; tmp_column_index < tmp_columns_count; tmp_column_index++)
					{
						if (PQgetisnull(tmp_results, tmp_row_index, tmp_column_index) == FALSE)
						{
							tmp_value_length = PQgetlength(tmp_results, tmp_row_index, tmp_column_index);
							if (tmp_nb_bytes + tmp_value_length > MAX_RESPONSE_LENGTH - 1)
							{
								in_session->m_last_results_set = tmp_results;
								LOG_ERROR(in_session, "net_db_do_query : response size exceed maximum size limit\n");
#if (defined(_WINDOWS) || defined(WIN32))
								tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%lu%c%lu%c%I64i%c%c",
#else
								tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%lu%c%lu%c%lli%c%c",
#endif
										PAQUET_START_CHAR, SEPARATOR_CHAR, NET_OUT_OF_RESPONSE_LIMIT_SIZE_ERROR,
										SEPARATOR_CHAR, tmp_rows_count, SEPARATOR_CHAR, tmp_columns_count, SEPARATOR_CHAR,
										db_calculate_results_length(in_session, tmp_results), SEPARATOR_CHAR, PAQUET_STOP_CHAR);
								return net_send_packet(in_session->m_socket, in_session->m_response, tmp_nb_bytes);
							}
							else
							{
								memcpy(tmp_ptr, PQgetvalue(tmp_results, tmp_row_index, tmp_column_index), tmp_value_length);
								tmp_ptr += tmp_value_length;
								tmp_nb_bytes += tmp_value_length;
							}
						}
						if (tmp_nb_bytes + 1 > MAX_RESPONSE_LENGTH - 1)
						{
							in_session->m_last_results_set = tmp_results;
							LOG_ERROR(in_session, "net_db_do_query : response size exceed maximum size limit\n");
#if (defined(_WINDOWS) || defined(WIN32))
								tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%lu%c%lu%c%I64i%c%c",
#else
								tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%lu%c%lu%c%lli%c%c",
#endif
									PAQUET_START_CHAR, SEPARATOR_CHAR, NET_OUT_OF_RESPONSE_LIMIT_SIZE_ERROR,
									SEPARATOR_CHAR, tmp_rows_count, SEPARATOR_CHAR, tmp_columns_count, SEPARATOR_CHAR,
									db_calculate_results_length(in_session, tmp_results), SEPARATOR_CHAR, PAQUET_STOP_CHAR);
							return net_send_packet(in_session->m_socket, in_session->m_response, tmp_nb_bytes);
						}
						else
						{
							tmp_ptr += sprintf(tmp_ptr, "%c", SEPARATOR_CHAR);
							tmp_nb_bytes++;
						}
					}
				}

				if (tmp_nb_bytes + 1 > MAX_RESPONSE_LENGTH - 1)
				{
					in_session->m_last_results_set = tmp_results;
					LOG_ERROR(in_session, "net_db_do_query : response size exceed maximum size limit\n");
#if (defined(_WINDOWS) || defined(WIN32))
								tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%lu%c%lu%c%I64i%c%c",
#else
								tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%lu%c%lu%c%lli%c%c",
#endif
							PAQUET_START_CHAR, SEPARATOR_CHAR, NET_OUT_OF_RESPONSE_LIMIT_SIZE_ERROR,
							SEPARATOR_CHAR, tmp_rows_count, SEPARATOR_CHAR, tmp_columns_count, SEPARATOR_CHAR,
							db_calculate_results_length(in_session, tmp_results), SEPARATOR_CHAR, PAQUET_STOP_CHAR);
					return net_send_packet(in_session->m_socket, in_session->m_response, tmp_nb_bytes);
				}
				else
				{
					if (tmp_get_last_results_set == FALSE)
					{
						PQclear(tmp_results);
						in_session->m_last_results_set = NULL;
					}
					tmp_ptr += sprintf(tmp_ptr, "%c", PAQUET_STOP_CHAR);
					return net_send_packet(in_session->m_socket, in_session->m_response, tmp_ptr - in_session->m_response);
				}
			}
			else
			{
				PQclear(tmp_results);
				in_session->m_last_results_set = NULL;
				sprintf(in_session->m_last_error_msg, "%s", PQresultErrorMessage (tmp_results));
				LOG_ERROR(in_session, "net_db_do_query : %s\n", in_query);
				LOG_ERROR(in_session, "net_db_do_query : result status is %s\n", in_session->m_last_error_msg);
				tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, DB_SQL_ILLEGAL_STATEMENT, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
				return net_send_packet(in_session->m_socket, in_session->m_response, tmp_nb_bytes);
			}

			PQclear(tmp_results);
			in_session->m_last_results_set = NULL;
		}
		else
		{
			sprintf(in_session->m_last_error_msg, "%s", PQerrorMessage((PGconn*)in_session->m_db_connexion));
			LOG_ERROR(in_session, "net_db_do_query : %s\n", in_query);
			LOG_ERROR(in_session, "net_db_do_query : %s\n", in_session->m_last_error_msg);
			tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, DB_CONNEXION_ERROR, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
			return net_send_packet(in_session->m_socket, in_session->m_response, tmp_nb_bytes);
		}
	}
	else
	{
		LOG_ERROR(in_session, "net_db_do_query : statement query is empty or results set is null\n");
		tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, EMPTY_OBJECT, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
		return net_send_packet(in_session->m_socket, in_session->m_response, tmp_nb_bytes);
	}
}



/***********************************************************************************	*
*	db_exec_cmd																																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Exécuter une commande												. 																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_session																											*
*											|		in_cmd																										*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		int																										*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20091123	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_exec_cmd(net_session *in_session, const char *in_cmd)
{

	PGresult						*tmp_results = NULL;
	ExecStatusType		tmp_res_status;
	int					tmp_return_result = DB_SQL_ERROR;

	if (in_session == NULL || in_session->m_db_connexion == NULL)	return DB_CONNEXION_ERROR;

	if (is_empty_string(in_cmd))	return EMPTY_OBJECT;

	LOG_TRACE(in_session, "db_exec_cmd : %s\n", in_cmd);

	tmp_results = PQexec((PGconn*)in_session->m_db_connexion, in_cmd);
	if (tmp_results != NULL)
	{
		tmp_res_status = PQresultStatus(tmp_results);
		if (tmp_res_status == PGRES_COMMAND_OK)
		{
			tmp_return_result = NOERR;
		}
		else
		{
			tmp_return_result = DB_SQL_ERROR;
			sprintf(in_session->m_last_error_msg, "%s", PQresultErrorMessage (tmp_results));
			LOG_ERROR(in_session, "db_exec_cmd : %s\n", in_cmd);
			LOG_ERROR(in_session, "db_exec_cmd : result status is %s\n", in_session->m_last_error_msg);
		}

		PQclear(tmp_results);
	}
	else
	{
		tmp_return_result = DB_CONNEXION_ERROR;
		sprintf(in_session->m_last_error_msg, "%s", PQerrorMessage((PGconn*)in_session->m_db_connexion));
		LOG_ERROR(in_session, "db_exec_cmd : %s\n", in_cmd);
		LOG_ERROR(in_session, "db_exec_cmd : %s\n", in_session->m_last_error_msg);
	}

	return tmp_return_result;
}




/***********************************************************************************	*
*	db_call_procedure																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Exécuter une procedure											. 																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_session																											*
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
void db_call_procedure(net_session *in_session, const char *in_procedure_name, int in_param_count, char **in_out_parameters)
{
	unsigned long	tmp_rows_count;
	unsigned long	tmp_columns_count;

	char						*tmp_ptr = in_session->m_last_query;

	int							tmp_param_index = 0;

	char						***tmp_returned_rows = NULL;
	unsigned long				tmp_row_index = 0;
	unsigned long				tmp_columns_index = 0;

	/* Vérification des paramètres */
	tmp_ptr += sprintf(tmp_ptr, "SELECT %s(", in_procedure_name);

	if (in_param_count > 0)
	{
		if (in_out_parameters[tmp_param_index])
			tmp_ptr += sprintf(tmp_ptr, "'%s'", in_out_parameters[tmp_param_index]);
		else
			tmp_ptr += sprintf(tmp_ptr, "null");
	}

	for (tmp_param_index = 1; tmp_param_index < in_param_count; tmp_param_index++)
	{
		if (in_out_parameters[tmp_param_index])
			tmp_ptr += sprintf(tmp_ptr, ",'%s'", in_out_parameters[tmp_param_index]);
		else
			tmp_ptr += sprintf(tmp_ptr, ",null");
	}

	tmp_ptr += sprintf(tmp_ptr, ");");

	tmp_returned_rows = db_do_query(in_session, in_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
	if (tmp_returned_rows)
	{
		for(tmp_row_index = 0; tmp_row_index < tmp_rows_count; tmp_row_index++)
		{
			for(tmp_columns_index = 0; tmp_columns_index < tmp_columns_count; tmp_columns_index++)
			{
				if (tmp_columns_index < (unsigned long)in_param_count)
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


int net_db_call_procedure(net_session *in_session, const char *in_request_str)
{
	char									*tmp_procedure_call_ptr = in_session->m_last_query;

	char									tmp_procedure_name[256];
	char									tmp_parameter[256];

	const char									*tmp_ptr = in_request_str;

	tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX+1, tmp_ptr, tmp_procedure_name, SEPARATOR_CHAR);
	/* Verifier les parametres */
	if (is_empty_string(tmp_procedure_name) == TRUE)
	{
		LOG_ERROR(in_session, "net_db_call_procedure : procedure name is empty\n");
		return net_send_simple_packet(in_session->m_socket, in_session->m_response, EMPTY_OBJECT);
	}

	tmp_procedure_call_ptr += sprintf(tmp_procedure_call_ptr, "SELECT %s(", tmp_procedure_name);

	tmp_ptr = net_get_field(0, tmp_ptr, tmp_parameter, SEPARATOR_CHAR);
	if (tmp_ptr != NULL)
	{
		if (is_empty_string(tmp_parameter) == FALSE)
			tmp_procedure_call_ptr = print_formated_column(tmp_procedure_call_ptr, tmp_parameter, STANDARD_FORMAT);
		else
			tmp_procedure_call_ptr += sprintf(tmp_procedure_call_ptr, "null");

		tmp_ptr = net_get_field(0, tmp_ptr, tmp_parameter, SEPARATOR_CHAR);
		while (tmp_ptr != NULL)
		{
			if (is_empty_string(tmp_parameter) == FALSE)
			{
				tmp_procedure_call_ptr += sprintf(tmp_procedure_call_ptr, ",");
				tmp_procedure_call_ptr = print_formated_column(tmp_procedure_call_ptr, tmp_parameter, STANDARD_FORMAT);
			}
			else
				tmp_procedure_call_ptr += sprintf(tmp_procedure_call_ptr, ",null");

			tmp_ptr = net_get_field(0, tmp_ptr, tmp_parameter, SEPARATOR_CHAR);
		}
	}

	tmp_procedure_call_ptr += sprintf(tmp_procedure_call_ptr, ");");

	return net_db_do_query(in_session, in_session->m_last_query, FALSE, 0, 0);
}



/***********************************************************************************	*
*	net_db_insert_rows																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Insérertion de données											. 																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_session																											*
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
int net_db_insert_rows(net_session *in_session, entity_def *in_entity_def, const char *in_insert_query)
{
	PGresult			*tmp_results = NULL;
	ExecStatusType		tmp_res_status;

	char 				*tmp_statement_ptr = in_session->m_last_query;
	const char			**tmp_keys_columns = NULL;

	/* Vérification des paramètres */
	if (in_session == NULL || is_empty_string(in_insert_query) == TRUE || in_entity_def == NULL)
		return net_send_simple_packet(in_session->m_socket, in_session->m_response, EMPTY_OBJECT);

	LOG_TRACE(in_session, "net_db_insert_rows  requête : %s\n", in_insert_query);

	// Lancer la requete
	tmp_results = PQexec((PGconn*)in_session->m_db_connexion, in_insert_query);
	if (tmp_results != NULL)
	{
		tmp_res_status = PQresultStatus(tmp_results);
		if (tmp_res_status == PGRES_COMMAND_OK)
		{
			PQclear(tmp_results);

			if (is_empty_string(in_entity_def->m_pk_seq_name) == FALSE)
			{
				/* Generer la requête */
				tmp_keys_columns = in_entity_def->m_entity_columns_names;
				tmp_statement_ptr += sprintf(tmp_statement_ptr, "select %s", *tmp_keys_columns);
				while ((*(++tmp_keys_columns)) != NULL)
					tmp_statement_ptr += sprintf(tmp_statement_ptr, ",%s", *tmp_keys_columns);

				tmp_statement_ptr += sprintf(tmp_statement_ptr, " from %s where %s=currval('%s');", in_entity_def->m_entity_name, *in_entity_def->m_entity_columns_names, in_entity_def->m_pk_seq_name);

				return net_db_do_query(in_session, in_session->m_last_query, FALSE, 0, 0);
			}
			else
			{
				return net_send_simple_packet(in_session->m_socket, in_session->m_response, NOERR);
			}
		}
		else
		{
			PQclear(tmp_results);
			sprintf(in_session->m_last_error_msg, "%s", PQresultErrorMessage (tmp_results));
			LOG_ERROR(in_session, "net_db_insert_rows : %s\n", in_insert_query);
			LOG_ERROR(in_session, "net_db_insert_rows : result status is %s\n", in_session->m_last_error_msg);
			return net_send_simple_packet(in_session->m_socket, in_session->m_response, DB_SQL_ERROR);
		}
	}
	else
	{
		sprintf(in_session->m_last_error_msg, "%s", PQerrorMessage((PGconn*)in_session->m_db_connexion));
		LOG_ERROR(in_session, "net_db_insert_rows : %s\n", in_insert_query);
		LOG_ERROR(in_session, "net_db_insert_rows : %s\n", in_session->m_last_error_msg);
		return net_send_simple_packet(in_session->m_socket, in_session->m_response, DB_CONNEXION_ERROR);
	}
}

/***********************************************************************************	*
*	db_delete_rows																																		*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Suppression de données											. 																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_session																											*
*											|		in_delete_query																						*
*											|		in_out_affected_rows_count																	*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		NOERR si traitement correct															*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_delete_rows(net_session *in_session, const char *in_delete_query, unsigned long *in_out_affected_rows_count)
{
	int								tmp_return_result = NOERR;

	PGresult						*tmp_results = NULL;
	ExecStatusType		tmp_res_status;

	char							*tmp_affected_rows_count = NULL;
	char							*tmp_ptr  =NULL;

	/* Vérification des paramètres */
	if (in_session == NULL || is_empty_string(in_delete_query) == TRUE || in_out_affected_rows_count == NULL)		return EMPTY_OBJECT;

	*in_out_affected_rows_count = 0;

	LOG_TRACE(in_session, "db_delete_rows : requête : %s\n", in_delete_query);

	// Lancer la requete
	tmp_results = PQexec((PGconn*)in_session->m_db_connexion, in_delete_query);
	if (tmp_results != NULL)
	{
		tmp_res_status = PQresultStatus(tmp_results);
		if (tmp_res_status == PGRES_COMMAND_OK)
		{
			tmp_affected_rows_count  = PQcmdTuples(tmp_results);
			if (is_empty_string(tmp_affected_rows_count) == FALSE)
				*in_out_affected_rows_count = strtol(tmp_affected_rows_count, &tmp_ptr, 0);
		}
		else
		{
			sprintf(in_session->m_last_error_msg, "%s", PQresultErrorMessage (tmp_results));
			LOG_ERROR(in_session, "db_delete_rows : %s\n", in_delete_query);
			LOG_ERROR(in_session, "db_delete_rows : result status is %s\n", in_session->m_last_error_msg);
			tmp_return_result = DB_SQL_ERROR;
		}

		PQclear(tmp_results);
	}
	else
	{
		sprintf(in_session->m_last_error_msg, "%s", PQerrorMessage((PGconn*)in_session->m_db_connexion));
		LOG_ERROR(in_session, "db_delete_rows : %s\n", in_delete_query);
		LOG_ERROR(in_session, "db_delete_rows : %s\n", in_session->m_last_error_msg);
		tmp_return_result = DB_CONNEXION_ERROR;
	}

	return tmp_return_result;
}


/***********************************************************************************	*
*	db_update_rows																																		*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Mise   jour de donn�ées											. 																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_session																											*
*											|		in_update_query																						*
*											|		in_out_affected_rows_count																	*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		NOERR si traitement correct															*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20081230	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_update_rows(net_session *in_session, const char *in_update_query, unsigned long *in_out_affected_rows_count)
{
	int								tmp_return_result = NOERR;

	PGresult						*tmp_results = NULL;
	ExecStatusType		tmp_res_status;

	char							*tmp_affected_rows_count = NULL;
	char							*tmp_ptr  =NULL;

	/* Vérification des paramètres */
	if (in_session == NULL || is_empty_string(in_update_query) == TRUE || in_out_affected_rows_count == NULL)		return EMPTY_OBJECT;

	*in_out_affected_rows_count = 0;

	LOG_TRACE(in_session, "db_update_rows : requête : %s\n", in_update_query);

	// Lancer la requete
	tmp_results = PQexec((PGconn*)in_session->m_db_connexion, in_update_query);
	if (tmp_results != NULL)
	{
		tmp_res_status = PQresultStatus(tmp_results);
		if (tmp_res_status == PGRES_COMMAND_OK)
		{
			tmp_affected_rows_count  = PQcmdTuples(tmp_results);
			if (is_empty_string(tmp_affected_rows_count) == FALSE)
				*in_out_affected_rows_count = strtol(tmp_affected_rows_count, &tmp_ptr, 0);
		}
		else
		{
			sprintf(in_session->m_last_error_msg, "%s", PQresultErrorMessage (tmp_results));
			LOG_ERROR(in_session, "db_update_rows : %s\n", in_update_query);
			LOG_ERROR(in_session, "db_update_rows : result status is %s\n", in_session->m_last_error_msg);
			tmp_return_result = DB_SQL_ERROR;
		}

		PQclear(tmp_results);
	}
	else
	{
		sprintf(in_session->m_last_error_msg, "%s", PQerrorMessage((PGconn*)in_session->m_db_connexion));
		LOG_ERROR(in_session, "db_update_rows : %s\n", in_update_query);
		LOG_ERROR(in_session, "db_update_rows : %s\n", in_session->m_last_error_msg);
		tmp_return_result = DB_CONNEXION_ERROR;
	}

	return tmp_return_result;
}

/***********************************************************************************	*
*	db_add_user																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Ajout d'un compte utilisateur. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_username																							*
*										|		in_password																							*
*										|		in_rolename																							*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		void pointeur																						*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20091124	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_add_user(net_session *in_session, const char *in_username, const char *in_password, const char *in_rolename)
{
	if (in_session == NULL || in_session->m_db_connexion == NULL) return DB_CONNEXION_ERROR;

	if (is_empty_string(in_username) || is_empty_string(in_password))	return EMPTY_OBJECT;

	if (is_empty_string(in_rolename) == FALSE)
		sprintf(in_session->m_last_query, "create user %s password '%s' role %s;", in_username, in_password, in_rolename);
	else
		sprintf(in_session->m_last_query, "create user %s password '%s';", in_username, in_password);

	return db_exec_cmd(in_session, in_session->m_last_query);
}


/***********************************************************************************	*
*	db_modify_user_password																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Modifier le mot de passe d'un utilisateur. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_session																							*
*										|		in_username																							*
*										|		in_new_password																							*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		void pointeur																						*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20091127	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_modify_user_password(net_session *in_session, const char *in_username, const char *in_new_password)
{
	if (in_session == NULL || in_session->m_db_connexion == NULL) return DB_CONNEXION_ERROR;

	if (is_empty_string(in_username) || is_empty_string(in_new_password))	return EMPTY_OBJECT;

	sprintf(in_session->m_last_query, "alter user %s password '%s';", in_username, in_new_password);

	return db_exec_cmd(in_session, in_session->m_last_query);
}


void get_where_clause_from_primaries_keys(entity_def *in_entity_def, const char *in_data_str, char *in_where_clause)
{
	const char 						**tmp_current_pk_name = NULL;
	char 							tmp_current_pk_value[16];
	int		 						tmp_current_pk_index = -1;
	const char						*tmp_ptr = in_data_str;
	char							*tmp_where_clause_ptr = in_where_clause;

	if (in_entity_def->m_primary_key != NULL)
	{
		tmp_current_pk_name = in_entity_def->m_primary_key;
		tmp_current_pk_index = index_for_key(in_entity_def, *tmp_current_pk_name);
		net_get_field(tmp_current_pk_index, tmp_ptr, tmp_current_pk_value, SEPARATOR_CHAR);
		if (is_empty_string(tmp_current_pk_value) == FALSE)
		{
			tmp_where_clause_ptr += sprintf(tmp_where_clause_ptr, "%s='%s'", *tmp_current_pk_name, tmp_current_pk_value);
		}
		else
		{
			tmp_where_clause_ptr[0] = '\0';
			return;
		}
		tmp_current_pk_name++;

		while (tmp_current_pk_name && *tmp_current_pk_name)
		{
			tmp_current_pk_index = index_for_key(in_entity_def, *tmp_current_pk_name);
			net_get_field(tmp_current_pk_index, tmp_ptr, tmp_current_pk_value, SEPARATOR_CHAR);
			if (is_empty_string(tmp_current_pk_value) == FALSE)
			{
				tmp_where_clause_ptr += sprintf(tmp_where_clause_ptr, " and %s='%s'", *tmp_current_pk_name, tmp_current_pk_value);
			}
			else
			{
				tmp_where_clause_ptr[0] = '\0';
				return;
			}
			tmp_current_pk_name++;
		}
	}
	else
	{
		tmp_ptr = net_get_field(0, tmp_ptr, tmp_current_pk_value, SEPARATOR_CHAR);
		if (is_empty_string(tmp_current_pk_value) == FALSE)
			tmp_where_clause_ptr += sprintf(tmp_where_clause_ptr, "%s=%s", *(in_entity_def->m_entity_columns_names), tmp_current_pk_value);
		else
			tmp_where_clause_ptr[0] = '\0';
	}

}


/***********************************************************************************	*
*	db_lock_row																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Verrouiller un enregistrement. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_session																							*
*										|		in_entity_signature_id																							*
*										|		in_primary_key_value																							*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		int code retour																						*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20100321	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_lock_row(net_session *in_session, int in_entity_signature_id, const char *in_primary_key_value)
{
	int			tmp_status = DB_RECORD_LOCK_UNAVAILABLE;
	char		*tmp_statement_ptr = in_session->m_last_query;
	char		***tmp_lock_result = NULL;

	unsigned long			tmp_rows_count = 0;
	unsigned long			tmp_columns_count = 0;

	/* Generer la requête */
	tmp_statement_ptr += sprintf(tmp_statement_ptr, "select pg_try_advisory_lock(%i,%s);", in_entity_signature_id, in_primary_key_value);

	/* Executer la requête */
	tmp_lock_result = db_do_query(in_session, in_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
	if (tmp_lock_result != NULL && tmp_rows_count == 1 && tmp_columns_count == 1)
	{
		if (is_empty_string(tmp_lock_result[0][0]) == FALSE && tmp_lock_result[0][0][0] == 't')
			tmp_status = NOERR;
	}

	free_rows_and_columns(&tmp_lock_result, tmp_rows_count, tmp_columns_count);

	return tmp_status;
}


/***********************************************************************************	*
*	db_unlock_row																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Deverrouiller un enregistrement. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_session																							*
*										|		in_entity_signature_id																							*
*										|		in_primary_key_value																							*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		int code retour																						*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20100321	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_unlock_row(net_session *in_session, int in_entity_signature_id, const char *in_primary_key_value)
{
	int			tmp_status = DB_RECORD_UNLOCK_UNAVAILABLE;
	char		*tmp_statement_ptr = in_session->m_last_query;
	char		***tmp_unlock_result = NULL;

	unsigned long			tmp_rows_count = 0;
	unsigned long			tmp_columns_count = 0;

	/* Generer la requête */
	tmp_statement_ptr += sprintf(tmp_statement_ptr, "select pg_advisory_unlock(%i,%s);", in_entity_signature_id, in_primary_key_value);

	/* Executer la requête */
	tmp_unlock_result = db_do_query(in_session, in_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
	if (tmp_unlock_result != NULL && tmp_rows_count == 1 && tmp_columns_count == 1)
	{
		if (is_empty_string(tmp_unlock_result[0][0]) == FALSE && tmp_unlock_result[0][0][0] == 't')
			tmp_status = NOERR;
	}

	free_rows_and_columns(&tmp_unlock_result, tmp_rows_count, tmp_columns_count);

	return tmp_status;
}


/***********************************************************************************	*
*	db_get_username_for_lock_row																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Renvoie le nom de l'utilisateur verrouillant un enregistrement. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_session																							*
*										|		in_entity_signature_id																							*
*										|		in_primary_key_value																							*
*										|		in_out_username																							*
*										|		in_out_username_length																							*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		int code retour																						*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20100321	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_get_username_for_lock_row(net_session *in_session, int in_entity_signature_id, const char *in_primary_key_value, char* in_out_username, int *in_out_username_length)
{
	int			tmp_status = NOERR;
	char		*tmp_statement_ptr = in_session->m_last_query;
	char		***tmp_results = NULL;

	unsigned long			tmp_rows_count = 0;
	unsigned long			tmp_columns_count = 0;

	*in_out_username_length = 0;
	in_out_username[0] = '\0';

	/* Generer la requête */
	tmp_statement_ptr += sprintf(tmp_statement_ptr, "select usename from pg_stat_activity, pg_locks where pg_stat_activity.procpid=pg_locks.pid and pg_locks.classid=%i and pg_locks.objid=%s;", in_entity_signature_id, in_primary_key_value);

	/* Executer la requête */
	tmp_results = db_do_query(in_session, in_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
	if (tmp_results != NULL && tmp_rows_count >= 1 && tmp_columns_count >= 1)
	{
		*in_out_username_length = sprintf(in_out_username, "%s", tmp_results[0][0]);
	}

	free_rows_and_columns(&tmp_results, tmp_rows_count, tmp_columns_count);

	return tmp_status;
}


/***********************************************************************************	*
*	db_transaction_start																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Debute une transaction. 							*
***********************************************************************************	*/
int db_transaction_start(net_session *in_session)
{
	return db_exec_cmd(in_session, "BEGIN;");
}

/***********************************************************************************	*
*	db_transaction_commit																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Valide une transaction. 							*
***********************************************************************************	*/
int db_transaction_commit(net_session *in_session)
{
	return db_exec_cmd(in_session, "COMMIT;");
}

/***********************************************************************************	*
*	db_transaction_rollback																																					*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Annule une transaction. 							*
***********************************************************************************	*/
int db_transaction_rollback(net_session *in_session)
{
	return db_exec_cmd(in_session, "ROLLBACK;");
}


/***********************************************************************************	*
*	db_clear_last_results_set																																					*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Vider le dernier ensemble de resultats. 							*
***********************************************************************************	*/
void db_clear_last_results_set(net_session *in_session)
{
	if (in_session != NULL && in_session->m_last_results_set != NULL)
	{
		PQclear(in_session->m_last_results_set);
		in_session->m_last_results_set = NULL;
	}
}


/***********************************************************************************	*
*	db_has_user_this_role																																					*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Verifier si un utilisateur à un rôle particulier. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|		in_session																							*
*										|		in_username																							*
*										|		in_rolename																							*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|		int code retour																						*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20110207	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int db_has_user_this_role(net_session *in_session, const char *in_username, const char *in_rolename)
{
	int 				tmp_return_value = FALSE;
	char				***tmp_returned_rows = NULL;
	unsigned long		tmp_rows_count = 0;
	unsigned long		tmp_columns_count = 0;

	sprintf(in_session->m_last_query, "select is_grantable from information_schema.applicable_roles where grantee='%s' and role_name='%s';", in_username, in_rolename);
	tmp_returned_rows = db_do_query(in_session, in_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
	if (tmp_returned_rows && tmp_rows_count == 1 && tmp_columns_count == 1)
		tmp_return_value = TRUE;

	free_rows_and_columns(&tmp_returned_rows, tmp_rows_count, tmp_columns_count);

	return tmp_return_value;
}


int db_server_version(net_session *in_session)
{
  int tmp_version = PQserverVersion(in_session->m_db_connexion);
  int tmp_major_version = tmp_version / 10000;
  int tmp_medium_version = (tmp_version - tmp_major_version * 10000) / 100;
  int tmp_minor_version = tmp_version - tmp_major_version * 10000 - tmp_medium_version * 100;

  return (tmp_major_version << 16) + (tmp_medium_version << 8) + tmp_minor_version;
}

int net_db_put_blob(net_session *in_session)
{
	int						tmp_status = NOERR;
	int						tmp_file = 0;
	long					tmp_file_size = 0;
	char					tmp_file_size_str[16];
	long					tmp_total_bytes_recv = 0;
	int						tmp_nb_bytes_recv = 0;

	Oid						tmp_blob_oid;
	int						tmp_nb_bytes = 0;

	net_get_field(NET_MESSAGE_TYPE_INDEX+2, in_session->m_request, tmp_file_size_str, SEPARATOR_CHAR);
	if (is_empty_string(tmp_file_size_str) == FALSE)
		tmp_file_size = atol(tmp_file_size_str);

	if (tmp_file_size > 0)
	{
	    tmp_blob_oid = lo_creat(in_session->m_db_connexion, INV_READ | INV_WRITE);
		if (tmp_blob_oid != InvalidOid)
		{
			LOG_TRACE(in_session, "net_db_put_blob : file creation %i\n", tmp_blob_oid);
			tmp_file = lo_open(in_session->m_db_connexion, tmp_blob_oid, INV_WRITE);
			if (tmp_file != -1)
			{
				LOG_TRACE(in_session, "net_db_put_blob : opening file %i\n", tmp_blob_oid);
#if (defined(_WINDOWS) || defined(WIN32))
				tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%i%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_blob_oid, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#else
				tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%i%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_blob_oid, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#endif
				tmp_status = net_send_packet(in_session->m_socket, in_session->m_response, tmp_nb_bytes);
				if (tmp_status == NOERR)
				{
					// Receptionner le contenu du fichier
					while (tmp_total_bytes_recv < tmp_file_size)
					{
						tmp_nb_bytes_recv = recv(in_session->m_socket, in_session->m_request, MAX_PAQUET_LENGTH - 1, 0);
						if (tmp_nb_bytes_recv > 0)
						{
							tmp_total_bytes_recv += tmp_nb_bytes_recv;
							tmp_nb_bytes_recv = lo_write(in_session->m_db_connexion, tmp_file, in_session->m_request, tmp_nb_bytes_recv);
						}
						else
						{
							LOG_SOCK_ERRORNO(in_session, "net_db_put_blob (Error receiving file)");
							tmp_status = NET_SEND_PAQUET_ERROR;
							break;
						}
					}

					if (tmp_status == NOERR)
						tmp_status = net_send_packet(in_session->m_socket, in_session->m_response, tmp_nb_bytes);
				}
				else
				{
					LOG_SOCK_ERRORNO(in_session, "net_db_put_blob");
					LOG_ERROR(in_session, "net_db_put_blob : error sending transmit ack (blob id=%i, file id=%i)\n.", tmp_blob_oid, tmp_file);
				}
				lo_close(in_session->m_db_connexion, tmp_file);
			}
			else
			{
				sprintf(in_session->m_last_error_msg, "%s", PQerrorMessage((PGconn*)in_session->m_db_connexion));
				LOG_ERROR(in_session, "net_db_put_blob : error lo_open (%s) (blob id=%i, file id=%i)\n.", in_session->m_last_error_msg, tmp_blob_oid, tmp_file);
				tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_response, FILE_OPENING_ERROR);
			}
		}
		else
		{
			sprintf(in_session->m_last_error_msg, "%s", PQerrorMessage((PGconn*)in_session->m_db_connexion));
			LOG_ERROR(in_session, "net_db_put_blob : error lo_creat (%s)\n.", in_session->m_last_error_msg);
			tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_response, FILE_OPENING_ERROR);
		}
	}
	else
	{
		tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_response, FILE_EMPTY_ERROR);
	}
	return tmp_status;
}


int net_db_get_blob(net_session *in_session)
{
	int						tmp_status = NOERR;
	int						tmp_file = 0;
	long					tmp_file_size = 0;
	long					tmp_total_bytes_send = 0;
	int						tmp_nb_bytes_send = 0;

	char					tmp_blob_oid_str[16];
	Oid						tmp_blob_oid = InvalidOid;
	int						tmp_nb_bytes = 0;

    PGresult				*tmp_res = NULL;

    net_get_field(NET_MESSAGE_TYPE_INDEX+1, in_session->m_request, tmp_blob_oid_str, SEPARATOR_CHAR);
	if (is_empty_string(tmp_blob_oid_str) == FALSE)
		tmp_blob_oid = atol(tmp_blob_oid_str);

	tmp_res = PQexec(in_session->m_db_connexion, "begin");
    PQclear(tmp_res);

	tmp_file = lo_open(in_session->m_db_connexion, tmp_blob_oid, INV_WRITE | INV_READ);
	if (tmp_file != -1)
	{
		LOG_TRACE(in_session, "net_db_get_blob : Opening file %i\n", tmp_blob_oid);

		// Calculer la taille du fichier en se positionnant à la fin
		tmp_file_size = lo_lseek(in_session->m_db_connexion, tmp_file, 0, SEEK_END);
		// Se repositionner au debut du fichier avant debut du transfert
		if (tmp_file_size > 0 && lo_lseek(in_session->m_db_connexion, tmp_file, 0, SEEK_SET) != 1)
		{
			LOG_TRACE(in_session, "net_db_get_blob : File size %li\n", tmp_file_size);
#if (defined(_WINDOWS) || defined(WIN32))
			tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%li%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_file_size, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#else
			tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%li%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_file_size, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#endif
			tmp_status = net_send_packet(in_session->m_socket, in_session->m_response, tmp_nb_bytes);
			if (tmp_status == NOERR)
			{
				// Confirmation de debut du transfert
				tmp_status = net_rcv_packet(in_session->m_socket, in_session->m_request);
				if (tmp_status == NOERR)
				{
					LOG_TRACE(in_session, "net_db_get_blob : reception client ok %s\n", in_session->m_request);

					tmp_status = net_get_packet_type(in_session->m_request);
					if (tmp_status == NOERR)
					{
						// Envoyer le contenu du fichier
						while (tmp_total_bytes_send < tmp_file_size)
						{
							tmp_nb_bytes_send = lo_read(in_session->m_db_connexion, tmp_file, in_session->m_response, MAX_PAQUET_LENGTH - 1);
							if (tmp_nb_bytes_send > 0)
							{
								tmp_nb_bytes_send = send(in_session->m_socket, in_session->m_response, tmp_nb_bytes_send, 0);
								if (tmp_nb_bytes_send > 0)
								{
									tmp_total_bytes_send += tmp_nb_bytes_send;
								}
								else
								{
									LOG_SOCK_ERRORNO(in_session, "net_db_get_blob (Error sending blob)");
									tmp_status = NET_SEND_PAQUET_ERROR;
									break;
								}
							}
							else
							{
								sprintf(in_session->m_last_error_msg, "%s", PQerrorMessage((PGconn*)in_session->m_db_connexion));
								LOG_ERROR(in_session, "net_db_get_blob : lo_read error : %s (blob id=%i file id=%i, file size %li)\n", in_session->m_last_error_msg, tmp_blob_oid, tmp_file, tmp_file_size);
								tmp_status = FILE_READING_ERROR;
								break;
							}
						}
						LOG_TRACE(in_session, "net_db_get_blob : Sending file end %i\n", tmp_blob_oid);

						in_session->m_response[0] = '\0';
					}
					else
					{
						LOG_ERROR(in_session, "net_db_get_blob : Error send by the client %i\n", tmp_status);
						LOG_ERROR(in_session, "net_db_get_blob : %s\n", in_session->m_request);
						tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_response, tmp_status);
					}
				}
			}
			else
			{
				LOG_SOCK_ERRORNO(in_session, "net_db_get_blob");
			}
			lo_close(in_session->m_db_connexion, tmp_file);
		}
		else
		{
			sprintf(in_session->m_last_error_msg, "%s", PQerrorMessage((PGconn*)in_session->m_db_connexion));
			LOG_ERROR(in_session, "net_db_get_blob : lo_lseek error : %s (blob id=%i, file id=%i, file size %li)\n", in_session->m_last_error_msg, tmp_blob_oid, tmp_file, tmp_file_size);
			tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_response, FILE_OPENING_ERROR);
		}
	}
	else
	{
		sprintf(in_session->m_last_error_msg, "%s", PQerrorMessage((PGconn*)in_session->m_db_connexion));
		LOG_ERROR(in_session, "net_db_get_blob : lo_open error : %s  (blob id=%i)\n", in_session->m_last_error_msg, tmp_blob_oid);
		tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_response, FILE_OPENING_ERROR);
	}

    tmp_res = PQexec(in_session->m_db_connexion, "end");
    PQclear(tmp_res);

	return tmp_status;
}


int net_db_delete_blob(net_session *in_session)
{
	char					tmp_blob_oid_str[16];
	Oid						tmp_blob_oid = InvalidOid;
	int						tmp_status = NOERR;

	net_get_field(NET_MESSAGE_TYPE_INDEX+1, in_session->m_request, tmp_blob_oid_str, SEPARATOR_CHAR);
	if (is_empty_string(tmp_blob_oid_str) == FALSE)
		tmp_blob_oid = atol(tmp_blob_oid_str);

	if (lo_unlink(in_session->m_db_connexion, tmp_blob_oid) == 1)
		tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_response, NOERR);
	else
		tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_response, FILE_NOT_EXISTS);

    return tmp_status;
}

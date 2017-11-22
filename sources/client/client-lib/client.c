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
#include <io.h>
#else
#ifdef LDAP
#include <ldap.h>
#endif
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#endif

#include "client.h"
#include "../common/utilities.h"
#include "../common/entities.h"
#include "../common/constants.h"
#include "../common/errors.h"

#if defined (__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

void cl_close_connexion(net_session *in_session)
{
	LOG_TRACE(in_session, "Fermeture socket %i\n", in_session->m_socket);

#if defined(_WINDOWS) || defined(WIN32)
	closesocket(in_session->m_socket);
#else
	close(in_session->m_socket);
#endif
	in_session->m_socket = 0;
}

int cl_connect_db(net_session *in_session)
{
	int			tmp_status = 0;
	int			tmp_nb_bytes = 0;

	char		tmp_protocol_major_version[4];
	char		tmp_protocol_medium_version[4];
	char		tmp_protocol_minor_version[4];
	char		tmp_protocol_maintenance_version[4];

	int			tmp_protocol_major_version_int;
	int			tmp_protocol_medium_version_int;
	int			tmp_protocol_minor_version_int;
	int			tmp_protocol_maintenance_version_int;

	const char		*tmp_ptr = in_session->m_response;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%s%c%d%c%d%c%d%c%d%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, START_SESSION,
		SEPARATOR_CHAR, in_session->m_username,
		SEPARATOR_CHAR, in_session->m_crypted_password,
		SEPARATOR_CHAR, (PROTOCOL_VERSION >> 24),
		SEPARATOR_CHAR, ((PROTOCOL_VERSION >> 16) & 0xFF),
		SEPARATOR_CHAR, ((PROTOCOL_VERSION >> 8) & 0xFF),
		SEPARATOR_CHAR, (PROTOCOL_VERSION & 0xFF),
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	LOG_TRACE(in_session, "_____________________ CONNECTION REQUEST ____________________\n");
	LOG_TRACE(in_session, "%s\n", in_session->m_request);

	tmp_status = net_send_packet(in_session->m_socket, in_session->m_request, tmp_nb_bytes);
	if (tmp_status != NOERR)
	{
		LOG_SOCK_ERRORNO(in_session, "cl_connect_db error (net_send_packet)");
		return tmp_status;
	}

	tmp_status = net_rcv_packet(in_session->m_socket, in_session->m_response);
	if (tmp_status != NOERR)
	{
		LOG_SOCK_ERRORNO(in_session, "cl_connect_db error (net_rcv_packet)");
		return tmp_status;
	}

	LOG_TRACE(in_session, "---------------------- CONNECTION RESPONSE -----------------------\n");
	LOG_TRACE(in_session, "%s\n", in_session->m_response);

	tmp_status = net_get_packet_type(in_session->m_response);
	if (tmp_status != NOERR)
	{
		cl_close_connexion(in_session);
		return tmp_status;
	}

	tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX+1, tmp_ptr, tmp_protocol_major_version, SEPARATOR_CHAR);
	tmp_ptr = net_get_field(0, tmp_ptr, tmp_protocol_medium_version, SEPARATOR_CHAR);
	tmp_ptr = net_get_field(0, tmp_ptr, tmp_protocol_minor_version, SEPARATOR_CHAR);
	tmp_ptr = net_get_field(0, tmp_ptr, tmp_protocol_maintenance_version, SEPARATOR_CHAR);

	if (is_empty_string(tmp_protocol_major_version) || is_empty_string(tmp_protocol_medium_version) || is_empty_string(tmp_protocol_minor_version) || is_empty_string(tmp_protocol_maintenance_version))
	{
		cl_close_connexion(in_session);
		return NET_PROTOCOL_VERSION_INCOMPATIBLE;
	}

	tmp_protocol_major_version_int = atoi(tmp_protocol_major_version);
	tmp_protocol_medium_version_int = atoi(tmp_protocol_medium_version);
	tmp_protocol_minor_version_int = atoi(tmp_protocol_minor_version);
	tmp_protocol_maintenance_version_int = atoi(tmp_protocol_maintenance_version);

	if (tmp_protocol_major_version_int != (PROTOCOL_VERSION >> 24) || tmp_protocol_medium_version_int != ((PROTOCOL_VERSION >> 16) & 0xFF))
	{
		cl_close_connexion(in_session);
		return NET_PROTOCOL_VERSION_INCOMPATIBLE;
	}

	return tmp_status;
}


int cl_connect_netserver(net_session *in_session)
{
	int			tmp_status = 0;
	struct	sockaddr_in	addr_client;

#if (defined(_WINDOWS) || defined(WIN32))
	DWORD recv_timeout;
	DWORD send_timeout;
#else
	struct timeval recv_timeout;
	struct timeval send_timeout;
#endif

	cl_close_connexion(in_session);

	/* Initialiser la connexion */
	tmp_status = net_init_cnx (&in_session->m_socket, 0, &addr_client);
	if (tmp_status != NOERR)
	{
		LOG_ERROR(in_session, "cl_connect_netserver : net_init_cnx error %i\n", tmp_status);
		return tmp_status;
	}

	/* constitution de l'adresse reseau du serveur */
	in_session->m_struct_host = gethostbyname(in_session->m_hostname);
	if (in_session->m_struct_host == NULL){
#if (defined(_WINDOWS) || defined(WIN32))
		LOG_ERROR(in_session, "cl_connect_netserver : gethostbyname error %i\n", WSAGetLastError());
#else
		LOG_ERROR(in_session, "cl_connect_netserver : gethostbyname error %i\n", errno);
#endif
		return NET_GET_HOST_BY_NAME_ERROR;
	}

	/* Initialisation des champs de addr_serveur */
	in_session->m_addr_host.sin_family = in_session->m_struct_host->h_addrtype;
	in_session->m_addr_host.sin_port = htons(in_session->m_host_port);
	in_session->m_addr_host.sin_addr = *(struct in_addr *)in_session->m_struct_host->h_addr_list[0];
	//printf("*** Adresse IP (decimal) : %s ***\n", inet_ntoa(addr_serveur.sin_addr));

	/* Connection au serveur */
	LOG_TRACE(in_session, "cl_connect : Connexion to %s (port %i)\n", in_session->m_hostname, in_session->m_host_port);

	tmp_status = connect(in_session->m_socket, (struct sockaddr*)&in_session->m_addr_host, sizeof(struct sockaddr_in));
	if (tmp_status == -1)
	{
#if (defined(_WINDOWS) || defined(WIN32))
		LOG_ERROR(in_session, "cl_connect_netserver : connect error %i\n", WSAGetLastError());
#else
		LOG_ERROR(in_session, "cl_connect_netserver : connect\n");
#endif
		return NET_CONNEXION_ERROR;
	}

	LOG_TRACE(in_session, "cl_connect_netserver : socket %i\n", in_session->m_socket);

	/* Parametrer un timeout de reception */
#if (defined(_WINDOWS) || defined(WIN32))
	recv_timeout = in_session->m_recv_timeout * 1000;
#else

	recv_timeout.tv_sec = in_session->m_recv_timeout;
	recv_timeout.tv_usec = 0;
#endif

	tmp_status = setsockopt(in_session->m_socket, SOL_SOCKET, SO_RCVTIMEO,
		(char*)&recv_timeout, sizeof(recv_timeout));
	if (tmp_status == -1)
	{
		LOG_SOCK_ERRORNO(in_session, "cl_connect_netserver (setsockopt SO_RCVTIMEO)");
		return NET_INIT_CONNEXION;
	}

	/* Parametrer un timeout d'emission */
#if (defined(_WINDOWS) || defined(WIN32))
	send_timeout = in_session->m_send_timeout * 1000;
#else
	send_timeout.tv_sec = in_session->m_send_timeout;
	send_timeout.tv_usec = 0;
#endif

	tmp_status = setsockopt(in_session->m_socket, SOL_SOCKET, SO_SNDTIMEO,
		(char*)&send_timeout, sizeof(send_timeout));
	if (tmp_status == -1)
	{
		LOG_SOCK_ERRORNO(in_session, "cl_connect_netserver (setsockopt SO_SNDTIMEO)");
		return NET_INIT_CONNEXION;
	}

	return cl_connect_db(in_session);
}


int cl_send_simple_request(net_session *in_session, int in_packet_type, int in_log_trace)
{
	int			tmp_status = NOERR;

	if (in_log_trace){
		LOG_TRACE(in_session, "_____________________ REQUEST %s ______________________\n", net_get_command_desc(net_get_packet_type(in_session->m_request)));
	}
	tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_request, in_packet_type);
	if (in_log_trace){
		LOG_TRACE(in_session, "%s\n", in_session->m_request);
	}
	if (tmp_status == NOERR)
	{
		tmp_status = net_rcv_packet(in_session->m_socket, in_session->m_response);
		if (tmp_status == NOERR)
		{
			if (in_log_trace){
				LOG_TRACE(in_session, "--------------------- RESPONSE ----------------------\n");
				LOG_TRACE(in_session, "%s\n", in_session->m_response);
			}
		}
	}

	if (tmp_status != NOERR)
	{
		LOG_TRACE(in_session, "Trying reconnect...(cause error %i)\n", tmp_status);
		tmp_status = cl_connect_netserver(in_session);
		if (tmp_status != NOERR)           return tmp_status;
	}
	else
		return net_get_packet_type(in_session->m_response);

	return tmp_status;
}


int cl_send_request(net_session *in_session, int in_nb_bytes)
{
	int			tmp_status = NOERR;

	LOG_TRACE(in_session, "_____________________ REQUEST %s _____________________\n", net_get_command_desc(net_get_packet_type(in_session->m_request)));
	LOG_TRACE(in_session, "%s\n", in_session->m_request);
	tmp_status = net_send_packet(in_session->m_socket, in_session->m_request, in_nb_bytes);
	if (tmp_status == NOERR)
	{
		tmp_status = net_rcv_packet(in_session->m_socket, in_session->m_response);
		if (tmp_status == NOERR)
		{
			LOG_TRACE(in_session, "--------------------- RESPONSE ----------------------\n");
			LOG_TRACE(in_session, "%s\n", in_session->m_response);
		}
	}


	if (tmp_status != NOERR)
	{
		LOG_SOCK_ERRORNO(in_session, "cl_send_request");
		LOG_TRACE(in_session, "Trying reconnect...(cause error %i)\n", tmp_status);
		tmp_status = cl_connect_netserver(in_session);
		if (tmp_status != NOERR)           return tmp_status;
	}
	else
		return net_get_packet_type(in_session->m_response);

	return tmp_status;
}


static char* cl_array_strings_to_net_str(unsigned int in_columns_count, char** in_columns_record, char *in_data_str)
{
	char							*tmp_data_ptr = in_data_str;
	unsigned int 		tmp_index = 0;

	if (in_columns_record != NULL)
	{
		for (tmp_index = 0; tmp_index < in_columns_count; tmp_index++)
		{
			if (in_columns_record[tmp_index] != NULL)
				tmp_data_ptr = format_string(in_columns_record[tmp_index], tmp_data_ptr);

			tmp_data_ptr += sprintf(tmp_data_ptr, "%c", SEPARATOR_CHAR);
		}
	}

	return tmp_data_ptr;
}


static char* cl_array_non_formated_strings_to_net_str(unsigned int in_columns_count, char** in_columns_record, char *in_data_str)
{
	char							*tmp_data_ptr = in_data_str;
	unsigned int 		tmp_index = 0;

	if (in_columns_record != NULL)
	{
		for (tmp_index = 0; tmp_index < in_columns_count; tmp_index++)
		{
			if (in_columns_record[tmp_index] != NULL)
				tmp_data_ptr += sprintf(tmp_data_ptr, "%s", in_columns_record[tmp_index]);

			tmp_data_ptr += sprintf(tmp_data_ptr, "%c", SEPARATOR_CHAR);
		}
	}

	return tmp_data_ptr;
}


static char* cl_arrays_strings_to_net_str(entity_def *in_entity_def, char** in_columns_record, char** in_originals_columns_record, char *in_data_str)
{
	char							*tmp_data_ptr = in_data_str;
	unsigned int 		tmp_index = 0;
	unsigned int		tmp_diff_columns = 0;

	if (in_originals_columns_record != NULL)
	{
		// Clé primaire
		tmp_data_ptr = format_string(in_columns_record[0], tmp_data_ptr);
		tmp_data_ptr += sprintf(tmp_data_ptr, "%c", SEPARATOR_CHAR);

		for (tmp_index = 1; tmp_index < in_entity_def->m_entity_columns_count; tmp_index++)
		{
			if ((in_entity_def->m_primary_key != NULL &&  index_of_string_in_array(in_entity_def->m_entity_columns_names[tmp_index], in_entity_def->m_primary_key) >= 0))
			{
				tmp_data_ptr = format_string(in_columns_record[tmp_index], tmp_data_ptr);
				tmp_data_ptr += sprintf(tmp_data_ptr, "%c", SEPARATOR_CHAR);
			}
			else
			{
				if (compare_values(in_columns_record[tmp_index], in_originals_columns_record[tmp_index]) != 0)
				{
					tmp_diff_columns++;
					if (is_empty_string(in_columns_record[tmp_index]))
					{
						tmp_data_ptr += sprintf(tmp_data_ptr, "''%c", SEPARATOR_CHAR);
					}
					else
					{
						tmp_data_ptr = format_string(in_columns_record[tmp_index], tmp_data_ptr);
						tmp_data_ptr += sprintf(tmp_data_ptr, "%c", SEPARATOR_CHAR);
					}
				}
				else
					tmp_data_ptr += sprintf(tmp_data_ptr, "%c", SEPARATOR_CHAR);
			}
		}

		// Aucun changement
		if (tmp_diff_columns == 0)
			return NULL;
	}
	else
	{
		for (tmp_index = 0; tmp_index < in_entity_def->m_entity_columns_count; tmp_index++)
		{
			if (in_columns_record[tmp_index] != NULL)
			{
				tmp_data_ptr = format_string(in_columns_record[tmp_index], tmp_data_ptr);
				tmp_data_ptr += sprintf(tmp_data_ptr, "%c", SEPARATOR_CHAR);
			}
			else
				tmp_data_ptr += sprintf(tmp_data_ptr, "''%c", SEPARATOR_CHAR);
		}
	}

	return tmp_data_ptr;
}


static char ***cl_parse_response_rows(net_session *in_session, entity_def *in_entity_def, char *in_data_str, unsigned long *out_rows_count, unsigned long *out_columns_count)
{
	const char							*tmp_data_ptr = in_data_str;
	const char							*tmp_current_data_ptr = NULL;
	char							tmp_rows_count_str[16];
	char							tmp_columns_count_str[16];
	char							***tmp_rows_columns_array = NULL;

	unsigned long		tmp_row_index = 0;
	unsigned long		tmp_column_index = 0;

	unsigned long		tmp_column_count = 0;

	tmp_data_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX+1, tmp_data_ptr, tmp_rows_count_str, SEPARATOR_CHAR);
	tmp_data_ptr = net_get_field(0, tmp_data_ptr, tmp_columns_count_str, SEPARATOR_CHAR);

	*out_rows_count = strtoul(tmp_rows_count_str, NULL, 10);
	*out_columns_count = strtoul(tmp_columns_count_str, NULL, 10);
	if (in_entity_def != NULL)
		tmp_column_count = in_entity_def->m_entity_columns_count;
	else
		tmp_column_count = (*out_columns_count);

	if ((*out_rows_count)  > 0 && (*out_columns_count) > 0)
	{
		tmp_rows_columns_array = (char***)malloc(sizeof(char*) * (*out_rows_count));
		memset(tmp_rows_columns_array, 0, sizeof(char*) * (*out_rows_count));

		for (tmp_row_index = 0; tmp_row_index < (*out_rows_count); tmp_row_index++)
		{
			tmp_rows_columns_array[tmp_row_index] = (char**)malloc(sizeof(char*) * tmp_column_count);
			memset(tmp_rows_columns_array[tmp_row_index], 0, sizeof(char*) * tmp_column_count);

			for (tmp_column_index = 0; tmp_column_index < (*out_columns_count); tmp_column_index++)
			{
				tmp_current_data_ptr = net_get_field(0, tmp_data_ptr, in_session->m_column_buffer, SEPARATOR_CHAR);
				if (tmp_column_index < tmp_column_count)
				{
					if (is_empty_string(in_session->m_column_buffer) == FALSE)
					{
						tmp_rows_columns_array[tmp_row_index][tmp_column_index] = (char*)malloc(tmp_current_data_ptr - tmp_data_ptr + 1);
						strcpy(tmp_rows_columns_array[tmp_row_index][tmp_column_index] , in_session->m_column_buffer);
					}
					else
						tmp_rows_columns_array[tmp_row_index][tmp_column_index] = NULL;
				}
				tmp_data_ptr = tmp_current_data_ptr;
			}
		}
	}

	return tmp_rows_columns_array;
}


static char*** cl_create_results_set(net_session *in_session, entity_def *in_entity_def, unsigned long *out_rows_count, unsigned long *out_columns_count, unsigned long long int *out_results_set_length)
{
	const char							*tmp_data_ptr = in_session->m_response;
	char							tmp_rows_count_str[16];
	char							tmp_columns_count_str[16];
	char							tmp_results_length_str[32];
	char							***tmp_rows_columns_array = NULL;

	unsigned long		tmp_row_index = 0;

	unsigned long		tmp_column_count = 0;


	tmp_data_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX+1, tmp_data_ptr, tmp_rows_count_str, SEPARATOR_CHAR);
	tmp_data_ptr = net_get_field(0, tmp_data_ptr, tmp_columns_count_str, SEPARATOR_CHAR);
	tmp_data_ptr = net_get_field(0, tmp_data_ptr, tmp_results_length_str, SEPARATOR_CHAR);

	*out_rows_count = strtoul(tmp_rows_count_str, NULL, 10);
	*out_columns_count = strtoul(tmp_columns_count_str, NULL, 10);
#if ((defined(_WINDOWS) || defined(WIN32)) && !defined(__GNUC__))
	*out_results_set_length = _atoi64(tmp_results_length_str);
#else
	*out_results_set_length = strtoll(tmp_results_length_str, NULL, 10);
#endif

	if (in_entity_def != NULL)
		tmp_column_count = in_entity_def->m_entity_columns_count;
	else
		tmp_column_count = (*out_columns_count);


	if ((*out_rows_count)  > 0 && (*out_columns_count) > 0 && (*out_results_set_length) > 0)
	{
		tmp_rows_columns_array = (char***)malloc(sizeof(char*) * (*out_rows_count));
		memset(tmp_rows_columns_array, 0, sizeof(char*) * (*out_rows_count));

		for (tmp_row_index = 0; tmp_row_index < (*out_rows_count); tmp_row_index++)
		{
			tmp_rows_columns_array[tmp_row_index] = (char**)malloc(sizeof(char*) * tmp_column_count);
			memset(tmp_rows_columns_array[tmp_row_index], 0, sizeof(char*) * tmp_column_count);
		}
	}

	return tmp_rows_columns_array;
}


static int cl_parse_response_rows_for_results_set(net_session *in_session, entity_def *in_entity_def, char *in_data_str, char*** in_results_set, unsigned long long int in_start_row_index, unsigned long long int *out_rows_count, unsigned long long int *out_columns_count, net_callback_fct *in_callback)
{
	const char							*tmp_data_ptr = in_data_str;
	const char							*tmp_current_data_ptr = NULL;
	char							tmp_rows_count_str[16];
	char							tmp_columns_count_str[16];

	unsigned long					tmp_row_index = 0;
	unsigned long long int			tmp_column_index = 0;

	unsigned long long int			tmp_column_count = 0;

	tmp_data_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX+1, tmp_data_ptr, tmp_rows_count_str, SEPARATOR_CHAR);
	tmp_data_ptr = net_get_field(0, tmp_data_ptr, tmp_columns_count_str, SEPARATOR_CHAR);

	*out_rows_count = strtoul(tmp_rows_count_str, NULL, 10);
	*out_columns_count = strtoul(tmp_columns_count_str, NULL, 10);
	if (in_entity_def != NULL)
		tmp_column_count = in_entity_def->m_entity_columns_count;
	else
		tmp_column_count = (*out_columns_count);


	if (in_results_set != NULL && (*out_rows_count)  > 0 && (*out_columns_count) > 0)
	{
		for (tmp_row_index = 0; tmp_row_index < (*out_rows_count); tmp_row_index++)
		{
			if (in_callback != NULL)
				in_callback(tmp_row_index + in_start_row_index, 0, 0, "Chargement des enregistrements...");

			for (tmp_column_index = 0; tmp_column_index < (*out_columns_count); tmp_column_index++)
			{
				tmp_current_data_ptr = net_get_field(0, tmp_data_ptr, in_session->m_column_buffer, SEPARATOR_CHAR);
				if (tmp_column_index < tmp_column_count)
				{
					if (in_results_set[tmp_row_index + in_start_row_index][tmp_column_index] != NULL)
						free(in_results_set[tmp_row_index + in_start_row_index][tmp_column_index]);

					if (is_empty_string(in_session->m_column_buffer) == FALSE)
					{
						in_results_set[tmp_row_index + in_start_row_index][tmp_column_index] = (char*)malloc(tmp_current_data_ptr - tmp_data_ptr + 1);
						strcpy(in_results_set[tmp_row_index + in_start_row_index][tmp_column_index] , in_session->m_column_buffer);
					}
					else
						in_results_set[tmp_row_index + in_start_row_index][tmp_column_index] = NULL;
				}
				tmp_data_ptr = tmp_current_data_ptr;
			}
		}
	}

	return NOERR;
}



static int cl_parse_response_columns(net_session *in_session, unsigned int in_columns_count, const char *in_data_str, char** in_out_columns_record)
{
	const char							*tmp_data_ptr = in_data_str;
	const char							*tmp_current_data_ptr = NULL;

	char							tmp_rows_count_str[16];
	char							tmp_columns_count_str[16];

	unsigned long		tmp_column_index = 0;

	unsigned int			tmp_row_count = 0;
	unsigned int			tmp_column_count = 0;


	tmp_data_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX+1, tmp_data_ptr, tmp_rows_count_str, SEPARATOR_CHAR);
	tmp_data_ptr = net_get_field(0, tmp_data_ptr, tmp_columns_count_str, SEPARATOR_CHAR);

	tmp_row_count = atoi(tmp_rows_count_str);
	tmp_column_count = atoi(tmp_columns_count_str);

	if (tmp_row_count  == 1)
	{
		for (tmp_column_index = 0; tmp_column_index < tmp_column_count; tmp_column_index++)
		{
			tmp_current_data_ptr = net_get_field(0, tmp_data_ptr, in_session->m_column_buffer, SEPARATOR_CHAR);
			if (tmp_column_index < in_columns_count)
			{
				if (in_out_columns_record[tmp_column_index] != NULL)
					free(in_out_columns_record[tmp_column_index]);

				if (is_empty_string(in_session->m_column_buffer) == FALSE)
				{
					in_out_columns_record[tmp_column_index] = (char*)malloc(tmp_current_data_ptr - tmp_data_ptr + 1);
					strcpy(in_out_columns_record[tmp_column_index] , in_session->m_column_buffer);
				}
				else
					in_out_columns_record[tmp_column_index] = NULL;
			}
			tmp_data_ptr = tmp_current_data_ptr;
		}

		if (tmp_column_count != in_columns_count)
		{
			LOG_ERROR(in_session, "%s\n", get_error_message(ENTITY_COLUMNS_COUNT_ERROR));
		}
	}
	else if (tmp_row_count  > 1)
	{
		return DB_SQL_TOO_MUCH_ROWS_FOUND;
	}
	else
	{
		LOG_ERROR(in_session, "*************\n%s\n", tmp_data_ptr);
		LOG_ERROR(in_session, "row=%i, clo=%i\n", tmp_row_count, tmp_column_count);
		return DB_SQL_NO_ROW_FOUND;
	}

	return NOERR;
}


static char*** cl_load_results_set(net_session *in_session, entity_def *in_entity_def, unsigned long *in_out_records_count, unsigned long *in_out_columns_count, net_callback_fct *in_callback)
{
	int						tmp_status = NOERR;
	unsigned long long int	tmp_results_set_length = 0;
	unsigned long long int	tmp_requests_count = 0;
	unsigned long long int	tmp_record_index = 0;
	unsigned long long int	tmp_records_step_count = 0;
	char					***tmp_rows_columns = NULL;

	tmp_rows_columns = cl_create_results_set(in_session, in_entity_def, in_out_records_count, in_out_columns_count, &tmp_results_set_length);
	if (tmp_rows_columns != NULL)
	{
		if (in_callback != NULL)
			in_callback(0, *in_out_records_count, 0, "Loading records...");

		/* Calcul du nombre de requete a faire pour recuperer l'ensemble des resultats */
		tmp_requests_count = tmp_results_set_length / (MAX_RESPONSE_LENGTH / 2);
		if (tmp_requests_count == 0)	tmp_requests_count = 1;
		tmp_records_step_count = (*in_out_records_count) / tmp_requests_count;
		for (tmp_record_index = 0; tmp_status == NOERR && tmp_record_index < (*in_out_records_count) && tmp_records_step_count > 0; tmp_record_index+=tmp_records_step_count)
		{
			tmp_status = cl_get_last_results_set(in_session, in_entity_def, tmp_rows_columns, tmp_record_index, &tmp_records_step_count, in_callback);
		}

		if (in_callback != NULL)
			in_callback(*in_out_records_count, *in_out_records_count, 0, "Loading records...");

		if (tmp_status != NOERR)
		{
			if (in_entity_def != NULL)
				cl_free_rows_columns_array(&tmp_rows_columns, *in_out_records_count, in_entity_def->m_entity_columns_count);
			else
				cl_free_rows_columns_array(&tmp_rows_columns, *in_out_records_count, *in_out_columns_count);
		}
	}

	tmp_status = cl_clear_last_results_set(in_session);

	return tmp_rows_columns;
}



DLLEXPORT int DLLCALL cl_connect(net_session **in_session, const char* in_hostname, int in_port, const char* in_username, const char* in_passwd, int in_debug, const char* in_log_trace_file_name, int in_log_level, long in_protocol_version, long in_recv_timeout, long in_send_timeout )
{
	char* tmp_encrypted_pwd = NULL;

	if (in_session == NULL)	return EMPTY_OBJECT;

	if (*in_session != NULL)
	{
		*in_session = net_session_free(*in_session);
	}

	// Verifier la version du protocole
	if (in_protocol_version != PROTOCOL_VERSION)
	{
		return NET_LIB_PROTOCOL_VERSION_INCOMPATIBLE;
	}

	*in_session = net_session_alloc();
	if (*in_session == NULL)
	{
		return MEM_ALLOC_ERROR;
	}

	(*in_session)->m_debug = in_debug;
	(*in_session)->m_log_level = in_log_level;
	(*in_session)->m_recv_timeout = in_recv_timeout;
	(*in_session)->m_send_timeout = in_send_timeout;

	if (is_empty_string(in_log_trace_file_name) == FALSE)
		(*in_session)->m_log_trace_file = fopen(in_log_trace_file_name, "a");

	if (is_empty_string(in_hostname))
	{
		LOG_ERROR((*in_session), "cl_connect : hostname is empty\n");
		return EMPTY_OBJECT;
	}
	strcpy((*in_session)->m_hostname, in_hostname);
	(*in_session)->m_host_port = in_port;

	if (is_empty_string(in_username))
	{
		LOG_ERROR((*in_session), "cl_connect : username is empty\n");
		return EMPTY_OBJECT;
	}
	strcpy((*in_session)->m_username, in_username);

	if (is_empty_string(in_passwd))
	{
		LOG_ERROR((*in_session), "cl_connect : password is empty\n");
		return EMPTY_OBJECT;
	}
	tmp_encrypted_pwd = encrypt_str(in_passwd);
	strcpy((*in_session)->m_crypted_password, tmp_encrypted_pwd);
	free(tmp_encrypted_pwd);

	return cl_connect_netserver((*in_session));
}


DLLEXPORT int DLLCALL cl_disconnect(net_session **in_session)
{
	int			tmp_status = 0;

	if (in_session == NULL || *in_session == NULL)	return EMPTY_OBJECT;
	tmp_status = cl_send_simple_request(*in_session, END_SESSION, TRUE);

	cl_close_connexion(*in_session);
	*in_session = net_session_free(*in_session);

	return tmp_status;
}

DLLEXPORT int DLLCALL cl_ping_server (net_session *in_session)
{
	int			tmp_status = 0;

	if (in_session == NULL)	return EMPTY_OBJECT;

	tmp_status = cl_send_simple_request(in_session, IS_RUNNING, FALSE);

	return tmp_status;
}


DLLEXPORT char*** DLLCALL cl_run_sql(net_session *in_session, const char* in_statement, unsigned long *out_rows_count, unsigned long *out_columns_count )
{
	int			tmp_status = 0;
	int			tmp_nb_bytes = 0;

	if (in_session == NULL || out_rows_count == NULL || out_columns_count == NULL || is_empty_string(in_statement)) return NULL;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, RUN_SQL,
		SEPARATOR_CHAR, in_statement,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	tmp_status = cl_send_request(in_session, tmp_nb_bytes);
	if (tmp_status != NOERR)
	{
		if (tmp_status == NET_OUT_OF_RESPONSE_LIMIT_SIZE_ERROR)
			return cl_load_results_set(in_session, NULL, out_rows_count, out_columns_count, NULL);

		return NULL;
	}

	return cl_parse_response_rows(in_session, NULL, in_session->m_response, out_rows_count, out_columns_count);
}




DLLEXPORT int DLLCALL cl_call_sql_procedure( net_session *in_session, const char* in_procedure_name, char** in_out_parameters )
{
	int						tmp_param_count = 0;
	int						tmp_status = NOERR;
	char					*tmp_ptr = NULL;

	if (in_session == NULL || in_out_parameters == NULL || is_empty_string(in_procedure_name)) return EMPTY_OBJECT;

	tmp_param_count = sizeof(in_out_parameters) / sizeof(char*);

	tmp_ptr = in_session->m_request;
	tmp_ptr += sprintf(tmp_ptr, "%c%c%i%c%s%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, CALL_SQL_PROC,
		SEPARATOR_CHAR, in_procedure_name,
		SEPARATOR_CHAR);

	tmp_ptr = cl_array_strings_to_net_str(tmp_param_count, in_out_parameters, tmp_ptr);
	tmp_ptr += sprintf(tmp_ptr, "%c", PAQUET_STOP_CHAR);

	// Execution de la procedure
	//db_call_procedure(in_session, in_procedure_name, tmp_param_count, in_out_parameters);
	tmp_status = cl_send_request(in_session, tmp_ptr - in_session->m_request);
	if (tmp_status != NOERR)		return tmp_status;

	tmp_status = cl_parse_response_columns(in_session, tmp_param_count, in_session->m_response, in_out_parameters);

	return tmp_status;
}


DLLEXPORT int DLLCALL cl_lock_record( net_session *in_session, int in_table_signature, const char *in_primary_key_value, int in_check_sync, char** in_columns_record )
{
	entity_def			*tmp_entity_def = NULL;
	char					*tmp_ptr = NULL;

	if (in_session == NULL || is_empty_string(in_primary_key_value))		return EMPTY_OBJECT;

	/* Chercher le definition de l'entite */
	if (get_table_def(in_table_signature, &tmp_entity_def) < 0)
		return UNKNOW_ENTITY;

	tmp_ptr = in_session->m_request;

	tmp_ptr += sprintf(in_session->m_request, "%c%c%i%c%i%c%s%c%i%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, LOCK_RECORD,
		SEPARATOR_CHAR, in_table_signature,
		SEPARATOR_CHAR, in_primary_key_value,
		SEPARATOR_CHAR, in_check_sync,
		SEPARATOR_CHAR);

	tmp_ptr = cl_array_non_formated_strings_to_net_str(tmp_entity_def->m_entity_columns_count, in_columns_record, tmp_ptr);
	tmp_ptr += sprintf(tmp_ptr, "%c", PAQUET_STOP_CHAR);

	return cl_send_request(in_session, tmp_ptr - in_session->m_request);
}


DLLEXPORT int DLLCALL cl_unlock_record( net_session *in_session, int in_table_signature, const char *in_primary_key_value )
{
	int						tmp_nb_bytes = 0;
	entity_def			*tmp_entity_def = NULL;

	if (in_session == NULL || is_empty_string(in_primary_key_value))		return EMPTY_OBJECT;

	/* Chercher le definition de l'entite */
	if (get_table_def(in_table_signature, &tmp_entity_def) < 0)
		return UNKNOW_ENTITY;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%i%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, UNLOCK_RECORD,
		SEPARATOR_CHAR, in_table_signature,
		SEPARATOR_CHAR, in_primary_key_value,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	return cl_send_request(in_session, tmp_nb_bytes);
}


DLLEXPORT int DLLCALL cl_load_record( net_session *in_session, int in_table_signature, char **in_out_columns_record, const char *in_primary_key_value )
{
	int						tmp_status = NOERR;
	int						tmp_nb_bytes = 0;
	entity_def				*tmp_entity_def = NULL;

	if (in_session == NULL || in_out_columns_record == NULL || is_empty_string(in_primary_key_value))		return EMPTY_OBJECT;

	/* Chercher le definition de l'entite */
	if (get_table_def(in_table_signature, &tmp_entity_def) < 0)
		return UNKNOW_ENTITY;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%i%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, LOAD_RECORD,
		SEPARATOR_CHAR, in_table_signature,
		SEPARATOR_CHAR, in_primary_key_value,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	tmp_status = cl_send_request(in_session, tmp_nb_bytes);
	if (tmp_status != NOERR)		return tmp_status;

	tmp_status = cl_parse_response_columns(in_session, tmp_entity_def->m_entity_columns_count, in_session->m_response, in_out_columns_record);

	return tmp_status;
}


DLLEXPORT char*** DLLCALL cl_load_records( net_session *in_session, int in_table_signature, const char *in_where_clause, const char *in_order_by_clause, unsigned long *in_out_records_count, net_callback_fct *in_callback )
{
	int				tmp_status = 0;
	int				tmp_nb_bytes = 0;
	unsigned long 	tmp_columns_count = 0;
	entity_def		*tmp_entity_def = NULL;
	char			***tmp_rows_columns = NULL;

	if (in_session == NULL)	return NULL;

	/* Chercher le definition de l'entite */
	if (get_table_def(in_table_signature, &tmp_entity_def) < 0)
		return NULL;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%i%c%s%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, in_callback == NULL ? LOAD_RECORDS : PREPARE_RESULTS_SET,
		SEPARATOR_CHAR, in_table_signature,
		SEPARATOR_CHAR, is_empty_string(in_where_clause) ? "" : in_where_clause,
		SEPARATOR_CHAR, is_empty_string(in_order_by_clause) ? "" : in_order_by_clause,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	tmp_status = cl_send_request(in_session, tmp_nb_bytes);
	if ((tmp_status == NOERR && in_callback != NULL) || (tmp_status == NET_OUT_OF_RESPONSE_LIMIT_SIZE_ERROR && in_callback == NULL))
	{
		tmp_rows_columns = cl_load_results_set(in_session, tmp_entity_def, in_out_records_count, &tmp_columns_count, in_callback);
	}
	else if (tmp_status == NOERR)
		tmp_rows_columns = cl_parse_response_rows(in_session, tmp_entity_def, in_session->m_response, in_out_records_count, &tmp_columns_count);

	return tmp_rows_columns;
}


DLLEXPORT int DLLCALL cl_insert_record( net_session *in_session, int in_table_signature, char** in_out_columns_record )
{
	int						tmp_status = NOERR;
	entity_def				*tmp_entity_def = NULL;
	char					*tmp_ptr = NULL;

	if (in_session == NULL || in_out_columns_record == NULL)		return EMPTY_OBJECT;

	/* Chercher le definition de l'entite */
	if (get_table_def(in_table_signature, &tmp_entity_def) < 0)
		return UNKNOW_ENTITY;

	tmp_ptr = in_session->m_request;
	tmp_ptr += sprintf(tmp_ptr, "%c%c%i%c%i%c", PAQUET_START_CHAR, SEPARATOR_CHAR, INSERT_RECORD, SEPARATOR_CHAR, in_table_signature, SEPARATOR_CHAR);
	tmp_ptr = cl_array_strings_to_net_str(tmp_entity_def->m_entity_columns_count, in_out_columns_record, tmp_ptr);
	tmp_ptr += sprintf(tmp_ptr, "%c", PAQUET_STOP_CHAR);

	tmp_status = cl_send_request(in_session, tmp_ptr - in_session->m_request);
	if (tmp_status != NOERR)		return tmp_status;

	if (is_empty_string(tmp_entity_def->m_pk_seq_name) == FALSE)
		tmp_status = cl_parse_response_columns(in_session, tmp_entity_def->m_entity_columns_count, in_session->m_response, in_out_columns_record);

	return tmp_status;
}


DLLEXPORT int DLLCALL cl_save_record( net_session *in_session, int in_table_signature, char** in_out_columns_record, char** in_originals_columns_record )
{
	entity_def			*tmp_entity_def = NULL;
	char					*tmp_ptr = NULL;

	if (in_session == NULL || in_out_columns_record == NULL)	return EMPTY_OBJECT;

	/* Chercher le definition de l'entite */
	if (get_table_def(in_table_signature, &tmp_entity_def) < 0)
		return UNKNOW_ENTITY;

	tmp_ptr = in_session->m_request;
	tmp_ptr += sprintf(tmp_ptr, "%c%c%i%c%i%c", PAQUET_START_CHAR, SEPARATOR_CHAR, SAVE_RECORD, SEPARATOR_CHAR, in_table_signature, SEPARATOR_CHAR);
	tmp_ptr = cl_arrays_strings_to_net_str(tmp_entity_def, in_out_columns_record, in_originals_columns_record, tmp_ptr);
	// Aucun changement
	if (tmp_ptr == NULL)
		return NOERR;

	tmp_ptr += sprintf(tmp_ptr, "%c", PAQUET_STOP_CHAR);

	return cl_send_request(in_session, tmp_ptr - in_session->m_request);
}


DLLEXPORT int DLLCALL cl_delete_record( net_session *in_session, int in_table_signature, char** in_columns_record )
{
	entity_def			*tmp_entity_def = NULL;
	char					*tmp_ptr = NULL;

	if (in_session == NULL || in_columns_record == NULL)	return EMPTY_OBJECT;

	/* Chercher le definition de l'entite */
	if (get_table_def(in_table_signature, &tmp_entity_def) < 0)
		return UNKNOW_ENTITY;

	tmp_ptr = in_session->m_request;
	tmp_ptr += sprintf(in_session->m_request, "%c%c%i%c%i%c", PAQUET_START_CHAR, SEPARATOR_CHAR, DELETE_RECORD, SEPARATOR_CHAR, in_table_signature, SEPARATOR_CHAR);
	tmp_ptr = cl_array_strings_to_net_str(tmp_entity_def->m_entity_columns_count, in_columns_record, tmp_ptr);
	tmp_ptr += sprintf(tmp_ptr, "%c", PAQUET_STOP_CHAR);

	return cl_send_request(in_session, tmp_ptr - in_session->m_request);
}


DLLEXPORT int DLLCALL cl_delete_records( net_session *in_session, int in_table_signature, const char* in_where_clause )
{
	int					tmp_nb_bytes = 0;
	entity_def			*tmp_entity_def = NULL;

	if (in_session == NULL || is_empty_string(in_where_clause))	return EMPTY_OBJECT;

	/* Chercher le definition de l'entite */
	if (get_table_def(in_table_signature, &tmp_entity_def) < 0)
		return UNKNOW_ENTITY;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%i%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, DELETE_RECORDS,
		SEPARATOR_CHAR, in_table_signature,
		SEPARATOR_CHAR, in_where_clause,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	return cl_send_request(in_session, tmp_nb_bytes);
}


#ifdef _LDAP
DLLEXPORT char*** DLLCALL cl_load_ldap_entries( int in_entry_signature, const char* in_ldap_url, const char* in_distinguish_name, const char* in_credencial, const char* in_base_query, const char* in_filter_query )
{
	return NULL;
}

DLLEXPORT int DLLCALL cl_load_ldap_entry( int in_entry_signature, const char* in_ldap_url, const char* in_distinguish_name, const char* in_credencial, char** in_out_columns_record, const char* in_base_query )
{
	return NOERR;
}

#endif

DLLEXPORT const char* DLLCALL cl_get_error_message( net_session *in_session, int in_error_id )
{
	char		*tmp_error_msg = NULL;
	int			tmp_nb_bytes = 0;
	int			tmp_status = 0;


	switch (in_error_id)
	{
	case DB_SQL_ERROR:
	case OUT_OF_COLUMN_ENTITY_SIZE_LIMIT:
		if (in_session == NULL)	return NULL;

		tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%i%c%c", PAQUET_START_CHAR,
			SEPARATOR_CHAR, GET_ERROR_MESSAGE,
			SEPARATOR_CHAR, in_error_id,
			SEPARATOR_CHAR, PAQUET_STOP_CHAR);

		tmp_status = cl_send_request(in_session, tmp_nb_bytes);
		if (tmp_status != NOERR)		return NULL;

		net_get_field(NET_MESSAGE_TYPE_INDEX+1, in_session->m_response, in_session->m_last_error_msg, SEPARATOR_CHAR);
		return in_session->m_last_error_msg;

		break;

	default:
		tmp_error_msg = (char*)get_error_message(in_error_id);
		if (tmp_error_msg == NULL && in_session != NULL)
		{
			sprintf(in_session->m_last_error_msg, "L'erreur %i est survenue.", in_error_id);
			return in_session->m_last_error_msg;
		}
	}

	return tmp_error_msg;
}


DLLEXPORT int DLLCALL cl_add_user(net_session *in_session, const char *in_username, const char *in_password, const char *in_rolename)
{
	int			tmp_nb_bytes = 0;

	if (in_session == NULL || is_empty_string(in_username) || is_empty_string(in_password))	return EMPTY_OBJECT;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%s%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, ADD_USER,
		SEPARATOR_CHAR, in_username,
		SEPARATOR_CHAR, encrypt_str(in_password),
		SEPARATOR_CHAR, in_rolename != NULL ? in_rolename : "",
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	return cl_send_request(in_session, tmp_nb_bytes);
}


DLLEXPORT int DLLCALL cl_remove_user(net_session *in_session, const char *in_username)
{
	int			tmp_nb_bytes = 0;

	if (in_session == NULL || is_empty_string(in_username))	return EMPTY_OBJECT;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, REMOVE_USER,
		SEPARATOR_CHAR, in_username,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	return cl_send_request(in_session, tmp_nb_bytes);
}


DLLEXPORT int DLLCALL cl_modify_user_password(net_session *in_session, const char *in_username, const char *in_new_password)
{
	int			tmp_nb_bytes = 0;

	if (in_session == NULL || is_empty_string(in_username) || is_empty_string(in_new_password))	return EMPTY_OBJECT;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, MODIFY_USER_PASSWORD,
		SEPARATOR_CHAR, in_username,
		SEPARATOR_CHAR, in_new_password,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	return cl_send_request(in_session, tmp_nb_bytes);
}


DLLEXPORT int DLLCALL cl_add_role_to_user(net_session *in_session, const char *in_rolename, const char *in_username)
{
	int			tmp_nb_bytes = 0;

	if (in_session == NULL || is_empty_string(in_username) || is_empty_string(in_rolename))	return EMPTY_OBJECT;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, ADD_ROLE_TO_USER,
		SEPARATOR_CHAR, in_username,
		SEPARATOR_CHAR, in_rolename,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	return cl_send_request(in_session, tmp_nb_bytes);
}


DLLEXPORT int DLLCALL cl_remove_role_from_user(net_session *in_session, const char *in_rolename, const char *in_username)
{
	int			tmp_nb_bytes = 0;

	if (in_session == NULL || is_empty_string(in_username) || is_empty_string(in_rolename))	return EMPTY_OBJECT;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, REMOVE_ROLE_FROM_USER,
		SEPARATOR_CHAR, in_username,
		SEPARATOR_CHAR, in_rolename,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	return cl_send_request(in_session, tmp_nb_bytes);
}


DLLEXPORT int DLLCALL cl_has_user_this_role(net_session *in_session, const char *in_username, const char *in_rolename)
{
	int			tmp_status = 0;
	int			tmp_nb_bytes = 0;
	char		tmp_return_result[16];

	if (in_session == NULL || is_empty_string(in_username) || is_empty_string(in_rolename))	return EMPTY_OBJECT;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, HAS_USER_THIS_ROLE,
		SEPARATOR_CHAR, in_username,
		SEPARATOR_CHAR, in_rolename,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	tmp_status = cl_send_request(in_session, tmp_nb_bytes);
	if (tmp_status != NOERR)		return tmp_status;

	net_get_field(NET_MESSAGE_TYPE_INDEX+1, in_session->m_response, tmp_return_result, SEPARATOR_CHAR);
	if (is_empty_string(tmp_return_result) == FALSE)
		return atoi(tmp_return_result);

	return FALSE;
}


DLLEXPORT char* DLLCALL cl_current_user(net_session *in_session)
{
	return in_session->m_username;
}


DLLEXPORT long DLLCALL cl_api_protocol_version()
{
	return PROTOCOL_VERSION;
}


/* Binary Large OBjects */
DLLEXPORT int DLLCALL cl_put_blob_from_buffer(net_session *in_session, const char *in_buffer, long in_buffer_size, char *out_file_id_str)
{
	int						tmp_status = NOERR;
	int						tmp_nb_bytes = 0;
	long					tmp_total_bytes_send = 0;
	int						tmp_send_bytes = 0;
	const char				*tmp_ptr = in_buffer;

	if (in_session == NULL || in_buffer == NULL || out_file_id_str == NULL)	return EMPTY_OBJECT;

	if (in_buffer_size > 0)
	{
		tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%ld%c%c", PAQUET_START_CHAR,
			SEPARATOR_CHAR, PUT_BLOB,
			SEPARATOR_CHAR, "test",
			SEPARATOR_CHAR, in_buffer_size,
			SEPARATOR_CHAR, PAQUET_STOP_CHAR);

		tmp_status = cl_send_request(in_session, tmp_nb_bytes);
		if (tmp_status == NOERR)
		{
			net_get_field(NET_MESSAGE_TYPE_INDEX+1, in_session->m_response, out_file_id_str, SEPARATOR_CHAR);
			// Envoyer le contenu du buffer
			while (tmp_total_bytes_send < in_buffer_size)
			{
				tmp_send_bytes = send(in_session->m_socket, tmp_ptr, MIN(in_buffer_size - tmp_total_bytes_send, MAX_PAQUET_LENGTH - 1), 0);
				if (tmp_send_bytes > 0)
				{
					tmp_total_bytes_send += tmp_send_bytes;
					tmp_ptr += tmp_send_bytes;
				}
				else
				{
					#if (defined(_WINDOWS) || defined(WIN32))
							LOG_ERROR(in_session, "cl_put_blob_from_buffer : error %i sending bytes\n", WSAGetLastError());
					#else
							LOG_ERROR(in_session, "cl_put_blob_from_buffer: error %i sending bytes\n", errno);
					#endif

					tmp_status = NET_SEND_PAQUET_ERROR;
					break;
				}
			}

			if (tmp_status == NOERR)
				tmp_status = net_rcv_packet(in_session->m_socket, in_session->m_response);
		}
	}
	else
		tmp_status = EMPTY_OBJECT;

	return tmp_status;
}



DLLEXPORT int DLLCALL cl_get_blob_to_buffer(net_session *in_session, const char *in_blob_id, char **in_buffer, long *in_buffer_size)
{
	int						tmp_status = NOERR;
	int						tmp_nb_bytes = 0;
	long					tmp_buffer_size = 0;
	char					tmp_buffer_size_str[16];
	long					tmp_total_bytes_recv = 0;
	int						tmp_nb_bytes_recv = 0;
	char					*tmp_ptr = NULL;

	if (in_session == NULL || is_empty_string(in_blob_id) || in_buffer == NULL)	return EMPTY_OBJECT;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, GET_BLOB,
		SEPARATOR_CHAR, in_blob_id,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	tmp_status = cl_send_request(in_session, tmp_nb_bytes);
	if (tmp_status == NOERR)
	{
		net_get_field(NET_MESSAGE_TYPE_INDEX+1, in_session->m_response, tmp_buffer_size_str, SEPARATOR_CHAR);
		if (is_empty_string(tmp_buffer_size_str) == FALSE)
			tmp_buffer_size = atol(tmp_buffer_size_str);

		LOG_TRACE(in_session, "cl_get_blob_to_buffer: downloading %li bytes\n", tmp_buffer_size);

		if (tmp_buffer_size > 0)
		{
			*in_buffer = cl_alloc_str(tmp_buffer_size);
			if (NULL != *in_buffer)
			{
				tmp_ptr = *in_buffer;

				tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_request, NOERR);
				if (tmp_status == NOERR)
				{
					// Receptionner le contenu du buffer
					while (tmp_total_bytes_recv < tmp_buffer_size)
					{
						tmp_nb_bytes_recv = recv(in_session->m_socket, tmp_ptr, MAX_PAQUET_LENGTH - 1, 0);
						if (tmp_nb_bytes_recv > 0)
						{
							tmp_total_bytes_recv += tmp_nb_bytes_recv;
							tmp_ptr += tmp_nb_bytes_recv;
						}
						else
						{
							#if (defined(_WINDOWS) || defined(WIN32))
									LOG_ERROR(in_session, "cl_get_blob_to_buffer : error %i receiving bytes\n", WSAGetLastError());
							#else
									LOG_ERROR(in_session, "cl_get_blob_to_buffer: error %i receiving bytes\n", errno);
							#endif

							tmp_status = NET_RCV_PAQUET_ERROR;
							break;
						}
					}
					LOG_TRACE(in_session, "cl_get_blob_to_buffer: recept %li bytes\n", tmp_total_bytes_recv);
				}

				if (NULL != in_buffer_size)
					*in_buffer_size = tmp_buffer_size;

			}
			else
			{
				tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_request, MEM_ALLOC_ERROR);
				if (tmp_status == NOERR)
					tmp_status = net_rcv_packet(in_session->m_socket, in_session->m_response);
			}
		}
	}

	return tmp_status;
}



DLLEXPORT int DLLCALL cl_put_blob(net_session *in_session, const char *in_filename, char *out_file_id_str)
{
	int						tmp_status = NOERR;
	int						tmp_nb_bytes = 0;
	int						tmp_file = 0;
	long					tmp_file_size = 0;
	long					tmp_total_bytes_send = 0;
	int						tmp_read_bytes = 0;

	char					*tmp_basename_ptr = (char*)in_filename;

#if (!defined(_WINDOWS) && !defined(WIN32))
	struct stat		tmp_file_stat;
#endif

	if (in_session == NULL || is_empty_string(in_filename) || out_file_id_str == NULL)	return EMPTY_OBJECT;

#if (defined(_WINDOWS) || defined(WIN32))
	tmp_file = _open(in_filename,  _O_RDONLY | _O_BINARY );
	if (tmp_file == -1)		return FILE_OPENING_ERROR;

	tmp_file_size = _filelength(tmp_file);
#else
	// Recuperer les informations du fichier source
	if (stat(in_filename, &tmp_file_stat) == -1)	return FILE_OPENING_ERROR;

	tmp_file = open(in_filename, O_RDONLY);
	if (tmp_file == -1)		return FILE_OPENING_ERROR;

	tmp_file_size = tmp_file_stat.st_size;
#endif

	if (tmp_file_size > 0)
	{
		tmp_basename_ptr = strrchr(in_filename, '/');
		tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%ld%c%c", PAQUET_START_CHAR,
			SEPARATOR_CHAR, PUT_BLOB,
			SEPARATOR_CHAR, tmp_basename_ptr ? tmp_basename_ptr + 1 : in_filename,
			SEPARATOR_CHAR, tmp_file_size,
			SEPARATOR_CHAR, PAQUET_STOP_CHAR);

		tmp_status = cl_send_request(in_session, tmp_nb_bytes);
		if (tmp_status == NOERR)
		{
			net_get_field(NET_MESSAGE_TYPE_INDEX+1, in_session->m_response, out_file_id_str, SEPARATOR_CHAR);
			// Envoyer le contenu du fichier
			while (tmp_total_bytes_send < tmp_file_size)
			{
#if (defined(_WINDOWS) || defined(WIN32))
				tmp_read_bytes = _read(tmp_file, in_session->m_request, MAX_PAQUET_LENGTH - 1);
#else
				tmp_read_bytes = read(tmp_file, in_session->m_request, MAX_PAQUET_LENGTH - 1);
#endif

				if (tmp_read_bytes > 0)
				{
					tmp_read_bytes = send(in_session->m_socket, in_session->m_request, tmp_read_bytes, 0);
					if (tmp_read_bytes > 0)
					{
						tmp_total_bytes_send += tmp_read_bytes;
					}
					else
					{
						#if (defined(_WINDOWS) || defined(WIN32))
								LOG_ERROR(in_session, "cl_put_blob : Erreur d'envoie du fichier %i\n", WSAGetLastError());
						#else
								LOG_ERROR(in_session, "cl_put_blob : Erreur d'envoie du fichier %i\n", errno);
						#endif

						tmp_status = NET_SEND_PAQUET_ERROR;
						break;
					}
				}
				else
				{
					tmp_status = FILE_READING_ERROR;
					break;
				}
			}

			if (tmp_status == NOERR)
				tmp_status = net_rcv_packet(in_session->m_socket, in_session->m_response);
		}
	}
	else
		tmp_status = FILE_EMPTY_ERROR;

#if (defined(_WINDOWS) || defined(WIN32))
	_close(tmp_file);
#else
	close(tmp_file);
#endif

	return tmp_status;
}


DLLEXPORT int DLLCALL cl_get_blob(net_session *in_session, const char *in_blob_id, const char *in_filename)
{
	int						tmp_status = NOERR;
	int						tmp_nb_bytes = 0;
	int						tmp_file = 0;
	long					tmp_file_size = 0;
	char					tmp_file_size_str[16];
	long					tmp_total_bytes_recv = 0;
	int						tmp_nb_bytes_recv = 0;

	if (in_session == NULL || is_empty_string(in_blob_id) || is_empty_string(in_filename))	return EMPTY_OBJECT;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, GET_BLOB,
		SEPARATOR_CHAR, in_blob_id,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	tmp_status = cl_send_request(in_session, tmp_nb_bytes);
	if (tmp_status == NOERR)
	{
		net_get_field(NET_MESSAGE_TYPE_INDEX+1, in_session->m_response, tmp_file_size_str, SEPARATOR_CHAR);
		if (is_empty_string(tmp_file_size_str) == FALSE)
			tmp_file_size = atol(tmp_file_size_str);

		LOG_TRACE(in_session, "cl_get_blob: downloading %li bytes\n", tmp_file_size);

		if (tmp_file_size > 0)
		{
#if (defined(_WINDOWS) || defined(WIN32))
			tmp_file = _open(in_filename,  _O_CREAT | _O_TRUNC | _O_WRONLY | _O_BINARY);
#else
			tmp_file = open(in_filename, O_CREAT | O_TRUNC | O_WRONLY, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
#endif
			if (tmp_file != -1)
			{
				tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_request, NOERR);
				if (tmp_status == NOERR)
				{
					// Receptionner le contenu du fichier
					while (tmp_total_bytes_recv < tmp_file_size)
					{
						tmp_nb_bytes_recv = recv(in_session->m_socket, in_session->m_response, MAX_PAQUET_LENGTH - 1, 0);
						if (tmp_nb_bytes_recv > 0)
						{
#if (defined(_WINDOWS) || defined(WIN32))
							tmp_nb_bytes_recv = _write(tmp_file, in_session->m_response, tmp_nb_bytes_recv);
#else
							tmp_nb_bytes_recv = write(tmp_file, in_session->m_response, tmp_nb_bytes_recv);
#endif
							if (tmp_nb_bytes_recv > 0)
							{
								tmp_total_bytes_recv += tmp_nb_bytes_recv;
							}
							else
							{
								tmp_status = FILE_WRITING_ERROR;
								break;
							}
						}
						else
						{
							#if (defined(_WINDOWS) || defined(WIN32))
									LOG_ERROR(in_session, "cl_get_blob : error %i receiving bytes\n", WSAGetLastError());
							#else
									LOG_ERROR(in_session, "cl_get_blob: error %i receiving bytes\n", errno);
							#endif

							tmp_status = NET_RCV_PAQUET_ERROR;
							break;
						}
					}
				}

#if (defined(_WINDOWS) || defined(WIN32))
				_close(tmp_file);
#else
				close(tmp_file);
#endif
			}
			else
			{
				tmp_status = net_send_simple_packet(in_session->m_socket, in_session->m_request, FILE_OPENING_ERROR);
				if (tmp_status == NOERR)
					tmp_status = net_rcv_packet(in_session->m_socket, in_session->m_response);
			}
		}
	}

	return tmp_status;
}


DLLEXPORT int DLLCALL cl_delete_blob(net_session *in_session, const char *in_blob_id)
{
	int			tmp_nb_bytes = 0;

	if (in_session == NULL || is_empty_string(in_blob_id))	return EMPTY_OBJECT;

	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%s%c%c", PAQUET_START_CHAR,
		SEPARATOR_CHAR, DELETE_BLOB,
		SEPARATOR_CHAR, in_blob_id,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	return cl_send_request(in_session, tmp_nb_bytes);
}


DLLEXPORT void DLLCALL cl_free_rows_columns_array(char ****in_rows_columns, int in_rows_count, int in_columns_count)
{
	free_rows_and_columns(in_rows_columns, in_rows_count, in_columns_count);
}

DLLEXPORT char** DLLCALL cl_alloc_columns_array(int in_columns_count)
{
	char **tmp_columns_array = (char**)malloc(sizeof(char*) * in_columns_count);
	memset(tmp_columns_array, 0, sizeof(char*) * in_columns_count);

	return tmp_columns_array;
}

DLLEXPORT void DLLCALL cl_free_columns_array(char ***in_columns_array_ptr, int in_columns_count)
{
	int tmp_column_index = 0;

	if (in_columns_array_ptr != NULL && ((*in_columns_array_ptr) != NULL))
	{
		for (tmp_column_index = 0; tmp_column_index < in_columns_count; tmp_column_index++)
		{
			if (((*in_columns_array_ptr)[tmp_column_index]) != NULL)
				free((*in_columns_array_ptr)[tmp_column_index]);
		}

		free(*in_columns_array_ptr);
		(*in_columns_array_ptr) = NULL;
	}
}


DLLEXPORT char* DLLCALL cl_alloc_str(int in_str_length)
{
	char *tmp_str = (char*)malloc(sizeof(char) * in_str_length + 1);

	return tmp_str;
}

DLLEXPORT void DLLCALL cl_free_str(char **in_str_ptr)
{
	if (in_str_ptr != NULL && ((*in_str_ptr) != NULL))
	{
		free(*in_str_ptr);
		(*in_str_ptr) = NULL;
	}
}


DLLEXPORT int DLLCALL cl_transaction_start( net_session *in_session )
{
	int			tmp_status = 0;

	if (in_session == NULL)	return EMPTY_OBJECT;

	tmp_status = cl_send_simple_request(in_session, START_TRANSACTION, TRUE);

	return tmp_status;
}

DLLEXPORT int DLLCALL cl_transaction_commit( net_session *in_session )
{
	int			tmp_status = 0;

	if (in_session == NULL)	return EMPTY_OBJECT;

	tmp_status = cl_send_simple_request(in_session, COMMIT_TRANSACTION, TRUE);

	return tmp_status;
}


DLLEXPORT int DLLCALL cl_transaction_rollback( net_session *in_session )
{
	int			tmp_status = 0;

	if (in_session == NULL)	return EMPTY_OBJECT;

	tmp_status = cl_send_simple_request(in_session, ROLLBACK_TRANSACTION, TRUE);

	return tmp_status;
}


DLLEXPORT int DLLCALL cl_get_server_infos( net_session *in_session )
{
	int			tmp_status = 0;

	if (in_session == NULL)	return EMPTY_OBJECT;

	tmp_status = cl_send_simple_request(in_session, SERVER_INFOS, TRUE);

	return tmp_status;
}


DLLEXPORT int DLLCALL cl_get_last_results_set(net_session *in_session, entity_def *in_entity_def, char ***in_last_results_set, unsigned long long int in_start_row_index, unsigned long long int *in_rows_count, net_callback_fct *in_callback )
{
	int				tmp_status = 0;
	int				tmp_nb_bytes = 0;

	unsigned long long int tmp_columns_count = 0;

	if (in_session == NULL || in_last_results_set == NULL) return EMPTY_OBJECT;

#if ((defined(_WINDOWS) || defined(WIN32)))
	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%I64u%c%I64u%c%c", PAQUET_START_CHAR,
#else
	tmp_nb_bytes = sprintf(in_session->m_request, "%c%c%i%c%llu%c%llu%c%c", PAQUET_START_CHAR,
#endif
		SEPARATOR_CHAR, GET_LAST_RESULTS_SET,
		SEPARATOR_CHAR, in_start_row_index,
		SEPARATOR_CHAR, *in_rows_count,
		SEPARATOR_CHAR, PAQUET_STOP_CHAR);

	tmp_status = cl_send_request(in_session, tmp_nb_bytes);
	if (tmp_status != NOERR)
	{
		return tmp_status;
	}

	return cl_parse_response_rows_for_results_set(in_session, in_entity_def, in_session->m_response, in_last_results_set, in_start_row_index, in_rows_count, &tmp_columns_count, in_callback);
}


DLLEXPORT int DLLCALL cl_clear_last_results_set(net_session *in_session)
{
	int			tmp_status = 0;

	if (in_session == NULL)	return EMPTY_OBJECT;

	tmp_status = cl_send_simple_request(in_session, CLEAR_LAST_RESULTS_SET, TRUE);

	return tmp_status;
}

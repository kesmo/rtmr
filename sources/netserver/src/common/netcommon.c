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

#include "netcommon.h"
#include "constants.h"

#include <math.h>
#include <errno.h>
#if defined (__APPLE__)
	#include <malloc/malloc.h>
#else
		#include <malloc.h>
#endif

#include <string.h>
#if !defined(_WINDOWS) && !defined(WIN32)
#include <unistd.h>
#endif
#include <stdlib.h>

#include <fcntl.h>

	/* Definition des entetes des structures utilisees */
const char* str_cmds_array[] = {
	"connect",
	"aquit",
	"paquet error",
	"request error",
	"netstop",
	"disconnect",
	"ping",
	"ready",
	"response end",
	"cd",
	"",
	"get",
	"put",
	"",
	"ls",
	"!cd",
	"!ls",
	"",
	"getd",
	"putd",
	NULL
};

const char	*key = "0dc43e4ae9f86c17c4744ac5d94ebe8fcb8b6341916915155be5db4d2e991e203a5510a0c4c9e4be40038b9ae1c88ffda6b04bf5ba6c74d1fc9f307d2d1f97fd682b5e46fdbaed95c526bb245cf1f25a5c39a044b7a732deebcc24153be35415843d41036c4555bc6061be4c46f94f3aa5a2bc374ecd75ab46cb2ee22ecafe28";

net_session* net_session_alloc()
{
	net_session	*tmp_session = (net_session*)malloc(sizeof(net_session));
	if (tmp_session != NULL)
	{
		memset(tmp_session, 0, sizeof(net_session));

		tmp_session->m_request = (char*)malloc(MAX_REQUEST_LENGTH);
		if (tmp_session->m_request == NULL)
		{
			tmp_session = net_session_free(tmp_session);
		}
		else
		{
			tmp_session->m_response = (char*)malloc(MAX_RESPONSE_LENGTH);
			if (tmp_session->m_response == NULL)
			{
				tmp_session = net_session_free(tmp_session);
			}
			else
			{
				tmp_session->m_last_query = (char*)malloc(MAX_SQL_STATEMENT_LENGTH);
				if (tmp_session->m_last_query == NULL)
				{
					tmp_session = net_session_free(tmp_session);
				}
				else
				{

				}
			}
		}
	}

	return tmp_session;
}


net_session* net_session_free(net_session *in_session)
{
	if (in_session)
	{
		if (in_session->m_request)
		{
			free(in_session->m_request);
			in_session->m_request = NULL;
		}

		if (in_session->m_response)
		{
			free(in_session->m_response);
			in_session->m_response = NULL;
		}

		if (in_session->m_last_query)
		{
			free(in_session->m_last_query);
			in_session->m_last_query = NULL;
		}

		if (in_session->m_db_connexion != NULL)
		{
			free(in_session->m_db_connexion);
			in_session->m_db_connexion = NULL;
		}

		if (in_session->m_log_trace_file)
		{
			fclose(in_session->m_log_trace_file);
			in_session->m_log_trace_file = NULL;
		}

		free(in_session);
	}



	return NULL;
}

/*************************************************************************************
*	net_init_cnx
*			Cree un point d'entree (socket) pour l'hote courant
*		Parametres :
*			sock (IN/OUT) : numero de socket
*			host_port (IN): numero du port d'ecoute
*			host_adress (IN/OUT): structure constituant l'adresse de l'hote
*		Retour :
*			NOERR si pas d'erreur CONNEXION_ERROR sinon
**************************************************************************************/
int net_init_cnx (int *sock, short host_port, struct sockaddr_in *host_adress)
{
	int		status;

	char	host_name[255];
	struct	hostent	*host_structure = NULL;

#if defined(_WINDOWS) || defined(WIN32)
	WORD	wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(1, 1);
	status = WSAStartup(wVersionRequested, &wsaData);
	if (status != 0)
		return NET_SOCKET_ERROR;

	if ( LOBYTE( wsaData.wVersion ) != 1 ||
			HIBYTE( wsaData.wVersion ) != 1 ) {
		WSACleanup();
		return NET_SOCKET_ERROR;
	}
#endif

	memset(host_name, 0, sizeof(host_name));
	memset(host_adress, 0, sizeof(struct sockaddr_in));

	/* creation de la socket */
	*sock = socket(PF_INET, SOCK_STREAM, 0);
	if (*sock == -1){
		return NET_SOCKET_ERROR;
	}

	/* Recuperation de l'adresse reseau de l'hote */
	status = gethostname(host_name, 255);
	if (status == -1){
		return NET_GET_HOST_NAME_ERROR;
	}

	/* Contruction de l'adresse reseau de l'hote */
	host_structure = gethostbyname(host_name);
	if (host_structure == NULL){
		host_adress->sin_family = AF_INET;
	}
	else {
		host_adress->sin_family = host_structure->h_addrtype;
	}

	/* Initialisation des champs de host_adress */
	host_adress->sin_port = htons(host_port);
	host_adress->sin_addr.s_addr = htonl(INADDR_ANY);
	//host_adress->sin_addr = *(struct in_addr *)host_structure->h_addr_list[0];
	//printf("*** Adresse IP (decimal) : %s ***\n", inet_ntoa(host_adress->sin_addr));

	return NOERR;
}


int net_send_simple_packet(int sock, char* data_ptr, int in_packet_type)
{
	int	tmp_nb_bytes = sprintf(data_ptr, "%c%c%i%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, in_packet_type, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
	return net_send_packet(sock, data_ptr, tmp_nb_bytes);
}


int net_send_simple_packet_with_message(int sock, char* data_ptr, int in_packet_type, const char *message)
{
	int	tmp_nb_bytes = sprintf(data_ptr, "%c%c%i%c%s%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, in_packet_type, SEPARATOR_CHAR, message, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
	return net_send_packet(sock, data_ptr, tmp_nb_bytes);
}


/*************************************************************************************
*	net_send_paquet
*			Envoie d'un paquet
*		Parametres :
*			sock (IN) : numero de sock ou envoyer les donnees
*			data_ptr (IN) : chaine a transmettre
*		Retour :
*			NOERR si pas d'erreur CONNEXION_ERROR sinon
**************************************************************************************/
int net_send_packet(int sock, char* data_ptr, int in_buffer_size)
{
	int			tmp_nb_bytes_send = 0;
	int			tmp_total_bytes_send = 0;

	char		*tmp_data_ptr = data_ptr;

#if defined SECURE
	int		i;
#endif

#if defined SECURE
	for (i = 0; i < in_buffer_size; i++)
		data_ptr[i] = data_ptr[i] ^ key[i%256];
#endif

	while (tmp_total_bytes_send < in_buffer_size)
	{
		tmp_nb_bytes_send = send(sock, tmp_data_ptr, MIN(MAX_PAQUET_LENGTH - 1, in_buffer_size - tmp_total_bytes_send), 0);
#if defined(__WINDOWS) ||  defined(WIN32)
		if (tmp_nb_bytes_send < 0)
		{
            if (WSAGetLastError() == WSAEWOULDBLOCK)
				return NET_SEND_TIMEOUT;
			else
				return NET_SEND_PAQUET_ERROR;
		}
		else if (tmp_nb_bytes_send == 0)
		{
			return NET_CONNEXION_CLOSED;
		}
#else
		if (tmp_nb_bytes_send <= 0)
		  {
                  if (errno == EAGAIN)
                            return NET_SEND_TIMEOUT;
                    else
                            return NET_SEND_PAQUET_ERROR;
		  }
#endif
		tmp_total_bytes_send += tmp_nb_bytes_send;

		if (tmp_total_bytes_send < in_buffer_size)
		{
			tmp_data_ptr += tmp_nb_bytes_send;
		}
	}

	return NOERR;
}

/*************************************************************************************
*	net_rcv_paquet
*			Reception d'un paquet
*		Parametres :
*			sock (IN) : numero de sock ou envoyer les donnees
*			data_ptr (IN) : chaine recue
*		Retour :
*			NOERR si pas d'erreur CONNEXION_ERROR sinon
**************************************************************************************/
int	net_rcv_packet(int sock, char* data_ptr)
{
	char		*tmp_data_ptr = data_ptr;

	int			tmp_nb_bytes_recv = 0;

	int			tmp_end_reach = FALSE;

#if defined CRYPTING
	int		i;
#endif

    do
	{
		tmp_nb_bytes_recv = recv(sock, tmp_data_ptr, MAX_PAQUET_LENGTH - 1, 0);
		if (tmp_nb_bytes_recv <= 0)
		{
#if defined(__WINDOWS) ||  defined(WIN32)
            		if (WSAGetLastError() == WSAEWOULDBLOCK)
				return NET_RECV_TIMEOUT;
			else if (WSAGetLastError() == WSAECONNRESET)
				return NET_CONNEXION_CLOSED;
			else
				return NET_RCV_PAQUET_ERROR;
#else
			if (tmp_nb_bytes_recv == 0)
				return NET_CONNEXION_CLOSED;
	            	else if (errno == EAGAIN)
				return NET_RECV_TIMEOUT;
			else
				return NET_RCV_PAQUET_ERROR;
#endif
		}
		else
		{
			tmp_data_ptr += tmp_nb_bytes_recv;

			tmp_end_reach = ((unsigned char)(tmp_data_ptr - 1)[0] == PAQUET_STOP_CHAR);
		}
	}
	while(!tmp_end_reach);

#if defined CRYPTING
	for (i = 0; i < tmp_rcv_status; i++)
		data_ptr[i] = data_ptr[i] ^ key[i%256];
#endif

	tmp_data_ptr[0] = '\0';

	return NOERR;
}


int	net_get_packet_type(const char* data_ptr)
{
	char		tmp_response_type_str[16];
	int			tmp_response_type = NET_REQUEST_UNKNOW;

	if (is_empty_string(data_ptr) == FALSE)
	{
		net_get_field(NET_MESSAGE_TYPE_INDEX, data_ptr, tmp_response_type_str, SEPARATOR_CHAR);
		if (is_empty_string(tmp_response_type_str) == FALSE)
		{
			tmp_response_type = atoi(tmp_response_type_str);
		}
	}

	return tmp_response_type;
}


const char	*net_get_command_desc(int in_cmd_id)
{
	int		tmp_commands_count = sizeof(net_commands_list) / sizeof(net_command);
	int		tmp_command_index = 0;

	for (tmp_command_index = 0; tmp_command_index < tmp_commands_count; ++tmp_command_index)
	{
		if ((int)net_commands_list[tmp_command_index].id == in_cmd_id)
		{
			return net_commands_list[tmp_command_index].description;
		}
	}

	return NULL;
}

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

#ifndef NETCOMMON_H_
#define NETCOMMON_H_

#if (defined(_WINDOWS) || defined(WIN32))
	#include <Windows.h>
	#include <Winsock.h>
	#include <process.h>	// execution de commande systeme
	#include <stdio.h>
	#include <direct.h>
	#include <io.h>				// structures et fonctions lies aux fichiers
#else
	#include <netinet/in.h>
	#include <sys/socket.h>
	#include <netdb.h>

	typedef unsigned long _fsize_t;
#endif

#include "entities.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define					PROTOCOL_VERSION					0x01050200

/* Ports de communication utilisés */
#define					SERVER_PORT							49900

/* Longueur maximum d'une requête */
#define					MAX_REQUEST_LENGTH			65536
/* Longueur maximum d'une réponse */
#define					MAX_RESPONSE_LENGTH		4194304
/* Longueur maximum d'un paquet (voir MTU) */
#define					MAX_PAQUET_LENGTH				1472

#ifdef _NETSERVER_DEBUG

	#define					PAQUET_START_CHAR						204
	#define					PAQUET_STOP_CHAR							185

	/* Separateur de champ */
	#define					SEPARATOR_CHAR			186

#else

	#define					PAQUET_START_CHAR						02
	#define					PAQUET_STOP_CHAR							03

	/* Separateur de champ */
	#define					SEPARATOR_CHAR			30

#endif


/* Autoriser la reutilisation d'adresses deja utilisees */
#define					ALLOW_ADDR_REUSE	1

#ifndef NOERR
	#define				NOERR			0
#endif

/* Index du type de message dans une requete */
#define					NET_MESSAGE_TYPE_INDEX	1

/* Objet du contenu des trames */
typedef enum {
	START_SESSION = 1,
	END_SESSION = 2,
	STOP_SERVER = 3,
	IS_RUNNING = 4,
	RUN_SQL = 5,
	CALL_SQL_PROC = 6,
	LOAD_RECORD = 7,
	LOAD_RECORDS = 8,
	INSERT_RECORD = 9,
	SAVE_RECORD = 10,
	DELETE_RECORD = 11,
	DELETE_RECORDS = 12,
	LOAD_LDAP_ENTRIES = 13,
	LOAD_LDAP_ENTRY = 14,
	GET_ERROR_MESSAGE = 15,
	ADD_USER = 16,
	REMOVE_USER = 17,
	MODIFY_USER_PASSWORD = 18,
	ADD_ROLE_TO_USER = 19,
	REMOVE_ROLE_FROM_USER = 20,
	HAS_USER_THIS_ROLE = 21,
	CURRENT_USER = 22,
	PUT_BLOB = 23,
	GET_BLOB = 24,
	DELETE_BLOB = 25,
	LOCK_RECORD = 26,
	UNLOCK_RECORD = 27,
	CURRENT_DATE_TIME = 28,
	START_TRANSACTION = 29,
	COMMIT_TRANSACTION = 30,
	ROLLBACK_TRANSACTION = 31,
	SERVER_INFOS = 32,
	PREPARE_RESULTS_SET = 33,
	GET_LAST_RESULTS_SET = 34,
	CLEAR_LAST_RESULTS_SET = 35
}
NET_CMD;

typedef struct _net_command
{
	NET_CMD		id;
	const char	*description;
}
net_command;

static const net_command net_commands_list[]  = {

        {START_SESSION, "OPEN SESSION"},
        {END_SESSION, "CLOSE SESSION"},
        {STOP_SERVER, "STOP SERVER"},
        {IS_RUNNING, "IS RUNNING"},
        {RUN_SQL, "RUN SQL"},
        {CALL_SQL_PROC, "CALL SQL PROCEDURE"},
        {LOAD_RECORD, "LOAD RECORD"},
        {LOAD_RECORDS, "LOAD RECORDS"},
        {INSERT_RECORD, "INSERT RECORD"},
        {SAVE_RECORD, "SAVE RECORD"},
        {DELETE_RECORD, "DELETE RECORD"},
        {DELETE_RECORDS, "DELETE RECORDS"},
        {LOAD_LDAP_ENTRIES, "LOAD LDAP ENTRIES"},
        {LOAD_LDAP_ENTRY, "LOAD LDAP ENTRIE"},
        {GET_ERROR_MESSAGE, "GET_ERROR_MESSAGE"},
        {ADD_USER, "ADD_USER"},
        {REMOVE_USER, "REMOVE_USER"},
        {MODIFY_USER_PASSWORD, "MODIFY_USER_PASSWORD"},
        {ADD_ROLE_TO_USER, "ADD_ROLE_TO_USER"},
        {REMOVE_ROLE_FROM_USER, "REMOVE_ROLE_FROM_USER"},
        {HAS_USER_THIS_ROLE, "HAS_USER_THIS_ROLE"},
        {CURRENT_USER, "CURRENT_USER"},
        {PUT_BLOB, "PUT_BLOB"},
        {GET_BLOB, "GET_BLOB"},
        {DELETE_BLOB, "DELETE_BLOB"},
        {LOCK_RECORD, "LOCK_RECORD"},
        {UNLOCK_RECORD, "UNLOCK_RECORD"},
        {CURRENT_DATE_TIME, "CURRENT_DATE_TIME"},
        {START_TRANSACTION, "START_TRANSACTION"},
        {COMMIT_TRANSACTION, "COMMIT_TRANSACTION"},
        {ROLLBACK_TRANSACTION, "ROLLBACK_TRANSACTION"},
        {SERVER_INFOS, "SERVER_INFOS"},
        {PREPARE_RESULTS_SET, "PREPARE_RESULTS_SET"},
        {GET_LAST_RESULTS_SET, "GET_LAST_RESULTS_SET"},
		{CLEAR_LAST_RESULTS_SET, "CLEAR_LAST_RESULTS_SET"}
};

/* Etat des sessions */
#define					NOT_CONNECTED		0
#define					CONNECTED			1

/* Definition d'une liste de sessions */
typedef struct _net_session{
	short							m_state;				// Etat de connexion
	int								m_socket;				// Numero de socket
	int								m_pid;					// Numero de process
	int								m_debug;
	char							*m_request;
	char							*m_response;
	char							*m_last_query;
	void							*m_last_results_set;
        // Buffers utilisês lors des traitements des requêtes
	char							m_entity_signature_buffer[16];
	char							m_columns_names_buffer[MAX_BUFFER_SIZE];
	char							m_columns_values_buffer[MAX_SQL_STATEMENT_LENGTH];
	char							m_column_buffer[MAX_COLUMN_LENGTH + 1];
	char							m_where_clause_buffer[MAX_BUFFER_SIZE];
	char							m_order_by_clause_buffer[MAX_BUFFER_SIZE];
	void							*m_db_connexion;
	char							m_hostname[128];
	int								m_host_port;
	char							m_username[128];
	char							m_crypted_password[128];
    int								m_last_error;
    char							m_last_error_msg[LARGE_TEXT_SIZE];
    FILE							*m_log_trace_file;
	int								m_log_level;
	struct sockaddr_in				m_addr_host;		// Adresse du client
	struct hostent					*m_struct_host;		// Structure du client
	int                             m_keepalive_active;
	long							m_recv_timeout;
	long							m_send_timeout;

}net_session;

/* Fonctions utilisees par client et serveur */
net_session* net_session_alloc();
net_session* net_session_free(net_session *in_session);

int net_init_cnx (int *sock, short host_port, struct sockaddr_in *host_adress);

int	net_send_packet(int sock, char* data_ptr, int datasize);
int	net_rcv_packet(int sock, char* data_ptr);

int net_send_simple_packet(int sock, char* data_ptr, int in_packet_type);
int net_send_simple_packet_with_message(int sock, char* data_ptr, int in_packet_type, const char *message);
int	net_get_packet_type(const char* data_ptr);

const char	*net_get_command_desc(int in_cmd_id);

#ifdef __cplusplus
}
#endif

#endif /*NETCOMMON_H_*/

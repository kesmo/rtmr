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

#ifndef _SERVEUR_H
#define _SERVEUR_H

#include "../common/netcommon.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define					SERVER_VERSION											0x01080000

/* Nom des parametre du serveur */
#define					PARAM_PORT													"PORT"
#define					PARAM_LICENSE												"LICENSE"
#define					PARAM_DB_HOSTNAME											"DB_HOSTNAME"
#define					PARAM_DB_HOST_PORT											"DB_HOST_PORT"
#define					PARAM_DB_SCHEMA												"DB_SCHEMA"
#define					PARAM_RUNPID_DIR									"RUNPID_DIR"
#define					PARAM_LOG_DIR									"LOG_DIR"
#define					PARAM_LOG_SESSIONS_LEVEL								"LOG_SESSIONS_LEVEL"
#define					PARAM_MAX_RESPONSE_LENGTH					"MAX_RESPONSE_LENGTH"
#define					PARAM_MAX_REQUEST_LENGTH						"MAX_REQUEST_LENGTH"
#define					PARAM_MAX_PAQUET_LENGTH						"MAX_PAQUET_LENGTH"
#define                                 PARAM_RECV_TIMEOUT                                         "RECV_TIMEOUT"
#define                                 PARAM_SEND_TIMEOUT                                         "SEND_TIMEOUT"

/* Paramètres par défaut */
#define					DEFAULT_SERVER_PORT					SERVER_PORT
#if (defined(_WINDOWS) || defined(WIN32))
	#define					DEFAULT_PARAM_DIR						"."
	#define					DEFAULT_RUNPID_DIR						"."
	#define					DEFAULT_LOG_DIR							".\\log"
#else
	#define					DEFAULT_PARAM_DIR						"/etc"
	#define					DEFAULT_RUNPID_DIR						"/var/run"
	#define					DEFAULT_LOG_DIR							"/var/log/netserver"
#endif
#define					DEFAULT_NETSERVER_HOME_PARAM_DIR		"conf"
#define					DEFAULT_NETSERVER_HOME_LOG_DIR			"logs"
#define					DEFAULT_NETSERVER_HOME_RUNPID_DIR		DEFAULT_NETSERVER_HOME_LOG_DIR
#define					DEFAULT_PARAM_FILE						"netserver.conf"
#define					DEFAULT_LOG_FILE							"netserver.log"
#define					DEFAULT_DB_HOSTNAME				""
#define					DEFAULT_DB_HOST_PORT				0
#define					DEFAULT_DB_SCHEMA						"rtmr"
#define					DEFAULT_LOG_LEVEL						LOG_LEVEL_NONE
#define                                 DEFAULT_RECV_TIMEOUT                           120
#define                                 DEFAULT_SEND_TIMEOUT                            10

#define					SESSIONS_COUNT			50

/* Structure de gestion des licenses */
typedef struct _license
{
	time_t			m_time_limit;
	int				m_simultaneous_clients_limit;
	int				m_product_major_version_number;
	int				m_product_medium_version_number;
}
license;

typedef struct _net_srv_session
{
	int											m_pid;				// Numero de process
	int											m_socket;			// Numero de socket
	struct sockaddr_in			m_host_address;		// Adresse du client
	struct hostent					*m_struct_host;		// Structure du client
	struct _net_srv_session		*m_next_session;
}
net_srv_session;

typedef struct _srv_config
{
	int							m_socket;
	int							m_server_port;
	int							m_debug;
    char						m_runpid_dir[255];
    char						m_runpid_file_path[255];
    char						m_log_dir[255];
    FILE						*m_log_trace_file;
    char						m_log_trace_file_path[255];
	int							m_log_level;
    net_srv_session				*m_sessions_array;
	int							m_sessions_count;
	volatile int				m_stop_serve;
	license						*m_license;
	char						*m_license_string;
	char						m_db_hostname[255];
	int							m_db_host_port;
	char						m_db_schema[255];
        int                                                     m_socket_recv_timeout;
        int                                                     m_socket_send_timeout;
}
srv_config;

/* Fonctions du serveur */
void stop_srv();
int launch_srv(int nbparam, char *tabparam[]);
int	srv_read_server_config(const char*in_param_filename);

license* srv_read_license(const char *in_crypted_license_str);

int srv_start_server(short in_port_number);
int	srv_accept_session(net_srv_session *in_session);
net_srv_session* srv_find_free_session();
void srv_del_session_with_pid (int in_pid);
void srv_treat_request (void *in_session);


/* Fonctions de traitement des requetes */
int srv_request_connect(net_session *in_session, const char *in_request_str );
int srv_request_disconnect(net_session *in_session);
int srv_request_ping_server (net_session *in_session);
int srv_request_run_sql( net_session *in_session, const char *in_request_str );
int srv_request_call_sql_procedure( net_session *in_session, const char *in_request_str );
int srv_request_load_record( net_session *in_session, const char *in_request_str );
int srv_request_load_records( net_session *in_session, const char *in_request_str, int in_prepare_results_set );
int srv_request_insert_record( net_session *in_session, const char *in_request_str );
int srv_request_save_record( net_session *in_session, const char *in_request_str );
int srv_request_delete_record( net_session *in_session, const char *in_request_str );
int srv_request_delete_records( net_session *in_session, const char *in_request_str );
#ifdef _LDAP
	int srv_request_load_ldap_entries( net_session *in_session, const char *in_request_str );
	int srv_request_load_ldap_entry( net_session *in_session, const char *in_request_str );
#endif
int srv_request_get_error_message( net_session *in_session, const char *in_request_str );
int srv_request_add_user(net_session *in_session, const char *in_request_str);
int srv_request_remove_user(net_session *in_session, const char *in_request_str);
int srv_request_modify_user_password(net_session *in_session, const char *in_request_str);
int srv_request_add_role_to_user(net_session *in_session, const char *in_request_str);
int srv_request_remove_role_from_user(net_session *in_session, const char *in_request_str);
int srv_request_has_user_this_role(net_session *in_session, const char *in_request_str);
int srv_request_current_user(net_session *in_session);

int srv_request_put_blob(net_session *in_session);
int srv_request_get_blob(net_session *in_session);
int srv_request_delete_blob(net_session *in_session);

int srv_request_lock_record( net_session *in_session, const char *in_request_str );
int srv_request_unlock_record( net_session *in_session, const char *in_request_str );

int srv_request_date_time (net_session *in_session);

int srv_request_transaction_start (net_session *in_session);
int srv_request_transaction_commit (net_session *in_session);
int srv_request_transaction_rollback (net_session *in_session);

int srv_request_server_infos (net_session *in_session);

int srv_request_prepare_results_set( net_session *in_session, const char *in_request_str );
int srv_request_get_last_results_set( net_session *in_session, const char *in_request_str );
int srv_request_clear_last_results_set( net_session *in_session );

#ifdef __cplusplus
}
#endif

#endif	//_SERVEUR_H

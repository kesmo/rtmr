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

#include "serveur.h"

// Directives de base de donnees
#ifdef _MYSQL
#include "../common/mysql/mysql-client-library.h"
#else
#ifdef _POSTGRES
#include "../common/postgres/postgres-client-library.h"
#else

#endif
#endif

#if (defined(_WINDOWS) || defined(WIN32))
#include <process.h>
#include <signal.h>
#if defined(__GNUC__)
#include <libgen.h>
#endif
#else
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <ucontext.h>
#include <arpa/inet.h>
#include <dirent.h>
#endif

#include <fcntl.h>
#include <sys/stat.h>

#include <string.h>
#include <stdio.h>
#if defined (__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif
#include <stdlib.h>
#include <errno.h>

#include "../common/constants.h"
#include "../common/utilities.h"
#include "../common/errors.h"

#if (defined(_WINDOWS) || defined(WIN32))
HANDLE	m_thread_mutex = NULL;
#endif

srv_config *m_server_config = NULL;

// #define free(aa) {printf("[%s][ligne %d] Liberation bloc %s a %p\n",__FILE__,__LINE__,#aa,aa);free(aa);}

static void
free_server_sessions(net_srv_session *in_session)
{
  if (in_session != NULL)
    {
      free_server_sessions(in_session->m_next_session);
      free(in_session);
    }
}

static void
free_server_config()
{
  if (m_server_config != NULL)
    {
      if (m_server_config->m_log_trace_file)
        fclose(m_server_config->m_log_trace_file);

      if (m_server_config->m_license != NULL)
        free(m_server_config->m_license);

      if (m_server_config->m_sessions_array != NULL)
        free_server_sessions(m_server_config->m_sessions_array);

      free(m_server_config);
      m_server_config = NULL;
    }
}

#if (!defined(_WINDOWS) && !defined(WIN32))

/* Fonction de traitement de fin du process principal par interception des signaux de terminaison de process */
static void
terminate_process_handler(int in_signal_number, siginfo_t *in_signal_info,
    void *in_signal_context  __attribute__ ((unused)))
{
  //struct ucontext 	*tmp_signal_context = (struct ucontext*)in_signal_context;
  LOG(m_server_config, 0, "Interception signal %s(%i) - pid=%i recieve from pid=%i/uid=%i\n",
      strsignal(in_signal_number),
      in_signal_number,
      getpid(),
      in_signal_info != NULL ? in_signal_info->si_pid : 0,
      in_signal_info != NULL ? in_signal_info->si_uid : 0);
  stop_srv();
}

/* Fonction de traitement de fin d'un process enfant par interception du signal SIGCHLD */
static void
child_stop_process_handler(int in_signal_number, siginfo_t *in_signal_info,
    void *in_signal_context __attribute__ ((unused)))
{
  //struct ucontext 	*tmp_signal_context = (struct ucontext*)in_signal_context;

  LOG(m_server_config, 0, "Interception signal SIGCHLD(%i) - child pid=%i\n", in_signal_number, in_signal_info->si_pid);
  srv_del_session_with_pid(in_signal_info->si_pid);
}

/* Fonction de communication inter-process par interception du signal SIGUSR1 */
static void
child_process_handler(int in_signal_number, siginfo_t *in_signal_info,
    void *in_signal_context __attribute__ ((unused)))
{
  LOG(m_server_config, 0, "Interception signal SIGUSR1(%i) - child pid=%i\n", in_signal_number, in_signal_info->si_pid);
}

#endif

int
srv_read_server_config(const char*in_param_filename)
{
  char *tmp_server_port_str = NULL;
  char *tmp_db_hostname_str = NULL;
  char *tmp_db_host_port_str = NULL;
  char *tmp_db_schema_str = NULL;
  char *tmp_log_dir = NULL;
  char *tmp_runpid_dir = NULL;
  char *tmp_log_sessions_level = NULL;
  char *tmp_recv_timeout_str = NULL;
  char *tmp_send_timeout_str = NULL;


  FILE *tmp_file_config = NULL;

  char tmp_file_config_full_path[MAX_PATH_LENGTH];

  struct stat tmpt_log_dir_stat;

#if (defined(_WINDOWS) || defined(WIN32))
  char tmp_executable_path[MAX_PATH_LENGTH];
  char tmp_executable_dir[MAX_PATH_LENGTH];
  char tmp_base_dir[MAX_PATH_LENGTH];
  char tmp_drive_letter[4];

  GetModuleFileNameA(NULL, tmp_executable_path, MAX_PATH_LENGTH);
  _splitpath(tmp_executable_path, tmp_drive_letter, tmp_executable_dir, NULL, NULL);
  sprintf(tmp_base_dir, "%s%s..", tmp_drive_letter, tmp_executable_dir);
#endif

  if (in_param_filename == NULL)
    {
#if (defined(_WINDOWS) || defined(WIN32))
      if (is_empty_string(tmp_base_dir) == FALSE)
        sprintf(tmp_file_config_full_path, "%s%c%s%c%s", tmp_base_dir,
            FILE_SEPARATOR, DEFAULT_NETSERVER_HOME_PARAM_DIR, FILE_SEPARATOR,
            DEFAULT_PARAM_FILE);
      else
#endif
        sprintf(tmp_file_config_full_path, "%s%c%s", DEFAULT_PARAM_DIR,
            FILE_SEPARATOR, DEFAULT_PARAM_FILE);
    }
  else
    strcpy(tmp_file_config_full_path, in_param_filename);

  tmp_file_config = fopen(tmp_file_config_full_path, "r");
  if (tmp_file_config != NULL)
    {
		fprintf(
          stdout,
          "srv_read_server_config : Reading configuration file %s.\n",
          tmp_file_config_full_path);
      fflush(stdout);

      tmp_server_port_str
          = get_app_param_from_file(tmp_file_config, PARAM_PORT);
      m_server_config->m_license_string = get_app_param_from_file(
          tmp_file_config, PARAM_LICENSE);
      tmp_db_hostname_str = get_app_param_from_file(tmp_file_config,
          PARAM_DB_HOSTNAME);
      tmp_db_host_port_str = get_app_param_from_file(tmp_file_config,
          PARAM_DB_HOST_PORT);
      tmp_db_schema_str = get_app_param_from_file(tmp_file_config,
          PARAM_DB_SCHEMA);
      tmp_log_dir = get_app_param_from_file(tmp_file_config, PARAM_LOG_DIR);
      tmp_runpid_dir = get_app_param_from_file(tmp_file_config,
          PARAM_RUNPID_DIR);
      tmp_log_sessions_level = get_app_param_from_file(tmp_file_config,
          PARAM_LOG_SESSIONS_LEVEL);
      tmp_recv_timeout_str
          = get_app_param_from_file(tmp_file_config, PARAM_RECV_TIMEOUT);
      tmp_send_timeout_str
          = get_app_param_from_file(tmp_file_config, PARAM_SEND_TIMEOUT);

      fclose(tmp_file_config);
    }
  else
    {
      fprintf(
          stderr,
          "srv_read_server_config : Configuration file %s is not available (%s).\n",
          tmp_file_config_full_path, strerror(errno));
      fflush(stderr);
    }

  // Repertoire du fichier pid
  if (is_empty_string(tmp_runpid_dir) == TRUE)
    {
#if (defined(_WINDOWS) || defined(WIN32))
      if (is_empty_string(tmp_base_dir) == FALSE)
        {
          sprintf(m_server_config->m_runpid_dir, "%s%c%s", tmp_base_dir,
              FILE_SEPARATOR, DEFAULT_NETSERVER_HOME_RUNPID_DIR);
        }
      else
#endif
          strcpy(m_server_config->m_runpid_dir, DEFAULT_RUNPID_DIR);
    }
  else
    {
      strcpy(m_server_config->m_runpid_dir, tmp_runpid_dir);
    }

  if (m_server_config->m_debug == FALSE)
    {
      // Repertoire de journalisation
      if (is_empty_string(tmp_log_dir) == TRUE)
        {
#if (defined(_WINDOWS) || defined(WIN32))
		if (is_empty_string(tmp_base_dir) == FALSE)
            {
              sprintf(m_server_config->m_log_dir, "%s%c%s", tmp_base_dir,
                  FILE_SEPARATOR, DEFAULT_NETSERVER_HOME_LOG_DIR);
            }
          else
#endif
              strcpy(m_server_config->m_log_dir, DEFAULT_LOG_DIR);
        }
      else
        {
          strcpy(m_server_config->m_log_dir, tmp_log_dir);
        }

      if (stat(m_server_config->m_log_dir, &tmpt_log_dir_stat))
        {
#if (defined(_WINDOWS) || defined(WIN32))
          if (_mkdir(m_server_config->m_log_dir))
#else
          if (mkdir(m_server_config->m_log_dir, 0744))
#endif
            {
              fprintf(
                  stderr,
                  "srv_read_server_config : Log dir %s is not available (%s).\n",
                  m_server_config->m_log_dir, strerror(errno));
              fflush(stderr);
            }
        }

      if (is_empty_string(m_server_config->m_log_trace_file_path) == TRUE)
        {
          sprintf(m_server_config->m_log_trace_file_path, "%s%c%s",
              m_server_config->m_log_dir, FILE_SEPARATOR, DEFAULT_LOG_FILE);
        }

      m_server_config->m_log_trace_file = fopen(
          m_server_config->m_log_trace_file_path, "a");
      if (m_server_config->m_log_trace_file == NULL)
        {
          fprintf(
              stderr,
              "srv_read_server_config : Log file %s is not available (%s).\n",
              m_server_config->m_log_trace_file_path, strerror(errno));
          fflush(stderr);
        }
    }

  m_server_config->m_license = srv_read_license(
      m_server_config->m_license_string);

  if (is_empty_string(tmp_server_port_str))
    m_server_config->m_server_port = DEFAULT_SERVER_PORT;
  else
    m_server_config->m_server_port = atoi(tmp_server_port_str);

  if (is_empty_string(tmp_db_hostname_str))
    strcpy(m_server_config->m_db_hostname, DEFAULT_DB_HOSTNAME);
  else
    strcpy(m_server_config->m_db_hostname, tmp_db_hostname_str);

  if (is_empty_string(tmp_db_host_port_str))
    m_server_config->m_db_host_port = DEFAULT_DB_HOST_PORT;
  else
    m_server_config->m_db_host_port = atoi(tmp_db_host_port_str);

  if (is_empty_string(tmp_db_schema_str))
    strcpy(m_server_config->m_db_schema, DEFAULT_DB_SCHEMA);
  else
    strcpy(m_server_config->m_db_schema, tmp_db_schema_str);

  if (is_empty_string(tmp_log_sessions_level))
    m_server_config->m_log_level = DEFAULT_LOG_LEVEL;
  else
    m_server_config->m_log_level = atoi(tmp_log_sessions_level);

  if (is_empty_string(tmp_recv_timeout_str))
    m_server_config->m_socket_recv_timeout = DEFAULT_RECV_TIMEOUT;
  else
    m_server_config->m_socket_recv_timeout = atoi(tmp_recv_timeout_str);

  if (is_empty_string(tmp_send_timeout_str))
    m_server_config->m_socket_send_timeout = DEFAULT_SEND_TIMEOUT;
  else
    m_server_config->m_socket_send_timeout = atoi(tmp_send_timeout_str);


  return NOERR;
}

license*
srv_read_license(const char *in_crypted_license_str)
{
  char tmp_year_str[5];
  char tmp_month_str[3];
  char tmp_day_str[3];

  char tmp_clients_count_str[5];

  char tmp_major_version_str[3];
  char tmp_medium_version_str[3];

  char *tmp_license_str = NULL;
  char *tmp_ptr = NULL;

  struct tm *tmp_time = NULL;

  license *tmp_license = NULL;

  if (is_empty_string(in_crypted_license_str) == FALSE)
    {
      tmp_license_str = decrypt_str(in_crypted_license_str);

      if (tmp_license_str != NULL && strlen(tmp_license_str) == 16)
        {
          tmp_license = (license*) malloc(sizeof(license));
          tmp_ptr = tmp_license_str;

          memset(tmp_license, 0, sizeof(license));

          strncpy(tmp_year_str, tmp_ptr, 4);
          tmp_year_str[4] = '\0';
          strncpy(tmp_month_str, tmp_ptr + 4, 2);
          tmp_month_str[2] = '\0';
          strncpy(tmp_day_str, tmp_ptr + 6, 2);
          tmp_day_str[2] = '\0';

          time(&tmp_license->m_time_limit);
          tmp_time = localtime(&tmp_license->m_time_limit);
          tmp_time->tm_year = atoi(tmp_year_str) - 1900;
          tmp_time->tm_mon = atoi(tmp_month_str) - 1;
          tmp_time->tm_mday = atoi(tmp_day_str);

          tmp_license->m_time_limit = mktime(tmp_time);

          strncpy(tmp_clients_count_str, tmp_ptr + 8, 4);
          tmp_clients_count_str[4] = '\0';
          tmp_license->m_simultaneous_clients_limit = atoi(
              tmp_clients_count_str);

          strncpy(tmp_major_version_str, tmp_ptr + 12, 2);
          tmp_major_version_str[2] = '\0';
          tmp_license->m_product_major_version_number = atoi(
              tmp_major_version_str);

          strncpy(tmp_medium_version_str, tmp_ptr + 14, 2);
          tmp_medium_version_str[2] = '\0';
          tmp_license->m_product_medium_version_number = atoi(
              tmp_medium_version_str);
        }
    }
  else
    {
      LOG_ERROR(m_server_config, "srv_read_license : license is empty\n");
    }

  return tmp_license;
}

/*************************************************************************************
 *	srv_start_server
 *			Lance le serveur
 *		Parametres :
 *			in_port_number (IN/OUT) : port d'ecoute
 *		Retour :
 *			NOERR si pas d'erreur CONNEXION_ERROR sinon
 **************************************************************************************/
int
srv_start_server(short in_port_number)
{
  int status = NOERR;

  char nom_serveur[255];
  struct sockaddr_in addr_serveur;

  int allow_addr_reuse = ALLOW_ADDR_REUSE;

#if (defined(_WINDOWS) || defined(WIN32))
  DWORD recv_timeout;
  DWORD send_timeout;
#else
  struct timeval recv_timeout;
  struct timeval send_timeout;
#endif

  memset(nom_serveur, 0, sizeof(nom_serveur));
  memset(&addr_serveur, 0, sizeof(addr_serveur));

  /* Initialiser la connexion */
  status = net_init_cnx(&(m_server_config->m_socket), in_port_number,
      &addr_serveur);
  if (status != NOERR)
    {
      LOG_ERROR(m_server_config, "srv_start_server : %s\n", get_error_message(status));
      return status;
    }

  /* Parametrer la socket pour utiliser des adresses deja utilisees */
  status = setsockopt(m_server_config->m_socket, SOL_SOCKET, SO_REUSEADDR,
      (char*)&allow_addr_reuse, sizeof(int));
  if (status == -1)
    {
#if (defined(_WINDOWS) || defined(WIN32))
		LOG_ERROR(m_server_config, "srv_start_server : setsockopt SO_REUSEADDR error %i\n", WSAGetLastError());
#else
		LOG_ERROR(m_server_config, "srv_start_server : setsockopt SO_REUSEADDR error %i\n", errno);
#endif
      return NET_INIT_CONNEXION;
    }

#if (defined(_WINDOWS) || defined(WIN32))

	recv_timeout = m_server_config->m_socket_recv_timeout * 1000;
	send_timeout = m_server_config->m_socket_send_timeout * 1000;

#else
  recv_timeout.tv_sec = m_server_config->m_socket_recv_timeout;
  recv_timeout.tv_usec = 0;

  send_timeout.tv_sec = m_server_config->m_socket_send_timeout;
  send_timeout.tv_usec = 0;

#endif
  /* Parametrer un timeout de reception */
  status = setsockopt(m_server_config->m_socket, SOL_SOCKET, SO_RCVTIMEO,
      (char*)&recv_timeout, sizeof(recv_timeout));
  if (status == -1)
    {
#if (defined(_WINDOWS) || defined(WIN32))
		LOG_ERROR(m_server_config, "srv_start_server : setsockopt SO_RCVTIMEO error %i\n", WSAGetLastError());
#else
		LOG_ERROR(m_server_config, "srv_start_server : setsockopt SO_RCVTIMEO error %i\n", errno);
#endif
      return NET_INIT_CONNEXION;
    }

  /* Parametrer un timeout d'emission */
  status = setsockopt(m_server_config->m_socket, SOL_SOCKET, SO_SNDTIMEO,
      (char*)&send_timeout, sizeof(send_timeout));
  if (status == -1)
    {
#if (defined(_WINDOWS) || defined(WIN32))
		LOG_ERROR(m_server_config, "srv_start_server : setsockopt SO_SNDTIMEO error %i\n", WSAGetLastError());
#else
		LOG_ERROR(m_server_config, "srv_start_server : setsockopt SO_SNDTIMEO error %i\n", errno);
#endif
      return NET_INIT_CONNEXION;
    }

  /* Associer l'adresse a la socket */
  status = bind(m_server_config->m_socket, (struct sockaddr *) &addr_serveur,
      sizeof(addr_serveur));
  if (status == -1)
    {
#if (defined(_WINDOWS) || defined(WIN32))
		LOG_ERROR(m_server_config, "srv_start_server : bind error %i\n", WSAGetLastError());
#else
		LOG_ERROR(m_server_config, "srv_start_server : bind error %i\n", errno);
#endif
      return NET_INIT_CONNEXION;
    }

  /* Mise en ecoute du port */
  status = listen(m_server_config->m_socket, 1);
  if (status == -1)
    {
#if (defined(_WINDOWS) || defined(WIN32))
		LOG_ERROR(m_server_config, "srv_start_server : listen error %i\n", WSAGetLastError());
#else
		LOG_ERROR(m_server_config, "srv_start_server : listen error %i\n", errno);
#endif
      return NET_INIT_CONNEXION;
    }

  m_server_config->m_sessions_array = (net_srv_session*) malloc(
      sizeof(net_srv_session));
  if (m_server_config->m_sessions_array == NULL)
    {
      LOG_ERROR(m_server_config, "srv_start_server : %s\n", get_error_message(MEM_ALLOC_ERROR));
      return MEM_ALLOC_ERROR;
    }

  memset(m_server_config->m_sessions_array, 0, sizeof(net_srv_session));

  return NOERR;
}

/*************************************************************************************
 *	AcceptSession
 *			Attente d'un client et apres acceptation creation d'une session
 *		Parametres :
 *			in_session (IN/OUT) : adresse d'une session
 *		Retour :
 *			NOERR si pas d'erreur CONNEXION_ERROR sinon
 **************************************************************************************/
int
srv_accept_session(net_srv_session *in_session)
{
	fd_set tmp_read_fs;
	int tmp_select_retval = 0;

	struct timeval tmp_accept_timeout;

#if (defined(_WINDOWS) || defined(WIN32))
  int lg_addr_client;
#else
  socklen_t lg_addr_client;
#endif

	tmp_accept_timeout.tv_sec = 10;
	tmp_accept_timeout.tv_usec = 0;

	FD_ZERO(&tmp_read_fs);
	FD_SET(m_server_config->m_socket, &tmp_read_fs);

	tmp_select_retval = select(m_server_config->m_socket + 1, &tmp_read_fs, NULL, NULL, &tmp_accept_timeout);
	if (tmp_select_retval <= 0){
		return NET_SEND_TIMEOUT;
	}

  /* Connection d'un nouveau client */
  lg_addr_client = (int) sizeof(in_session->m_host_address);
  in_session->m_socket = accept(m_server_config->m_socket,
      (struct sockaddr*) &in_session->m_host_address, &lg_addr_client);
  if (in_session->m_socket == -1)
    {
      return NET_CONNEXION_ERROR;
    }

  return NOERR;
}

net_srv_session*
srv_find_free_session()
{
  net_srv_session *tmp_sessions_array = m_server_config->m_sessions_array;

  if (tmp_sessions_array == NULL)
    return NULL;

  while (tmp_sessions_array->m_next_session != NULL)
    {
      if (tmp_sessions_array->m_pid == 0)
        {
          return tmp_sessions_array;
        }

      tmp_sessions_array = tmp_sessions_array->m_next_session;
    }

  if (tmp_sessions_array->m_pid == 0)
    {
      return tmp_sessions_array;
    }

  /* Aucun slot n'est disponible : allouer une nouvelle session */
  tmp_sessions_array->m_next_session = (net_srv_session*) malloc(
      sizeof(net_srv_session));
  if (tmp_sessions_array->m_next_session == NULL)
    {
      return NULL;
    }

  memset(tmp_sessions_array->m_next_session, 0, sizeof(net_srv_session));

  return tmp_sessions_array;
}

void
srv_del_session_with_pid(int in_pid)
{
  net_srv_session *tmp_current_session = m_server_config->m_sessions_array;

#if (defined(_WINDOWS) || defined(WIN32))
  WaitForSingleObject(m_thread_mutex, INFINITE);
#endif

  m_server_config->m_sessions_count--;

  /* Se placer a la fin de la liste en la parcourant */
  while (tmp_current_session != NULL)
    {
      if (tmp_current_session->m_pid == in_pid)
        {
#if defined(_WINDOWS) || defined(WIN32)
          closesocket(tmp_current_session->m_socket);
#else
          close(tmp_current_session->m_socket);
#endif

          tmp_current_session->m_pid = 0;
          tmp_current_session->m_socket = 0;
          tmp_current_session->m_struct_host = NULL;
        }

      tmp_current_session = tmp_current_session->m_next_session;
    }

#if defined(_WINDOWS) || defined(WIN32)
    ReleaseMutex(m_thread_mutex);
#endif
}

/*************************************************************************************
 *	TreatRequest
 *			Reception d'une requete cliente
 *		Parametres :
 *			in_session (IN) : adresse de la session cliente
 *		Retour :
 *			Aucun
 **************************************************************************************/
void
srv_treat_request(void *in_session)
{
  int tmp_status = NOERR;
  int tmp_close_session = FALSE;
  net_session *tmp_session = net_session_alloc();
  net_srv_session *tmp_server_session = (net_srv_session*) in_session;

  char tmp_log_trace_file_name[256];

  time_t tmp_ctime = time(NULL);
  struct tm *tmp_mtime = localtime(&tmp_ctime);

#if (!defined(_WINDOWS) && !defined(WIN32))

  struct sigaction tmp_signal_action;

  tmp_signal_action.sa_flags = SA_SIGINFO;
  tmp_signal_action.sa_sigaction = child_process_handler;
  if (sigaction(SIGUSR1, &tmp_signal_action, NULL))
    {
      printf("Sigaction returned error = %d\n", errno);
      _exit(0);
    }

#endif

  tmp_session->m_socket = tmp_server_session->m_socket;
  tmp_session->m_pid = tmp_server_session->m_pid;

  tmp_session->m_debug = m_server_config->m_debug;
  tmp_session->m_log_level = m_server_config->m_log_level;

  if (m_server_config->m_debug == FALSE)
    {
      if (m_server_config->m_log_level >= LOG_LEVEL_ERRORS)
        {
          if (tmp_server_session->m_struct_host)
            sprintf(tmp_log_trace_file_name, "%s%c%s_%i%.2i%.2i.log",
                m_server_config->m_log_dir, FILE_SEPARATOR,
                tmp_server_session->m_struct_host->h_name,
                tmp_mtime->tm_year + 1900, tmp_mtime->tm_mon + 1,
                tmp_mtime->tm_mday);
          else
            sprintf(tmp_log_trace_file_name, "%s%c%s_%i%.2i%.2i.log",
                m_server_config->m_log_dir, FILE_SEPARATOR, inet_ntoa(
                    tmp_server_session->m_host_address.sin_addr),
                tmp_mtime->tm_year + 1900, tmp_mtime->tm_mon + 1,
                tmp_mtime->tm_mday);

          tmp_session->m_log_trace_file = fopen(tmp_log_trace_file_name, "a");
          if (tmp_session->m_log_trace_file == NULL)
            {
              LOG_ERRORNO(m_server_config, "srv_treat_request (open log trace file)");
            }
        }
    }

  while (tmp_status == NOERR && !tmp_close_session
      && !m_server_config->m_stop_serve)
    {
      /* Reception de la requete */
      tmp_status = net_rcv_packet(tmp_session->m_socket, tmp_session->m_request);
      if (tmp_status == NOERR)
        {
		  /* Vérifier que le serveur répond */
          if (net_get_packet_type(tmp_session->m_request) == IS_RUNNING)
		  {
            tmp_status = srv_request_ping_server(tmp_session);
		  }
		  else
		  {

			  LOG_TRACE(tmp_session, "_____________________ REQUEST %s _____________________\n", net_get_command_desc(net_get_packet_type(tmp_session->m_request)));
			  LOG_TRACE(tmp_session, "%s\n", tmp_session->m_request);

			  /* Interpretation de la requete */
			  switch (net_get_packet_type(tmp_session->m_request))
				{
			  /* Ouverture de session */
			  case START_SESSION:
				tmp_status = srv_request_connect(tmp_session,
					tmp_session->m_request);
				tmp_close_session = (tmp_session->m_db_connexion == NULL);
				break;

				/* Avertissement de fin de session */
			  case END_SESSION:
				tmp_status = srv_request_disconnect(tmp_session);
				tmp_close_session = TRUE;
				break;

				/* Executer une requête sql */
			  case RUN_SQL:
				tmp_status = srv_request_run_sql(tmp_session,
					tmp_session->m_request);
				break;

				/* Appeler une procedure sql */
			  case CALL_SQL_PROC:
				tmp_status = srv_request_call_sql_procedure(tmp_session,
					tmp_session->m_request);
				break;

				/* Charger un enregistrement */
			  case LOAD_RECORD:
				tmp_status = srv_request_load_record(tmp_session,
					tmp_session->m_request);
				break;

				/* Charger plusieurs enregistrements */
			  case LOAD_RECORDS:
				tmp_status = srv_request_load_records(tmp_session,
					tmp_session->m_request, FALSE);
				break;

				/* Insérer un enregistrement */
			  case INSERT_RECORD:
				tmp_status = srv_request_insert_record(tmp_session,
					tmp_session->m_request);
				break;

				/* Sauver un enregistrement */
			  case SAVE_RECORD:
				tmp_status = srv_request_save_record(tmp_session,
					tmp_session->m_request);
				break;

				/* Suprimer un enregistrement */
			  case DELETE_RECORD:
				tmp_status = srv_request_delete_record(tmp_session,
					tmp_session->m_request);
				break;

				/* Suprimer plusieurs enregistrements */
			  case DELETE_RECORDS:
				tmp_status = srv_request_delete_records(tmp_session,
					tmp_session->m_request);
				break;

	#ifdef _LDAP
				case LOAD_LDAP_ENTRIES :
				tmp_status = srv_request_load_ldap_entries(tmp_session, tmp_session->m_request);
				break;

				case LOAD_LDAP_ENTRY :
				tmp_status = srv_request_load_ldap_entry(tmp_session, tmp_session->m_request);
				break;
	#endif

			  case GET_ERROR_MESSAGE:
				tmp_status = srv_request_get_error_message(tmp_session,
					tmp_session->m_request);
				break;

				/* Ajouter un utilisateur */
			  case ADD_USER:
				tmp_status = srv_request_add_user(tmp_session,
					tmp_session->m_request);
				break;

				/* Supprimer un utilisateur */
			  case REMOVE_USER:
				tmp_status = srv_request_remove_user(tmp_session,
					tmp_session->m_request);
				break;

				/* Modifier le mot de passe d'un utilisateur */
			  case MODIFY_USER_PASSWORD:
				tmp_status = srv_request_modify_user_password(tmp_session,
					tmp_session->m_request);
				break;

				/* Ajouter un rôle à un utilisateur */
			  case ADD_ROLE_TO_USER:
				tmp_status = srv_request_add_role_to_user(tmp_session,
					tmp_session->m_request);
				break;

				/* Supprimer le rôle d'un utilisateur */
			  case REMOVE_ROLE_FROM_USER:
				tmp_status = srv_request_remove_role_from_user(tmp_session,
					tmp_session->m_request);
				break;

				/* Vérifier le rôle d'un utilisateur */
			  case HAS_USER_THIS_ROLE:
				tmp_status = srv_request_has_user_this_role(tmp_session,
					tmp_session->m_request);
				break;

				/* Renvoie le nom de l'utilisateur connecté */
			  case CURRENT_USER:
				tmp_status = srv_request_current_user(tmp_session);
				break;

				/* Inserer un BLOB */
			  case PUT_BLOB:
				tmp_status = srv_request_put_blob(tmp_session);
				break;

				/* Renvoyer un BLOB */
			  case GET_BLOB:
				tmp_status = srv_request_get_blob(tmp_session);
				break;

				/* Supprimer un BLOB */
			  case DELETE_BLOB:
				tmp_status = srv_request_delete_blob(tmp_session);
				break;

				/* Arreter le serveur */
			  case STOP_SERVER:
				stop_srv();
				/* Repondre */
				tmp_status = net_send_simple_packet(tmp_session->m_socket,
					tmp_session->m_response, NOERR);
				break;

				/* Verrouiller un enregistrement */
			  case LOCK_RECORD:
				tmp_status = srv_request_lock_record(tmp_session,
					tmp_session->m_request);
				break;

				/* Deverrouiller un enregistrement */
			  case UNLOCK_RECORD:
				tmp_status = srv_request_unlock_record(tmp_session,
					tmp_session->m_request);
				break;

				/* Renvoyer la date et l'heure courante */
			  case CURRENT_DATE_TIME:
				tmp_status = srv_request_date_time(tmp_session);
				break;

				/* Debuter une transaction */
			  case START_TRANSACTION:
				tmp_status = srv_request_transaction_start(tmp_session);
				break;

				/* Valider une transaction */
			  case COMMIT_TRANSACTION:
				tmp_status = srv_request_transaction_commit(tmp_session);
				break;

				/* Annuler une transaction */
			  case ROLLBACK_TRANSACTION:
				tmp_status = srv_request_transaction_rollback(tmp_session);
				break;

				/* Recuperer les informations du serveur */
			  case SERVER_INFOS:
				tmp_status = srv_request_server_infos(tmp_session);
				break;

				/* Preparer un ensemble d'enregistrements */
			  case PREPARE_RESULTS_SET:
				tmp_status = srv_request_prepare_results_set(tmp_session,
					tmp_session->m_request);
				break;

				/* Charger un ensemble d'enregistrements */
			  case GET_LAST_RESULTS_SET:
				tmp_status = srv_request_get_last_results_set(tmp_session,
					tmp_session->m_request);
				break;

				/* Vider l'ensemble des derniers enregistrements */
			  case CLEAR_LAST_RESULTS_SET:
				tmp_status
					= srv_request_clear_last_results_set(tmp_session);
				break;

				/* Requete non reconnue */
			  default:
				tmp_status = net_send_simple_packet(tmp_session->m_socket,
					tmp_session->m_response, NET_REQUEST_UNKNOW);
				break;
				}

			  LOG_TRACE(tmp_session, "--------------------- RESPONSE ----------------------\n");
			  LOG_TRACE(tmp_session, "%s\n", tmp_session->m_response);
			 }
        }
      else
        {
          // Ne pas traiter l'erreur de timeout si celui-ci n'est pas actif pour la connexion courante
          if (tmp_status == NET_RECV_TIMEOUT && !tmp_session->m_keepalive_active)
		  {
            tmp_status = NOERR;
		  }
          else if (tmp_status != NET_CONNEXION_CLOSED)
		  {
			LOG_SOCK_ERRORNO(tmp_session, "srv_treat_request");
            LOG_ERROR(tmp_session, "%s\n", get_error_message(tmp_status));
		  }
        }
    }

  db_die(tmp_session);
  tmp_session = net_session_free(tmp_session);

#if (defined(_WINDOWS) || defined(WIN32))

  srv_del_session_with_pid(((net_srv_session*)in_session)->m_pid);
  _endthread();

#endif
}

void
stop_srv()
{
#if (defined(_WINDOWS) || defined(WIN32))
  WaitForSingleObject(m_thread_mutex, INFINITE);
#endif

  if (m_server_config)
    m_server_config->m_stop_serve = TRUE;

#if (defined(_WINDOWS) || defined(WIN32))
  ReleaseMutex(m_thread_mutex);
#endif
}

/******************************************************************************************/
/******************************************************************************************/
/*************************************	  MAIN PROGRAM	  *********************************/
/******************************************************************************************/
/******************************************************************************************/
int
launch_srv(int nbparam, char *tabparam[])
{
  long tmp_return_value = NOERR;
  net_srv_session *tmp_current_session = NULL;
  int tmp_param_index = 0;
  char *tmp_param_filename = NULL;
  FILE *tmp_file = NULL;

#if (defined(_WINDOWS) || defined(WIN32))
  int tmp_current_session_pid = -1;

  m_thread_mutex = CreateMutex(NULL, FALSE, NULL);
#else

  pid_t tmp_master_pid = 0;
  pid_t tmp_current_session_pid = -1;

#if (!defined(__APPLE__))
  struct stat tmp_file_stat;
  char tmp_master_pid_str[16];
#endif

  struct sigaction tmp_signal_action;

  struct sigaction tmp_term_signal_action;

#endif

  m_server_config = (srv_config*) malloc(sizeof(srv_config));
  memset(m_server_config, 0, sizeof(srv_config));

  if (nbparam > 1)
    {
      for (tmp_param_index = 1; tmp_param_index < nbparam; tmp_param_index++)
        {
          if (is_empty_string(tabparam[tmp_param_index]) == FALSE)
            {
              // Version
              if (strcmp(tabparam[tmp_param_index], "-v") == 0)
                {
                  fprintf(stdout,
                      "%s version is %d.%d.%d.%d (protocol %d.%d.%d.%d).\n",
                      tabparam[0], SERVER_VERSION >> 24, (SERVER_VERSION >> 16)
                          & 0xFF, (SERVER_VERSION >> 8) & 0xFF, SERVER_VERSION
                          & 0xFF, PROTOCOL_VERSION >> 24, (PROTOCOL_VERSION
                          >> 16) & 0xFF, (PROTOCOL_VERSION >> 8) & 0xFF,
                      PROTOCOL_VERSION & 0xFF);
                  fflush(stdout);
                  free_server_config();
                  return 0;
                }
              // Mode debug
              else if (strcmp(tabparam[tmp_param_index], "-d") == 0)
                {
                  m_server_config->m_debug = TRUE;
                  fprintf(stdout, "Debugging is ON.\n");
                  fflush(stdout);
                }
              // Fichier de configuration
              else if (strcmp(tabparam[tmp_param_index], "-f") == 0)
                {
                  if (tmp_param_index + 1 < nbparam)
                    {
                      tmp_file = fopen(tabparam[tmp_param_index + 1], "r");
                      if (tmp_file != NULL)
                        {
                          tmp_param_filename = tabparam[tmp_param_index + 1];
                          fclose(tmp_file);
                          tmp_param_index++;
                        }
                      else
                        {
                          fprintf(stderr, "unable to open %s\n",
                              tabparam[tmp_param_index + 1]);
                          fflush(stderr);
                          free_server_config();
                          return -1;
                        }
                    }
                  else
                    {
                      fprintf(stderr,
                          "Parameters file name is necessary after -f option.\n");
                      fflush(stderr);
                      free_server_config();
                      return -1;
                    }
                }
              // Génération de license
              else if (strcmp(tabparam[tmp_param_index], "--encrypt") == 0)
                {
                  if (tmp_param_index + 1 < nbparam)
                    {
                      fprintf(stdout, "License for %s is %s\n",
                          tabparam[tmp_param_index + 1], encrypt_str(
                              tabparam[tmp_param_index + 1]));
                      fflush(stdout);
                      tmp_param_index++;
                      free_server_config();
                      return 0;
                    }
                  else
                    {
                      fprintf(stderr, "License to crypt is necessary.\n");
                      fflush(stderr);
                      free_server_config();
                      return -1;
                    }
                }
              // Vérification de license
              else if (strcmp(tabparam[tmp_param_index], "--decrypt") == 0)
                {
                  if (tmp_param_index + 1 < nbparam)
                    {
                      fprintf(stdout, "Crypted license for %s is %s\n",
                          tabparam[tmp_param_index + 1], decrypt_str(
                              tabparam[tmp_param_index + 1]));
                      fflush(stdout);
                      tmp_param_index++;
                      free_server_config();
                      return 0;
                    }
                  else
                    {
                      fprintf(stderr, "License to decrypt is necessary.\n");
                      fflush(stderr);
                      free_server_config();
                      return -1;
                    }
                }
              else
                {
                  fprintf(stderr, "Incorrect parameter %s.\n",
                      tabparam[tmp_param_index]);
                  fflush(stderr);
                  free_server_config();
                  return -1;
                }
            }
        }
    }

  srv_read_server_config(tmp_param_filename);

  LOG(m_server_config, 0, "Starting server  - version %d.%d.%d.%d (protocol %d.%d.%d.%d).\n",
                      SERVER_VERSION >> 24, (SERVER_VERSION >> 16)
                          & 0xFF, (SERVER_VERSION >> 8) & 0xFF, SERVER_VERSION
                          & 0xFF, PROTOCOL_VERSION >> 24, (PROTOCOL_VERSION
                          >> 16) & 0xFF, (PROTOCOL_VERSION >> 8) & 0xFF,
                      PROTOCOL_VERSION & 0xFF);

  LOG_TRACE(m_server_config, "\t%s=%s\n", PARAM_LOG_DIR, m_server_config->m_log_dir);
  LOG_TRACE(m_server_config, "\t%s=%s\n", PARAM_RUNPID_DIR, m_server_config->m_runpid_dir);
  LOG_TRACE(m_server_config, "\t%s=%s\n", PARAM_LICENSE, m_server_config->m_license_string);
  LOG_TRACE(m_server_config, "\t%s=%i\n", PARAM_PORT, m_server_config->m_server_port);
  LOG_TRACE(m_server_config, "\t%s=%s\n", PARAM_DB_HOSTNAME, m_server_config->m_db_hostname);
  LOG_TRACE(m_server_config, "\t%s=%i\n", PARAM_DB_HOST_PORT, m_server_config->m_db_host_port);
  LOG_TRACE(m_server_config, "\t%s=%s\n", PARAM_DB_SCHEMA, m_server_config->m_db_schema);
  LOG_TRACE(m_server_config, "\t%s=%i\n", PARAM_LOG_SESSIONS_LEVEL, m_server_config->m_log_level);
  LOG_TRACE(m_server_config, "\t%s=%i\n", PARAM_RECV_TIMEOUT, m_server_config->m_socket_recv_timeout);
  LOG_TRACE(m_server_config, "\t%s=%i\n", PARAM_SEND_TIMEOUT, m_server_config->m_socket_send_timeout);

  /* Lancement du serveur */
  tmp_return_value = srv_start_server(m_server_config->m_server_port);
  if (tmp_return_value == NOERR)
    {
#if (!defined(_WINDOWS) && !defined(WIN32))
      /* Gestion specifique pour launchd sous MacOS X */
#if (!defined(__APPLE__))
      if (m_server_config->m_debug == FALSE)
        {
          tmp_master_pid = fork();
          if (tmp_master_pid == 0)
            {
#endif
              // Gestion de l'arret du process principal
              tmp_term_signal_action.sa_flags = SA_SIGINFO;
              tmp_term_signal_action.sa_sigaction = terminate_process_handler;
              if (sigaction(SIGHUP, &tmp_term_signal_action, NULL))
                {
                  LOG_ERRORNO(m_server_config, "launch_srv (sigaction on SIGHUP)");
                  return -1;
                }

              if (sigaction(SIGINT, &tmp_term_signal_action, NULL))
                {
                  LOG_ERRORNO(m_server_config, "launch_srv (sigaction on SIGINT)");
                  return -1;
                }

              if (sigaction(SIGQUIT, &tmp_term_signal_action, NULL))
                {
                  LOG_ERRORNO(m_server_config, "launch_srv (sigaction on SIGQUIT)");
                  return -1;
                }

              if (sigaction(SIGTERM, &tmp_term_signal_action, NULL))
                {
                  LOG_ERRORNO(m_server_config, "launch_srv (sigaction on SIGTERM)");
                  return -1;
                }

#if (!defined(__APPLE__))
            }
          else
            {
              // Creation du fichier runpid
              sprintf(m_server_config->m_runpid_file_path, "%s%cnetserver.pid",
                  m_server_config->m_runpid_dir, FILE_SEPARATOR);
              // Le fichier existe
              if (stat(m_server_config->m_runpid_file_path, &tmp_file_stat)
                  == 0)
                {
                  LOG_TRACE(m_server_config, "The file %s already exists.\n", m_server_config->m_runpid_file_path);
                  // on tente de le supprimer
                  remove(m_server_config->m_runpid_file_path);
                }

              tmp_file = fopen(m_server_config->m_runpid_file_path, "w");
              if (tmp_file != NULL)
                {
                  sprintf(tmp_master_pid_str, "%i", tmp_master_pid);
                  fputs(tmp_master_pid_str, tmp_file);
                  fclose(tmp_file);
                }
              else
                {
                  fprintf(stderr, "The file %s is not accessible (%s).\n",
                      m_server_config->m_runpid_file_path, strerror(errno));
                  fflush(stderr);
                }

              free_server_config();
              return 0;
            }
        }
#endif
      if (m_server_config->m_debug == TRUE || tmp_master_pid == 0)
        {
          // Gestion de l'arret des process enfants
          tmp_signal_action.sa_flags = SA_SIGINFO | SA_NOCLDWAIT;
          tmp_signal_action.sa_sigaction = child_stop_process_handler;
          if (sigaction(SIGCHLD, &tmp_signal_action, NULL))
            {
              LOG_ERRORNO(m_server_config, "launch_srv (sigaction on SIGCHLD)");
              return -1;
            }
        }
#endif

      tmp_current_session = m_server_config->m_sessions_array;
      while (!m_server_config->m_stop_serve)
        {
          /* Connection d'un client */
          tmp_return_value = srv_accept_session(tmp_current_session);
          if (tmp_return_value == NOERR)
            {
              m_server_config->m_sessions_count++;

              /* Recuperation des infos du client connecte */
              tmp_current_session->m_struct_host = gethostbyaddr(
                  (char*) &tmp_current_session->m_host_address.sin_addr,
                  sizeof(struct in_addr),
                  tmp_current_session->m_host_address.sin_family);
              if (tmp_current_session->m_struct_host)
                {
                  LOG_TRACE(m_server_config, "Treating requests from %s (socket:%i)\n", tmp_current_session->m_struct_host->h_name, tmp_current_session->m_socket);
                }
              else
                {
                  LOG_TRACE(m_server_config, "Treating requests from %s (socket:%i)\n", inet_ntoa(tmp_current_session->m_host_address.sin_addr), tmp_current_session->m_socket);
                }

              /* Traitement des requetes */
#if (defined(_WINDOWS) || defined(WIN32))
              tmp_current_session_pid = _beginthread(srv_treat_request, 0, tmp_current_session);
              tmp_current_session->m_pid = tmp_current_session_pid;

              tmp_current_session = srv_find_free_session();
              if (tmp_current_session == NULL)
                {
					stop_srv();
                }
#else
              tmp_current_session_pid = fork();
              if (tmp_current_session_pid == 0)
                {
                  srv_treat_request(tmp_current_session);
                  stop_srv();
                  if (tmp_current_session->m_struct_host)
                    {
                      LOG_TRACE(m_server_config, "End of treating requests from %s (socket:%i)\n", tmp_current_session->m_struct_host->h_name, tmp_current_session->m_socket);
                    }
                  else
                    {
                      LOG_TRACE(m_server_config, "End of treating requests from %s (socket:%i)\n", inet_ntoa(tmp_current_session->m_host_address.sin_addr), tmp_current_session->m_socket);
                    }
                }
              else
                {
                  tmp_current_session->m_pid = tmp_current_session_pid;

                  tmp_current_session = srv_find_free_session();
                  if (tmp_current_session == NULL)
                    {
                      stop_srv();
                    }
                }
#endif
            }
        }
    }
  else
    {
      LOG_ERROR(m_server_config, "%s\n", get_error_message(tmp_return_value));
    }

  if (tmp_current_session_pid != 0)
    {
#if (defined(_WINDOWS) || defined(WIN32))
		CloseHandle(m_thread_mutex);
#elif !defined(__APPLE__)
      remove(m_server_config->m_runpid_file_path);
#endif
      LOG(m_server_config, 0, "Stopping server\n");
    }

  free_server_config();

  return tmp_return_value;
}

/*************************************************************************************
 *	srv_request_connect
 *			Traitement d'une requête de connexion à la base de données
 *		Parametres :
 *			in_session (IN) : adresse de la session cliente
 * 			in_request_str (IN) : contenu de la requête
 *		Retour :
 *			Aucun
 **************************************************************************************/
int
srv_request_connect(net_session *in_session, const char *in_request_str)
{
  char tmp_username[256];
  char tmp_passwd[256];
  char tmp_protocol_major_version[4];
  char tmp_protocol_medium_version[4];
  char tmp_protocol_minor_version[4];
  char tmp_protocol_maintenance_version[4];

  int tmp_protocol_major_version_int;
  int tmp_protocol_medium_version_int;
  int tmp_protocol_minor_version_int;
  int tmp_protocol_maintenance_version_int;

  int tmp_return = DB_CONNEXION_ERROR;

  int tmp_nb_bytes = 0;

  const char *tmp_ptr = in_request_str;

  if (in_session->m_db_connexion != NULL)
    db_die(in_session);

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr, tmp_username,
      SEPARATOR_CHAR);

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_passwd, SEPARATOR_CHAR);

  // Verifier les paramètres
  if (is_empty_string(tmp_username))
    {
      LOG_ERROR(in_session, "srv_request_connect : username is null\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  if (is_empty_string(tmp_passwd))
    {
      LOG_ERROR(in_session, "srv_request_connect : passwd is null\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_protocol_major_version,
      SEPARATOR_CHAR);
  tmp_ptr = net_get_field(0, tmp_ptr, tmp_protocol_medium_version,
      SEPARATOR_CHAR);
  tmp_ptr = net_get_field(0, tmp_ptr, tmp_protocol_minor_version,
      SEPARATOR_CHAR);
  tmp_ptr = net_get_field(0, tmp_ptr, tmp_protocol_maintenance_version,
      SEPARATOR_CHAR);

  if (is_empty_string(tmp_protocol_major_version) || is_empty_string(
      tmp_protocol_medium_version) || is_empty_string(
      tmp_protocol_minor_version) || is_empty_string(
      tmp_protocol_maintenance_version))
    {
      sprintf(
          in_session->m_last_error_msg,
          "La version du protocole d'échange de données client (vide) n'est pas compatible avec la version du serveur (%d.%d.%d.%d).",
          (PROTOCOL_VERSION >> 24), ((PROTOCOL_VERSION >> 16) & 0xFF),
          ((PROTOCOL_VERSION >> 8) & 0xFF), (PROTOCOL_VERSION & 0xFF));

      LOG_ERROR(in_session, "srv_request_connect : protocol version is empty\n");
      return net_send_simple_packet_with_message(in_session->m_socket,
          in_session->m_response, NET_PROTOCOL_VERSION_INCOMPATIBLE,
          in_session->m_last_error_msg);
    }

  tmp_protocol_major_version_int = atoi(tmp_protocol_major_version);
  tmp_protocol_medium_version_int = atoi(tmp_protocol_medium_version);
  tmp_protocol_minor_version_int = atoi(tmp_protocol_minor_version);
  tmp_protocol_maintenance_version_int = atoi(tmp_protocol_maintenance_version);

  if (tmp_protocol_major_version_int != (PROTOCOL_VERSION >> 24)
      || tmp_protocol_medium_version_int != ((PROTOCOL_VERSION >> 16) & 0xFF))
    {
      sprintf(
          in_session->m_last_error_msg,
          "La version du protocole d'échange de données client (%d.%d.%d.%d) n'est pas compatible avec la version du serveur (%d.%d.%d.%d).",
          tmp_protocol_major_version_int, tmp_protocol_medium_version_int,
          tmp_protocol_minor_version_int, tmp_protocol_maintenance_version_int,
          (PROTOCOL_VERSION >> 24), ((PROTOCOL_VERSION >> 16) & 0xFF),
          ((PROTOCOL_VERSION >> 8) & 0xFF), (PROTOCOL_VERSION & 0xFF));

      LOG_ERROR(in_session, "srv_request_connect : protocol version is incompatible\n");
      return net_send_simple_packet_with_message(in_session->m_socket,
          in_session->m_response, NET_PROTOCOL_VERSION_INCOMPATIBLE,
          in_session->m_last_error_msg);
    }

  // Activer la deconnexion automatique (timeout) des clients dont le protocole de communication est >= 1.5.1
  in_session->m_keepalive_active = (tmp_protocol_major_version_int > 1 ||
      (tmp_protocol_major_version_int == 1 && (tmp_protocol_medium_version_int > 5 ||
          (tmp_protocol_medium_version_int == 5 && tmp_protocol_minor_version_int >= 1))));

  LOG_TRACE(in_session, "srv_request_connect : activation keepalive %i\n", in_session->m_keepalive_active);

  tmp_return = db_start(in_session, m_server_config->m_db_hostname,
      m_server_config->m_db_host_port, m_server_config->m_db_schema,
      tmp_username, decrypt_str(tmp_passwd), NULL);
  if (tmp_return == NOERR)
    {
      tmp_nb_bytes = sprintf(in_session->m_response,
          "%c%c%i%c%d%c%d%c%d%c%d%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR,
          tmp_return, SEPARATOR_CHAR, (PROTOCOL_VERSION >> 24), SEPARATOR_CHAR,
          ((PROTOCOL_VERSION >> 16) & 0xFF), SEPARATOR_CHAR, ((PROTOCOL_VERSION
              >> 8) & 0xFF), SEPARATOR_CHAR, (PROTOCOL_VERSION & 0xFF),
          SEPARATOR_CHAR, PAQUET_STOP_CHAR);
      return net_send_packet(in_session->m_socket, in_session->m_response,
          tmp_nb_bytes);
    }
  else
    return net_send_simple_packet(in_session->m_socket, in_session->m_response,
        tmp_return);
}

int
srv_request_disconnect(net_session *in_session)
{
  db_die(in_session);

  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      NOERR);
}

int
srv_request_ping_server(net_session *in_session)
{
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      NOERR);
}

int
srv_request_run_sql(net_session *in_session, const char *in_request_str)
{
  net_get_field(NET_MESSAGE_TYPE_INDEX + 1, in_request_str,
      in_session->m_last_query, SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_last_query))
    {
      LOG_ERROR(in_session, "srv_request_run_sql : statement is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  return net_db_do_query(in_session, in_session->m_last_query, FALSE, 0, 0);
}

int
srv_request_call_sql_procedure(net_session *in_session,
    const char *in_request_str)
{
  // Execution de la procedure
  return net_db_call_procedure(in_session, in_request_str);
}

int
srv_request_load_record(net_session *in_session, const char *in_request_str)
{
  char *tmp_statement_ptr = in_session->m_last_query;

  char tmp_primary_key_value[PRIMARY_KEY_SIZE];
  const char *tmp_ptr = in_request_str;

  entity_def *tmp_entity_def = NULL;
  const char **tmp_keys_columns = NULL;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr,
      in_session->m_entity_signature_buffer, SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_entity_signature_buffer))
    {
      LOG_ERROR(in_session, "srv_request_load_record : entity signature is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Chercher le definition de l'entite */
  if (get_table_def(atoi(in_session->m_entity_signature_buffer),
      &tmp_entity_def) < 0)
    {
      LOG_ERROR(in_session, "srv_request_load_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  tmp_keys_columns = tmp_entity_def->m_entity_columns_names;
  if (tmp_keys_columns == NULL || is_empty_string(*tmp_keys_columns) == TRUE)
    {
      LOG_ERROR(in_session, "srv_request_load_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_primary_key_value, SEPARATOR_CHAR);
  if (is_empty_string(tmp_primary_key_value))
    {
      LOG_ERROR(in_session, "srv_request_load_record : primary key is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Generer la requête */
  tmp_statement_ptr += sprintf(tmp_statement_ptr, "select %s",
      *tmp_keys_columns);
  while ((*(++tmp_keys_columns)) != NULL)
    tmp_statement_ptr += sprintf(tmp_statement_ptr, ",%s", *tmp_keys_columns);

  tmp_statement_ptr += sprintf(tmp_statement_ptr, " from %s where %s=%s;",
      tmp_entity_def->m_entity_name, *tmp_entity_def->m_entity_columns_names,
      tmp_primary_key_value);

  /* Executer la requête */
  return net_db_do_query(in_session, in_session->m_last_query, FALSE, 0, 0);
}

int
srv_request_load_records(net_session *in_session, const char *in_request_str,
    int in_prepare_results_set)
{
  char *tmp_statement_ptr = in_session->m_last_query;

  const char *tmp_ptr = in_request_str;

  entity_def *tmp_entity_def = NULL;
  const char **tmp_keys_columns = NULL;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr,
      in_session->m_entity_signature_buffer, SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_entity_signature_buffer))
    {
      LOG_ERROR(in_session, "srv_request_load_records : entity signature is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Chercher le definition de l'entite */
  if (get_table_def(atoi(in_session->m_entity_signature_buffer),
      &tmp_entity_def) < 0)
    {
      LOG_ERROR(in_session, "srv_request_load_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  tmp_keys_columns = tmp_entity_def->m_entity_columns_names;
  if (tmp_keys_columns == NULL || is_empty_string(*tmp_keys_columns) == TRUE)
    {
      LOG_ERROR(in_session, "srv_request_load_records : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, in_session->m_where_clause_buffer,
      SEPARATOR_CHAR);

  tmp_ptr = net_get_field(0, tmp_ptr, in_session->m_order_by_clause_buffer,
      SEPARATOR_CHAR);

  /* Generer la requête */
  tmp_statement_ptr += sprintf(tmp_statement_ptr, "select %s",
      *tmp_keys_columns);
  while (*(++tmp_keys_columns) != NULL)
    tmp_statement_ptr += sprintf(tmp_statement_ptr, ",%s", *tmp_keys_columns);

  tmp_statement_ptr += sprintf(tmp_statement_ptr, " from %s",
      tmp_entity_def->m_entity_name);

  if (is_empty_string(in_session->m_where_clause_buffer) == FALSE)
    tmp_statement_ptr += sprintf(tmp_statement_ptr, " where %s",
        in_session->m_where_clause_buffer);

  if (is_empty_string(in_session->m_order_by_clause_buffer) == FALSE)
    tmp_statement_ptr += sprintf(tmp_statement_ptr, " order by %s",
        in_session->m_order_by_clause_buffer);

  tmp_statement_ptr += sprintf(tmp_statement_ptr, ";");

  /* Executer la requête */
  return net_db_do_query(in_session, in_session->m_last_query,
      in_prepare_results_set, 0, 0);
}

int
srv_request_insert_record(net_session *in_session, const char *in_request_str)
{
  const char *tmp_ptr = in_request_str;

  entity_def *tmp_entity_def = NULL;
  const char **tmp_current_key = NULL;
  const char **tmp_current_column_format = NULL;

  char *tmp_statement_ptr = in_session->m_last_query;

  char *tmp_columns_names_ptr = in_session->m_columns_names_buffer;

  char *tmp_columns_values_ptr = in_session->m_columns_values_buffer;

  int tmp_column_index = 0;
  unsigned int tmp_column_length = 0;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr,
      in_session->m_entity_signature_buffer, SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_entity_signature_buffer))
    {
      LOG_ERROR(in_session, "srv_request_insert_record : entity signature is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Chercher le definition de l'entite */
  if (get_table_def(atoi(in_session->m_entity_signature_buffer),
      &tmp_entity_def) < 0)
    {
      LOG_ERROR(in_session, "srv_request_insert_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  tmp_current_column_format = tmp_entity_def->m_entity_columns_formats;
  tmp_current_key = tmp_entity_def->m_entity_columns_names;
  if (tmp_current_key == NULL || is_empty_string(*tmp_current_key) == TRUE)
    {
      LOG_ERROR(in_session, "srv_request_insert_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  /* Generer la requête */
  while (*tmp_current_key != NULL)
    {
      tmp_column_length = net_find_field(0, &tmp_ptr, SEPARATOR_CHAR);
      if (tmp_column_length > 0)
        {
          if (tmp_entity_def->m_entity_columns_sizes != NULL
              && tmp_column_length
                  > tmp_entity_def->m_entity_columns_sizes[tmp_column_index])
            {
              sprintf(
                  in_session->m_last_error_msg,
                  "La valeur est trop grande pour la colonne %s (taille max=%i).",
                  tmp_entity_def->m_entity_columns_names[tmp_column_index],
                  tmp_entity_def->m_entity_columns_sizes[tmp_column_index]);

              LOG_ERROR(in_session, "%s\n", in_session->m_last_error_msg);
              return net_send_simple_packet(in_session->m_socket,
                  in_session->m_response, OUT_OF_COLUMN_ENTITY_SIZE_LIMIT);
            }
          else
            {
              tmp_columns_names_ptr += sprintf(tmp_columns_names_ptr, "%s,",
                  *tmp_current_key);
              if (tmp_current_column_format && *tmp_current_column_format)
                tmp_columns_values_ptr = nprint_formated_column(
                    tmp_columns_values_ptr, tmp_column_length, tmp_ptr,
                    (*tmp_current_column_format));
              else
                tmp_columns_values_ptr = nprint_formated_column(
                    tmp_columns_values_ptr, tmp_column_length, tmp_ptr,
                    STANDARD_FORMAT);

              tmp_columns_values_ptr += sprintf(tmp_columns_values_ptr, ",");
            }
          tmp_ptr += tmp_column_length;
        }

      tmp_ptr++;
      tmp_column_index++;

      if (tmp_current_column_format)
        tmp_current_column_format++;

      tmp_current_key++;
    }

  if (tmp_columns_names_ptr > in_session->m_columns_names_buffer
      && tmp_columns_values_ptr > in_session->m_columns_values_buffer)
    {
      // Enlever la dernière virgule inutile
      (tmp_columns_names_ptr - 1)[0] = '\0';
      (tmp_columns_values_ptr - 1)[0] = '\0';
      tmp_statement_ptr += sprintf(tmp_statement_ptr,
          "insert into %s (%s) values (%s);", tmp_entity_def->m_entity_name,
          in_session->m_columns_names_buffer,
          in_session->m_columns_values_buffer);
    }
  else
    tmp_statement_ptr
        += sprintf(tmp_statement_ptr, "insert into %s (%s) values (%s);",
            tmp_entity_def->m_entity_name, in_session->m_columns_names_buffer,
            in_session->m_columns_values_buffer);

  return net_db_insert_rows(in_session, tmp_entity_def,
      in_session->m_last_query);
}

int
srv_request_save_record(net_session *in_session, const char *in_request_str)
{
  const char *tmp_ptr = in_request_str;

  entity_def *tmp_entity_def = NULL;
  const char **tmp_current_key = NULL;
  const char **tmp_current_column_format = NULL;

  char *tmp_statement_ptr = in_session->m_last_query;

  int tmp_column_index = 0;
  int tmp_changed_columns_count = 0;

  unsigned long tmp_rows_count = 0;

  int tmp_return = NOERR;

  unsigned int tmp_column_length = 0;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr,
      in_session->m_entity_signature_buffer, SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_entity_signature_buffer))
    {
      LOG_ERROR(in_session, "srv_request_save_record : entity signature is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Chercher le definition de l'entite */
  if (get_table_def(atoi(in_session->m_entity_signature_buffer),
      &tmp_entity_def) < 0)
    {
      LOG_ERROR(in_session, "srv_request_save_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  tmp_current_column_format = tmp_entity_def->m_entity_columns_formats;
  tmp_current_key = tmp_entity_def->m_entity_columns_names;
  if (tmp_current_key == NULL || is_empty_string(*tmp_current_key) == TRUE)
    {
      LOG_ERROR(in_session, "srv_request_save_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  get_where_clause_from_primaries_keys(tmp_entity_def, tmp_ptr,
      in_session->m_where_clause_buffer);

  /* Generer la requête */
  if (is_empty_string(in_session->m_where_clause_buffer) == FALSE)
    {
      tmp_statement_ptr += sprintf(tmp_statement_ptr, "update %s set ",
          tmp_entity_def->m_entity_name);

      // Passer la clé primaire (première colonne)
      tmp_ptr = net_get_field(0, tmp_ptr, in_session->m_column_buffer,
          SEPARATOR_CHAR);

      do
        {
          tmp_current_key++;
          tmp_column_index++;

          if (tmp_current_key != NULL && (*tmp_current_key) != NULL)
            {
              if (tmp_current_column_format)
                tmp_current_column_format++;

              // Recuperer la valeur de la colonne courante
              tmp_column_length = net_find_field(0, &tmp_ptr, SEPARATOR_CHAR);

              if (tmp_column_length > 0)
                {
                  tmp_changed_columns_count++;

                  if (tmp_column_index > 1 && tmp_changed_columns_count > 1)
                    tmp_statement_ptr += sprintf(tmp_statement_ptr, ",");

                  if (strncmp(tmp_ptr, "''", 2) != 0)
                    {
                      if (tmp_entity_def->m_entity_columns_sizes != NULL
                          && tmp_column_length
                              > tmp_entity_def->m_entity_columns_sizes[tmp_column_index])
                        {
                          sprintf(
                              in_session->m_last_error_msg,
                              "La valeur est trop grande pour la colonne %s (taille max=%i).",
                              tmp_entity_def->m_entity_columns_names[tmp_column_index],
                              tmp_entity_def->m_entity_columns_sizes[tmp_column_index]);

                          LOG_ERROR(in_session, "%s\n", in_session->m_last_error_msg);
                          return net_send_simple_packet(in_session->m_socket,
                              in_session->m_response,
                              OUT_OF_COLUMN_ENTITY_SIZE_LIMIT);
                        }
                      else
                        {
                          tmp_statement_ptr += sprintf(tmp_statement_ptr,
                              "%s=", (*tmp_current_key));
                          if (tmp_current_column_format
                              && *tmp_current_column_format)
                            tmp_statement_ptr = nprint_formated_column(
                                tmp_statement_ptr, tmp_column_length, tmp_ptr,
                                (*tmp_current_column_format));
                          else
                            tmp_statement_ptr = nprint_formated_column(
                                tmp_statement_ptr, tmp_column_length, tmp_ptr,
                                STANDARD_FORMAT);
                        }
                    }
                  else
                    tmp_statement_ptr += sprintf(tmp_statement_ptr, "%s=null",
                        *tmp_current_key);

                  tmp_ptr += tmp_column_length;
                }
              tmp_ptr++;
            }
        }
      while (tmp_current_key != NULL && (*tmp_current_key) != NULL);

      tmp_statement_ptr += sprintf(tmp_statement_ptr, " where %s;",
          in_session->m_where_clause_buffer);

      if (tmp_changed_columns_count > 0)
        {
          tmp_return = db_update_rows(in_session, in_session->m_last_query,
              &tmp_rows_count);
          if (tmp_return == NOERR && tmp_rows_count <= 0)
            tmp_return = DB_SQL_NO_ROW_FOUND;

          return net_send_simple_packet(in_session->m_socket,
              in_session->m_response, tmp_return);
        }
      else
        {
          return net_send_simple_packet(in_session->m_socket,
              in_session->m_response, NOERR);
        }
    }
  else
    {
      LOG_ERROR(in_session, "srv_request_save_record : where clause is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }
}

int
srv_request_delete_record(net_session *in_session, const char *in_request_str)
{
  const char *tmp_ptr = in_request_str;

  entity_def *tmp_entity_def = NULL;

  unsigned long tmp_rows_count = 0;
  int tmp_return = NOERR;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr,
      in_session->m_entity_signature_buffer, SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_entity_signature_buffer))
    {
      LOG_ERROR(in_session, "srv_request_delete_record : entity signature is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Chercher le definition de l'entite */
  if (get_table_def(atoi(in_session->m_entity_signature_buffer),
      &tmp_entity_def) < 0)
    {
      LOG_ERROR(in_session, "srv_request_delete_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  get_where_clause_from_primaries_keys(tmp_entity_def, tmp_ptr,
      in_session->m_where_clause_buffer);
  if (is_empty_string(in_session->m_where_clause_buffer))
    {
      LOG_ERROR(in_session, "srv_request_delete_record : where clause is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Generer la requête */
  sprintf(in_session->m_last_query, "delete from %s where %s;",
      tmp_entity_def->m_entity_name, in_session->m_where_clause_buffer);

  /* Executer la requête */
  tmp_return = db_delete_rows(in_session, in_session->m_last_query,
      &tmp_rows_count);
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      tmp_return);
}

int
srv_request_delete_records(net_session *in_session, const char *in_request_str)
{
  const char *tmp_ptr = in_request_str;

  entity_def *tmp_entity_def = NULL;

  unsigned long tmp_rows_count = 0;
  int tmp_return = NOERR;

  /* Verifier les parametres */
  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr,
      in_session->m_entity_signature_buffer, SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_entity_signature_buffer))
    {
      LOG_ERROR(in_session, "srv_request_delete_records : entity signature is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Chercher le definition de l'entite */
  if (get_table_def(atoi(in_session->m_entity_signature_buffer),
      &tmp_entity_def) < 0)
    {
      LOG_ERROR(in_session, "srv_request_delete_records : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, in_session->m_where_clause_buffer,
      SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_where_clause_buffer) == TRUE)
    {
      LOG_ERROR(in_session, "srv_request_delete_records : where clause is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Generer la requête */
  sprintf(in_session->m_last_query, "delete from %s where %s;",
      tmp_entity_def->m_entity_name, in_session->m_where_clause_buffer);

  /* Executer la requête */
  tmp_return = db_delete_rows(in_session, in_session->m_last_query,
      &tmp_rows_count);
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      tmp_return);
}

#ifdef _LDAP
int srv_request_load_ldap_entries( net_session *in_session, const char *in_request_str )
  {

  }

int srv_request_load_ldap_entry( net_session *in_session, const char *in_request_str )
  {

  }

#endif

int
srv_request_get_error_message(net_session *in_session,
    const char *in_request_str)
{
  const char *tmp_ptr = in_request_str;
  char tmp_error_id_str[16];

  int tmp_error_id = 0;
  int tmp_nb_bytes = 0;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr,
      tmp_error_id_str, SEPARATOR_CHAR);
  if (is_empty_string(tmp_error_id_str))
    {
      LOG_ERROR(in_session, "srv_request_get_error_message : error id is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_error_id = atoi(tmp_error_id_str);
  switch (tmp_error_id)
    {
  case DB_SQL_ERROR:
#if (defined(_WINDOWS) || defined(WIN32))
    tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%s\n%s%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, get_error_message(tmp_error_id), in_session->m_last_error_msg, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#else
    tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%s\n%s%c%c",
        PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR,
        get_error_message(tmp_error_id), in_session->m_last_error_msg,
        SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#endif
    break;

  case OUT_OF_COLUMN_ENTITY_SIZE_LIMIT:
#if (defined(_WINDOWS) || defined(WIN32))
    tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%s%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, in_session->m_last_error_msg, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#else
    tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%s%c%c",
        PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR,
        in_session->m_last_error_msg, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#endif
    break;

  default:
#if (defined(_WINDOWS) || defined(WIN32))
    tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%s%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, get_error_message(tmp_error_id), SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#else
    tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%s%c%c",
        PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR,
        get_error_message(tmp_error_id), SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#endif
    }

  return net_send_packet(in_session->m_socket, in_session->m_response,
      tmp_nb_bytes);
}

int
srv_request_add_user(net_session *in_session, const char *in_request_str)
{
  const char *tmp_ptr = in_request_str;

  char tmp_username[64];
  char tmp_password[64];
  char tmp_rolename[64];

  int tmp_return = NOERR;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr, tmp_username,
      SEPARATOR_CHAR);
  if (is_empty_string(tmp_username))
    {
      LOG_ERROR(in_session, "srv_request_add_user : username is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_password, SEPARATOR_CHAR);
  if (is_empty_string(tmp_password))
    {
      LOG_ERROR(in_session, "srv_request_add_user : password is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_rolename, SEPARATOR_CHAR);

  tmp_return = db_add_user(in_session, tmp_username, decrypt_str(tmp_password),
      tmp_rolename);
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      tmp_return);
}

int
srv_request_remove_user(net_session *in_session, const char *in_request_str)
{
  const char *tmp_ptr = in_request_str;
  char tmp_username[64];
  int tmp_return = NOERR;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr, tmp_username,
      SEPARATOR_CHAR);
  if (is_empty_string(tmp_username))
    {
      LOG_ERROR(in_session, "srv_request_remove_user : username is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  sprintf(in_session->m_last_query, "drop user %s;", tmp_username);

  tmp_return = db_exec_cmd(in_session, in_session->m_last_query);
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      tmp_return);
}

int
srv_request_modify_user_password(net_session *in_session,
    const char *in_request_str)
{
  const char *tmp_ptr = in_request_str;
  char tmp_username[64];
  char tmp_password[64];
  int tmp_return = NOERR;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr, tmp_username,
      SEPARATOR_CHAR);
  if (is_empty_string(tmp_username))
    {
      LOG_ERROR(in_session, "srv_request_modify_user_password : username is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_password, SEPARATOR_CHAR);
  if (is_empty_string(tmp_password))
    {
      LOG_ERROR(in_session, "srv_request_modify_user_password : password is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_return = db_modify_user_password(in_session, tmp_username, tmp_password);
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      tmp_return);
}

int
srv_request_add_role_to_user(net_session *in_session,
    const char *in_request_str)
{
  const char *tmp_ptr = in_request_str;
  char tmp_username[64];
  char tmp_rolename[64];
  int tmp_return = NOERR;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr, tmp_username,
      SEPARATOR_CHAR);
  if (is_empty_string(tmp_username))
    {
      LOG_ERROR(in_session, "srv_request_add_role_to_user : username is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_rolename, SEPARATOR_CHAR);
  if (is_empty_string(tmp_rolename))
    {
      LOG_ERROR(in_session, "srv_request_add_role_to_user : role name is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  sprintf(in_session->m_last_query, "grant %s to %s;", tmp_rolename,
      tmp_username);

  tmp_return = db_exec_cmd(in_session, in_session->m_last_query);
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      tmp_return);
}

int
srv_request_remove_role_from_user(net_session *in_session,
    const char *in_request_str)
{
  const char *tmp_ptr = in_request_str;
  char tmp_username[64];
  char tmp_rolename[64];
  int tmp_return = NOERR;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr, tmp_username,
      SEPARATOR_CHAR);
  if (is_empty_string(tmp_username))
    {
      LOG_ERROR(in_session, "srv_request_remove_role_from_user : username is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_rolename, SEPARATOR_CHAR);
  if (is_empty_string(tmp_rolename))
    {
      LOG_ERROR(in_session, "srv_request_remove_role_from_user : role name is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  sprintf(in_session->m_last_query, "revoke %s from %s;", tmp_rolename,
      tmp_username);

  tmp_return = db_exec_cmd(in_session, in_session->m_last_query);
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      tmp_return);
}

int
srv_request_has_user_this_role(net_session *in_session,
    const char *in_request_str)
{
  int tmp_return_value = FALSE;

  const char *tmp_ptr = in_request_str;
  char tmp_username[64];
  char tmp_rolename[64];
  int tmp_nb_bytes = 0;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr, tmp_username,
      SEPARATOR_CHAR);
  if (is_empty_string(tmp_username))
    {
      LOG_ERROR(in_session, "srv_request_remove_role_from_user : username is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_rolename, SEPARATOR_CHAR);
  if (is_empty_string(tmp_rolename))
    {
      LOG_ERROR(in_session, "srv_request_remove_role_from_user : role name is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_return_value = db_has_user_this_role(in_session, tmp_username,
      tmp_rolename);

#if (defined(_WINDOWS) || defined(WIN32))
  tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%i%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_return_value, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#else
  tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%i%c%c",
      PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR,
      tmp_return_value, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#endif
  return net_send_packet(in_session->m_socket, in_session->m_response,
      tmp_nb_bytes);

}

int
srv_request_current_user(net_session *in_session)
{
  char ***tmp_returned_rows = NULL;
  unsigned long tmp_rows_count = 0;
  unsigned long tmp_columns_count = 0;
  int tmp_nb_bytes = 0;

  sprintf(in_session->m_last_query, SELECT_CURRENT_USER);
  tmp_returned_rows = db_do_query(in_session, in_session->m_last_query,
      &tmp_rows_count, &tmp_columns_count);
  if (tmp_returned_rows && tmp_rows_count == 1 && tmp_columns_count == 1)
    {
#if (defined(_WINDOWS) || defined(WIN32))
      tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%s%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_returned_rows[0][0], SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#else
      tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%s%c%c",
          PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR,
          tmp_returned_rows[0][0], SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#endif
    }
  else
#if (defined(_WINDOWS) || defined(WIN32))
    tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#else
    tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%c%c",
        PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR,
        SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#endif

  free_rows_and_columns(&tmp_returned_rows, tmp_rows_count, tmp_columns_count);

  return net_send_packet(in_session->m_socket, in_session->m_response,
      tmp_nb_bytes);
}

int
srv_request_put_blob(net_session *in_session)
{
  return net_db_put_blob(in_session);
}

int
srv_request_get_blob(net_session *in_session)
{
  return net_db_get_blob(in_session);
}

int
srv_request_delete_blob(net_session *in_session)
{
  return net_db_delete_blob(in_session);
}

int
srv_request_lock_record(net_session *in_session, const char *in_request_str)
{
  char *tmp_statement_ptr = in_session->m_last_query;

  char tmp_primary_key_value[PRIMARY_KEY_SIZE];
  char tmp_check_sync_str[2];
  const char *tmp_ptr = in_request_str;

  char ***tmp_current_columns_values = NULL;

  const char **tmp_keys_columns = NULL;

  unsigned long tmp_rows_count = 0;
  unsigned long tmp_columns_count = 0;

  unsigned long tmp_columns_index = 0;

  entity_def *tmp_entity_def = NULL;

  int tmp_status = NOERR;
  int tmp_nb_bytes = 0;
  int tmp_username_length = 0;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr,
      in_session->m_entity_signature_buffer, SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_entity_signature_buffer))
    {
      LOG_ERROR(in_session, "srv_request_lock_record : entity signature is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Chercher le definition de l'entite */
  if (get_table_def(atoi(in_session->m_entity_signature_buffer),
      &tmp_entity_def) < 0)
    {
      LOG_ERROR(in_session, "srv_request_lock_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  tmp_keys_columns = tmp_entity_def->m_entity_columns_names;
  if (tmp_keys_columns == NULL || is_empty_string(*tmp_keys_columns) == TRUE)
    {
      LOG_ERROR(in_session, "srv_request_lock_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_primary_key_value, SEPARATOR_CHAR);
  if (is_empty_string(tmp_primary_key_value))
    {
      LOG_ERROR(in_session, "srv_request_lock_record : primary key is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_check_sync_str, SEPARATOR_CHAR);
  if (is_empty_string(tmp_check_sync_str))
    {
      LOG_ERROR(in_session, "srv_request_lock_record : check sync is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Vérifier la synchronisation des données avant positionnement du verrou */
  if (atoi(tmp_check_sync_str) == TRUE)
    {
      /* Generer la requête */
      tmp_statement_ptr += sprintf(tmp_statement_ptr, "select %s",
          *tmp_keys_columns);
      while ((*(++tmp_keys_columns)) != NULL)
        tmp_statement_ptr += sprintf(tmp_statement_ptr, ",%s",
            *tmp_keys_columns);

      tmp_statement_ptr += sprintf(tmp_statement_ptr, " from %s where %s=%s;",
          tmp_entity_def->m_entity_name,
          *tmp_entity_def->m_entity_columns_names, tmp_primary_key_value);

      /* Executer la requête */
      tmp_current_columns_values = db_do_query(in_session,
          in_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
      if (tmp_current_columns_values != NULL && tmp_rows_count == 1)
        {
          /* Vérifier la synchronisation des données colonne par colonne de l'enregistrement à verrouiller */
          for (tmp_columns_index = 0; tmp_columns_index < tmp_columns_count; tmp_columns_index++)
            {
              tmp_ptr = net_get_field(0, tmp_ptr, in_session->m_column_buffer,
                  SEPARATOR_CHAR);
              if (tmp_ptr != NULL)
                {
                  if (compare_values(in_session->m_column_buffer,
                      tmp_current_columns_values[0][tmp_columns_index]) != 0)
                    {
                      tmp_status = DB_RECORD_OUT_OF_SYNC;
                      break;
                    }
                }
            }
        }
      else
        tmp_status = DB_RECORD_OUT_OF_SYNC;

      free_rows_and_columns(&tmp_current_columns_values, tmp_rows_count,
          tmp_columns_count);
    }

  if (tmp_status == NOERR)
    {
      tmp_status = db_lock_row(in_session,
          tmp_entity_def->m_entity_signature_id, tmp_primary_key_value);
      if (tmp_status == DB_RECORD_LOCK_UNAVAILABLE)
        {
          db_get_username_for_lock_row(in_session,
              tmp_entity_def->m_entity_signature_id, tmp_primary_key_value,
              in_session->m_column_buffer, &tmp_username_length);

          tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%s%c%c",
              PAQUET_START_CHAR, SEPARATOR_CHAR, tmp_status, SEPARATOR_CHAR,
              in_session->m_column_buffer, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
          return net_send_packet(in_session->m_socket, in_session->m_response,
              tmp_nb_bytes);
        }
    }

  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      tmp_status);
}

int
srv_request_unlock_record(net_session *in_session, const char *in_request_str)
{
  char tmp_primary_key_value[PRIMARY_KEY_SIZE];
  const char *tmp_ptr = in_request_str;

  entity_def *tmp_entity_def = NULL;

  int tmp_status = NOERR;

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr,
      in_session->m_entity_signature_buffer, SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_entity_signature_buffer))
    {
      LOG_ERROR(in_session, "srv_request_unlock_record : entity signature is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  /* Chercher le definition de l'entite */
  if (get_table_def(atoi(in_session->m_entity_signature_buffer),
      &tmp_entity_def) < 0)
    {
      LOG_ERROR(in_session, "srv_request_unlock_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  if (tmp_entity_def->m_entity_columns_names == NULL || is_empty_string(
      *tmp_entity_def->m_entity_columns_names) == TRUE)
    {
      LOG_ERROR(in_session, "srv_request_unlock_record : unknow entity\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, UNKNOW_ENTITY);
    }

  tmp_ptr = net_get_field(0, tmp_ptr, tmp_primary_key_value, SEPARATOR_CHAR);
  if (is_empty_string(tmp_primary_key_value))
    {
      LOG_ERROR(in_session, "srv_request_unlock_record : primary key is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_status = db_unlock_row(in_session, tmp_entity_def->m_entity_signature_id,
      tmp_primary_key_value);

  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      tmp_status);
}

int
srv_request_date_time(net_session *in_session)
{
  char ***tmp_returned_rows = NULL;
  unsigned long tmp_rows_count = 0;
  unsigned long tmp_columns_count = 0;
  int tmp_nb_bytes = 0;

  sprintf(in_session->m_last_query, SELECT_CURRENT_DATE_TIME);
  tmp_returned_rows = db_do_query(in_session, in_session->m_last_query,
      &tmp_rows_count, &tmp_columns_count);
  if (tmp_returned_rows && tmp_rows_count == 1 && tmp_columns_count == 1)
    {
#if (defined(_WINDOWS) || defined(WIN32))
      tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%s%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, tmp_returned_rows[0][0], SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#else
      tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%s%c%c",
          PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR,
          tmp_returned_rows[0][0], SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#endif
    }
  else
#if (defined(_WINDOWS) || defined(WIN32))
    tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%c%c", PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR, SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#else
    tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%i%c%c%c",
        PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR,
        SEPARATOR_CHAR, PAQUET_STOP_CHAR);
#endif

  free_rows_and_columns(&tmp_returned_rows, tmp_rows_count, tmp_columns_count);

  return net_send_packet(in_session->m_socket, in_session->m_response,
      tmp_nb_bytes);
}

int
srv_request_transaction_start(net_session *in_session)
{
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      db_transaction_start(in_session));
}

int
srv_request_transaction_commit(net_session *in_session)
{
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      db_transaction_commit(in_session));
}

int
srv_request_transaction_rollback(net_session *in_session)
{
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      db_transaction_rollback(in_session));
}

int
srv_request_server_infos(net_session *in_session)
{
  char ***tmp_returned_rows = NULL;
  unsigned long tmp_rows_count = 0;
  unsigned long tmp_columns_count = 0;
  int tmp_database_version_retrieve = FALSE;
  int tmp_db_server_version = 0;
  int tmp_nb_bytes = 0;

  sprintf(in_session->m_last_query, SELECT_DATABASE_VERSION);
  tmp_returned_rows = db_do_query(in_session, in_session->m_last_query,
      &tmp_rows_count, &tmp_columns_count);
  tmp_database_version_retrieve = (tmp_returned_rows && tmp_rows_count == 1
      && tmp_columns_count == 1 && is_empty_string(tmp_returned_rows[0][0])
      == FALSE);

  tmp_db_server_version = db_server_version(in_session);

#if (defined(_WINDOWS) || defined(WIN32))
  tmp_nb_bytes = sprintf(in_session->m_response, "%c%c%li%c%d.%d.%d.%d%c%d.%d.%d.%d%c%s%c%d.%d.%d%c%c",
#else
  tmp_nb_bytes = sprintf(in_session->m_response,
      "%c%c%i%c%d.%d.%d.%d%c%d.%d.%d.%d%c%s%c%d.%d.%d%c%c",
#endif
      PAQUET_START_CHAR, SEPARATOR_CHAR, NOERR, SEPARATOR_CHAR,
      SERVER_VERSION >> 24, (SERVER_VERSION >> 16) & 0xFF,
      (SERVER_VERSION >> 8) & 0xFF, SERVER_VERSION & 0xFF, SEPARATOR_CHAR,
      PROTOCOL_VERSION >> 24, (PROTOCOL_VERSION >> 16) & 0xFF,
      (PROTOCOL_VERSION >> 8) & 0xFF, PROTOCOL_VERSION & 0xFF, SEPARATOR_CHAR,
      tmp_database_version_retrieve ? tmp_returned_rows[0][0] : "", SEPARATOR_CHAR,
	  (tmp_db_server_version >> 16) & 0xFF,
	  (tmp_db_server_version >> 8) & 0xFF,
	  tmp_db_server_version & 0xFF,
      SEPARATOR_CHAR, PAQUET_STOP_CHAR);

  if (tmp_returned_rows)
    free_rows_and_columns(&tmp_returned_rows, tmp_rows_count, tmp_columns_count);

  return net_send_packet(in_session->m_socket, in_session->m_response,
      tmp_nb_bytes);
}

int
srv_request_prepare_results_set(net_session *in_session,
    const char *in_request_str)
{
  return srv_request_load_records(in_session, in_request_str, TRUE);
}

int
srv_request_get_last_results_set(net_session *in_session,
    const char *in_request_str)
{
  const char *tmp_ptr = in_request_str;

  unsigned long tmp_start_row_index = 0;
  unsigned long tmp_rows_count = 0;

  if (in_session->m_last_results_set == NULL)
    {
      LOG_ERROR(in_session, "srv_request_get_last_results_set : there isn't results set\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }

  tmp_ptr = net_get_field(NET_MESSAGE_TYPE_INDEX + 1, tmp_ptr,
      in_session->m_column_buffer, SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_column_buffer))
    {
      LOG_ERROR(in_session, "srv_request_get_last_results_set : start row index is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }
  tmp_start_row_index = strtoul(in_session->m_column_buffer, NULL, 10);

  tmp_ptr = net_get_field(0, tmp_ptr, in_session->m_column_buffer,
      SEPARATOR_CHAR);
  if (is_empty_string(in_session->m_column_buffer))
    {
      LOG_ERROR(in_session, "srv_request_get_last_results_set : rows count is empty\n");
      return net_send_simple_packet(in_session->m_socket,
          in_session->m_response, EMPTY_OBJECT);
    }
  tmp_rows_count = strtoul(in_session->m_column_buffer, NULL, 10);

  /* Charger l'ensemble des resultats */
  return net_db_do_query(in_session, NULL, TRUE, tmp_start_row_index,
      tmp_rows_count);
}

int
srv_request_clear_last_results_set(net_session *in_session)
{
  db_clear_last_results_set(in_session);
  return net_send_simple_packet(in_session->m_socket, in_session->m_response,
      NOERR);
}

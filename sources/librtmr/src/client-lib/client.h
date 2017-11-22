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

/** \file client.h
* This file contains all the functions used to communicate with the gateway server.<br>
*/

#ifndef CLIENT_H_
#define CLIENT_H_

#include "../common/netcommon.h"
#include "../common/utilities.h"

#ifdef __cplusplus
extern "C"
{
#endif

	void cl_close_connexion(net_session *in_session);

	int cl_connect_db(net_session *in_session);

	int cl_connect_netserver(net_session *in_session);

	int cl_send_simple_request(net_session *in_session, int in_packet_type, int in_log_trace);

	int cl_send_request(net_session *in_session, int in_nb_bytes);

	DLLEXPORT int DLLCALL cl_connect(net_session **in_session, const char* in_hostname, int in_port, const char* in_username, const char* in_passwd, int in_debug, const char* in_log_trace_file_name, int in_log_level, long in_protocol_version , long in_recv_timeout, long in_send_timeout);

        DLLEXPORT int DLLCALL cl_disconnect(net_session **in_session);

	DLLEXPORT int DLLCALL cl_ping_server (net_session *in_session);

        DLLEXPORT char*** DLLCALL cl_run_sql( net_session *in_session, const char* in_statement, unsigned long *out_rows_count, unsigned long *out_columns_count );

        DLLEXPORT int DLLCALL cl_call_sql_procedure( net_session *in_session, const char* in_procedure_name, char** in_out_parameters );

        DLLEXPORT int DLLCALL cl_load_record( net_session *in_session, int in_table_signature, char **in_out_columns_record, const char *in_primary_key_value );

        DLLEXPORT char*** DLLCALL cl_load_records( net_session *in_session, int in_table_signature, const char *in_where_clause, const char *in_order_by_clause, unsigned long *in_out_records_count , net_callback_fct *in_callback);

        DLLEXPORT int DLLCALL cl_insert_record( net_session *in_session, int in_table_signature, char** in_out_columns_record );

        DLLEXPORT int DLLCALL cl_save_record( net_session *in_session, int in_table_signature, char** in_out_columns_record, char** in_originals_columns_record );

        DLLEXPORT int DLLCALL cl_delete_record( net_session *in_session, int in_table_signature, char** in_columns_record );

        DLLEXPORT int DLLCALL cl_delete_records( net_session *in_session, int in_table_signature, const char* in_where_clause );

#ifdef _LDAP
        DLLEXPORT char*** DLLCALL cl_load_ldap_entries( net_session *in_session, int in_entry_signature, const char* in_ldap_url, const char* in_distinguish_name, const char* in_credencial, const char* in_base_query, const char* in_filter_query );

        DLLEXPORT int DLLCALL cl_load_ldap_entry( net_session *in_session, int in_entry_signature, const char* in_ldap_url, const char* in_distinguish_name, const char* in_credencial, char** in_out_columns_record, const char* in_base_query );
#endif

        DLLEXPORT const char*  DLLCALL cl_get_error_message( net_session *in_session, int in_error_id );

        DLLEXPORT int DLLCALL cl_add_user(net_session *in_session, const char *in_username, const char *in_password, const char *in_rolename);

        DLLEXPORT int DLLCALL cl_remove_user(net_session *in_session, const char *in_username);

	DLLEXPORT int DLLCALL cl_modify_user_password(net_session *in_session, const char *in_username, const char *in_new_password);

	DLLEXPORT int DLLCALL cl_add_role_to_user(net_session *in_session, const char *in_rolename, const char *in_username);

        DLLEXPORT int DLLCALL cl_remove_role_from_user(net_session *in_session, const char *in_rolename, const char *in_username);

        DLLEXPORT int DLLCALL cl_has_user_this_role(net_session *in_session, const char *in_username, const char *in_rolename);

        DLLEXPORT char* DLLCALL cl_current_user(net_session *in_session);

        DLLEXPORT long DLLCALL cl_api_protocol_version();

        /* Binary Large OBjects */
        DLLEXPORT int DLLCALL cl_put_blob_from_buffer(net_session *in_session, const char *in_buffer, long in_buffer_size, char *out_file_id_str);

        DLLEXPORT int DLLCALL cl_get_blob_to_buffer(net_session *in_session, const char *in_blob_id, char **in_buffer, long *in_buffer_size);

        DLLEXPORT int DLLCALL cl_put_blob(net_session *in_session, const char *in_filename, char *out_file_id_str);

        DLLEXPORT int DLLCALL cl_get_blob(net_session *in_session, const char *in_blob_id, const char *in_filename);

        DLLEXPORT int DLLCALL cl_delete_blob(net_session *in_session, const char *in_blob_id);

	/* Memory management */
	DLLEXPORT void DLLCALL cl_free_rows_columns_array(char ****in_rows_columns, int in_rows_count, int in_columns_count);

	DLLEXPORT char** DLLCALL cl_alloc_columns_array(int in_columns_count);

        DLLEXPORT void DLLCALL cl_free_columns_array(char ***in_columns_array_ptr, int in_columns_count);

	DLLEXPORT char* DLLCALL cl_alloc_str(int in_str_length);

	DLLEXPORT void DLLCALL cl_free_str(char **in_str_ptr);

	/* Gestion des verrous */
	DLLEXPORT int DLLCALL cl_lock_record( net_session *in_session, int in_table_signature, const char *in_primary_key_value, int in_check_sync, char** in_columns_record );

	DLLEXPORT int DLLCALL cl_unlock_record( net_session *in_session, int in_table_signature, const char *in_primary_key_value );

	/* Gestion des transactions */
	DLLEXPORT int DLLCALL cl_transaction_start( net_session *in_session );

	DLLEXPORT int DLLCALL cl_transaction_commit( net_session *in_session );

	DLLEXPORT int DLLCALL cl_transaction_rollback( net_session *in_session );

	DLLEXPORT int DLLCALL cl_get_server_infos( net_session *in_session );

	DLLEXPORT int DLLCALL cl_get_last_results_set(net_session *in_session, entity_def *in_entity_def, char ***in_last_results_set, unsigned long long int in_start_row_index, unsigned long long int *in_rows_count, net_callback_fct *in_callback );

	DLLEXPORT int DLLCALL cl_clear_last_results_set(net_session *in_session);

#ifdef __cplusplus
}
#endif


#endif /*CLIENT_H_*/

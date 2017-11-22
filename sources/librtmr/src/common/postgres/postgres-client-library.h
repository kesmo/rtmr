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

#ifndef EJPOSTGRESLIBRARY_H_
#define EJPOSTGRESLIBRARY_H_

#include <libpq-fe.h>
#include <libpq/libpq-fs.h>
#include "../../common/netcommon.h"
#include "../../common/entities.h"
#include "../../common/constants.h"

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************************	*
*	db_start																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Connection à la base de données. 							*
************************************************************************************/
int db_start(net_session *in_session, const char *hostname, unsigned int port, const char *dbname, const char *username, const char *passwd, char** in_server_args);

/***********************************************************************************	*
*	db_die																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Déconnexion de la base de donnée					. 							*
************************************************************************************/
void db_die(net_session *in_session);

/***********************************************************************************	*
*	db_connect																																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Connection à une base de donnée					. 																				*
************************************************************************************/
void *db_connect(const char *hostname, unsigned int port, const char *in_dbname, const char *in_username, const char *in_passwd);

/***********************************************************************************	*
*	db_disconnect																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Déconnexion d'une base de donnée					. 																			*
************************************************************************************/
void db_disconnect(net_session *in_session);

/***********************************************************************************	*
*	db_do_query																																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Exécuter une requête												. 																			*
************************************************************************************/
char ***db_do_query(net_session *in_session, const char *in_query, unsigned long *out_rows_count, unsigned long *out_columns_count);
int net_db_do_query(net_session *in_session, const char *in_query, int in_prepare_results_set, unsigned long in_start_result_set_index, unsigned long in_results_set_count);

/***********************************************************************************	*
*	db_exec_cmd																																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Exécuter une commande												. 																			*
*************************************************************************************/
int db_exec_cmd(net_session *in_session, const char *in_cmd);

/***********************************************************************************	*
*	db_call_procedure																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Exécuter une procedure											. 																			*
************************************************************************************/
void db_call_procedure(net_session *in_session, const char *in_procedure_name, int in_param_count, char **in_out_parameters);
int net_db_call_procedure(net_session *in_session, const char *in_request_str);

/***********************************************************************************	*
*	net_db_insert_rows																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Insérertion de données											. 																			*
************************************************************************************/
int net_db_insert_rows(net_session *in_session, entity_def *in_entity_def, const char *in_insert_query);

/***********************************************************************************	*
*	db_delete_rows																																		*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Suppression de données											. 																			*
************************************************************************************/
int db_delete_rows(net_session *in_session, const char *in_delete_query, unsigned long *in_out_affected_rows_count);

/***********************************************************************************	*
*	db_update_rows																																		*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Mise à jour de données											. 																				*
************************************************************************************/
int db_update_rows(net_session *in_session, const char *in_update_query, unsigned long *in_out_affected_rows_count);

/***********************************************************************************	*
*	db_add_user																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Ajout d'un compte utilisateur. 							*
*************************************************************************************/
int db_add_user(net_session *in_session, const char *in_username, const char *in_password, const char *in_rolename);

/***********************************************************************************	*
*	db_modify_user_password																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Modifier le mot de passe d'un utilisateur. 							*
*************************************************************************************/
int db_modify_user_password(net_session *in_session, const char *in_username, const char *in_new_password);

void get_where_clause_from_primaries_keys(entity_def *in_entity_def, const char *in_data_str, char *in_where_clause);

/***********************************************************************************	*
*	db_lock_row																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Verrouiller un enregistrement. 							*
*************************************************************************************/
int db_lock_row(net_session *in_session, int in_entity_signature_id, const char *in_primary_key_value);

/***********************************************************************************	*
*	db_unlock_row																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Deverrouiller un enregistrement. 							*
*************************************************************************************/
int db_unlock_row(net_session *in_session, int in_entity_signature_id, const char *in_primary_key_value);

/***********************************************************************************	*
*	db_get_username_for_lock_row																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Renvoie le nom de l'utilisateur verrouillant un enregistrement. 							*
***********************************************************************************	*/
int db_get_username_for_lock_row(net_session *in_session, int in_entity_signature_id, const char *in_primary_key_value, char* in_out_username, int *in_out_username_length);

/***********************************************************************************	*
*	db_transaction_start																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Debute une transaction. 							*
***********************************************************************************	*/
int db_transaction_start(net_session *in_session);

/***********************************************************************************	*
*	db_transaction_commit																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Valide une transaction. 							*
***********************************************************************************	*/
int db_transaction_commit(net_session *in_session);

/***********************************************************************************	*
*	db_transaction_rollback																																					*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Annule une transaction. 							*
***********************************************************************************	*/
int db_transaction_rollback(net_session *in_session);

/***********************************************************************************	*
*	db_clear_last_results_set																																					*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Vider le dernier ensemble de resultats. 							*
***********************************************************************************	*/
void db_clear_last_results_set(net_session *in_session);

/***********************************************************************************	*
*	db_has_user_this_role																																					*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Verifier si un utilisateur à un rôle particulier. 							*
***********************************************************************************	*/
int db_has_user_this_role(net_session *in_session, const char *in_username, const char *in_rolename);

/***********************************************************************************	*
*	db_server_version																																					*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Renvoie la version du serveur de la base de données. 							*
***********************************************************************************	*/
int db_server_version(net_session *in_session);

int net_db_put_blob(net_session *in_session);
int net_db_get_blob(net_session *in_session);
int net_db_delete_blob(net_session *in_session);

#ifdef __cplusplus
}
#endif

#endif /*EJPOSTGRESLIBRARY_H_*/

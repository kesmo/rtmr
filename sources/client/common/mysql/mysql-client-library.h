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

#ifndef EJMYSQLLIBRARY_H_
#define EJMYSQLLIBRARY_H_

#include <mysql.h>
#include "../entities.h"

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************************	*
*	db_start																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Initialisation du serveur mysql et de la connexion à la base de données. 							*
************************************************************************************/
void *db_start(const char *hostname, unsigned int port, const char *dbname, const char *username, const char *passwd, char** in_server_args);

/***********************************************************************************	*
*	db_die																																						*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Déconnexion de la base de donnée et arrêt du serveur mysql					. 							*
************************************************************************************/
void db_die(void *in_db, char *in_fmt, ...);

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
void db_disconnect(void *in_db);

/***********************************************************************************	*
*	db_do_query																																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Exécuter une requête												. 																			*
************************************************************************************/
char ***db_do_query(void *in_db, const char *in_query, unsigned long *out_rows_count, unsigned long *out_columns_count);

/***********************************************************************************	*
*	db_call_procedure																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Exécuter une procedure											. 																			*
************************************************************************************/
void db_call_procedure(void *in_db, const char *in_procedure_name, int in_param_count, char **in_out_parameters);

/***********************************************************************************	*
*	db_insert_rows																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Insérertion de données											. 																			*
************************************************************************************/
int db_insert_rows(void *in_db, entity_def *in_entity_def, const char *in_insert_query, unsigned long *in_out_affected_rows_count, unsigned long *in_out_last_pk_value);

/***********************************************************************************	*
*	db_delete_rows																																		*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Suppression de données											. 																			*
************************************************************************************/
int db_delete_rows(void *in_db, const char *in_delete_query, unsigned long *in_out_affected_rows_count);

/***********************************************************************************	*
*	db_update_rows																																		*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Mise à jour de données											. 																				*
************************************************************************************/
int db_update_rows(void *in_db, const char *in_insert_query, unsigned long *in_out_affected_rows_count);

#ifdef __cplusplus
}
#endif

#endif /*EJMYSQLLIBRARY_H_*/

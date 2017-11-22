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

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
  {
#endif

#ifdef TRUE
#undef TRUE
#endif

#define		TRUE																		1

#ifdef FALSE
#undef FALSE
#endif
#define		FALSE																		0


#define		MAX_PATH_LENGTH												512


#define		MAX_BUFFER_SIZE												1024

#define		PRIMARY_KEY_SIZE											16
#define		DATE_SIZE													32
#define		NUMBER_SIZE													32
#define		SHORT_TEXT_SIZE												64
#define		SMEDIUM_TEXT_SIZE											128
#define		LMEDIUM_TEXT_SIZE											256
#define		SLARGE_TEXT_SIZE											512
#define		S1LARGE_TEXT_SIZE											1024
#define		S2LARGE_TEXT_SIZE											2048
#define		LARGE_TEXT_SIZE												4096
#define		MLARGE_TEXT_SIZE											8192
#define		LLARGE_TEXT_SIZE											16384
#define		HUGE_TEXT_SIZE												32768

#define		MAX_APP_PARAM_LINE_SIZE								1024

#define		MAX_SQL_STATEMENT_LENGTH							65536
#define		MAX_SQL_WHERE_CLAUSE_LENGTH					1024
#define		MAX_SQL_ORDER_BY_CLAUSE_LENGTH			1024

#define		MAX_ROWS_BY_QUERY										1024
#define		MAX_COLUMN_LENGTH										HUGE_TEXT_SIZE

#define		NO																	"N"
#define		YES																	"Y"
#define		COLUMN_SEPARATOR_STR												"|"

#define		NUMBER_FORMAT														"%s"

#define		CLIENT_MACRO_NOW													"NOW"

#ifdef _POSTGRES
#define		STRING_FORMAT													"E'%s'"
#define		DATE_FORMAT														"date('%s')"
#define		MD5_FORMAT														"MD5(E'%s')"
#define		BINARY_FORMAT													"E'%s'::bytea"

#define		NOW																		"now()"

#define		SELECT_CURRENT_USER												"select current_user;"
#define		SELECT_CURRENT_DATE_TIME										"select now();"

#else

#ifdef _MYSQL

#define		STRING_FORMAT													"'%s'"
#define		DATE_FORMAT														"to_date('%s')"
#define		MD5_FORMAT														"MD5('%s')"

#define		NOW																"sysdate"

#define		SELECT_CURRENT_USER												""
#define		SELECT_CURRENT_DATE_TIME										""

#endif

#endif

#define		SELECT_DATABASE_VERSION											"select database_version_number from database_version_table;"

#define		STANDARD_FORMAT												STRING_FORMAT

#ifdef NOERR
#undef NOERR
#endif

#define		NOERR																0

#if (defined(_WINDOWS) || defined(WIN32))
#define					FILE_SEPARATOR						'\\'
#else
#define					FILE_SEPARATOR						'/'
#endif

#define		MEM_ALLOC_ERROR												-10
#define		NO_LICENSE_AVAILABLE										-20
#define		LICENSE_CONNEXIONS_COUNT_ERROR			-21
#define		LICENSE_TIME_ERROR											-22
#define		NET_INIT_CONNEXION											-99
#define		NET_CONNEXION_ERROR									-100
#define		NET_SOCKET_ERROR											-101
#define		NET_GET_HOST_NAME_ERROR							-102
#define		NET_GET_HOST_BY_NAME_ERROR						-103
#define		NET_RCV_PAQUET_ERROR									-104
#define		NET_SEND_PAQUET_ERROR									-105
#define		NET_RCV_ACK_ERROR											-106
#define		NET_SEND_ACK_ERROR										-107
#define		NET_OUT_OF_REQUEST_LIMIT_SIZE_ERROR	-108
#define		NET_OUT_OF_RESPONSE_LIMIT_SIZE_ERROR	-109
#define		NET_SEND_TIMEOUT											-110
#define		NET_REQUEST_UNKNOW										-111
#define		NET_RESULTS_SET_INDEX_OUT_OF_BOUND						-112
#define		NET_PROTOCOL_VERSION_INCOMPATIBLE						-113
#define		NET_RECV_TIMEOUT											-114
#define		NET_LIB_PROTOCOL_VERSION_INCOMPATIBLE						-115
#define		NET_CONNEXION_CLOSED									-116

#define		DB_CONNEXION_ERROR										-1000
#define		DB_UNKNOW_USER_ERROR								-1001
#define		DB_INCORRECT_PASSWORD_ERROR					-1002

#define		DB_USER_ACCOUNT_CREATION_ERROR			-1003
#define		DB_USER_ACCOUNT_ACTIVATION_ERROR		-1004
#define		DB_USER_ACCOUNT_ALREADY_ACTIVATED		-1005

#define		DB_SQL_FUNCTION_ERROR								-1006
#define		DB_SQL_NO_ROW_FOUND									-1007
#define		DB_SQL_TOO_MUCH_ROWS_FOUND				-1008
#define		DB_SQL_ILLEGAL_STATEMENT							-1009
#define		DB_SQL_ERROR														-1010

#define		DB_RECORD_LOCK_UNAVAILABLE						-1020
#define		DB_RECORD_UNLOCK_UNAVAILABLE					-1021
#define		DB_RECORD_OUT_OF_SYNC								-1022

#define		DB_USER_ALREADY_EXISTS									-1100

#define		DB_FILE_ACCESS_DENIED									-1200

#define		DB_GROUP_DOESNT_OWN_GROUP					-1300
#define		DB_GROUP_DOESNT_OWN_USER						-1301

#define		UNKNOW_ENTITY													-2000
#define		EMPTY_OBJECT														-2001
#define		UNKNOW_COLUMN_ENTITY								-2002
#define		OUT_OF_COLUMN_ENTITY_SIZE_LIMIT				-2003
#define		ENTITY_COLUMNS_COUNT_ERROR					-2004

#define		UNKNOW_LDAP_TYPE_ENTRY								-2200

#define		SYSTEM_USER_HOME_CREATION_ERROR			-2100
#define		SYSTEM_USERS_DISK_ERROR								-2101
#define		SYSTEM_SEND_MAIL_ERROR								-2102

#define		FILE_NOT_EXISTS													-3000
#define		FILE_ALREADY_EXISTS											-3001
#define		FILE_CREATION_ERROR										-3002
#define		FILE_RENAMING_ERROR										-3003
#define		FILE_OPENING_ERROR											-3004
#define		FILE_READING_ERROR											-3005
#define		FILE_WRITING_ERROR											-3006
#define		FILE_EMPTY_ERROR											-3007

#define		GROUP_NAME_EMPTY											-3100
#define		GROUP_PARENT_EMPTY										-3101
#define		GROUP_CANT_BE_PARENT_OF_HIMSELF			-3102

#define		CONTACT_NAME_EMPTY										-3200

#define		LDAP_CONNEXION_ERROR									-4000
#define		LDAP_INVALID_CREDENTIAL								-4001
#define		LDAP_QUERY_ERROR											-4002
#define		LDAP_NO_ENTRY_FOUND									-4003
#define		LDAP_TOO_MUCH_ENTRIES_FOUND					-4004

#define		LDAP_DB_USER_SYNC_ERROR							-4010

#define		UNKNOW_ERROR   -9999

#ifdef __cplusplus
  }
#endif
#endif /*CONSTANTS_H_*/

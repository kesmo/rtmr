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

/** \file entities.h
* This file contains the definition for all entities.<br>
* Each entity corresponds to a database table or a database view, with their own columns definitions.
*/
#ifndef ENTITIES_DEF_H_
#define ENTITIES_DEF_H_

#include "constants.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define			RECORD_STATUS_OWN_LOCK			1
#define			RECORD_STATUS_MODIFIABLE			0
#define			RECORD_STATUS_OUT_OF_SYNC		-1
#define			RECORD_STATUS_LOCKED					-2
#define			RECORD_STATUS_BROKEN					-3

    /**
* Base structure for entity definition
*/
    typedef struct _entity_def
    {
	//! Entity signature identifier
	const int			m_entity_signature_id;
	//! Entity name (equals to table/view name)
	const char		*m_entity_name;
	//! \a m_primary_key may be NULL if the primary key of the entity is the first column of \a m_entity_columns_names
	const char      **m_primary_key;
	//! Sequence generator of the primary key
	const char      *m_pk_seq_name;
	//! Array of columns names
	const char		**m_entity_columns_names;
	//! Array of columns formats (string, number, timestamp)
	const char		**m_entity_columns_formats;
	//! Array of columns sizes
	const unsigned int		*m_entity_columns_sizes;
	//! Number of columns
	const unsigned int			m_entity_columns_count;
    }
    entity_def;


    enum TABLE_SIG_IDS {
	// Administration tables
	FILES_TABLE_SIG_ID = 100,
	USERS_TABLE_SIG_ID, // 101
    GROUPS_TABLE_SIG_ID, // 102 -- not used
    SHARED_FILES_TABLE_SIG_ID, // 103 -- not used
    USERS_GROUPS_TABLE_SIG_ID, // 104 -- not used
    CONTACTS_TABLE_SIG_ID, // 105 -- not used
    CONTACTS_GROUPS_TABLE_SIG_ID, // 106 -- not used
	LDAP_USERS_SIG_ID, // 107

	// Data tables
	PROJECTS_TABLE_SIG_ID =	200,
	PROJECTS_VERSIONS_TABLE_SIG_ID, // 201
	TESTS_CONTENTS_TABLE_SIG_ID, // 202
	TESTS_TABLE_SIG_ID, // 203
	ACTIONS_TABLE_SIG_ID, // 204
	REQUIREMENTS_CONTENTS_TABLE_SIG_ID, // 205
	REQUIREMENTS_TABLE_SIG_ID, // 206
	AUTOMATED_ACTIONS_TABLE_SIG_ID, // 207
	TESTS_REQUIREMENTS_TABLE_SIG_ID, // 208,
	CAMPAIGNS_TABLE_SIG_ID, // 209
	TESTS_CAMPAIGNS_TABLE_SIG_ID, // 210
	EXECUTIONS_CAMPAIGNS_TABLE_SIG_ID, // 211
	EXECUTIONS_TESTS_TABLE_SIG_ID, // 212
	EXECUTIONS_ACTIONS_TABLE_SIG_ID	, // 213
	EXECUTIONS_REQUIREMENTS_TABLE_SIG_ID, // 214
	PROJECTS_PARAMETERS_TABLE_SIG_ID, // 215
	PROJECTS_GRANTS_TABLE_SIG_ID, // 216
	TESTS_HIERARCHY_SIG_ID, // 217
	REQUIREMENTS_HIERARCHY_SIG_ID, // 218
	EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_SIG_ID, // 219
	TESTS_CONTENTS_FILES_TABLE_SIG_ID, // 220
	BUGS_TABLE_SIG_ID, // 221
	EXECUTIONS_TESTS_PARAMETERS_TABLE_SIG_ID, // 222
	AUTOMATED_ACTIONS_VALIDATIONS_TABLE_SIG_ID, // 223
    AUTOMATED_EXECUTIONS_ACTIONS_TABLE_SIG_ID, // 224
	CUSTOM_FIELDS_DESC_TABLE_SIG_ID, // 225
	CUSTOM_TEST_FIELDS_TABLE_SIG_ID, // 226
	CUSTOM_REQUIREMENT_FIELDS_TABLE_SIG_ID, // 227

	// Static tables
	REQUIREMENTS_CATEGORIES_TABLE_SIG_ID = 300,
	TESTS_RESULTS_TABLE_SIG_ID, // 301
	ACTIONS_RESULTS_TABLE_SIG_ID, // 302
	STATUS_TABLE_SIG_ID, // 303
    TESTS_TYPES_TABLE_SIG_ID // 304
    };

    /*-------------------------------------------------------------------
 * Données des documents
 -------------------------------------------------------------------*/
#define		FILES_TABLE_SIG									"files_table"

#define		FILES_TABLE_FILE_ID_SEQ					"files_file_id_seq"
#define		FILES_TABLE_FILE_ID								"file_id"
#define		FILES_TABLE_BASENAME						"basename"
#define		FILES_TABLE_PARENT_ID						"parent_id"
#define		FILES_TABLE_USER_ID							"user_id"
#define		FILES_TABLE_GROUP_ID						"group_id"
#define		FILES_TABLE_STATUS							"status"
#define		FILES_TABLE_FILE_TYPE						"file_type"
#define		FILES_TABLE_SHARE_NAME					"share_name"

    typedef struct _file
    {
	char	*file_id;
	char	*basename;
	char	*parent_id;
	char	*user_id;
	char	*group_id;
	char	*status;
	char	*file_type;
	char	*share_name;
    }file;

    static const char *FILES_TABLE_COLUMNS_NAMES[] = {
	FILES_TABLE_FILE_ID,
	FILES_TABLE_BASENAME,
	FILES_TABLE_PARENT_ID,
	FILES_TABLE_USER_ID,
	FILES_TABLE_GROUP_ID,
	FILES_TABLE_STATUS,
	FILES_TABLE_FILE_TYPE,
	FILES_TABLE_SHARE_NAME,
	NULL
    };

    static const char *FILES_TABLE_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const entity_def files_table_def = {
	FILES_TABLE_SIG_ID,
	FILES_TABLE_SIG,
	NULL,
	FILES_TABLE_FILE_ID_SEQ,
	FILES_TABLE_COLUMNS_NAMES,
        FILES_TABLE_COLUMNS_FORMATS,
	NULL,
        sizeof(FILES_TABLE_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * Données des utilisateurs
 -------------------------------------------------------------------*/
#define		LDAP_USERS_DN									"dn"
#define		LDAP_USERS_GIVEN_NAME					"givenName"
#define		LDAP_USERS_SN									"sn"
#define		LDAP_USERS_UID									"uid"
#define		LDAP_USERS_USER_PASSWORD			"userPassword"
#define		LDAP_USERS_UID_NUMBER					"uidNumber"
#define		LDAP_USERS_GID_NUMBER					"gidNumber"
#define		LDAP_USERS_HOME_DIR						"homeDirectory"
#define		LDAP_USERS_CN									"cn"
#define		LDAP_USERS_MAIL								"mail"

    static const char *LDAP_USERS_ATTRIBUTES_NAMES[] = {
	LDAP_USERS_DN,
	LDAP_USERS_GIVEN_NAME,
	LDAP_USERS_SN,
	LDAP_USERS_UID,
	LDAP_USERS_USER_PASSWORD,
	LDAP_USERS_UID_NUMBER,
	LDAP_USERS_GID_NUMBER,
	LDAP_USERS_HOME_DIR,
	LDAP_USERS_CN,
	LDAP_USERS_MAIL,
	NULL
    };

    static const entity_def ldap_users_table_def = {
        LDAP_USERS_SIG_ID,
        NULL,
	NULL,
	NULL,
        LDAP_USERS_ATTRIBUTES_NAMES,
        NULL,
	NULL,
	sizeof(LDAP_USERS_ATTRIBUTES_NAMES)/sizeof(char*) - 1
    };

#define		USERS_TABLE_SIG								"users_table"

#define		USERS_TABLE_USER_ID_SEQ				"users_user_id_seq"
#define		USERS_TABLE_USER_ID						"user_id"
#define		USERS_TABLE_USERNAME					"username"
#define		USERS_TABLE_GROUP_ID					"group_id"
#define		USERS_TABLE_EMAIL							"email"
#define		USERS_TABLE_PASSWORD					"password"
#define		USERS_TABLE_STATUS							"status"
#ifndef _RTMR
#define		USERS_TABLE_CREATION_DATE			"creation_date"
#define		USERS_TABLE_THEME							"theme"
#define		USERS_TABLE_WINDOWS_COLOR		"windows_color"
#define		USERS_TABLE_SHOW_TOOLTIPS		"show_tooltips"
#endif

    typedef struct _user
    {
	char	*user_id;
	char	*username;
	char	*group_id;
	char	*email;
	char	*password;
	char	*status;
#ifndef _RTMR
	char	*creation_date;
	char	*theme;
	char	*windows_color;
	char	*show_tooltips;
#endif
    }user;

    static const char *USERS_TABLE_COLUMNS_NAMES[] = {
	USERS_TABLE_USER_ID,
	USERS_TABLE_USERNAME,
	USERS_TABLE_GROUP_ID,
	USERS_TABLE_EMAIL,
	USERS_TABLE_PASSWORD,
	USERS_TABLE_STATUS,
#ifndef _RTMR
	USERS_TABLE_CREATION_DATE,
	USERS_TABLE_THEME,
	USERS_TABLE_WINDOWS_COLOR,
	USERS_TABLE_SHOW_TOOLTIPS,
#endif
	NULL
    };

    static const char *USERS_TABLE_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	MD5_FORMAT,
	STRING_FORMAT,
#ifndef _RTMR
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
#endif
	NULL
    };

    static const entity_def users_table_def = {
	USERS_TABLE_SIG_ID,
	USERS_TABLE_SIG,
	NULL,
	USERS_TABLE_USER_ID_SEQ,
	USERS_TABLE_COLUMNS_NAMES,
	USERS_TABLE_COLUMNS_FORMATS,
	NULL,
	sizeof(USERS_TABLE_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * Données des groupes
 -------------------------------------------------------------------*/
#define		GROUPS_TABLE_SIG								"groups_table"

#define		GROUPS_TABLE_GROUP_ID_SEQ		"groups_group_id_seq"
#define		GROUPS_TABLE_GROUP_ID					"group_id"
#define		GROUPS_TABLE_GROUP_NAME			"group_name"
#define		GROUPS_TABLE_OWNER_ID				"owner_id"
#define		GROUPS_TABLE_PARENT_ID				"parent_id"

    typedef struct _group
    {
	char	*group_id;
	char	*group_name;
	char	*owner_id;
	char	*parent_id;
    }group;

    static const char *GROUPS_TABLE_COLUMNS_NAMES[] = {
	GROUPS_TABLE_GROUP_ID,
	GROUPS_TABLE_GROUP_NAME,
	GROUPS_TABLE_OWNER_ID,
	GROUPS_TABLE_PARENT_ID,
	NULL
    };

    static const char *GROUPS_TABLE_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NULL
    };

    static const entity_def groups_table_def = {
	GROUPS_TABLE_SIG_ID,
	GROUPS_TABLE_SIG,
	NULL,
	GROUPS_TABLE_GROUP_ID_SEQ,
	GROUPS_TABLE_COLUMNS_NAMES,
	GROUPS_TABLE_COLUMNS_FORMATS,
	NULL,
	sizeof(GROUPS_TABLE_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * Données des dossiers partagés
 -------------------------------------------------------------------*/
#define		SHARED_FILES_TABLE_SIG					"shared_files_table"
#define		SHARED_FILES_TABLE_FILE_ID				"file_id"
#define		SHARED_FILES_TABLE_GROUP_ID		"group_id"

    typedef struct _share
    {
	char	*file_id;
	char	*group_id;
    }share;

    static const char *SHARED_FILES_TABLE_COLUMNS_NAMES[] = {
	SHARED_FILES_TABLE_FILE_ID,
	SHARED_FILES_TABLE_GROUP_ID,
	NULL
    };

    static const char *SHARED_FILES_TABLE_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NULL
    };

    static const char *SHARED_FILES_TABLE_PRIMARY_KEY[] = {
	SHARED_FILES_TABLE_FILE_ID,
	SHARED_FILES_TABLE_GROUP_ID,
	NULL
    };

    static const entity_def shared_files_table_def = {
	SHARED_FILES_TABLE_SIG_ID,
	SHARED_FILES_TABLE_SIG,
	SHARED_FILES_TABLE_PRIMARY_KEY,
	NULL,
	SHARED_FILES_TABLE_COLUMNS_NAMES,
	SHARED_FILES_TABLE_COLUMNS_FORMATS,
	NULL,
	sizeof(SHARED_FILES_TABLE_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * Données concernant l'appartenance des utilisateurs à des groupes
 -------------------------------------------------------------------*/
#define		USERS_GROUPS_TABLE_SIG				"users_groups_table"
#define		USERS_GROUPS_TABLE_USER_ID		"user_id"
#define		USERS_GROUPS_TABLE_GROUP_ID	"group_id"

    typedef struct _user_group
    {
	char	*user_id;
	char	*group_id;
    }user_group;

    static const char *USERS_GROUPS_TABLE_COLUMNS_NAMES[] = {
	USERS_GROUPS_TABLE_USER_ID,
	USERS_GROUPS_TABLE_GROUP_ID,
	NULL
    };

    static const char *USERS_GROUPS_TABLE_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NULL
    };

    static const char *USERS_GROUPS_TABLE_PRIMARY_KEY[] = {
	USERS_GROUPS_TABLE_USER_ID,
	USERS_GROUPS_TABLE_GROUP_ID,
	NULL
    };

    static const entity_def users_groups_table_def = {
	USERS_GROUPS_TABLE_SIG_ID,
	USERS_GROUPS_TABLE_SIG,
	USERS_GROUPS_TABLE_PRIMARY_KEY,
	NULL,
	USERS_GROUPS_TABLE_COLUMNS_NAMES,
	USERS_GROUPS_TABLE_COLUMNS_FORMATS,
	NULL,
	sizeof(USERS_GROUPS_TABLE_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * Données des contacts
 -------------------------------------------------------------------*/
#define		CONTACTS_TABLE_SIG								"contacts_table"
#define		CONTACTS_TABLE_CONTACT_ID_SEQ		"contacts_contact_id_seq"
#define		CONTACTS_TABLE_CONTACT_ID				"contact_id"
#define		CONTACTS_TABLE_OWNER_ID					"owner_id"
#define		CONTACTS_TABLE_NAME							"name"
#define		CONTACTS_TABLE_FIRSTNAME					"firstname"
#define		CONTACTS_TABLE_COMPANY						"company"
#define		CONTACTS_TABLE_EMAIL_1						"email1"
#define		CONTACTS_TABLE_EMAIL_2						"email2"
#define		CONTACTS_TABLE_EMAIL_3						"email3"
#define		CONTACTS_TABLE_PHONE_MOBILE			"phone_mobile"
#define		CONTACTS_TABLE_PHONE_WORK				"phone_work"
#define		CONTACTS_TABLE_PHONE_HOME				"phone_home"

    typedef struct _contact
    {
	char	*contact_id;
	char	*owner_id;
	char	*name;
	char	*fisrtname;
	char	*company;
	char	*email1;
	char	*email2;
	char	*email3;
	char	*phone_mobile;
	char	*phone_work;
	char	*phone_home;
    }contact;

    static const char *CONTACTS_TABLE_COLUMNS_NAMES[] = {
	CONTACTS_TABLE_CONTACT_ID,
	CONTACTS_TABLE_OWNER_ID,
	CONTACTS_TABLE_NAME,
	CONTACTS_TABLE_FIRSTNAME,
	CONTACTS_TABLE_COMPANY,
	CONTACTS_TABLE_EMAIL_1,
	CONTACTS_TABLE_EMAIL_2,
	CONTACTS_TABLE_EMAIL_3,
	CONTACTS_TABLE_PHONE_MOBILE,
	CONTACTS_TABLE_PHONE_WORK,
	CONTACTS_TABLE_PHONE_HOME,
	NULL
    };

    static const char *CONTACTS_TABLE_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const entity_def contacts_table_def = {
	CONTACTS_TABLE_SIG_ID,
	CONTACTS_TABLE_SIG,
	NULL,
	CONTACTS_TABLE_CONTACT_ID_SEQ,
	CONTACTS_TABLE_COLUMNS_NAMES,
	CONTACTS_TABLE_COLUMNS_FORMATS,
	NULL,
	sizeof(CONTACTS_TABLE_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * Donneees concernant l'appartenance des contacts a des groupes
 -------------------------------------------------------------------*/
#define		CONTACTS_GROUPS_TABLE_SIG					"contacts_groups_table"
#define		CONTACTS_GROUPS_TABLE_CONTACT_ID	"contact_id"
#define		CONTACTS_GROUPS_TABLE_GROUP_ID		"group_id"

    typedef struct _contact_group
    {
	char	*contact_id;
	char	*group_id;
    }contact_group;

    static const char *CONTACTS_GROUPS_COLUMNS_NAMES[] = {
	CONTACTS_GROUPS_TABLE_CONTACT_ID,
	CONTACTS_GROUPS_TABLE_GROUP_ID,
	NULL
    };

    static const char *CONTACTS_GROUPS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NULL
    };

    static const char *CONTACTS_GROUPS_PRIMARY_KEY[] = {
	CONTACTS_GROUPS_TABLE_CONTACT_ID,
	CONTACTS_GROUPS_TABLE_GROUP_ID,
	NULL
    };

    static const entity_def contacts_groups_table_def = {
	CONTACTS_GROUPS_TABLE_SIG_ID,
	CONTACTS_GROUPS_TABLE_SIG,
	CONTACTS_GROUPS_PRIMARY_KEY,
	NULL,
	CONTACTS_GROUPS_COLUMNS_NAMES,
	CONTACTS_GROUPS_COLUMNS_FORMATS,
	NULL,
	sizeof(CONTACTS_GROUPS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 								PROJETS
 -------------------------------------------------------------------*/
    //! Projects table name
#define		PROJECTS_TABLE_SIG										"projects_table"
#define		PROJECTS_TABLE_PROJECT_ID_SEQ				"projects_project_id_seq"
    //! Projects table signature identifier
#define		PROJECTS_TABLE_PROJECT_ID						"project_id"
#define		PROJECTS_TABLE_OWNER_ID							"owner_id"
#define		PROJECTS_TABLE_SHORT_NAME						"short_name"
#define		PROJECTS_TABLE_DESCRIPTION						"description"

    static const char *PROJECTS_COLUMNS_NAMES[] = {
	PROJECTS_TABLE_PROJECT_ID,
	PROJECTS_TABLE_OWNER_ID,
	PROJECTS_TABLE_SHORT_NAME,
	PROJECTS_TABLE_DESCRIPTION,
	NULL
    };

    static const char *PROJECTS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int PROJECTS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	SMEDIUM_TEXT_SIZE,
	LLARGE_TEXT_SIZE
    };

    static const entity_def projects_table_def = {
	PROJECTS_TABLE_SIG_ID,
	PROJECTS_TABLE_SIG,
	NULL,
	PROJECTS_TABLE_PROJECT_ID_SEQ,
	PROJECTS_COLUMNS_NAMES,
	PROJECTS_COLUMNS_FORMATS,
	PROJECTS_COLUMNS_SIZES,
	sizeof(PROJECTS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 				PARAMETRES DE PROJETS
 -------------------------------------------------------------------*/
    //! Parameters projects table name
#define		PROJECTS_PARAMETERS_TABLE_SIG									"projects_parameters_table"
#define		PROJECTS_PARAMETERS_TABLE_PROJECT_PARAMETER_ID_SEQ				"projects_parameters_project_parameter_id_seq"
#define		PROJECTS_PARAMETERS_TABLE_PROJECT_PARAMETER_ID				"project_parameter_id"
#define		PROJECTS_PARAMETERS_TABLE_PROJECT_ID						"project_id"
#define		PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME						"parameter_name"
#define		PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE						"parameter_value"

    static const char *PROJECTS_PARAMETERS_COLUMNS_NAMES[] = {
	PROJECTS_PARAMETERS_TABLE_PROJECT_PARAMETER_ID,
	PROJECTS_PARAMETERS_TABLE_PROJECT_ID,
	PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME,
	PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE,
	NULL
    };

    static const char *PROJECTS_PARAMETERS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int PROJECTS_PARAMETERS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	LMEDIUM_TEXT_SIZE,
	LMEDIUM_TEXT_SIZE
    };


    static const entity_def projects_parameters_table_def = {
	PROJECTS_PARAMETERS_TABLE_SIG_ID,
	PROJECTS_PARAMETERS_TABLE_SIG,
	NULL,
	PROJECTS_PARAMETERS_TABLE_PROJECT_PARAMETER_ID_SEQ,
	PROJECTS_PARAMETERS_COLUMNS_NAMES,
	PROJECTS_PARAMETERS_COLUMNS_FORMATS,
	PROJECTS_PARAMETERS_COLUMNS_SIZES,
	sizeof(PROJECTS_PARAMETERS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 								VERSIONS DES PROJETS
 -------------------------------------------------------------------*/
    //! Projects versions table name
#define		PROJECTS_VERSIONS_TABLE_SIG										"projects_versions_table"
#define		PROJECTS_VERSIONS_TABLE_PROJECT_VERSION_ID		"project_version_id"
#define		PROJECTS_VERSIONS_TABLE_PROJECT_ID						"project_id"
#define		PROJECTS_VERSIONS_TABLE_VERSION								"version"
#define		PROJECTS_VERSIONS_TABLE_DESCRIPTION						"description"
#define		PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE				"bug_tracker_type"
#define		PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST				"bug_tracker_host"
#define		PROJECTS_VERSIONS_TABLE_BUG_TRACKER_URL				"bug_tracker_url"
#define		PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_ID				"bug_tracker_project_id"
#define		PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_VERSION			"bug_tracker_project_version"

    static const char *PROJECTS_VERSIONS_COLUMNS_NAMES[] = {
	PROJECTS_VERSIONS_TABLE_PROJECT_VERSION_ID,
	PROJECTS_VERSIONS_TABLE_PROJECT_ID,
	PROJECTS_VERSIONS_TABLE_VERSION,
	PROJECTS_VERSIONS_TABLE_DESCRIPTION,
	PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE,
	PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST,
	PROJECTS_VERSIONS_TABLE_BUG_TRACKER_URL,
	PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_ID,
	PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_VERSION,
	NULL
    };

    static const char *PROJECTS_VERSIONS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const char *PROJECTS_VERSIONS_PRIMARY_KEY[] = {
	PROJECTS_VERSIONS_TABLE_PROJECT_ID,
	PROJECTS_VERSIONS_TABLE_VERSION,
	NULL
    };

    static const unsigned int PROJECTS_VERSIONS_PRIMARY_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	12,
	LLARGE_TEXT_SIZE,
	SHORT_TEXT_SIZE,
	SHORT_TEXT_SIZE,
	LMEDIUM_TEXT_SIZE,
	SHORT_TEXT_SIZE,
	SHORT_TEXT_SIZE
    };

    static const entity_def projects_versions_table_def = {
	PROJECTS_VERSIONS_TABLE_SIG_ID,
	PROJECTS_VERSIONS_TABLE_SIG,
	PROJECTS_VERSIONS_PRIMARY_KEY,
	NULL,
	PROJECTS_VERSIONS_COLUMNS_NAMES,
	PROJECTS_VERSIONS_COLUMNS_FORMATS,
	PROJECTS_VERSIONS_PRIMARY_SIZES,
	sizeof(PROJECTS_VERSIONS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 								TESTS_CONTENTS
 -------------------------------------------------------------------*/
    //! Tests contents table name
#define		TESTS_CONTENTS_TABLE_SIG											"tests_contents_table"
#define		TESTS_CONTENTS_TABLE_TEST_CONTENET_ID_SEQ		"tests_contents_test_content_id_seq"
#define		TESTS_CONTENTS_TABLE_TEST_CONTENT_ID				"test_content_id"
#define		TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID				"original_test_content_id"
#define		TESTS_CONTENTS_TABLE_PROJECT_ID								"project_id"
#define		TESTS_CONTENTS_TABLE_VERSION									"version"
#define		TESTS_CONTENTS_TABLE_SHORT_NAME							"short_name"
#define		TESTS_CONTENTS_TABLE_DESCRIPTION							"description"
#define		TESTS_CONTENTS_TABLE_PRIORITY_LEVEL						"priority_level"
#define		TESTS_CONTENTS_TABLE_CATEGORY_ID								"category_id"
#define		TESTS_CONTENTS_TABLE_STATUS								"status"
#define		TESTS_CONTENTS_TABLE_AUTOMATED							"automated"
#define		TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND					"automation_command"
#define		TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND_PARAMETERS		"automation_command_parameters"


#define		TESTS_CONTENTS_TABLE_TYPE								"type"
#define		TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE					"limit_test_case"

#define		TEST_CONTENT_TYPE_NOMINAL						"N"
#define		TEST_CONTENT_TYPE_ALTERNATE						"A"
#define		TEST_CONTENT_TYPE_EXCEPTION						"E"

    static const char *TESTS_CONTENTS_COLUMNS_NAMES[] = {
	TESTS_CONTENTS_TABLE_TEST_CONTENT_ID,
	TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID,
	TESTS_CONTENTS_TABLE_PROJECT_ID,
	TESTS_CONTENTS_TABLE_VERSION,
	TESTS_CONTENTS_TABLE_SHORT_NAME,
	TESTS_CONTENTS_TABLE_DESCRIPTION,
        TESTS_CONTENTS_TABLE_PRIORITY_LEVEL,
        TESTS_CONTENTS_TABLE_CATEGORY_ID,
        TESTS_CONTENTS_TABLE_STATUS,
        TESTS_CONTENTS_TABLE_AUTOMATED,
        TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND,
        TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND_PARAMETERS,
	TESTS_CONTENTS_TABLE_TYPE,
	TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE,
        NULL
    };

    static const char *TESTS_CONTENTS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
        NUMBER_FORMAT,
        STRING_FORMAT,
        STRING_FORMAT,
        STRING_FORMAT,
        STRING_FORMAT,
        STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
        NULL
    };

    static const unsigned int TESTS_CONTENTS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	12,
	SMEDIUM_TEXT_SIZE,
	LLARGE_TEXT_SIZE,
	1,
	1,
	1,
        1,
        SLARGE_TEXT_SIZE,
        SLARGE_TEXT_SIZE,
	1,
	1
    };


    static const entity_def tests_contents_table_def = {
	TESTS_CONTENTS_TABLE_SIG_ID,
	TESTS_CONTENTS_TABLE_SIG,
	NULL,
	TESTS_CONTENTS_TABLE_TEST_CONTENET_ID_SEQ,
	TESTS_CONTENTS_COLUMNS_NAMES,
	TESTS_CONTENTS_COLUMNS_FORMATS,
	TESTS_CONTENTS_COLUMNS_SIZES,
	sizeof(TESTS_CONTENTS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 								TESTS_CONTENTS_FILES
 -------------------------------------------------------------------*/
    //! Tests attachments table name
#define		TESTS_CONTENTS_FILES_TABLE_SIG											"tests_contents_files_table"
#define		TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILE_ID				"test_content_file_id"
#define		TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_ID				"test_content_id"
#define		TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME				"test_content_filename"
#define		TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_LO_ID				"test_content_lo_oid"

    static const char *TESTS_CONTENTS_FILES_COLUMNS_NAMES[] = {
	TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILE_ID,
	TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_ID,
	TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_FILENAME,
        TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_LO_ID,
	NULL
    };

    static const char *TESTS_CONTENTS_FILES_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
        NUMBER_FORMAT,
	STRING_FORMAT,
        NUMBER_FORMAT,
	NULL
    };

    static const unsigned int TESTS_CONTENTS_FILES_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
        PRIMARY_KEY_SIZE,
        LMEDIUM_TEXT_SIZE,
        PRIMARY_KEY_SIZE
    };


    static const entity_def tests_contents_files_table_def = {
	TESTS_CONTENTS_FILES_TABLE_SIG_ID,
	TESTS_CONTENTS_FILES_TABLE_SIG,
	NULL,
	NULL,
	TESTS_CONTENTS_FILES_COLUMNS_NAMES,
	TESTS_CONTENTS_FILES_COLUMNS_FORMATS,
	TESTS_CONTENTS_FILES_COLUMNS_SIZES,
	sizeof(TESTS_CONTENTS_FILES_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 								TESTS
 -------------------------------------------------------------------*/
    //! Tests table name
#define		TESTS_TABLE_SIG											"tests_table"
#define		TESTS_TABLE_TEST_ID_SEQ						"tests_test_id_seq"
#define		TESTS_TABLE_TEST_ID									"test_id"
#define		TESTS_TABLE_ORIGINAL_TEST_ID				"original_test_id"
#define		TESTS_TABLE_PARENT_TEST_ID					"parent_test_id"
#define		TESTS_TABLE_PREVIOUS_TEST_ID				"previous_test_id"
#define		TESTS_TABLE_PROJECT_ID							"project_id"
#define		TESTS_TABLE_VERSION								"version"
#define		TESTS_TABLE_TEST_CONTENT_ID				"test_content_id"

    static const char *TESTS_COLUMNS_NAMES[] = {
	TESTS_TABLE_TEST_ID,
	TESTS_TABLE_ORIGINAL_TEST_ID,
	TESTS_TABLE_PARENT_TEST_ID,
	TESTS_TABLE_PREVIOUS_TEST_ID,
	TESTS_TABLE_PROJECT_ID,
	TESTS_TABLE_VERSION,
	TESTS_TABLE_TEST_CONTENT_ID,
	NULL
    };

    static const char *TESTS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	NULL
    };

    static const unsigned int TESTS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	12,
	PRIMARY_KEY_SIZE
    };

    static const entity_def tests_table_def = {
	TESTS_TABLE_SIG_ID,
	TESTS_TABLE_SIG,
	NULL,
	TESTS_TABLE_TEST_ID_SEQ,
	TESTS_COLUMNS_NAMES,
	TESTS_COLUMNS_FORMATS,
	TESTS_COLUMNS_SIZES,
	sizeof(TESTS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 								ACTIONS
 -------------------------------------------------------------------*/
    //! Tests cases actions table name
#define		ACTIONS_TABLE_SIG										"actions_table"
#define		ACTIONS_TABLE_ACTION_ID_SEQ					"actions_action_id_seq"
#define		ACTIONS_TABLE_ACTION_ID							"action_id"
#define		ACTIONS_TABLE_PREVIOUS_ACTION_ID		"previous_action_id"
#define		ACTIONS_TABLE_TEST_CONTENT_ID				"test_content_id"
#define		ACTIONS_TABLE_SHORT_NAME						"short_name"
#define		ACTIONS_TABLE_DESCRIPTION						"description"
#define		ACTIONS_TABLE_WAIT_RESULT						"wait_result"
#define		ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID				"link_original_test_content_id"

    static const char *ACTIONS_COLUMNS_NAMES[] = {
	ACTIONS_TABLE_ACTION_ID,
	ACTIONS_TABLE_PREVIOUS_ACTION_ID,
	ACTIONS_TABLE_TEST_CONTENT_ID,
	ACTIONS_TABLE_SHORT_NAME,
	ACTIONS_TABLE_DESCRIPTION,
	ACTIONS_TABLE_WAIT_RESULT,
	ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID,
	NULL
    };

    static const char *ACTIONS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	NULL
    };

    static const unsigned int ACTIONS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	SMEDIUM_TEXT_SIZE,
	LLARGE_TEXT_SIZE,
	LLARGE_TEXT_SIZE,
	PRIMARY_KEY_SIZE
    };

    static const entity_def actions_table_def = {
	ACTIONS_TABLE_SIG_ID,
	ACTIONS_TABLE_SIG,
	NULL,
	ACTIONS_TABLE_ACTION_ID_SEQ,
	ACTIONS_COLUMNS_NAMES,
	ACTIONS_COLUMNS_FORMATS,
	ACTIONS_COLUMNS_SIZES,
	sizeof(ACTIONS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 						AUTOMATED ACTIONS
 -------------------------------------------------------------------*/
    //! Automated actions table name
#define		AUTOMATED_ACTIONS_TABLE_SIG										"automated_actions_table"
#define		AUTOMATED_ACTIONS_TABLE_ACTION_ID_SEQ					"automated_actions_action_id_seq"
#define		AUTOMATED_ACTIONS_TABLE_ACTION_ID							"automated_action_id"
#define		AUTOMATED_ACTIONS_TABLE_PREVIOUS_ACTION_ID		"previous_automated_action_id"
#define		AUTOMATED_ACTIONS_TABLE_TEST_CONTENT_ID				"test_content_id"
#define		AUTOMATED_ACTIONS_TABLE_WINDOW_ID				"window_id"
#define		AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE				"message_type"
#define		AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA				"message_data"
#define		AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY			"message_time_delay"

typedef enum {
	None,
	Keyboard,
    Mouse,
    StartRecord,
    StopRecord
}
EventMessageType;

#if (defined(_WINDOWS) || defined(WIN32))
#define		EVT_MSG_MOUSE_BUTTON_LEFT_DOWN		WM_LBUTTONDOWN
#define		EVT_MSG_MOUSE_BUTTON_LEFT_UP		WM_LBUTTONUP
#define		EVT_MSG_MOUSE_WHEEL_VERTICAL		WM_MOUSEWHEEL
#define		EVT_MSG_MOUSE_WHEEL_HORIZONTAL		WM_MOUSEHWHEEL
#define		EVT_MSG_MOUSE_BUTTON_RIGHT_DOWN		WM_RBUTTONDOWN
#define		EVT_MSG_MOUSE_BUTTON_RIGHT_UP		WM_RBUTTONUP
#define		EVT_MSG_MOUSE_MOVE			WM_MOUSEMOVE
#define		EVT_MSG_KEYBOARD_KEY_DOWN		WM_KEYDOWN
#define		EVT_MSG_KEYBOARD_KEY_UP			WM_KEYUP
#define		EVT_MSG_KEYBOARD_SYSTEM_KEY_DOWN	WM_SYSKEYDOWN
#define		EVT_MSG_KEYBOARD_SYSTEM_KEY_UP		WM_SYSKEYUP
#else
#define		EVT_MSG_MOUSE_BUTTON_LEFT_DOWN		0
#define		EVT_MSG_MOUSE_BUTTON_LEFT_UP		1
#define		EVT_MSG_MOUSE_WHEEL_VERTICAL		2
#define		EVT_MSG_MOUSE_WHEEL_HORIZONTAL		3
#define		EVT_MSG_MOUSE_BUTTON_RIGHT_DOWN		4
#define		EVT_MSG_MOUSE_BUTTON_RIGHT_UP		5
#define		EVT_MSG_MOUSE_MOVE			6
#define		EVT_MSG_KEYBOARD_KEY_DOWN		7
#define		EVT_MSG_KEYBOARD_KEY_UP			8
#define		EVT_MSG_KEYBOARD_SYSTEM_KEY_DOWN	9
#define		EVT_MSG_KEYBOARD_SYSTEM_KEY_UP		10
#endif

    static const char *AUTOMATED_ACTIONS_COLUMNS_NAMES[] = {
	AUTOMATED_ACTIONS_TABLE_ACTION_ID,
	AUTOMATED_ACTIONS_TABLE_PREVIOUS_ACTION_ID,
	AUTOMATED_ACTIONS_TABLE_TEST_CONTENT_ID,
	AUTOMATED_ACTIONS_TABLE_WINDOW_ID,
	AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE,
	AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA,
	AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY,
	NULL
    };

    static const char *AUTOMATED_ACTIONS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	NULL
    };

    static const unsigned int AUTOMATED_ACTIONS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	S1LARGE_TEXT_SIZE,
    NUMBER_SIZE,
	S1LARGE_TEXT_SIZE,
    NUMBER_SIZE
    };

    static const entity_def automated_actions_table_def = {
	AUTOMATED_ACTIONS_TABLE_SIG_ID,
	AUTOMATED_ACTIONS_TABLE_SIG,
	NULL,
	AUTOMATED_ACTIONS_TABLE_ACTION_ID_SEQ,
	AUTOMATED_ACTIONS_COLUMNS_NAMES,
	AUTOMATED_ACTIONS_COLUMNS_FORMATS,
	AUTOMATED_ACTIONS_COLUMNS_SIZES,
	sizeof(AUTOMATED_ACTIONS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 					AUTOMATED ACTIONS VALIDATIONS
 -------------------------------------------------------------------*/
    //! Automated actions validations table name
#define		AUTOMATED_ACTIONS_VALIDATIONS_TABLE_SIG										"automated_actions_validations_table"
#define		AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_ID_SEQ					"automated_actions_validations_validation_id_seq"
#define		AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_ID							"validation_id"
#define		AUTOMATED_ACTIONS_VALIDATIONS_TABLE_PREVIOUS_VALIDATION_ID		"previous_validation_id"
#define		AUTOMATED_ACTIONS_VALIDATIONS_TABLE_AUTOMATED_ACTION_ID				"automated_action_id"
#define		AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_NAME				"module_name"
#define		AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_VERSION				"module_version"
#define		AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME				"module_function_name"
#define		AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_PARAMETERS				"module_function_parameters"

    static const char *AUTOMATED_ACTIONS_VALIDATIONS_COLUMNS_NAMES[] = {
	AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_ID,
	AUTOMATED_ACTIONS_VALIDATIONS_TABLE_PREVIOUS_VALIDATION_ID,
	AUTOMATED_ACTIONS_VALIDATIONS_TABLE_AUTOMATED_ACTION_ID,
    AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_NAME,
    AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_VERSION,
    AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME,
	AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_PARAMETERS,
	NULL
    };

    static const char *AUTOMATED_ACTIONS_VALIDATIONS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
    STRING_FORMAT,
	STRING_FORMAT,
    NULL
    };

    static const unsigned int AUTOMATED_ACTIONS_VALIDATIONS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
    SMEDIUM_TEXT_SIZE,
    SHORT_TEXT_SIZE,
    SMEDIUM_TEXT_SIZE,
	LMEDIUM_TEXT_SIZE,
    };

    static const entity_def automated_actions_validations_table_def = {
	AUTOMATED_ACTIONS_VALIDATIONS_TABLE_SIG_ID,
	AUTOMATED_ACTIONS_VALIDATIONS_TABLE_SIG,
	NULL,
	AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_ID_SEQ,
	AUTOMATED_ACTIONS_VALIDATIONS_COLUMNS_NAMES,
	AUTOMATED_ACTIONS_VALIDATIONS_COLUMNS_FORMATS,
	AUTOMATED_ACTIONS_VALIDATIONS_COLUMNS_SIZES,
	sizeof(AUTOMATED_ACTIONS_VALIDATIONS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 						CONTENUS D'EXIGENCES
 -------------------------------------------------------------------*/
    //! Requirements contents table name
#define		REQUIREMENTS_CONTENTS_TABLE_SIG													"requirements_contents_table"
#define		REQUIREMENTS_CONTENTS_TABLE_REQUIREMENT_ID_SEQ					"requirements_contents_requirement_content_id_seq"
#define		REQUIREMENTS_CONTENTS_TABLE_REQUIREMENT_CONTENT_ID		"requirement_content_id"
#define		REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID		"original_requirement_content_id"
#define		REQUIREMENTS_CONTENTS_TABLE_PROJECT_ID									"project_id"
#define		REQUIREMENTS_CONTENTS_TABLE_VERSION											"version"
#define		REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME									"short_name"
#define		REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION									"description"
#define		REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID								"category_id"
#define		REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL						"priority_level"
#define		REQUIREMENTS_CONTENTS_TABLE_STATUS						"status"

    static const char *REQUIREMENTS_CONTENTS_COLUMNS_NAMES[] = {
	REQUIREMENTS_CONTENTS_TABLE_REQUIREMENT_CONTENT_ID,
	REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID,
	REQUIREMENTS_CONTENTS_TABLE_PROJECT_ID,
	REQUIREMENTS_CONTENTS_TABLE_VERSION,
	REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME,
	REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION,
	REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID,
	REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL,
	REQUIREMENTS_CONTENTS_TABLE_STATUS,
	NULL
    };

    static const char *REQUIREMENTS_CONTENTS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int REQUIREMENTS_CONTENTS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	12,
	SMEDIUM_TEXT_SIZE,
	LLARGE_TEXT_SIZE,
	1,
	1,
	1
    };


    static const entity_def requirements_contents_table_def = {
	REQUIREMENTS_CONTENTS_TABLE_SIG_ID,
	REQUIREMENTS_CONTENTS_TABLE_SIG,
	NULL,
	REQUIREMENTS_CONTENTS_TABLE_REQUIREMENT_ID_SEQ,
	REQUIREMENTS_CONTENTS_COLUMNS_NAMES,
	REQUIREMENTS_CONTENTS_COLUMNS_FORMATS,
	REQUIREMENTS_CONTENTS_COLUMNS_SIZES,
	sizeof(REQUIREMENTS_CONTENTS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 								EXIGENCES
 -------------------------------------------------------------------*/
    //! Requirements table name
#define		REQUIREMENTS_TABLE_SIG												"requirements_table"
#define		REQUIREMENTS_TABLE_REQUIREMENT_ID_SEQ				"requirements_requirement_id_seq"
#define		REQUIREMENTS_TABLE_REQUIREMENT_ID						"requirement_id"
#define		REQUIREMENTS_TABLE_REQUIREMENT_CONTENT_ID		"requirement_content_id"
#define		REQUIREMENTS_TABLE_PARENT_REQUIREMENT_ID			"parent_requirement_id"
#define		REQUIREMENTS_TABLE_PREVIOUS_REQUIREMENT_ID		"previous_requirement_id"
#define		REQUIREMENTS_TABLE_PROJECT_ID									"project_id"
#define		REQUIREMENTS_TABLE_VERSION										"version"

    static const char *REQUIREMENTS_COLUMNS_NAMES[] = {
	REQUIREMENTS_TABLE_REQUIREMENT_ID,
	REQUIREMENTS_TABLE_REQUIREMENT_CONTENT_ID,
	REQUIREMENTS_TABLE_PARENT_REQUIREMENT_ID,
	REQUIREMENTS_TABLE_PREVIOUS_REQUIREMENT_ID,
	REQUIREMENTS_TABLE_PROJECT_ID,
	REQUIREMENTS_TABLE_VERSION,
	NULL
    };

    static const char *REQUIREMENTS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int REQUIREMENTS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	12
    };


    static const entity_def requirements_table_def = {
	REQUIREMENTS_TABLE_SIG_ID,
	REQUIREMENTS_TABLE_SIG,
	NULL,
	REQUIREMENTS_TABLE_REQUIREMENT_ID_SEQ,
	REQUIREMENTS_COLUMNS_NAMES,
	REQUIREMENTS_COLUMNS_FORMATS,
	REQUIREMENTS_COLUMNS_SIZES,
	sizeof(REQUIREMENTS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 							CATEGORIES D'EXIGENCES
 -------------------------------------------------------------------*/
    //! Requirements categories table name
#define		REQUIREMENTS_CATEGORIES_TABLE_SIG								"requirements_categories_table"
#define		REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_ID				"category_id"
#define		REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_LABEL		"category_label"

    static const char *REQUIREMENTS_CATEGORIES_COLUMNS_NAMES[] = {
	REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_ID,
	REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_LABEL,
	NULL
    };

    static const char *REQUIREMENTS_CATEGORIES_COLUMNS_FORMATS[] = {
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const entity_def requirements_categories_table_def = {
	REQUIREMENTS_CATEGORIES_TABLE_SIG_ID,
	REQUIREMENTS_CATEGORIES_TABLE_SIG,
	NULL,
	NULL,
	REQUIREMENTS_CATEGORIES_COLUMNS_NAMES,
	REQUIREMENTS_CATEGORIES_COLUMNS_FORMATS,
	NULL,
	sizeof(REQUIREMENTS_CATEGORIES_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 								EXIGENCES DE TESTS
 -------------------------------------------------------------------*/
    //! Requirements and tests associations table name
#define		TESTS_REQUIREMENTS_TABLE_SIG								"tests_requirements_table"
#define		TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID						"test_content_id"
#define		TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID		"original_requirement_content_id"

    static const char *TESTS_REQUIREMENTS_COLUMNS_NAMES[] = {
	TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID,
	TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID,
	NULL
    };

    static const char *TESTS_REQUIREMENTS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NULL
    };

    static const unsigned int TESTS_REQUIREMENTS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE
    };


    static const char *TESTS_REQUIREMENTS_PRIMARY_KEY[] = {
	TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID,
	TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID,
	NULL
    };

    static const entity_def tests_requirements_table_def = {
	TESTS_REQUIREMENTS_TABLE_SIG_ID,
	TESTS_REQUIREMENTS_TABLE_SIG,
	TESTS_REQUIREMENTS_PRIMARY_KEY,
	NULL,
	TESTS_REQUIREMENTS_COLUMNS_NAMES,
	TESTS_REQUIREMENTS_COLUMNS_FORMATS,
	TESTS_REQUIREMENTS_COLUMNS_SIZES,
	sizeof(TESTS_REQUIREMENTS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 								CAMPAGNES
 -------------------------------------------------------------------*/
    //! Campaigns table name
#define		CAMPAIGNS_TABLE_SIG										"campaigns_table"
#define		CAMPAIGNS_TABLE_CAMPAIGN_ID_SEQ			"campaigns_campaign_id_seq"
#define		CAMPAIGNS_TABLE_CAMPAIGN_ID						"campaign_id"
#define		CAMPAIGNS_TABLE_PROJECT_ID							"project_id"
#define		CAMPAIGNS_TABLE_PROJECT_VERSION				"version"
#define		CAMPAIGNS_TABLE_SHORT_NAME						"short_name"
#define		CAMPAIGNS_TABLE_DESCRIPTION						"description"

    static const char *CAMPAIGNS_COLUMNS_NAMES[] = {
	CAMPAIGNS_TABLE_CAMPAIGN_ID,
	CAMPAIGNS_TABLE_PROJECT_ID,
	CAMPAIGNS_TABLE_PROJECT_VERSION,
	CAMPAIGNS_TABLE_SHORT_NAME,
	CAMPAIGNS_TABLE_DESCRIPTION,
	NULL
    };

    static const char *CAMPAIGNS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int CAMPAIGNS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	12,
	SMEDIUM_TEXT_SIZE,
	LLARGE_TEXT_SIZE
    };


    static const entity_def campaigns_table_def = {
	CAMPAIGNS_TABLE_SIG_ID,
	CAMPAIGNS_TABLE_SIG,
	NULL,
	CAMPAIGNS_TABLE_CAMPAIGN_ID_SEQ,
	CAMPAIGNS_COLUMNS_NAMES,
	CAMPAIGNS_COLUMNS_FORMATS,
	CAMPAIGNS_COLUMNS_SIZES,
	sizeof(CAMPAIGNS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 						TESTS DE CAMPAGNES
 -------------------------------------------------------------------*/
    //! Tests campaigns table name
#define		TESTS_CAMPAIGNS_TABLE_SIG													"tests_campaigns_table"
#define		TESTS_CAMPAIGNS_TABLE_TEST_CAMPAIGN_ID_SEQ				"tests_campaigns_test_campaign_id_seq"
#define		TESTS_CAMPAIGNS_TABLE_TEST_CAMPAIGN_ID						"test_campaign_id"
#define		TESTS_CAMPAIGNS_TABLE_CAMPAIGN_ID								"campaign_id"
#define		TESTS_CAMPAIGNS_TABLE_TEST_ID											"test_id"
#define		TESTS_CAMPAIGNS_TABLE_PREVIOUS_TEST_CAMPAIGN_ID	"previous_test_campaign_id"

    static const char *TESTS_CAMPAIGNS_COLUMNS_NAMES[] = {
	TESTS_CAMPAIGNS_TABLE_TEST_CAMPAIGN_ID,
	TESTS_CAMPAIGNS_TABLE_CAMPAIGN_ID,
	TESTS_CAMPAIGNS_TABLE_TEST_ID,
	TESTS_CAMPAIGNS_TABLE_PREVIOUS_TEST_CAMPAIGN_ID,
	NULL
    };

    static const char *TESTS_CAMPAIGNS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NULL
    };

    static const unsigned int TESTS_CAMPAIGNS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE
    };


    static const entity_def tests_campaigns_table_def = {
	TESTS_CAMPAIGNS_TABLE_SIG_ID,
	TESTS_CAMPAIGNS_TABLE_SIG,
	NULL,
	TESTS_CAMPAIGNS_TABLE_TEST_CAMPAIGN_ID_SEQ,
	TESTS_CAMPAIGNS_COLUMNS_NAMES,
	TESTS_CAMPAIGNS_COLUMNS_FORMATS,
	TESTS_CAMPAIGNS_COLUMNS_SIZES,
	sizeof(TESTS_CAMPAIGNS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 					EXECUTIONS DE CAMPAGNES
 -------------------------------------------------------------------*/
    //! Executions campaigns table name
#define		EXECUTIONS_CAMPAIGNS_TABLE_SIG													"executions_campaigns_table"
#define		EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_CAMPAIGN_ID_SEQ		"executions_campaigns_execution_campaign_id_seq"
#define		EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_CAMPAIGN_ID				"execution_campaign_id"
#define		EXECUTIONS_CAMPAIGNS_TABLE_CAMPAIGN_ID										"campaign_id"
#define		EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_DATE								"execution_date"
#define		EXECUTIONS_CAMPAIGNS_TABLE_REVISION												"revision"
#define		EXECUTIONS_CAMPAIGNS_TABLE_USER_ID												"user_id"

    static const char *EXECUTIONS_CAMPAIGNS_COLUMNS_NAMES[] = {
	EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_CAMPAIGN_ID,
	EXECUTIONS_CAMPAIGNS_TABLE_CAMPAIGN_ID,
	EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_DATE,
	EXECUTIONS_CAMPAIGNS_TABLE_REVISION,
	EXECUTIONS_CAMPAIGNS_TABLE_USER_ID,
	NULL
    };

    static const char *EXECUTIONS_CAMPAIGNS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	DATE_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	NULL
    };

    static const unsigned int EXECUTIONS_CAMPAIGNS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	DATE_SIZE,
	24,
	PRIMARY_KEY_SIZE
    };



    static const entity_def executions_campaigns_table_def = {
	EXECUTIONS_CAMPAIGNS_TABLE_SIG_ID,
	EXECUTIONS_CAMPAIGNS_TABLE_SIG,
	NULL,
	EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_CAMPAIGN_ID_SEQ,
	EXECUTIONS_CAMPAIGNS_COLUMNS_NAMES,
	EXECUTIONS_CAMPAIGNS_COLUMNS_FORMATS,
	EXECUTIONS_CAMPAIGNS_COLUMNS_SIZES,
	sizeof(EXECUTIONS_CAMPAIGNS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 				PARAMETRES D'EXECUTIONS DE CAMPAGNES
 -------------------------------------------------------------------*/
    //! Executions campaigns parameters table name
#define		EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_SIG									"executions_campaigns_parameters_table"
#define		EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_ID_SEQ				"executions_campaigns_parameters_parameter_id_seq"
#define		EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_EXECUTION_CAMPAIGN_PARAMETER_ID				"execution_campaign_parameter_id"
#define		EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_EXECUTION_CAMPAIGN_ID						"execution_campaign_id"
#define		EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_NAME						"parameter_name"
#define		EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_VALUE						"parameter_value"

    static const char *EXECUTIONS_CAMPAIGNS_PARAMETERS_COLUMNS_NAMES[] = {
	EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_EXECUTION_CAMPAIGN_PARAMETER_ID,
	EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_EXECUTION_CAMPAIGN_ID,
	EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_NAME,
	EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_VALUE,
	NULL
    };

    static const char *EXECUTIONS_CAMPAIGNS_PARAMETERS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int EXECUTIONS_CAMPAIGNS_PARAMETERS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	LMEDIUM_TEXT_SIZE,
	LMEDIUM_TEXT_SIZE
    };


    static const entity_def executions_campaigns_parameters_table_def = {
	EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_SIG_ID,
	EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_SIG,
	NULL,
	EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_ID_SEQ,
	EXECUTIONS_CAMPAIGNS_PARAMETERS_COLUMNS_NAMES,
	EXECUTIONS_CAMPAIGNS_PARAMETERS_COLUMNS_FORMATS,
	EXECUTIONS_CAMPAIGNS_PARAMETERS_COLUMNS_SIZES,
	sizeof(EXECUTIONS_CAMPAIGNS_PARAMETERS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 			EXECUTIONS DE TESTS DE CAMPAGNES
 -------------------------------------------------------------------*/
    //! Executions tests campaigns name
#define		EXECUTIONS_TESTS_TABLE_SIG														"executions_tests_table"
#define		EXECUTIONS_TESTS_TABLE_EXECUTION_TEST_ID_SEQ					"executions_tests_execution_test_id_seq"
#define		EXECUTIONS_TESTS_TABLE_EXECUTION_TEST_ID							"execution_test_id"
#define		EXECUTIONS_TESTS_TABLE_EXECUTION_CAMPAIGN_ID				"execution_campaign_id"
#define		EXECUTIONS_TESTS_TABLE_PARENT_EXECUTION_TEST_ID			"parent_execution_test_id"
#define		EXECUTIONS_TESTS_TABLE_PREVIOUS_EXECUTION_TEST_ID		"previous_execution_test_id"
#define		EXECUTIONS_TESTS_TABLE_TEST_ID												"test_id"
#define		EXECUTIONS_TESTS_TABLE_EXECUTION_DATE								"execution_date"
#define		EXECUTIONS_TESTS_TABLE_RESULT_ID											"result_id"
#define		EXECUTIONS_TESTS_TABLE_COMMENTS											"comments"

    static const char *EXECUTIONS_TESTS_COLUMNS_NAMES[] = {
	EXECUTIONS_TESTS_TABLE_EXECUTION_TEST_ID,
	EXECUTIONS_TESTS_TABLE_EXECUTION_CAMPAIGN_ID,
	EXECUTIONS_TESTS_TABLE_PARENT_EXECUTION_TEST_ID,
	EXECUTIONS_TESTS_TABLE_PREVIOUS_EXECUTION_TEST_ID,
	EXECUTIONS_TESTS_TABLE_TEST_ID,
	EXECUTIONS_TESTS_TABLE_EXECUTION_DATE,
	EXECUTIONS_TESTS_TABLE_RESULT_ID,
	EXECUTIONS_TESTS_TABLE_COMMENTS,
	NULL
    };

    static const char *EXECUTIONS_TESTS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	DATE_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int EXECUTIONS_TESTS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	DATE_SIZE,
	1,
	LLARGE_TEXT_SIZE
    };



    static const entity_def executions_tests_table_def = {
	EXECUTIONS_TESTS_TABLE_SIG_ID,
	EXECUTIONS_TESTS_TABLE_SIG,
	NULL,
	EXECUTIONS_TESTS_TABLE_EXECUTION_TEST_ID_SEQ,
	EXECUTIONS_TESTS_COLUMNS_NAMES,
	EXECUTIONS_TESTS_COLUMNS_FORMATS,
	EXECUTIONS_TESTS_COLUMNS_SIZES,
	sizeof(EXECUTIONS_TESTS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 				PARAMETRES D'EXECUTIONS DE TESTS
 -------------------------------------------------------------------*/
    //! Executions parameters table name
#define		EXECUTIONS_TESTS_PARAMETERS_TABLE_SIG									"executions_tests_parameters_table"
#define		EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_ID_SEQ				"executions_tests_parameters_parameter_id_seq"
#define		EXECUTIONS_TESTS_PARAMETERS_TABLE_EXECUTION_TEST_PARAMETER_ID				"execution_test_parameter_id"
#define		EXECUTIONS_TESTS_PARAMETERS_TABLE_EXECUTION_TEST_ID						"execution_test_id"
#define		EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_NAME						"parameter_name"
#define		EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_VALUE						"parameter_value"

    static const char *EXECUTIONS_TESTS_PARAMETERS_COLUMNS_NAMES[] = {
	EXECUTIONS_TESTS_PARAMETERS_TABLE_EXECUTION_TEST_PARAMETER_ID,
	EXECUTIONS_TESTS_PARAMETERS_TABLE_EXECUTION_TEST_ID,
	EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_NAME,
	EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_VALUE,
	NULL
    };

    static const char *EXECUTIONS_TESTS_PARAMETERS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int EXECUTIONS_TESTS_PARAMETERS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	LMEDIUM_TEXT_SIZE,
	LMEDIUM_TEXT_SIZE
    };


    static const entity_def executions_tests_parameters_table_def = {
	EXECUTIONS_TESTS_PARAMETERS_TABLE_SIG_ID,
	EXECUTIONS_TESTS_PARAMETERS_TABLE_SIG,
	NULL,
	EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_ID_SEQ,
	EXECUTIONS_TESTS_PARAMETERS_COLUMNS_NAMES,
	EXECUTIONS_TESTS_PARAMETERS_COLUMNS_FORMATS,
	EXECUTIONS_TESTS_PARAMETERS_COLUMNS_SIZES,
	sizeof(EXECUTIONS_TESTS_PARAMETERS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 	RESULTATS D'EXECUTIONS DE TESTS DE CAMPAGNES
 -------------------------------------------------------------------*/
    //! Executions tests results table name
#define		TESTS_RESULTS_TABLE_SIG														"tests_results_table"
#define		TESTS_RESULTS_TABLE_RESULT_ID											"result_id"
#define		TESTS_RESULTS_TABLE_DESCRIPTION										"description"

#define		EXECUTION_TEST_VALIDATED													"0"
#define		EXECUTION_TEST_INVALIDATED													"1"
#define		EXECUTION_TEST_BYPASSED														"2"
#define		EXECUTION_TEST_INCOMPLETED													"3"

    static const char *TESTS_RESULTS_COLUMNS_NAMES[] = {
	TESTS_RESULTS_TABLE_RESULT_ID,
	TESTS_RESULTS_TABLE_DESCRIPTION,
	NULL
    };

    static const char *TESTS_RESULTS_COLUMNS_FORMATS[] = {
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const entity_def tests_results_table_def = {
	TESTS_RESULTS_TABLE_SIG_ID,
	TESTS_RESULTS_TABLE_SIG,
	NULL,
	NULL,
	TESTS_RESULTS_COLUMNS_NAMES,
	TESTS_RESULTS_COLUMNS_FORMATS,
	NULL,
	sizeof(TESTS_RESULTS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 	EXECUTIONS D'ACTIONS DE TESTS DE CAMPAGNES
 -------------------------------------------------------------------*/
    //! Executions actions name
#define		EXECUTIONS_ACTIONS_TABLE_SIG																"executions_actions_table"
#define		EXECUTIONS_ACTIONS_TABLE_EXECUTION_ACTION_ID_SEQ					"executions_actions_execution_action_id_seq"
#define		EXECUTIONS_ACTIONS_TABLE_EXECUTION_ACTION_ID							"execution_action_id"
#define		EXECUTIONS_ACTIONS_TABLE_EXECUTION_TEST_ID									"execution_test_id"
#define		EXECUTIONS_ACTIONS_TABLE_ACTION_ID													"action_id"
#define		EXECUTIONS_ACTIONS_TABLE_PREVIOUS_EXECUTION_ACTION_ID		"previous_execution_action_id"
#define		EXECUTIONS_ACTIONS_TABLE_RESULT_ID													"result_id"
#define		EXECUTIONS_ACTIONS_TABLE_COMMENTS												"comments"

    static const char *EXECUTIONS_ACTIONS_COLUMNS_NAMES[] = {
	EXECUTIONS_ACTIONS_TABLE_EXECUTION_ACTION_ID,
	EXECUTIONS_ACTIONS_TABLE_EXECUTION_TEST_ID,
	EXECUTIONS_ACTIONS_TABLE_ACTION_ID,
	EXECUTIONS_ACTIONS_TABLE_PREVIOUS_EXECUTION_ACTION_ID,
	EXECUTIONS_ACTIONS_TABLE_RESULT_ID,
	EXECUTIONS_ACTIONS_TABLE_COMMENTS,
	NULL
    };

    static const char *EXECUTIONS_ACTIONS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int EXECUTIONS_ACTIONS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	1,
	LLARGE_TEXT_SIZE
    };


    static const entity_def executions_actions_table_def = {
	EXECUTIONS_ACTIONS_TABLE_SIG_ID,
	EXECUTIONS_ACTIONS_TABLE_SIG,
	NULL,
	EXECUTIONS_ACTIONS_TABLE_EXECUTION_ACTION_ID_SEQ,
	EXECUTIONS_ACTIONS_COLUMNS_NAMES,
	EXECUTIONS_ACTIONS_COLUMNS_FORMATS,
	EXECUTIONS_ACTIONS_COLUMNS_SIZES,
	sizeof(EXECUTIONS_ACTIONS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 	EXECUTIONS D'ACTIONS AUTOMATISEES DE TESTS DE CAMPAGNES
 -------------------------------------------------------------------*/
    //! Executions actions name
#define		AUTOMATED_EXECUTIONS_ACTIONS_TABLE_SIG								"automated_executions_actions_table"
#define		AUTOMATED_EXECUTIONS_ACTIONS_TABLE_EXECUTION_ACTION_ID_SEQ					"automated_executions_actions_execution_action_id_seq"
#define		AUTOMATED_EXECUTIONS_ACTIONS_TABLE_EXECUTION_ACTION_ID							"automated_execution_action_id"
#define		AUTOMATED_EXECUTIONS_ACTIONS_TABLE_EXECUTION_TEST_ID									"execution_test_id"
#define		AUTOMATED_EXECUTIONS_ACTIONS_TABLE_ACTION_ID													"automated_action_id"
#define		AUTOMATED_EXECUTIONS_ACTIONS_TABLE_PREVIOUS_EXECUTION_ACTION_ID		"previous_automated_execution_action_id"
#define		AUTOMATED_EXECUTIONS_ACTIONS_TABLE_RESULT_ID													"result_id"
#define		AUTOMATED_EXECUTIONS_ACTIONS_TABLE_COMMENTS												"comments"

    static const char *AUTOMATED_EXECUTIONS_ACTIONS_COLUMNS_NAMES[] = {
	AUTOMATED_EXECUTIONS_ACTIONS_TABLE_EXECUTION_ACTION_ID,
	AUTOMATED_EXECUTIONS_ACTIONS_TABLE_EXECUTION_TEST_ID,
	AUTOMATED_EXECUTIONS_ACTIONS_TABLE_ACTION_ID,
	AUTOMATED_EXECUTIONS_ACTIONS_TABLE_PREVIOUS_EXECUTION_ACTION_ID,
	AUTOMATED_EXECUTIONS_ACTIONS_TABLE_RESULT_ID,
	AUTOMATED_EXECUTIONS_ACTIONS_TABLE_COMMENTS,
	NULL
    };

    static const char *AUTOMATED_EXECUTIONS_ACTIONS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int AUTOMATED_EXECUTIONS_ACTIONS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	1,
	LLARGE_TEXT_SIZE
    };


    static const entity_def automated_executions_actions_table_def = {
	AUTOMATED_EXECUTIONS_ACTIONS_TABLE_SIG_ID,
	AUTOMATED_EXECUTIONS_ACTIONS_TABLE_SIG,
	NULL,
	AUTOMATED_EXECUTIONS_ACTIONS_TABLE_EXECUTION_ACTION_ID_SEQ,
	AUTOMATED_EXECUTIONS_ACTIONS_COLUMNS_NAMES,
	AUTOMATED_EXECUTIONS_ACTIONS_COLUMNS_FORMATS,
	AUTOMATED_EXECUTIONS_ACTIONS_COLUMNS_SIZES,
	sizeof(AUTOMATED_EXECUTIONS_ACTIONS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 	RESULTATS D'EXECUTIONS D'ACTIONS DE CAMPAGNES
 -------------------------------------------------------------------*/
    //! Excutions actions results table name
#define		ACTIONS_RESULTS_TABLE_SIG														"actions_results_table"
#define		ACTIONS_RESULTS_TABLE_RESULT_ID											"result_id"
#define		ACTIONS_RESULTS_TABLE_DESCRIPTION										"description"

#define		EXECUTION_ACTION_VALIDATED													"0"
#define		EXECUTION_ACTION_INVALIDATED												"1"
#define		EXECUTION_ACTION_BYPASSED														"2"

    static const char *ACTIONS_RESULTS_COLUMNS_NAMES[] = {
	ACTIONS_RESULTS_TABLE_RESULT_ID,
	ACTIONS_RESULTS_TABLE_DESCRIPTION,
	NULL
    };

    static const char *ACTIONS_RESULTS_COLUMNS_FORMATS[] = {
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const entity_def actions_results_table_def = {
	ACTIONS_RESULTS_TABLE_SIG_ID,
	ACTIONS_RESULTS_TABLE_SIG,
	NULL,
	NULL,
	ACTIONS_RESULTS_COLUMNS_NAMES,
	ACTIONS_RESULTS_COLUMNS_FORMATS,
	NULL,
	sizeof(ACTIONS_RESULTS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 	EXECUTIONS D'EXIGENCES DE CAMPAGNES
 -------------------------------------------------------------------*/
    //! Executions requirements table name
#define		EXECUTIONS_REQUIREMENTS_TABLE_SIG													"executions_requirements_table"
#define		EXECUTIONS_REQUIREMENTS_TABLE_EXECUTION_ACTION_ID_SEQ		"executions_requirements_execution_requirement_seq"
#define		EXECUTIONS_REQUIREMENTS_TABLE_EXECUTION_REQUIREMENT_ID		"execution_requirement_id"
#define		EXECUTIONS_REQUIREMENTS_TABLE_EXECUTION_CAMPAIGN_ID			"execution_campaign_id"
#define		EXECUTIONS_REQUIREMENTS_TABLE_TEST_CONTENT_ID						"test_content_id"
#define		EXECUTIONS_REQUIREMENTS_TABLE_REQUIREMENT_CONTENT__ID		"requirement_content_id"
#define		EXECUTIONS_REQUIREMENTS_TABLE_RESULT_ID										"result_id"
#define		EXECUTIONS_REQUIREMENTS_TABLE_COMMENTS										"comments"

    static const char *EXECUTIONS_REQUIREMENTS_COLUMNS_NAMES[] = {
	EXECUTIONS_REQUIREMENTS_TABLE_EXECUTION_REQUIREMENT_ID,
	EXECUTIONS_REQUIREMENTS_TABLE_EXECUTION_CAMPAIGN_ID,
	EXECUTIONS_REQUIREMENTS_TABLE_TEST_CONTENT_ID,
	EXECUTIONS_REQUIREMENTS_TABLE_REQUIREMENT_CONTENT__ID,
	EXECUTIONS_REQUIREMENTS_TABLE_RESULT_ID,
	EXECUTIONS_REQUIREMENTS_TABLE_COMMENTS,
	NULL
    };

    static const char *EXECUTIONS_REQUIREMENTS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int EXECUTIONS_REQUIREMENTS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	1,
	LARGE_TEXT_SIZE
    };

    static const entity_def executions_requirements_table_def = {
	EXECUTIONS_REQUIREMENTS_TABLE_SIG_ID,
	EXECUTIONS_REQUIREMENTS_TABLE_SIG,
	NULL,
	EXECUTIONS_REQUIREMENTS_TABLE_EXECUTION_ACTION_ID_SEQ,
	EXECUTIONS_REQUIREMENTS_COLUMNS_NAMES,
	EXECUTIONS_REQUIREMENTS_COLUMNS_FORMATS,
	EXECUTIONS_REQUIREMENTS_COLUMNS_SIZES,
	sizeof(EXECUTIONS_REQUIREMENTS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 								DROITS SUR LES PROJETS
 -------------------------------------------------------------------*/
    //! Projects grants table name
#define		PROJECTS_GRANTS_TABLE_SIG							"projects_grants_table"
#define		PROJECTS_GRANTS_TABLE_PROJECT_ID					"project_id"
#define		PROJECTS_GRANTS_TABLE_USERNAME						"username"
#define		PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC			"manage_tests_indic"
#define		PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC		"manage_requirements_indic"
#define		PROJECTS_GRANTS_TABLE_MANAGE_CAMPAIGNS_INDIC		"manage_campaigns_indic"
#define		PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC		"manage_executions_indic"

#define		PROJECT_GRANT_NONE			"N"
#define		PROJECT_GRANT_READ			"R"
#define		PROJECT_GRANT_WRITE			"W"

    static const char *PROJECTS_GRANTS_COLUMNS_NAMES[] = {
	PROJECTS_GRANTS_TABLE_PROJECT_ID,
	PROJECTS_GRANTS_TABLE_USERNAME,
	PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC,
	PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC,
	PROJECTS_GRANTS_TABLE_MANAGE_CAMPAIGNS_INDIC,
	PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC,
	NULL
    };

    static const char *PROJECTS_GRANTS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int PROJECTS_GRANTS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	SHORT_TEXT_SIZE,
	1,
	1,
	1,
	1
    };

    static const char *PROJECTS_GRANTS_PRIMARY_KEY[] = {
	PROJECTS_GRANTS_TABLE_PROJECT_ID,
	PROJECTS_GRANTS_TABLE_USERNAME,
	NULL
    };


    static const entity_def projects_grants_table_def = {
	PROJECTS_GRANTS_TABLE_SIG_ID,
	PROJECTS_GRANTS_TABLE_SIG,
	PROJECTS_GRANTS_PRIMARY_KEY,
	NULL,
	PROJECTS_GRANTS_COLUMNS_NAMES,
	PROJECTS_GRANTS_COLUMNS_FORMATS,
	PROJECTS_GRANTS_COLUMNS_SIZES,
	sizeof(PROJECTS_GRANTS_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 							HIERARCHIE DE TESTS
 -------------------------------------------------------------------*/
    //! Tests hierarchy view name
#define		TESTS_HIERARCHY_SIG										"tests_hierarchy"
#define		TESTS_HIERARCHY_TEST_ID									"test_id"
#define		TESTS_HIERARCHY_ORIGINAL_TEST_ID						"original_test_id"
#define		TESTS_HIERARCHY_PARENT_TEST_ID							"parent_test_id"
#define		TESTS_HIERARCHY_PREVIOUS_TEST_ID						"previous_test_id"
#define		TESTS_HIERARCHY_PROJECT_ID								"project_id"
#define		TESTS_HIERARCHY_VERSION									"version"
#define		TESTS_HIERARCHY_TEST_CONTENT_ID							"test_content_id"
#define		TESTS_HIERARCHY_SHORT_NAME								"short_name"
#define		TESTS_HIERARCHY_CATEGORY_ID								"category_id"
#define		TESTS_HIERARCHY_PRIORITY_LEVEL							"priority_level"
#define		TESTS_HIERARCHY_CONTENT_VERSION							"content_version"
#define		TESTS_HIERARCHY_STATUS									"status"
#define		TESTS_HIERARCHY_ORIGINAL_TEST_CONTENT_ID				"original_test_content_id"
#define		TESTS_HIERARCHY_TEST_CONTENT_AUTOMATED				    "content_automated"
#define		TESTS_HIERARCHY_TEST_CONTENT_TYPE					    "content_type"

    static const char *TESTS_HIERARCHY_COLUMNS_NAMES[] = {
	TESTS_HIERARCHY_TEST_ID,
	TESTS_HIERARCHY_ORIGINAL_TEST_ID,
	TESTS_HIERARCHY_PARENT_TEST_ID,
	TESTS_HIERARCHY_PREVIOUS_TEST_ID,
	TESTS_HIERARCHY_PROJECT_ID,
	TESTS_HIERARCHY_VERSION,
        TESTS_HIERARCHY_TEST_CONTENT_ID,
	TESTS_HIERARCHY_SHORT_NAME,
	TESTS_HIERARCHY_CATEGORY_ID,
	TESTS_HIERARCHY_PRIORITY_LEVEL,
	TESTS_HIERARCHY_CONTENT_VERSION,
	TESTS_HIERARCHY_STATUS,
	TESTS_HIERARCHY_ORIGINAL_TEST_CONTENT_ID,
	TESTS_HIERARCHY_TEST_CONTENT_AUTOMATED,
	TESTS_HIERARCHY_TEST_CONTENT_TYPE,
	NULL
    };

    static const char *TESTS_HIERARCHY_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
        NUMBER_FORMAT,
        STRING_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int TESTS_HIERARCHY_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	12,
        PRIMARY_KEY_SIZE,
	SHORT_TEXT_SIZE,
	1,
	1,
	12,
	1,
	PRIMARY_KEY_SIZE,
	1,
	1
    };

    static const entity_def tests_hierarchy_def = {
	TESTS_HIERARCHY_SIG_ID,
	TESTS_HIERARCHY_SIG,
	NULL,
	NULL,
	TESTS_HIERARCHY_COLUMNS_NAMES,
	TESTS_HIERARCHY_COLUMNS_FORMATS,
	TESTS_HIERARCHY_COLUMNS_SIZES,
	sizeof(TESTS_HIERARCHY_COLUMNS_NAMES)/sizeof(char*) - 1
    };

    /*-------------------------------------------------------------------
 * 								HIERARCHIE D'EXIGENCES
 -------------------------------------------------------------------*/
    //! Requirements hierarchy view name
#define		REQUIREMENTS_HIERARCHY_SIG							"requirements_hierarchy"
#define		REQUIREMENTS_HIERARCHY_REQUIREMENT_ID				"requirement_id"
#define		REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID		"requirement_content_id"
#define		REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID		"parent_requirement_id"
#define		REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID		"previous_requirement_id"
#define		REQUIREMENTS_HIERARCHY_PROJECT_ID					"project_id"
#define		REQUIREMENTS_HIERARCHY_VERSION						"version"
#define		REQUIREMENTS_HIERARCHY_SHORT_NAME					"short_name"
#define		REQUIREMENTS_HIERARCHY_CATEGORY_ID					"category_id"
#define		REQUIREMENTS_HIERARCHY_PRIORITY_LEVEL				"priority_level"
#define		REQUIREMENTS_HIERARCHY_CONTENT_VERSION				"content_version"
#define		REQUIREMENTS_HIERARCHY_STATUS						"status"
#define		REQUIREMENTS_HIERARCHY_ORIGINAL_REQUIREMENT_CONTENT_ID		"original_requirement_content_id"

    static const char *REQUIREMENTS_HIERARCHY_COLUMNS_NAMES[] = {
	REQUIREMENTS_HIERARCHY_REQUIREMENT_ID,
	REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID,
	REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID,
	REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID,
	REQUIREMENTS_HIERARCHY_PROJECT_ID,
	REQUIREMENTS_HIERARCHY_VERSION,
	REQUIREMENTS_HIERARCHY_SHORT_NAME,
	REQUIREMENTS_HIERARCHY_CATEGORY_ID,
	REQUIREMENTS_HIERARCHY_PRIORITY_LEVEL,
	REQUIREMENTS_HIERARCHY_CONTENT_VERSION,
	REQUIREMENTS_HIERARCHY_STATUS,
	REQUIREMENTS_HIERARCHY_ORIGINAL_REQUIREMENT_CONTENT_ID,
	NULL
    };

    static const char *REQUIREMENTS_HIERARCHY_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NUMBER_FORMAT,
	NULL
    };

    static const unsigned int REQUIREMENTS_HIERARCHY_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	12,
	SHORT_TEXT_SIZE,
	1,
	1,
	12,
	1,
	PRIMARY_KEY_SIZE
    };


    static const entity_def requirements_hierarchy_def = {
	REQUIREMENTS_HIERARCHY_SIG_ID,
	REQUIREMENTS_HIERARCHY_SIG,
	NULL,
	NULL,
	REQUIREMENTS_HIERARCHY_COLUMNS_NAMES,
	REQUIREMENTS_HIERARCHY_COLUMNS_FORMATS,
	REQUIREMENTS_HIERARCHY_COLUMNS_SIZES,
	sizeof(REQUIREMENTS_HIERARCHY_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 	ANOMALIES
 -------------------------------------------------------------------*/
    //! Bugs table name
#define		BUGS_TABLE_SIG																"bugs_table"
#define		BUGS_TABLE_BUG_ID_SEQ														"bugs_bug_id_seq"
#define		BUGS_TABLE_BUG_ID															"bug_id"
#define		BUGS_TABLE_EXECUTION_TEST_ID												"execution_test_id"
#define		BUGS_TABLE_EXECUTION_ACTION_ID												"execution_action_id"
#define		BUGS_TABLE_CREATION_DATE													"creation_date"
#define		BUGS_TABLE_SHORT_NAME														"short_name"
#define		BUGS_TABLE_PRIORITY															"priority"
#define		BUGS_TABLE_SEVERITY															"severity"
#define		BUGS_TABLE_REPRODUCIBILITY														"reproducibility"
#define		BUGS_TABLE_PLATFORM															"platform"
#define		BUGS_TABLE_SYSTEM															"system"
#define		BUGS_TABLE_DESCRIPTION														"description"
#define		BUGS_TABLE_BUGTRACKER_BUG_ID												"bugtracker_bug_id"
#define		BUGS_TABLE_STATUS															"status"

#define		BUG_STATUS_OPENED															"O"
#define		BUG_STATUS_CLOSED															"C"

    static const char *BUGS_COLUMNS_NAMES[] = {
	BUGS_TABLE_BUG_ID,
	BUGS_TABLE_EXECUTION_TEST_ID,
	BUGS_TABLE_EXECUTION_ACTION_ID,
	BUGS_TABLE_CREATION_DATE,
	BUGS_TABLE_SHORT_NAME,
	BUGS_TABLE_PRIORITY,
	BUGS_TABLE_SEVERITY,
	BUGS_TABLE_REPRODUCIBILITY,
	BUGS_TABLE_PLATFORM,
	BUGS_TABLE_SYSTEM,
	BUGS_TABLE_DESCRIPTION,
	BUGS_TABLE_BUGTRACKER_BUG_ID,
	BUGS_TABLE_STATUS,
	NULL
    };

    static const char *BUGS_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	NUMBER_FORMAT,
	DATE_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int BUGS_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	PRIMARY_KEY_SIZE,
	DATE_SIZE,
	SMEDIUM_TEXT_SIZE,
	SHORT_TEXT_SIZE,
	SHORT_TEXT_SIZE,
	SHORT_TEXT_SIZE,
	SHORT_TEXT_SIZE,
	SHORT_TEXT_SIZE,
	LLARGE_TEXT_SIZE,
	SHORT_TEXT_SIZE,
	1
    };


    static const entity_def bugs_table_def = {
	BUGS_TABLE_SIG_ID,
	BUGS_TABLE_SIG,
	NULL,
	BUGS_TABLE_BUG_ID_SEQ,
	BUGS_COLUMNS_NAMES,
	BUGS_COLUMNS_FORMATS,
	BUGS_COLUMNS_SIZES,
	sizeof(BUGS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 		STATUS DES CONTENUS D'EXIGENCES ET DES CONTENUS DE TESTS
 -------------------------------------------------------------------*/
    //! Requirements and tests status table name
#define		STATUS_TABLE_SIG									"status_table"
#define		STATUS_TABLE_STATUS_ID			"status_id"
#define		STATUS_TABLE_STATUS_LABEL		"status_label"

    static const char *STATUS_COLUMNS_NAMES[] = {
	STATUS_TABLE_STATUS_ID,
	STATUS_TABLE_STATUS_LABEL,
	NULL
    };

    static const char *STATUS_COLUMNS_FORMATS[] = {
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const entity_def status_table_def = {
	STATUS_TABLE_SIG_ID,
	STATUS_TABLE_SIG,
	NULL,
	NULL,
	STATUS_COLUMNS_NAMES,
	STATUS_COLUMNS_FORMATS,
	NULL,
	sizeof(STATUS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 		TYPES DE TESTS
 -------------------------------------------------------------------*/
    //! Tests types table name
#define		TESTS_TYPES_TABLE_SIG									"tests_types_table"
#define		TESTS_TYPES_TABLE_TEST_TYPE_ID			"test_type_id"
#define		TESTS_TYPES_TABLE_TEST_TYPE_LABEL		"test_type_label"

    static const char *TESTS_TYPES_COLUMNS_NAMES[] = {
	TESTS_TYPES_TABLE_TEST_TYPE_ID,
	TESTS_TYPES_TABLE_TEST_TYPE_LABEL,
	NULL
    };

    static const char *TESTS_TYPES_COLUMNS_FORMATS[] = {
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const entity_def tests_types_table_def = {
	TESTS_TYPES_TABLE_SIG_ID,
	TESTS_TYPES_TABLE_SIG,
	NULL,
	NULL,
	TESTS_TYPES_COLUMNS_NAMES,
	TESTS_TYPES_COLUMNS_FORMATS,
	NULL,
	sizeof(TESTS_TYPES_COLUMNS_NAMES)/sizeof(char*) - 1
    };


    /*-------------------------------------------------------------------
 * 		DEFINITION DES CHAMPS PERSONNALISES
 -------------------------------------------------------------------*/
    //! Custom fields table name
#define		CUSTOM_FIELDS_DESC_TABLE_SIG						"custom_fields_desc_table"
#define     CUSTOM_FIELDS_DESC_TABLE_ID_SEQ                                            "custom_fields_desc_id_seq"
#define		CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_ID			"custom_field_desc_id"
#define		CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_ENTITY			"entity_type"
#define		CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME			"tab_name"
#define		CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_LABEL			"field_label"
#define		CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE			"field_type"
#define		CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_MANDATORY			"mandatory"
#define		CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE			"default_value"
#define		CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_VALUES			"comma_separated_values"


#define CUSTOM_FIELDS_TEST             "T"
#define CUSTOM_FIELDS_REQUIERMENT     "R"

#define CUSTOM_FIELDS_PLAIN_TEXT    "P"
#define CUSTOM_FIELDS_RICH_TEXT     "R"
#define CUSTOM_FIELDS_CHECKBOX		"C"
#define CUSTOM_FIELDS_LIST          "L"
#define CUSTOM_FIELDS_INTEGER		"I"
#define CUSTOM_FIELDS_FLOAT 		"F"
#define CUSTOM_FIELDS_TIME          "H"
#define CUSTOM_FIELDS_DATE          "J"
#define CUSTOM_FIELDS_DATETIME      "D"

    static const char *CUSTOM_FIELDS_DESC_COLUMNS_NAMES[] = {
	CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_ID,
	CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_ENTITY,
	CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME,
	CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_LABEL,
	CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE,
	CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_MANDATORY,
	CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE,
	CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_VALUES,
	NULL
    };

    static const char *CUSTOM_FIELDS_DESC_COLUMNS_FORMATS[] = {
	NUMBER_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	STRING_FORMAT,
	NULL
    };

    static const unsigned int CUSTOM_FIELDS_DESC_COLUMNS_SIZES[] = {
	PRIMARY_KEY_SIZE,
	1,
	SMEDIUM_TEXT_SIZE,
	SMEDIUM_TEXT_SIZE,
	1,
	1,
    LLARGE_TEXT_SIZE,
	LLARGE_TEXT_SIZE
    };


    static const entity_def custom_fields_desc_table_def = {
	CUSTOM_FIELDS_DESC_TABLE_SIG_ID,
	CUSTOM_FIELDS_DESC_TABLE_SIG,
	NULL,
	CUSTOM_FIELDS_DESC_TABLE_ID_SEQ,
	CUSTOM_FIELDS_DESC_COLUMNS_NAMES,
	CUSTOM_FIELDS_DESC_COLUMNS_FORMATS,
	CUSTOM_FIELDS_DESC_COLUMNS_SIZES,
	sizeof(CUSTOM_FIELDS_DESC_COLUMNS_NAMES)/sizeof(char*) - 1
    };


/*-------------------------------------------------------------------
 *              CHAMPS PERSONNALISES DE TEST
 -------------------------------------------------------------------*/
    //! Custom test fields table name
#define         CUSTOM_TEST_FIELDS_TABLE_SIG                                            "custom_test_fields_table"
#define         CUSTOM_TEST_FIELDS_TABLE_ID_SEQ                                         "custom_test_fields_id_seq"
#define         CUSTOM_TEST_FIELDS_TABLE_CUSTOM_TEST_FIELD_ID                   "custom_test_field_id"
#define         CUSTOM_TEST_FIELDS_TABLE_CUSTOM_FIELD_DESC_ID                   "custom_field_desc_id"
#define         CUSTOM_TEST_FIELDS_TABLE_TEST_CONTENT_ID                       "test_content_id"
#define         CUSTOM_TEST_FIELDS_TABLE_FIELD_VALUE                        "field_value"

    static const char *CUSTOM_TEST_FIELDS_COLUMNS_NAMES[] = {
      CUSTOM_TEST_FIELDS_TABLE_CUSTOM_TEST_FIELD_ID,
      CUSTOM_TEST_FIELDS_TABLE_CUSTOM_FIELD_DESC_ID,
      CUSTOM_TEST_FIELDS_TABLE_TEST_CONTENT_ID,
      CUSTOM_TEST_FIELDS_TABLE_FIELD_VALUE,
        NULL
    };

    static const char *CUSTOM_TEST_FIELDS_COLUMNS_FORMATS[] = {
        NUMBER_FORMAT,
        NUMBER_FORMAT,
        NUMBER_FORMAT,
        STRING_FORMAT,
        NULL
    };

    static const unsigned int CUSTOM_TEST_FIELDS_COLUMNS_SIZES[] = {
        PRIMARY_KEY_SIZE,
        PRIMARY_KEY_SIZE,
        PRIMARY_KEY_SIZE,
        LLARGE_TEXT_SIZE
    };


    static const entity_def custom_test_fields_table_def = {
        CUSTOM_TEST_FIELDS_TABLE_SIG_ID,
        CUSTOM_TEST_FIELDS_TABLE_SIG,
        NULL,
        CUSTOM_TEST_FIELDS_TABLE_ID_SEQ,
        CUSTOM_TEST_FIELDS_COLUMNS_NAMES,
        CUSTOM_TEST_FIELDS_COLUMNS_FORMATS,
        CUSTOM_TEST_FIELDS_COLUMNS_SIZES,
        sizeof(CUSTOM_TEST_FIELDS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


/*-------------------------------------------------------------------
*              CHAMPS PERSONNALISES D'EXIGENCE
-------------------------------------------------------------------*/
//! Custom requirement fields table name
#define         CUSTOM_REQUIREMENT_FIELDS_TABLE_SIG                                            "custom_requirement_fields_table"
#define         CUSTOM_REQUIREMENT_FIELDS_TABLE_ID_SEQ                                         "custom_requirement_fields_id_seq"
#define         CUSTOM_REQUIREMENT_FIELDS_TABLE_CUSTOM_REQUIREMENT_FIELD_ID                   "custom_requirement_field_id"
#define         CUSTOM_REQUIREMENT_FIELDS_TABLE_CUSTOM_FIELD_DESC_ID                   "custom_field_desc_id"
#define         CUSTOM_REQUIREMENT_FIELDS_TABLE_REQUIREMENT_CONTENT_ID                       "requirement_content_id"
#define         CUSTOM_REQUIREMENT_FIELDS_TABLE_FIELD_VALUE                        "field_value"

    static const char *CUSTOM_REQUIREMENT_FIELDS_COLUMNS_NAMES[] = {
      CUSTOM_REQUIREMENT_FIELDS_TABLE_CUSTOM_REQUIREMENT_FIELD_ID,
      CUSTOM_REQUIREMENT_FIELDS_TABLE_CUSTOM_FIELD_DESC_ID,
      CUSTOM_REQUIREMENT_FIELDS_TABLE_REQUIREMENT_CONTENT_ID,
      CUSTOM_REQUIREMENT_FIELDS_TABLE_FIELD_VALUE,
        NULL
    };

    static const char *CUSTOM_REQUIREMENT_FIELDS_COLUMNS_FORMATS[] = {
        NUMBER_FORMAT,
        NUMBER_FORMAT,
        NUMBER_FORMAT,
        STRING_FORMAT,
        NULL
    };

    static const unsigned int CUSTOM_REQUIREMENT_FIELDS_COLUMNS_SIZES[] = {
        PRIMARY_KEY_SIZE,
        PRIMARY_KEY_SIZE,
        PRIMARY_KEY_SIZE,
        LLARGE_TEXT_SIZE
    };


    static const entity_def custom_requirement_fields_table_def = {
      CUSTOM_REQUIREMENT_FIELDS_TABLE_SIG_ID,
        CUSTOM_REQUIREMENT_FIELDS_TABLE_SIG,
        NULL,
        CUSTOM_REQUIREMENT_FIELDS_TABLE_ID_SEQ,
        CUSTOM_REQUIREMENT_FIELDS_COLUMNS_NAMES,
        CUSTOM_REQUIREMENT_FIELDS_COLUMNS_FORMATS,
        CUSTOM_REQUIREMENT_FIELDS_COLUMNS_SIZES,
        sizeof(CUSTOM_REQUIREMENT_FIELDS_COLUMNS_NAMES)/sizeof(char*) - 1
    };


#ifdef __cplusplus
}
#endif

#endif /*ENTITIES_DEF_H_*/

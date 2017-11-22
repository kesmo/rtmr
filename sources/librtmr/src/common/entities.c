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

#if defined (__APPLE__)
	#include <malloc/malloc.h>
#else
		#include <malloc.h>
#endif

#include "constants.h"
#include "entities.h"

#include <stdio.h>
#include <string.h>


/***********************************************************************************	*
*	get_table_def																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Recuperer la définition d'une table																		. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|	in_signature : signature de la table														*
*											|	in_out_entity_def : definition de l'entite*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|																															*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20090102	|	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
DLLEXPORT int DLLCALL get_table_def(int in_signature, entity_def **in_out_entity_def)
{
	if(in_out_entity_def == NULL)	return EMPTY_OBJECT;

	/* Chercher le définition de l'entités */
	switch (in_signature)
	{
		case FILES_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&files_table_def;
			break;

		case USERS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&users_table_def;
			break;

		case GROUPS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&groups_table_def;
			break;

		case SHARED_FILES_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&shared_files_table_def;
			break;

		case USERS_GROUPS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&users_groups_table_def;
			break;

		case CONTACTS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&contacts_table_def;
			break;

		case CONTACTS_GROUPS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&contacts_groups_table_def;
			break;

		case PROJECTS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&projects_table_def;
			break;

		case PROJECTS_VERSIONS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&projects_versions_table_def;
			break;

		case TESTS_CONTENTS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&tests_contents_table_def;
			break;

		case TESTS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&tests_table_def;
			break;

		case ACTIONS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&actions_table_def;
			break;

		case REQUIREMENTS_CONTENTS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&requirements_contents_table_def;
			break;

		case REQUIREMENTS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&requirements_table_def;
			break;

		case TESTS_REQUIREMENTS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&tests_requirements_table_def;
			break;

		case PROJECTS_PARAMETERS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&projects_parameters_table_def;
			break;

		case AUTOMATED_ACTIONS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&automated_actions_table_def;
			break;
			
		case REQUIREMENTS_CATEGORIES_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&requirements_categories_table_def;
			break;

		case CAMPAIGNS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&campaigns_table_def;
			break;

		case TESTS_CAMPAIGNS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&tests_campaigns_table_def;
			break;

		case EXECUTIONS_CAMPAIGNS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&executions_campaigns_table_def;
			break;

		case EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&executions_campaigns_parameters_table_def;
			break;

		case EXECUTIONS_TESTS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&executions_tests_table_def;
			break;

		case TESTS_RESULTS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&tests_results_table_def;
			break;

		case EXECUTIONS_ACTIONS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&executions_actions_table_def;
			break;

		case ACTIONS_RESULTS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&actions_results_table_def;
			break;

		case EXECUTIONS_REQUIREMENTS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&executions_requirements_table_def;
			break;

		case TESTS_CONTENTS_FILES_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&tests_contents_files_table_def;
			break;

		case PROJECTS_GRANTS_TABLE_SIG_ID:
			*in_out_entity_def = (entity_def*)&projects_grants_table_def;
			break;

		case TESTS_HIERARCHY_SIG_ID:
			*in_out_entity_def = (entity_def*)&tests_hierarchy_def;
			break;

		case REQUIREMENTS_HIERARCHY_SIG_ID:
			*in_out_entity_def = (entity_def*)&requirements_hierarchy_def;
			break;

		case BUGS_TABLE_SIG_ID:
			*in_out_entity_def = (entity_def*)&bugs_table_def;
			break;

		case STATUS_TABLE_SIG_ID:
			*in_out_entity_def = (entity_def*)&status_table_def;
			break;

		case TESTS_TYPES_TABLE_SIG_ID:
			*in_out_entity_def = (entity_def*)&tests_types_table_def;
			break;

		case EXECUTIONS_TESTS_PARAMETERS_TABLE_SIG_ID:
			*in_out_entity_def = (entity_def*)&executions_tests_parameters_table_def;
			break;

		case AUTOMATED_ACTIONS_VALIDATIONS_TABLE_SIG_ID :
			*in_out_entity_def = (entity_def*)&automated_actions_validations_table_def;
			break;
			
        case CUSTOM_FIELDS_DESC_TABLE_SIG_ID:
                *in_out_entity_def = (entity_def*)&custom_fields_desc_table_def;
                break;

        case CUSTOM_TEST_FIELDS_TABLE_SIG_ID:
                *in_out_entity_def = (entity_def*)&custom_test_fields_table_def;
                break;

        case CUSTOM_REQUIREMENT_FIELDS_TABLE_SIG_ID:
                *in_out_entity_def = (entity_def*)&custom_requirement_fields_table_def;
                break;

		default:
			return UNKNOW_ENTITY;
			break;
	}

	return NOERR;
}

/***********************************************************************************	*
*	get_ldap_entry_def																																			*
*----------------------------------------------------------------------------------------------------------------------------	*
*	Recuperer la définition d'une entree ldap																		. 							*
*----------------------------------------------------------------------------------------------------------------------------	*
*					Arguments	|	in_signature : signature de la table														*
*											|	in_out_table_name : nom de l'entree ldap cherchée									*
*											|	in_out_ldap_attributes_names : attributs de l'entree ldap*
*----------------------------------------------------------------------------------------------------------------------------	*
*							Retour	|																															*
*************************************************************************************
*	Date			|	Auteur	|	Version	|	Description																				*
*----------------------------------------------------------------------------------------------------------------------------	*
*	20090618 |	EJO			|	1.0.0		|	Création																					*
*************************************************************************************/
int get_ldap_entry_def(int in_signature, char ***in_out_ldap_attributes_names)
{
	if(in_out_ldap_attributes_names == NULL)	return EMPTY_OBJECT;

	/* Chercher le définition de l'entités */
	switch (in_signature)
	{
		case LDAP_USERS_SIG_ID :
                        *in_out_ldap_attributes_names = (char**)LDAP_USERS_ATTRIBUTES_NAMES;
			break;

		default:
			return UNKNOW_LDAP_TYPE_ENTRY;
			break;
	}

	return NOERR;
}


char* value_for_key(entity_def *in_entity_def, char** in_columns_values, const char* in_key)
{
	unsigned int tmp_index = 0;

	if (in_entity_def == NULL || in_columns_values == NULL || in_key == NULL)	return NULL;

	for (tmp_index = 0; tmp_index < in_entity_def->m_entity_columns_count; tmp_index++)
	{
		if (strcmp(in_key, in_entity_def->m_entity_columns_names[tmp_index]) == 0)
			return in_columns_values[tmp_index];
	}

	return NULL;
}


int index_for_key(entity_def *in_entity_def, const char* in_key)
{
	unsigned int tmp_index = 0;

	if (in_entity_def == NULL || in_key == NULL)	return -1;

	for (tmp_index = 0; tmp_index < in_entity_def->m_entity_columns_count; tmp_index++)
	{
		if (strcmp(in_key, in_entity_def->m_entity_columns_names[tmp_index]) == 0)
			return tmp_index;
	}

	return -1;
}

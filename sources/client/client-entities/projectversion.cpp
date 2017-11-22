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

#include "projectgrant.h"
#include "projectversion.h"
#include "testhierarchy.h"
#include "testcontent.h"
#include "genericrecord.h"
#include "requirementhierarchy.h"
#include "requirementcontent.h"
#include "campaign.h"
#include "client.h"
#include "entities.h"
#include "utilities.h"

#include <string.h>
#if defined (__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

/**
  Constructeur
**/
ProjectVersion::ProjectVersion() : Hierarchy(getEntityDef())
{
    _m_project = NULL;
}


/**
  Constructeur
**/
ProjectVersion::ProjectVersion(Project *in_project) : Hierarchy(getEntityDef())
{
    _m_project = NULL;
    setProject(in_project);
}

ProjectVersion::~ProjectVersion()
{
    qDeleteAll(_m_tests_hierarchy);
    qDeleteAll(_m_requirements_hierarchy);
    qDeleteAll(_m_campaigns_list);
    delete _m_project;
    destroy();
}


/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* ProjectVersion::getEntityDef() const
{
    return &projects_versions_table_def;
}


void ProjectVersion::setProject(Project *in_project)
{
    _m_project = in_project;
}

bool ProjectVersion::insertCopyOfChildren(int position, int count, GenericRecord *in_item, int in_child_type)
{
    TestHierarchy	*tmp_test = NULL;
    RequirementHierarchy	*tmp_requirement = NULL;
    net_session		*tmp_session = CLIENT_SESSION;
    char		***tmp_results = NULL;

    unsigned long	tmp_rows_count, tmp_columns_count;

    bool                tmp_return = false;

    if (in_item != NULL)
    {
        if (in_child_type == TESTS_HIERARCHY_SIG_ID)
        {
            if (in_item->getEntityDefSignatureId() == TESTS_HIERARCHY_SIG_ID)
            {
                sprintf(tmp_session->m_last_query, "select create_test_from_test(%s, '%s', %s, %s, %s, NULL);",
                        _m_project->getIdentifier(),
                        getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION),
                        "NULL",
                        (position > 0 ? _m_tests_hierarchy[position -1]->getIdentifier() : "NULL"),
                        in_item->getIdentifier());
                tmp_results = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
                if (tmp_results != NULL)
                {
                    if (tmp_rows_count == 1 && tmp_columns_count == 1)
                    {
                        if (is_empty_string(tmp_results[0][0]) == FALSE)
                        {
                            tmp_test = new TestHierarchy(this);
                            if (tmp_test->loadRecord(tmp_results[0][0]) == NOERR)
                            {
                                if (insertChildren(position, count, tmp_test, in_child_type))
                                {
                                    tmp_return = (tmp_test->loadChilds() == NOERR);
                                }
                            }
                        }
                    }

                    cl_free_rows_columns_array(&tmp_results, tmp_rows_count, tmp_columns_count);
                }

            }
            else if (in_item->getEntityDefSignatureId() == REQUIREMENTS_HIERARCHY_SIG_ID)
            {
                sprintf(tmp_session->m_last_query, "select create_test_from_requirement(%s, '%s', %s, %s, %s);",
                        _m_project->getIdentifier(),
                        getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION),
                        "NULL",
                        (position > 0 ? _m_tests_hierarchy[position -1]->getIdentifier() : "NULL"),
                        in_item->getIdentifier());
                tmp_results = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
                if (tmp_results != NULL)
                {
                    if (tmp_rows_count == 1 && tmp_columns_count == 1)
                    {
                        if (is_empty_string(tmp_results[0][0]) == FALSE)
                        {
                            tmp_test = new TestHierarchy(this);
                            if (tmp_test->loadRecord(tmp_results[0][0]) == NOERR)
                            {
                                if (insertChildren(position, count, tmp_test, in_child_type))
                                {
                                    tmp_return = (tmp_test->loadChilds() == NOERR);
                                }
                            }
                        }
                    }

                    cl_free_rows_columns_array(&tmp_results, tmp_rows_count, tmp_columns_count);
                }
            }
        }
        else if (in_child_type == REQUIREMENTS_HIERARCHY_SIG_ID)
        {
            sprintf(tmp_session->m_last_query, "select create_requirement_from_requirement(%s, '%s', %s, %s, %s, NULL);",
                    _m_project->getIdentifier(),
                    getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION),
                    "NULL",
                    (position > 0 ? _m_requirements_hierarchy[position -1]->getIdentifier() : "NULL"),
                    in_item->getIdentifier());
            tmp_results = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
            if (tmp_results != NULL)
            {
                if (tmp_rows_count == 1 && tmp_columns_count == 1)
                {
                    if (is_empty_string(tmp_results[0][0]) == FALSE)
                    {
                        tmp_requirement = new RequirementHierarchy(this);
                        if (tmp_requirement->loadRecord(tmp_results[0][0]) == NOERR)
                        {
                            if (insertChildren(position, count, tmp_requirement, in_child_type))
                            {
                                tmp_return = (tmp_requirement->loadChilds() == NOERR);
                            }
                        }
                    }
                }

                cl_free_rows_columns_array(&tmp_results, tmp_rows_count, tmp_columns_count);
            }
        }
    }

    return tmp_return;
}


bool ProjectVersion::insertChildren(int position, int /*count*/, Hierarchy *in_child, int in_child_type)
{
    TestHierarchy*			tmp_child_test = NULL;
    RequirementHierarchy*		tmp_child_requirement = NULL;

    TestContent*		tmp_test_content = NULL;
    RequirementContent*	tmp_requirement_content = NULL;

    int			tmp_save_result = NOERR;

    switch (in_child_type)
    {
    case TESTS_HIERARCHY_SIG_ID:
        if (in_child == NULL)
        {
            tmp_test_content = new TestContent(this);
            tmp_test_content->setValueForKey("", TESTS_CONTENTS_TABLE_SHORT_NAME);
            tmp_save_result = tmp_test_content->saveRecord();
            if (tmp_save_result == NOERR)
            {
                tmp_child_test = new TestHierarchy(this);
                tmp_child_test->setDataFromTestContent(tmp_test_content);

                return insertTestAtIndex(tmp_child_test, position);
            }
            delete tmp_test_content;
        }
        else if (in_child->getEntityDefSignatureId() == TESTS_HIERARCHY_SIG_ID)
        {
            return insertTestAtIndex((TestHierarchy*)in_child, position);
        }
        else if (in_child->getEntityDefSignatureId() == REQUIREMENTS_HIERARCHY_SIG_ID)
        {
            tmp_child_test = new TestHierarchy(this);
            if (tmp_child_test->setDataFromRequirement((RequirementHierarchy*)in_child) == NOERR)
            {
                if (insertTestAtIndex(tmp_child_test, position))
                {
                    return (tmp_child_test->saveChildsRecords() == NOERR);
                }
            }
        }
        else
            return false;

        return false;
        break;

    case REQUIREMENTS_HIERARCHY_SIG_ID:
        if (in_child == NULL)
        {
            tmp_requirement_content = new RequirementContent(this);
            tmp_requirement_content->setValueForKey("", REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME);
            tmp_save_result = tmp_requirement_content->saveRecord();
            if (tmp_save_result == NOERR)
            {
                tmp_child_requirement = new RequirementHierarchy(this);
                tmp_child_requirement->setDataFromRequirementContent(tmp_requirement_content);
                return insertRequirementAtIndex(tmp_child_requirement, position);
            }
            delete tmp_requirement_content;
        }
        else if (in_child->getEntityDefSignatureId() == REQUIREMENTS_HIERARCHY_SIG_ID)
        {
            return insertRequirementAtIndex((RequirementHierarchy*)in_child, position);
        }

        return false;
        break;

    default:
        break;
    }
    return false;
}


bool ProjectVersion::removeChildren(int position, int count, bool in_move_indic, int in_child_type)
{
    switch (in_child_type)
    {
    case TESTS_HIERARCHY_SIG_ID:
        return removeTestsAtIndex(position, count, in_move_indic);
        break;

    case REQUIREMENTS_HIERARCHY_SIG_ID:
        return removeRequirementsAtIndex(position, count, in_move_indic);
        break;

    default:
        break;
    }

    return false;
}


bool ProjectVersion::isWritable(int in_child_type)
{
    switch (in_child_type)
    {
    case TESTS_HIERARCHY_SIG_ID:
        if (_m_project->projectGrants() != NULL)
            return (compare_values(_m_project->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC), PROJECT_GRANT_WRITE) == 0);
        break;

    case REQUIREMENTS_HIERARCHY_SIG_ID:
        if (_m_project->projectGrants() != NULL)
            return (compare_values(_m_project->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC), PROJECT_GRANT_WRITE) == 0);
        break;

    default:
        break;
    }

    return false;
}


QVariant ProjectVersion::data(int /*column*/, int /*role*/)
{
    return QVariant();
}


Hierarchy* ProjectVersion::parent()
{
    return NULL;
}


int ProjectVersion::row() const
{
    return 0;
}


Hierarchy* ProjectVersion::child(int number, int in_child_type)
{
    switch (in_child_type)
    {
    case TESTS_HIERARCHY_SIG_ID:
        return _m_tests_hierarchy[number];
        break;

    case REQUIREMENTS_HIERARCHY_SIG_ID:
        return _m_requirements_hierarchy[number];
        break;

    default:
        break;
    }

    return NULL;
}


int ProjectVersion::childCount(int in_child_type)
{
    switch (in_child_type)
    {
    case TESTS_HIERARCHY_SIG_ID :
        return _m_tests_hierarchy.count();
        break;

    case REQUIREMENTS_HIERARCHY_SIG_ID:
        return _m_requirements_hierarchy.count();
        break;

    default:
        break;
    }

    return 0;
}


bool ProjectVersion::insertTestAtIndex(TestHierarchy *in_test, int in_index)
{
    int     tmp_save_result = NOERR;

    if (in_test != NULL && _m_tests_hierarchy.indexOf(in_test) < 0)
    {
        if (in_index > 0)
            in_test->setValueForKey(_m_tests_hierarchy[in_index - 1]->getIdentifier(), TESTS_HIERARCHY_PREVIOUS_TEST_ID);
        else
            in_test->setValueForKey(NULL, TESTS_HIERARCHY_PREVIOUS_TEST_ID);

        in_test->setProjectVersion(this);
        tmp_save_result = in_test->saveRecord();
        if (tmp_save_result == NOERR)
        {
            _m_tests_hierarchy.insert(in_index, in_test);

            if (_m_tests_hierarchy.isEmpty() == false && in_index + 1 < _m_tests_hierarchy.count())
            {
                _m_tests_hierarchy[in_index + 1]->setValueForKey(in_test->getIdentifier(), TESTS_HIERARCHY_PREVIOUS_TEST_ID);
                tmp_save_result = _m_tests_hierarchy[in_index + 1]->saveRecord();
            }

            return (tmp_save_result == NOERR);
        }
    }

    return false;
}


bool ProjectVersion::removeTestsAtIndex(int in_index, int in_count, bool in_move_indic)
{
    TestHierarchy *tmp_test = NULL;
    int     tmp_save_result = NOERR;

    if (in_index + in_count < _m_tests_hierarchy.size())
    {
        if (in_index > 0)
            _m_tests_hierarchy[in_index + in_count]->setValueForKey(_m_tests_hierarchy[in_index - 1]->getIdentifier(), TESTS_HIERARCHY_PREVIOUS_TEST_ID);
        else
            _m_tests_hierarchy[in_index + in_count]->setValueForKey(NULL, TESTS_HIERARCHY_PREVIOUS_TEST_ID);

        tmp_save_result = _m_tests_hierarchy[in_index + in_count]->saveRecord();
    }

    if (tmp_save_result == NOERR)
    {
        for (int tmp_index = 0; tmp_index < in_count; tmp_index++)
        {
            tmp_test = _m_tests_hierarchy.takeAt(in_index);
            if (in_move_indic == false)
            {
                tmp_test->unLink();
                tmp_save_result = tmp_test->deleteRecord();
            }
        }
    }

    return (tmp_save_result == NOERR);
}


void ProjectVersion::removeCampaign(Campaign *in_campaign)
{
    for(int tmp_index = 0; tmp_index < _m_campaigns_list.count(); tmp_index++)
    {
        if (in_campaign == _m_campaigns_list[tmp_index])
        {
            _m_campaigns_list.removeAt(tmp_index);
            in_campaign->deleteRecord();
        }
    }

}



bool ProjectVersion::insertRequirementAtIndex(RequirementHierarchy *in_requirement, int in_index)
{
    int	    tmp_save_result = NOERR;

    if (in_requirement != NULL && _m_requirements_hierarchy.indexOf(in_requirement) < 0)
    {
        if (in_index > 0)
            in_requirement->setValueForKey(_m_requirements_hierarchy[in_index - 1]->getIdentifier(), REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);
        else
            in_requirement->setValueForKey(NULL, REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);

        in_requirement->setProjectVersion(this);
        tmp_save_result = in_requirement->saveRecord();
        if (tmp_save_result == NOERR)
        {
            _m_requirements_hierarchy.insert(in_index, in_requirement);

            if (_m_requirements_hierarchy.isEmpty() == false && in_index + 1 < _m_requirements_hierarchy.count())
            {
                _m_requirements_hierarchy[in_index + 1]->setValueForKey(in_requirement->getIdentifier(), REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);
                tmp_save_result = _m_requirements_hierarchy[in_index + 1]->saveRecord();
            }

            return (tmp_save_result == NOERR);
        }
    }

    return false;
}


bool ProjectVersion::removeRequirementsAtIndex(int in_index, int in_count, bool in_move_indic)
{
    RequirementHierarchy *tmp_requirement = NULL;

    int     tmp_save_result = NOERR;

    if (in_index + in_count < _m_requirements_hierarchy.size())
    {
        if (in_index > 0)
            _m_requirements_hierarchy[in_index + in_count]->setValueForKey(_m_requirements_hierarchy[in_index - 1]->getIdentifier(), REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);
        else
            _m_requirements_hierarchy[in_index + in_count]->setValueForKey(NULL, REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);

        tmp_save_result = _m_requirements_hierarchy[in_index + in_count]->saveRecord();
    }

    if (tmp_save_result == NOERR)
    {
        for (int tmp_index = 0; tmp_index < in_count; tmp_index++)
        {
            tmp_requirement = _m_requirements_hierarchy.takeAt(in_index);
            if (in_move_indic == false)
            {
                tmp_requirement->deleteRecord();
            }
        }
    }

    return (tmp_save_result == NOERR);
}


int ProjectVersion::saveRecord()
{
    int tmp_return = NOERR;

    if (is_empty_string(getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID)))
    {
        if (_m_project != NULL)
            setValueForKey(_m_project->getIdentifier(), PROJECTS_VERSIONS_TABLE_PROJECT_ID);

        tmp_return = Record::insertRecord();
    }
    else
        tmp_return = Record::saveRecord();

    return tmp_return;
}


int ProjectVersion::saveTests()
{
    int     tmp_result = saveHierarchicalList(_m_tests_hierarchy, TESTS_HIERARCHY_PREVIOUS_TEST_ID);

    return tmp_result;
}

int ProjectVersion::saveRequirements()
{
    int     tmp_result = NOERR;

    tmp_result = saveHierarchicalList(_m_requirements_hierarchy, REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);

    return tmp_result;
}



int ProjectVersion::saveCampaigns()
{
    int     tmp_result = NOERR;

    foreach(Campaign *tmp_campaign, _m_campaigns_list)
    {
        tmp_result = tmp_campaign->saveRecord();
        if (tmp_result != NOERR)
            break;
    }

    return tmp_result;
}


Hierarchy* ProjectVersion::copy()
{
    ProjectVersion *tmp_copy = new ProjectVersion(_m_project);

    return tmp_copy;
}

Hierarchy* ProjectVersion::cloneForMove(bool /*in_recursivly*/)
{
    return NULL;
}


QList < TestHierarchy* > ProjectVersion::orderedProjectTests(ProjectVersion *in_project_version, TestHierarchy **in_project_tests, unsigned long in_tests_count, const char *in_parent_id, net_callback_fct *in_callback)
{
    TestHierarchy         *tmp_test_hierarchy = NULL;
    TestHierarchy         *tmp_original_test_hierarchy = NULL;
    QList < TestHierarchy* >    tmp_tests_list;
    QList < TestHierarchy* >    tmp_tests_childs_list;

    const char                  *tmp_previous_id = NULL;

    unsigned long         tmp_tests_index = 0;
    unsigned long         tmp_original_tests_index = 0;

    int			  tmp_broken_tests_count = 0;

    bool					tmp_orignal_test_found = false;

    if (in_project_tests != NULL && in_tests_count > 0)
    {
        do
        {
            for (tmp_tests_index = 0; tmp_tests_index < in_tests_count; tmp_tests_index++)
            {
                tmp_test_hierarchy = in_project_tests[tmp_tests_index];
                if (tmp_test_hierarchy != NULL
                        && compare_values(in_parent_id, tmp_test_hierarchy->getValueForKey(TESTS_HIERARCHY_PARENT_TEST_ID)) == 0
                        && compare_values(tmp_previous_id, tmp_test_hierarchy->getValueForKey(TESTS_HIERARCHY_PREVIOUS_TEST_ID)) == 0)
                {
                    tmp_previous_id = tmp_test_hierarchy->getIdentifier();
                    tmp_tests_list.append(tmp_test_hierarchy);
                    if (in_callback != NULL)
                        in_callback(0, 0, 1, "Construction de la hierarchie des tests...");

                    break;
                }
            }
        }
        while (is_empty_string(tmp_previous_id) == FALSE && !(tmp_tests_index >= in_tests_count));

        for (tmp_tests_index = 0; tmp_tests_index < in_tests_count; tmp_tests_index++)
        {
            tmp_test_hierarchy = in_project_tests[tmp_tests_index];
            if (tmp_test_hierarchy != NULL && compare_values(in_parent_id, tmp_test_hierarchy->getValueForKey(TESTS_HIERARCHY_PARENT_TEST_ID)) == 0)
            {
                // Associer au projet
                tmp_test_hierarchy->setProjectVersion(in_project_version);

                // Test original
                if (is_empty_string(tmp_test_hierarchy->getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_ID)) == FALSE)
                {
                    tmp_orignal_test_found = false;
                    for (tmp_original_tests_index = 0; tmp_original_tests_index < in_tests_count; tmp_original_tests_index++)
                    {
                        tmp_original_test_hierarchy = in_project_tests[tmp_original_tests_index];
                        if (tmp_original_test_hierarchy != NULL
                                && compare_values(tmp_original_test_hierarchy->getIdentifier(), tmp_test_hierarchy->getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_ID)) == 0)
                        {
                            tmp_orignal_test_found = true;
                            // Appel recursif : Charger les tests enfants du test original
                            tmp_tests_childs_list = orderedProjectTests(in_project_version, in_project_tests, in_tests_count, tmp_original_test_hierarchy->getIdentifier(), in_callback);
                            if (tmp_tests_childs_list.count() > 0)
                                tmp_original_test_hierarchy->setChilds(tmp_tests_childs_list);

                            tmp_test_hierarchy->setAsLinkOf(tmp_original_test_hierarchy);
                        }
                    }

                    // Rechercher dans les tests du projet
                    if (tmp_orignal_test_found == false)
                    {
                        tmp_original_test_hierarchy = TestHierarchy::findTestWithId(in_project_version->_m_tests_hierarchy, tmp_test_hierarchy->getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_ID), true);
                        if (tmp_original_test_hierarchy != NULL)
                            tmp_test_hierarchy->setAsLinkOf(tmp_original_test_hierarchy);
                    }
                }

                // Appel recursif : Charger les tests enfants
                tmp_tests_childs_list = orderedProjectTests(in_project_version, in_project_tests, in_tests_count, tmp_test_hierarchy->getIdentifier(), in_callback);
                if (tmp_tests_childs_list.count() > 0)
                    tmp_test_hierarchy->setChilds(tmp_tests_childs_list);

                if (TestHierarchy::findTestWithId(tmp_tests_list, tmp_test_hierarchy->getIdentifier(), false) == NULL)
                {
                    tmp_test_hierarchy->setRecordStatus(RECORD_STATUS_BROKEN);

                    tmp_broken_tests_count++;
                    if (tmp_broken_tests_count == 1)
                    {
                        if (tmp_tests_list.count() > 0)
                        {
                            if (tmp_test_hierarchy->parent()->isWritable() == false || tmp_tests_list[tmp_tests_list.count() - 1]->lockRecord(true) != NOERR)
                            {
                                tmp_tests_list.append(tmp_test_hierarchy);
                                continue;
                            }
                        }
                    }

                    if (tmp_test_hierarchy->parent()->isWritable() && tmp_test_hierarchy->lockRecord(true) == NOERR){
                        tmp_test_hierarchy->saveRecord();
                        tmp_test_hierarchy->unlockRecord();
                    }

                    tmp_tests_list.append(tmp_test_hierarchy);
                }
            }
        }
    }

    return tmp_tests_list;
}


QList < RequirementHierarchy* > ProjectVersion::orderedProjectRequirements(ProjectVersion *in_project_version, RequirementHierarchy **in_project_requirements, unsigned long in_requirements_count, const char *in_parent_id, net_callback_fct *in_callback)
{
    RequirementHierarchy                *tmp_requirement_hierarchy = NULL;
    QList < RequirementHierarchy* >    tmp_requirements_list;
    QList < RequirementHierarchy* >    tmp_requirements_childs_list;

    const char                  		*tmp_previous_id = NULL;
    unsigned long         		tmp_requirements_index = 0;

    int				tmp_broken_requirements_count = 0;

    if (in_project_requirements != NULL && in_requirements_count > 0)
    {
        do
        {
            for (tmp_requirements_index = 0; tmp_requirements_index < in_requirements_count; tmp_requirements_index++)
            {
                tmp_requirement_hierarchy = in_project_requirements[tmp_requirements_index];
                if (tmp_requirement_hierarchy != NULL
                        && compare_values(in_parent_id, tmp_requirement_hierarchy->getValueForKey(REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID)) == 0
                        && compare_values(tmp_previous_id, tmp_requirement_hierarchy->getValueForKey(REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID)) == 0)
                {
                    tmp_previous_id = tmp_requirement_hierarchy->getIdentifier();
                    tmp_requirements_list.append(tmp_requirement_hierarchy);
                    if (in_callback != NULL)
                        in_callback(0, 0, 1, "Construction de la hierarchie des exigences...");
                    break;
                }
            }
        }
        while (is_empty_string(tmp_previous_id) == FALSE && !(tmp_requirements_index >= in_requirements_count));

        for (tmp_requirements_index = 0; tmp_requirements_index < in_requirements_count; tmp_requirements_index++)
        {
            tmp_requirement_hierarchy = in_project_requirements[tmp_requirements_index];
            if (tmp_requirement_hierarchy != NULL && compare_values(in_parent_id, tmp_requirement_hierarchy->getValueForKey(REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID)) == 0)
            {
                // Associer au projet
                tmp_requirement_hierarchy->setProjectVersion(in_project_version);

                // Appel recursif : Charger les exigences filles
                tmp_requirements_childs_list = orderedProjectRequirements(in_project_version, in_project_requirements, in_requirements_count, tmp_requirement_hierarchy->getIdentifier(), in_callback);
                if (tmp_requirements_childs_list.count() > 0)
                    tmp_requirement_hierarchy->setChilds(tmp_requirements_childs_list);

                if (RequirementHierarchy::findRequirementWithId(tmp_requirements_list, tmp_requirement_hierarchy->getIdentifier()) == NULL)
                {
                    tmp_requirement_hierarchy->setRecordStatus(RECORD_STATUS_BROKEN);

                    tmp_broken_requirements_count++;
                    if (tmp_broken_requirements_count == 1)
                    {
                        if (tmp_requirements_list.count() > 0)
                        {
                            if (tmp_requirement_hierarchy->parent()->isWritable() == false || tmp_requirements_list[tmp_requirements_list.count() - 1]->lockRecord(true) != NOERR)
                            {
                                tmp_requirements_list.append(tmp_requirement_hierarchy);
                                continue;
                            }
                        }
                    }

                    if (tmp_requirement_hierarchy->parent()->isWritable() && tmp_requirement_hierarchy->lockRecord(true) == NOERR){
                        tmp_requirement_hierarchy->saveRecord();
                        tmp_requirement_hierarchy->unlockRecord();
                    }

                    tmp_requirements_list.append(tmp_requirement_hierarchy);
                }

            }
        }
    }

    return tmp_requirements_list;
}

void ProjectVersion::loadRequirements(net_callback_fct *in_callback)
{
    setRequirementsHierarchy(loadProjectRequirements(in_callback));
}

void ProjectVersion::loadTests(net_callback_fct *in_callback)
{
    setTestsHierarchy(loadProjectTests(in_callback));
}

void ProjectVersion::loadCampaigns()
{
    _m_campaigns_list = loadProjectCampaigns();
}


QList < RequirementHierarchy* > ProjectVersion::loadProjectRequirements(net_callback_fct *in_callback)
{
    RequirementHierarchy		**tmp_project_requirements = NULL;
    QList < RequirementHierarchy* >	tmp_requirements_list;
    unsigned long			tmp_requirements_count = 0;
    net_session				*tmp_session = CLIENT_SESSION;

    sprintf(tmp_session->m_last_query, "%s=%s and %s='%s'", REQUIREMENTS_HIERARCHY_PROJECT_ID, _m_project->getIdentifier(), REQUIREMENTS_HIERARCHY_VERSION, getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION));

    tmp_project_requirements = Record::loadRecords<RequirementHierarchy>(tmp_session, &requirements_hierarchy_def, tmp_session->m_last_query, NULL, &tmp_requirements_count);
    if (tmp_project_requirements != NULL)
    {
        if (tmp_requirements_count > 0)
        {
            if(in_callback != NULL)
                in_callback(0, tmp_requirements_count, 0, "Loading requirements...");
            tmp_requirements_list = orderedProjectRequirements(this, tmp_project_requirements, tmp_requirements_count, NULL, in_callback);
            if(in_callback != NULL)
                in_callback(tmp_requirements_count, tmp_requirements_count, 0, "Loading requirements...");
        }

        Record::freeRecords<RequirementHierarchy>(tmp_project_requirements, 0);
    }

    return tmp_requirements_list;
}



QList < TestHierarchy* > ProjectVersion::loadProjectTests(net_callback_fct *in_callback)
{
    TestHierarchy		**tmp_project_tests = NULL;
    QList < TestHierarchy* >    tmp_tests_list;

    unsigned long		tmp_tests_count = 0;
    net_session			*tmp_session = CLIENT_SESSION;

    sprintf(tmp_session->m_last_query, "%s=%s and %s='%s'", TESTS_HIERARCHY_PROJECT_ID, _m_project->getIdentifier(), TESTS_HIERARCHY_VERSION, getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION));

    tmp_project_tests = Record::loadRecords<TestHierarchy>(tmp_session, &tests_hierarchy_def, tmp_session->m_last_query, NULL, &tmp_tests_count, in_callback);
    if (tmp_project_tests != NULL)
    {
        if (tmp_tests_count > 0)
        {
            if(in_callback != NULL)
                in_callback(0, tmp_tests_count, 0, "Loading tests...");
            tmp_tests_list = ProjectVersion::orderedProjectTests(this, tmp_project_tests, tmp_tests_count, NULL, in_callback);
            if(in_callback != NULL)
                in_callback(tmp_tests_count, tmp_tests_count, 0, "Loading tests...");
        }

        Record::freeRecords<TestHierarchy>(tmp_project_tests, 0);
    }

    return tmp_tests_list;
}


ProjectVersion* ProjectVersion::loadVersionForProject(const char *in_version, Project *in_project)
{
    ProjectVersion **tmp_project_versions = NULL;
    unsigned long tmp_project_versions_count = 0;

    ProjectVersion *tmp_version = NULL;

    sprintf(CLIENT_SESSION->m_last_query, "%s=%s and %s='%s'", PROJECTS_VERSIONS_TABLE_PROJECT_ID, in_project->getIdentifier(), PROJECTS_VERSIONS_TABLE_VERSION, in_version);
    tmp_project_versions = Record::loadRecords<ProjectVersion>(CLIENT_SESSION, &projects_versions_table_def,
                                                               CLIENT_SESSION->m_last_query, PROJECTS_VERSIONS_TABLE_VERSION, &tmp_project_versions_count);
    if (tmp_project_versions != NULL)
    {
        if (tmp_project_versions_count == 1)
        {
            tmp_version = tmp_project_versions[0];
            tmp_version->setProject(in_project->duplicate());

            Record::freeRecords<ProjectVersion>(tmp_project_versions, 0);
        }
        else
            Record::freeRecords<ProjectVersion>(tmp_project_versions, tmp_project_versions_count);
    }

    return tmp_version;
}


QList < Campaign* > ProjectVersion::loadProjectCampaigns()
{
    Campaign				**tmp_campaigns = NULL;
    QList < Campaign* >			tmp_campaigns_list;

    unsigned long			tmp_campaigns_count = 0;
    net_session				*tmp_session = CLIENT_SESSION;

    sprintf(tmp_session->m_last_query, "%s=%s and %s='%s'", CAMPAIGNS_TABLE_PROJECT_ID, _m_project->getIdentifier(), CAMPAIGNS_TABLE_PROJECT_VERSION, getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION));

    tmp_campaigns = Record::loadRecords<Campaign>(tmp_session, &campaigns_table_def, tmp_session->m_last_query, NULL, &tmp_campaigns_count);
    if (tmp_campaigns != NULL)
    {
        for (unsigned long tmp_index = 0; tmp_index < tmp_campaigns_count; tmp_index++)
        {
            if (tmp_campaigns[tmp_index] != NULL)
            {
                tmp_campaigns[tmp_index]->setProjectVersion(this);
                tmp_campaigns_list.append(tmp_campaigns[tmp_index]);
            }
        }

        Record::freeRecords<Campaign>(tmp_campaigns, 0);
    }

    return tmp_campaigns_list;
}



void ProjectVersion::loadProjectVersionDatas(net_callback_fct *in_callback)
{
    if (_m_project != NULL
            && _m_project->projectGrants() != NULL
            && compare_values(_m_project->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC), PROJECT_GRANT_NONE) != 0)
        loadRequirements(in_callback);

    if (_m_project != NULL
            && _m_project->projectGrants() != NULL
            && compare_values(_m_project->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC), PROJECT_GRANT_NONE) != 0)
        loadTests(in_callback);

    if (_m_project != NULL
            && _m_project->projectGrants() != NULL
            && compare_values(_m_project->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_CAMPAIGNS_INDIC), PROJECT_GRANT_NONE) != 0)
        loadCampaigns();
}


int ProjectVersion::dbChildCount(int in_child_type)
{
    char			tmp_statement_str[256];
    char			***tmp_results = NULL;
    unsigned long	tmp_rows_count = 0;
    unsigned long	tmp_columns_count = 0;
    int				tmp_records_count = -1;

    if (is_empty_string(getIdentifier()) == false)
    {
        switch (in_child_type)
        {
        case TESTS_HIERARCHY_SIG_ID :
            sprintf(tmp_statement_str, "SELECT COUNT(%s) FROM %s WHERE %s=%s AND %s='%s';"
                    , TESTS_TABLE_TEST_ID
                    , TESTS_TABLE_SIG
                    , TESTS_TABLE_PROJECT_ID
                    , getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID)
                    , TESTS_TABLE_VERSION
                    , getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION));
            break;

        case REQUIREMENTS_HIERARCHY_SIG_ID:
            sprintf(tmp_statement_str, "SELECT COUNT(%s) FROM %s WHERE %s=%s AND %s='%s';"
                    , REQUIREMENTS_TABLE_REQUIREMENT_ID
                    , REQUIREMENTS_TABLE_SIG
                    , REQUIREMENTS_TABLE_PROJECT_ID
                    , getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID)
                    , REQUIREMENTS_TABLE_VERSION
                    , getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION));
            break;

        default:
            break;
        }

        tmp_results = cl_run_sql(CLIENT_SESSION, tmp_statement_str, &tmp_rows_count, &tmp_columns_count);
        if (tmp_results != NULL)
        {
            tmp_records_count = atoi(tmp_results[0][0]);
            cl_free_rows_columns_array(&tmp_results, tmp_rows_count, tmp_columns_count);
        }
    }

    return tmp_records_count;
}


Hierarchy* ProjectVersion::findItemWithId(const char* in_item_id, int in_child_type, bool in_recursive)
{
    switch (in_child_type)
    {
    case TESTS_HIERARCHY_SIG_ID :
        return TestHierarchy::findTestWithId(_m_tests_hierarchy, in_item_id, in_recursive);
        break;

    case REQUIREMENTS_HIERARCHY_SIG_ID:
        return RequirementHierarchy::findRequirementWithId(_m_requirements_hierarchy, in_item_id, in_recursive);
        break;

    default:
        break;
    }

    return NULL;
}


void ProjectVersion::writeXml(QXmlStreamWriter & in_xml_writer)
{
    QList < RequirementHierarchy* >	tmp_requirements_list;
    QList < TestHierarchy* >		tmp_tests_list;
    QList < Campaign* >			tmp_campaigns_list;

    in_xml_writer.writeStartElement("version");
    in_xml_writer.writeAttribute("number", getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION));

    in_xml_writer.writeTextElement("description", getValueForKey(PROJECTS_VERSIONS_TABLE_DESCRIPTION));

    tmp_requirements_list = loadProjectRequirements(NULL);
    if (tmp_requirements_list.count() > 0)
    {
        in_xml_writer.writeStartElement("requirements");
        foreach(RequirementHierarchy *tmp_requirement, tmp_requirements_list)
        {
            tmp_requirement->writeXml(in_xml_writer);
        }
        in_xml_writer.writeEndElement();
        qDeleteAll(tmp_requirements_list);
    }

    tmp_tests_list = loadProjectTests(NULL);
    if (tmp_tests_list.count() > 0)
    {
        in_xml_writer.writeStartElement("tests");
        foreach(TestHierarchy *tmp_test, tmp_tests_list)
        {
            tmp_test->writeXml(in_xml_writer);
        }
        in_xml_writer.writeEndElement();
        qDeleteAll(tmp_tests_list);
    }

    tmp_campaigns_list = loadProjectCampaigns();
    if (tmp_campaigns_list.count() > 0)
    {
        in_xml_writer.writeStartElement("campaigns");
        foreach(Campaign *tmp_campaign, tmp_campaigns_list)
        {
            tmp_campaign->writeXml(in_xml_writer);
        }
        in_xml_writer.writeEndElement();
        qDeleteAll(tmp_campaigns_list);
    }

    in_xml_writer.writeEndElement();
}


QString ProjectVersion::formatProjectVersionNumber(const char *in_long_version_number)
{
    QString	tmp_version_string;
    QStringList tmp_version_numbers;

    if (is_empty_string(in_long_version_number) == FALSE)
    {
        tmp_version_numbers = QString(in_long_version_number).split(".");
        if (tmp_version_numbers.count() > 0)
            tmp_version_string += QString::number(tmp_version_numbers[0].toInt());
        if (tmp_version_numbers.count() > 1)
            tmp_version_string += "." + QString::number(tmp_version_numbers[1].toInt());
        if (tmp_version_numbers.count() > 2)
            tmp_version_string += "." + QString::number(tmp_version_numbers[2].toInt());
        if (tmp_version_numbers.count() > 3)
            tmp_version_string += "." + QString::number(tmp_version_numbers[3].toInt());
    }

    return tmp_version_string;
}


bool ProjectVersion::readXml(QXmlStreamReader & in_xml_reader)
{
    RequirementHierarchy	*tmp_requirement = NULL;
    TestHierarchy		*tmp_test = NULL;
    Campaign			*tmp_campaign = NULL;

    QString		tmp_text;
    QString		tmp_version = in_xml_reader.attributes().value("number").toString();

    const char			*tmp_previous_test_id = NULL;
    const char			*tmp_previous_requirement_id = NULL;

    setValueForKey(tmp_version.toStdString().c_str(), PROJECTS_VERSIONS_TABLE_VERSION);

    while (in_xml_reader.readNextStartElement())
    {
        // Description
        if (in_xml_reader.name() == "description")
        {
            tmp_text = in_xml_reader.readElementText();
            setValueForKey(tmp_text.toStdString().c_str(), PROJECTS_VERSIONS_TABLE_DESCRIPTION);
        }
        else if (in_xml_reader.name() == "tests")
        {
            while (in_xml_reader.readNextStartElement())
            {
                if (in_xml_reader.name() == "test")
                {
                    tmp_test = new TestHierarchy(this);
                    tmp_test->setValueForKey(tmp_previous_test_id, TESTS_HIERARCHY_PREVIOUS_TEST_ID);
                    tmp_test->readXml(in_xml_reader);
                    tmp_previous_test_id = tmp_test->getIdentifier();
                    _m_tests_hierarchy.append(tmp_test);
                }
                else
                {
                    LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                    in_xml_reader.skipCurrentElement();
                }
            }
        }
        else if (in_xml_reader.name() == "requirements")
        {
            while (in_xml_reader.readNextStartElement())
            {
                if (in_xml_reader.name() == "requirement")
                {
                    tmp_requirement = new RequirementHierarchy(this);
                    tmp_requirement->setValueForKey(tmp_previous_requirement_id, REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID);
                    tmp_requirement->readXml(in_xml_reader);
                    tmp_previous_requirement_id = tmp_requirement->getIdentifier();
                    _m_requirements_hierarchy.append(tmp_requirement);
                }
                else
                {
                    LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                    in_xml_reader.skipCurrentElement();
                }
            }
        }
        else if (in_xml_reader.name() == "campaigns")
        {
            while (in_xml_reader.readNextStartElement())
            {
                if (in_xml_reader.name() == "campaign")
                {
                    tmp_campaign = new Campaign(this);
                    tmp_campaign->readXml(in_xml_reader);
                    _m_campaigns_list.append(tmp_campaign);
                }
                else
                {
                    LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                    in_xml_reader.skipCurrentElement();
                }
            }
        }
        else
        {
            LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
            in_xml_reader.skipCurrentElement();
        }
    }

    return true;
}


int ProjectVersion::saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas)
{
    int		tmp_return = NOERR;

    tmp_return = saveRecord();
    if (tmp_return == NOERR)
    {
        foreach(RequirementHierarchy *tmp_requirement, _m_requirements_hierarchy)
        {
            tmp_requirement->saveFromXmlProjectDatas(in_xml_datas);
        }

        foreach(TestHierarchy *tmp_test, _m_tests_hierarchy)
        {
            tmp_test->saveFromXmlProjectDatas(in_xml_datas);
        }

        foreach(Campaign *tmp_campaign, _m_campaigns_list)
        {
            tmp_campaign->saveFromXmlProjectDatas(in_xml_datas);
        }
    }

    return tmp_return;
}


QList<Bug*> ProjectVersion::loadBugs()
{
    Bug					**tmp_bugs = NULL;
    unsigned long			tmp_bugs_count = 0;
    net_session				*tmp_session = CLIENT_SESSION;
    unsigned long			tmp_index = 0;
    QList<Bug*>				tmp_bugs_list;

    sprintf(tmp_session->m_last_query, "%s IN (SELECT %s FROM %s,%s WHERE %s.%s=%s.%s AND %s.%s=%s AND %s.%s='%s')",
            BUGS_TABLE_EXECUTION_TEST_ID,
            EXECUTIONS_TESTS_TABLE_EXECUTION_TEST_ID,
            EXECUTIONS_TESTS_TABLE_SIG,
            TESTS_TABLE_SIG,
            TESTS_TABLE_SIG,
            TESTS_TABLE_TEST_ID,
            EXECUTIONS_TESTS_TABLE_SIG,
            EXECUTIONS_TESTS_TABLE_TEST_ID,
            TESTS_TABLE_SIG,
            TESTS_TABLE_PROJECT_ID,
            getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID),
            TESTS_TABLE_SIG,
            TESTS_TABLE_VERSION,
            getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION));

    tmp_bugs = Record::loadRecords<Bug>(tmp_session, &bugs_table_def, tmp_session->m_last_query, BUGS_TABLE_CREATION_DATE, &tmp_bugs_count);
    if (tmp_bugs != NULL)
    {
        if (tmp_bugs_count > 0)
        {
            for(tmp_index = 0; tmp_index < tmp_bugs_count; tmp_index++)
            {
                tmp_bugs_list.append(tmp_bugs[tmp_index]);
            }
        }

        Record::freeRecords<Bug>(tmp_bugs, 0);
    }

    return tmp_bugs_list;
}

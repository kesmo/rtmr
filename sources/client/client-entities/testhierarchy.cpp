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

#include "testhierarchy.h"
#include "testcontent.h"
#include "action.h"
#include "requirement.h"
#include "requirementhierarchy.h"
#include "testrequirement.h"
#include "requirementcontent.h"
#include "utilities.h"
#include "projectgrant.h"
#include "session.h"
#include "customtestfield.h"

#include <QFont>
#include <QIcon>
#include <QPixmap>

/**
  Constructeur
**/
TestHierarchy::TestHierarchy() : Hierarchy(getEntityDef())
{
    _m_project_version = NULL;
    _m_parent = NULL;
    _m_original = NULL;
    _m_is_a_copy = false;
}


/**
  Constructeur
**/
TestHierarchy::TestHierarchy(ProjectVersion *in_project) : Hierarchy(getEntityDef())
{
    _m_project_version = in_project;
    _m_parent = NULL;
    _m_original = NULL;
    _m_is_a_copy = false;
    setProjectVersion(_m_project_version);
}


/**
  Constructeur
**/
TestHierarchy::TestHierarchy(TestHierarchy* in_parent) : Hierarchy(getEntityDef())
{
    _m_project_version = NULL;
    _m_parent = in_parent;
    _m_original = NULL;
    _m_is_a_copy = false;
    if (_m_parent != NULL)
    {
        setProjectVersion(_m_parent->projectVersion());
    }
}


/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* TestHierarchy::getEntityDef() const
{
    return &tests_hierarchy_def;
}


/**
  Destructeur
**/
TestHierarchy::~TestHierarchy()
{
    qDeleteAll(_m_childs);
    destroy();
}


void TestHierarchy::setProjectVersion(ProjectVersion* in_project_version)
{
    if (in_project_version != NULL)
    {
        _m_project_version = in_project_version;
        if (is_empty_string(getIdentifier()))
        {
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), TESTS_HIERARCHY_PROJECT_ID);
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), TESTS_HIERARCHY_VERSION);
        }
    }
}


TestHierarchy* TestHierarchy::findTestWithId(const QList<TestHierarchy*>& in_tests_list, const char* in_test_id, bool in_recursive)
{
    TestHierarchy *tmp_found_test = NULL;

    foreach(TestHierarchy *tmp_test, in_tests_list)
    {
        if (!tmp_test->_m_is_a_copy)
        {
            if (compare_values(tmp_test->getIdentifier(), in_test_id) == 0)
                return tmp_test;

            if (in_recursive)
            {
                tmp_found_test = findTestWithId(tmp_test->_m_childs, in_test_id);
                if (tmp_found_test != NULL)
                    return tmp_found_test;
            }
        }
    }

    return NULL;
}


/**
  Renvoie le test enfant
**/
Hierarchy* TestHierarchy::child(int number, int /*in_child_type*/)
{
    return _m_childs.value(number);
}



/**
  Renvoie le nombre de tests enfants
**/
int TestHierarchy::childCount(int /*in_child_type*/)
{
    return _m_childs.count();
}


int TestHierarchy::dbChildCount(int /*in_child_type*/)
{
    char			tmp_statement_str[256];
    char			***tmp_results = NULL;
    unsigned long	tmp_rows_count = 0;
    unsigned long	tmp_columns_count = 0;
    int				tmp_records_count = -1;

    if (is_empty_string(getIdentifier()) == false)
    {
        /* Verifier qu'il n'existe pas un nouvel element a la suite */
        sprintf(tmp_statement_str, "SELECT COUNT(%s) FROM %s WHERE %s=%s;"
        , TESTS_TABLE_TEST_ID
        , TESTS_TABLE_SIG
        , TESTS_TABLE_PARENT_TEST_ID
        , getIdentifier());
        tmp_results = cl_run_sql(CLIENT_SESSION, tmp_statement_str, &tmp_rows_count, &tmp_columns_count);
        if (tmp_results != NULL)
        {
            tmp_records_count = atoi(tmp_results[0][0]);
            cl_free_rows_columns_array(&tmp_results, tmp_rows_count, tmp_columns_count);
        }
    }

    return tmp_records_count;
}

/**
  Renvoie la valeur de la colonne <i>column</i>
**/
QVariant TestHierarchy::data(int /*column*/, int in_role)
{
    QFont   tmp_font;

    switch (in_role)
    {
        /* Icone */
        case Qt::DecorationRole:
            switch (_m_record_status)
            {
                case RECORD_STATUS_OUT_OF_SYNC:
                    return QIcon(QPixmap(QString::fromUtf8(":/images/22x22/clock.png")));
                    break;

                case RECORD_STATUS_LOCKED:
                    return QIcon(QPixmap(QString::fromUtf8(":/images/22x22/lock.png")));
                    break;

                case RECORD_STATUS_BROKEN:
                    return QIcon(QPixmap(QString::fromUtf8(":/images/22x22/pellicule_broken.png")));
                    break;

                default:
                    if (isAutomatedTest())
                    {
                        return QIcon(QPixmap(QString::fromUtf8(":/images/22x22/pellicule_auto.png")));
                    }
                    else
                    {
                        if (childCount() > 0)
                            return QIcon(QPixmap(QString::fromUtf8(":/images/22x22/folder_green.png")));
                        else
                            return QIcon(QPixmap(QString::fromUtf8(":/images/22x22/pellicule.png")));
                    }
            }

            break;

            // Renvoie le style de police de caracteres du item courant
        case Qt::FontRole:
            if (_m_original != NULL)
                tmp_font.setItalic(true);

            return QVariant(tmp_font);
            break;

            // Renvoie le titre du item courant
        case Qt::ToolTipRole:
        case Qt::DisplayRole:
        case Qt::EditRole:
            if (_m_original != NULL)
                return QVariant(QString(_m_original->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)));
            else
                return QVariant(QString(getValueForKey(TESTS_HIERARCHY_SHORT_NAME)));
            break;

        default:
            return QVariant();

    }


    return QVariant();
}


int TestHierarchy::setDataFromRequirement(RequirementHierarchy *in_requirement)
{
    TestHierarchy			*tmp_child_test = NULL;
    RequirementHierarchy		*tmp_child_requirement = NULL;
    RequirementContent	*tmp_requirement_content = NULL;
    TestRequirement	*tmp_test_requirement = NULL;
    TestContent          *tmp_test_content = NULL;

    int                     tmp_result = NOERR;

    if (in_requirement != NULL)
    {
        tmp_requirement_content = new RequirementContent(_m_project_version);
        tmp_result = tmp_requirement_content->loadRecord(in_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID));
        if (tmp_result == NOERR)
        {
            tmp_test_content = new TestContent(_m_project_version);
            tmp_test_content->setValueForKey(tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME), TESTS_CONTENTS_TABLE_SHORT_NAME);
            tmp_test_content->setValueForKey(tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION), TESTS_CONTENTS_TABLE_DESCRIPTION);
            tmp_test_content->setValueForKey(tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL), TESTS_CONTENTS_TABLE_PRIORITY_LEVEL);
            tmp_test_content->setValueForKey(tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID), TESTS_CONTENTS_TABLE_CATEGORY_ID);
            tmp_result = tmp_test_content->saveRecord();
            if (tmp_result == NOERR)
            {
                setValueForKey(tmp_test_content->getIdentifier(), TESTS_HIERARCHY_TEST_CONTENT_ID);
                setValueForKey(tmp_test_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME), TESTS_HIERARCHY_SHORT_NAME);
                setValueForKey(tmp_test_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL), TESTS_HIERARCHY_PRIORITY_LEVEL);
                setValueForKey(tmp_test_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID), TESTS_HIERARCHY_CATEGORY_ID);
                setValueForKey(tmp_test_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_VERSION), TESTS_HIERARCHY_CONTENT_VERSION);

                tmp_test_requirement = new TestRequirement();
                tmp_test_requirement->setValueForKey(tmp_test_content->getIdentifier(), TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID);
                tmp_test_requirement->setValueForKey(tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID), TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);
                tmp_result = tmp_test_requirement->insertRecord();
                for(int tmp_index = 0; tmp_index < in_requirement->childCount() && tmp_result == NOERR; tmp_index++)
                {
                    tmp_child_requirement = (RequirementHierarchy*)in_requirement->child(tmp_index);
                    if (tmp_child_requirement != NULL)
                    {
                        tmp_child_test = new TestHierarchy(this);
                        tmp_result = tmp_child_test->setDataFromRequirement(tmp_child_requirement);
                        if (tmp_result == NOERR)
                        {
                            _m_childs.append(tmp_child_test);
                        }
                    }
                }
                delete tmp_test_requirement;
            }
            delete tmp_test_content;
        }
        delete tmp_requirement_content;
    }

    return tmp_result;
}


bool TestHierarchy::insertCopyOfChildren(int in_index, int count, GenericRecord *in_item, int in_child_type)
{
    TestHierarchy	*tmp_test = NULL;
    net_session		*tmp_session = CLIENT_SESSION;
    char		***tmp_results = NULL;
    unsigned long	tmp_rows_count, tmp_columns_count;

    bool                tmp_return = false;

    if (in_item != NULL)
    {
        if (in_item->getEntityDefSignatureId() == TESTS_HIERARCHY_SIG_ID)
        {
            sprintf(tmp_session->m_last_query, "select create_test_from_test(%s, '%s', %s, %s, %s, NULL);",
            _m_project_version->project()->getIdentifier(),
            _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION),
            getIdentifier(),
            (in_index > 0 ? _m_childs[in_index -1]->getIdentifier() : "NULL"),
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
                            if (insertChildren(in_index, count, tmp_test, in_child_type))
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
            _m_project_version->project()->getIdentifier(),
            _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION),
            getIdentifier(),
            (in_index > 0 ? _m_childs[in_index -1]->getIdentifier() : "NULL"),
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
                            if (insertChildren(in_index, count, tmp_test, in_child_type))
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

    return tmp_return;
}

/**
  Insertion d'un nouveau test enfant
**/
bool TestHierarchy::insertChildren(int in_index, int /* count */, Hierarchy *in_child, int /*in_child_type*/)
{
    TestHierarchy     *tmp_test = NULL;
    TestHierarchy     *tmp_child_test = NULL;
    TestHierarchy     *tmp_next_test = NULL;
    int         tmp_save_result = NOERR;
    TestContent*		tmp_test_content = NULL;

    if (in_child == NULL)
    {
        tmp_test_content = new TestContent(_m_project_version);
        tmp_test_content->setValueForKey("", TESTS_CONTENTS_TABLE_SHORT_NAME);
        tmp_save_result = tmp_test_content->saveRecord();
        if (tmp_save_result == NOERR)
        {
            tmp_test = new TestHierarchy(_m_project_version);
            tmp_test->setDataFromTestContent(tmp_test_content);
        }
        delete tmp_test_content;
    }
    else
        tmp_test = (TestHierarchy*)in_child;

    if (tmp_save_result == NOERR)
    {
        tmp_test->_m_parent = this;
        tmp_test->setProjectVersion(_m_project_version);
        if (in_index > 0)
            tmp_test->setValueForKey(_m_childs[in_index - 1]->getIdentifier(), TESTS_HIERARCHY_PREVIOUS_TEST_ID);
        else
            tmp_test->setValueForKey(NULL, TESTS_HIERARCHY_PREVIOUS_TEST_ID);

        tmp_save_result = tmp_test->saveRecord();

        if (tmp_save_result == NOERR)
        {
            _m_childs.insert(in_index, tmp_test);

            // Traitement du test suivant
            if (_m_childs.isEmpty() == false && in_index + 1 < _m_childs.size())
            {
                tmp_next_test = _m_childs[in_index + 1];
                if (tmp_next_test != NULL)
                {
                    tmp_next_test->setValueForKey(tmp_test->getIdentifier(), TESTS_HIERARCHY_PREVIOUS_TEST_ID);
                    tmp_save_result = tmp_next_test->saveRecord();
                }
            }

            // Mettre a jour les copies du test courant
            foreach(TestHierarchy* tmp_test_copy, _m_links)
            {
                tmp_child_test = tmp_test->duplicate(true);
                tmp_child_test->_m_parent = tmp_test_copy;
                tmp_test_copy->_m_childs.append(tmp_child_test);
            }

            return (tmp_save_result == NOERR);
        }
    }

    return false;
}


bool TestHierarchy::setChilds(QList < TestHierarchy * > in_tests_list)
{
    _m_childs = in_tests_list;

    foreach(TestHierarchy *tmp_test, _m_childs)
    {
        tmp_test->_m_parent = this;
    }

    return true;
}


/**
  Creer un nouveau test à partir d'une copie du test courant
**/
Hierarchy* TestHierarchy::copy()
{
    TestHierarchy*		    tmp_test = NULL;
    TestContent*	    tmp_test_content = NULL;
    int			    tmp_save_result = NOERR;

    tmp_test_content = new TestContent(_m_project_version);
    tmp_test_content->setValueForKey(getValueForKey(TESTS_HIERARCHY_SHORT_NAME), TESTS_CONTENTS_TABLE_SHORT_NAME);
    tmp_test_content->setValueForKey(getValueForKey(TESTS_HIERARCHY_PRIORITY_LEVEL), TESTS_CONTENTS_TABLE_PRIORITY_LEVEL);
    tmp_test_content->setValueForKey(getValueForKey(TESTS_HIERARCHY_CATEGORY_ID), TESTS_CONTENTS_TABLE_CATEGORY_ID);
    tmp_save_result = tmp_test_content->saveRecord();
    if (tmp_save_result == NOERR)
    {
        tmp_test = new TestHierarchy(_m_project_version);
        tmp_test->setDataFromTestContent(tmp_test_content);
    }
    delete tmp_test_content;

    return tmp_test;
}



bool TestHierarchy::canMove(int /* in_child_type */)
{
    if (_m_parent != NULL && _m_parent->_m_original != NULL)
        return false;

    return true;
}



bool TestHierarchy::mayHaveCyclicRedundancy(Hierarchy *in_dest_item, bool in_check_link)
{
    TestHierarchy *tmp_parent = NULL;

    if (in_check_link && _m_original != NULL)	return true;

    if (in_dest_item != NULL && in_dest_item->getEntityDefSignatureId() == TESTS_HIERARCHY_SIG_ID)
    {
        tmp_parent = (TestHierarchy*)in_dest_item;
        while (tmp_parent != NULL)
        {
            if (_m_original != NULL && compare_values(tmp_parent->getValueForKey(TESTS_HIERARCHY_TEST_ID), _m_original->getValueForKey(TESTS_HIERARCHY_TEST_ID)) == 0)
                return true;

            if (compare_values(tmp_parent->getValueForKey(TESTS_HIERARCHY_TEST_ID), getValueForKey(TESTS_HIERARCHY_TEST_ID)) == 0)
                return true;

            if (findItemWithValueForKey(tmp_parent->getValueForKey(TESTS_HIERARCHY_TEST_ID), TESTS_HIERARCHY_ORIGINAL_TEST_ID) != NULL)
                return true;

            tmp_parent = tmp_parent->_m_parent;
        }
    }

    return false;
}



/**
  Creer un nouveau test lie au test courant
**/
Hierarchy* TestHierarchy::link()
{
    TestHierarchy* tmp_link = NULL;
    TestHierarchy* tmp_child_test = NULL;

    tmp_link = (TestHierarchy*)copy();

    tmp_link->_m_original = this;
    _m_links.append(tmp_link);

    foreach(TestHierarchy *tmp_test, _m_childs)
    {
        tmp_child_test = tmp_test->duplicate(true);
        tmp_child_test->_m_parent = tmp_link;
        tmp_link->_m_childs.append(tmp_child_test);
    }

    return tmp_link;
}


/**
  Renvoie le test parent
**/
Hierarchy *TestHierarchy::parent()
{
    if (_m_parent != NULL)
        return _m_parent;
    else
        return _m_project_version;
}

int TestHierarchy::row() const
{
    if (_m_parent)
        return _m_parent->_m_childs.indexOf(const_cast<TestHierarchy*>(this));
    else
        return _m_project_version->testsHierarchy().indexOf(const_cast<TestHierarchy*>(this));

    return 0;
}

/**
  Supprimer un test enfant
**/
bool TestHierarchy::removeChildren(int in_index, int count, bool in_move_indic, int /* in_child_type */)
{
    TestHierarchy *tmp_test = NULL;
    int     tmp_save_result = NOERR;

    if (in_index + count < _m_childs.size())
    {
        if (in_index > 0)
            _m_childs[in_index + count]->setValueForKey(_m_childs[in_index - 1]->getIdentifier(), TESTS_HIERARCHY_PREVIOUS_TEST_ID);
        else
            _m_childs[in_index + count]->setValueForKey(NULL, TESTS_HIERARCHY_PREVIOUS_TEST_ID);

        tmp_save_result = _m_childs[in_index + count]->saveRecord();
    }

    if (tmp_save_result == NOERR)
    {
        for (int tmp_index = 0; tmp_index < count; tmp_index++)
        {
            tmp_test = _m_childs.takeAt(in_index);
            if (in_move_indic == false)
            {
                tmp_test->unLink();
                tmp_test->unsetLink();
                tmp_save_result = tmp_test->deleteRecord();
            }

            // Mettre a jour les copies du test courant
            foreach(TestHierarchy* tmp_test_copy, _m_links)
            {
                foreach(TestHierarchy *tmp_child_test, tmp_test_copy->_m_childs)
                {
                    if (compare_values(tmp_child_test->getIdentifier(), tmp_test->getIdentifier()) == 0)
                        tmp_test_copy->_m_childs.removeOne(tmp_child_test);
                }
            }
        }
    }

    return (tmp_save_result == NOERR);
}


bool TestHierarchy::isWritable(int /* in_child_type */)
{
    if (_m_is_a_copy || _m_original != NULL || _m_project_version == NULL || _m_project_version->project() == NULL || _m_project_version->project()->projectGrants() == NULL)
        return false;

    return compare_values(_m_project_version->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC), PROJECT_GRANT_WRITE) == 0;
}




int TestHierarchy::deleteRecord()
{
    Test	*tmp_test = NULL;
    int		tmp_result = NOERR;

    if (is_empty_string(getIdentifier()) == FALSE)
    {
        tmp_test = new Test();
        tmp_test->setValueForKey(getIdentifier(), TESTS_TABLE_TEST_ID);
        tmp_result = tmp_test->deleteRecord();
        delete tmp_test;
    }

    return tmp_result;
}



/**
  Definir le test comme copie liee du test passe en parametre
**/
void TestHierarchy::setAsLinkOf(TestHierarchy* in_original)
{
    TestHierarchy* tmp_child_test = NULL;
    TestHierarchy* tmp_original = in_original;

    _m_original = in_original;

    while (tmp_original != NULL)
    {
        if (tmp_original->_m_links.indexOf(this) < 0)
            tmp_original->_m_links.append(this);

        tmp_original = tmp_original->_m_original;
    }

    foreach(TestHierarchy *tmp_test, _m_original->_m_childs)
    {
        tmp_child_test = tmp_test->duplicate(true);
        tmp_child_test->_m_parent = this;
        _m_childs.append(tmp_child_test);
    }
}


TestHierarchy* TestHierarchy::original()
{
    TestHierarchy		*tmp_original = _m_original;

    while (tmp_original != NULL && tmp_original->_m_original != NULL)
        tmp_original = tmp_original->_m_original;

    return tmp_original;
}


/**
  Creer une copie liee au test courant
**/
TestHierarchy* TestHierarchy::duplicate(bool in_recursivly)
{
    TestHierarchy*    tmp_clone = clone<TestHierarchy>();
    TestHierarchy*    tmp_child_test = NULL;
    TestHierarchy*	tmp_original = this;

    tmp_clone->_m_record_status = _m_record_status;
    tmp_clone->_m_project_version = _m_project_version;

    tmp_clone->_m_original = this;

    while (tmp_original != NULL)
    {
        tmp_original->_m_links.append(tmp_clone);
        tmp_original = tmp_original->_m_original;
    }

    tmp_clone->_m_is_a_copy = true;

    if (in_recursivly)
    {
        foreach(TestHierarchy *tmp_test, _m_childs)
        {
            tmp_child_test = tmp_test->duplicate(in_recursivly);
            tmp_child_test->_m_parent = tmp_clone;
            tmp_clone->_m_childs.append(tmp_child_test);
        }

    }

    return tmp_clone;
}


/**
  Creer une copie du test en vue d'un deplacement de l'original
**/
Hierarchy* TestHierarchy::cloneForMove(bool in_recursivly)
{
    TestHierarchy*    tmp_clone = clone<TestHierarchy>();
    TestHierarchy*    tmp_child_test = NULL;

    int         tmp_original_copy_index = 0;

    tmp_clone->_m_record_status = _m_record_status;
    tmp_clone->_m_links = _m_links;
    foreach(TestHierarchy* tmp_copies, _m_links)
        tmp_copies->_m_original = tmp_clone;
    tmp_clone->_m_is_a_copy = _m_is_a_copy;

    tmp_clone->_m_original = _m_original;
    if (_m_original != NULL)
    {
        tmp_original_copy_index = _m_original->_m_links.indexOf(this);
        if (tmp_original_copy_index >= 0)
            _m_original->_m_links.replace(tmp_original_copy_index, tmp_clone);
    }

    tmp_clone->_m_project_version = _m_project_version;

    if (in_recursivly)
    {
        foreach(TestHierarchy *tmp_test, _m_childs)
        {
            tmp_child_test = (TestHierarchy*)tmp_test->cloneForMove(in_recursivly);
            tmp_child_test->_m_parent = tmp_clone;
            tmp_clone->_m_childs.append(tmp_child_test);
        }

    }

    return tmp_clone;
}


/**
  Enlever le lien vers le test original
**/
void TestHierarchy::unsetLink()
{
    TestHierarchy	*tmp_original = _m_original;
    int				tmp_index = 0;

    while (tmp_original != NULL)
    {
        tmp_index = tmp_original->_m_links.indexOf(this);
        if (tmp_index >= 0)
            tmp_original->_m_links.removeAt(tmp_index);

        tmp_original = tmp_original->_m_original;
    }

    _m_original = NULL;
    qDeleteAll(_m_childs);
}


/**
  Supprimer tous les tests lies
**/
void TestHierarchy::unLink()
{
    foreach(TestHierarchy* tmp_test_copy, _m_links)
    {
        tmp_test_copy->_m_original = NULL;
        tmp_test_copy->_m_childs.clear();
    }

    _m_links.clear();
}


/**
  Enregistrer le test
**/
int TestHierarchy::saveRecord()
{
    int     tmp_result = NOERR;
    Test    *tmp_test_record = cloneTestRecord();

    if (_m_original == NULL)
    {
        setValueForKey(NULL, TESTS_HIERARCHY_ORIGINAL_TEST_ID);
    }
    else
    {

        // Enregistrer le test original
        if (is_empty_string(_m_original->getIdentifier()))
        {
            tmp_result = _m_original->saveRecord();
            if (tmp_result == NOERR)
                setValueForKey(_m_original->getIdentifier(), TESTS_HIERARCHY_ORIGINAL_TEST_ID);
            else
                return tmp_result;
        }
        else
            setValueForKey(_m_original->getIdentifier(), TESTS_HIERARCHY_ORIGINAL_TEST_ID);
    }

    // Enregistrer le test parent
    if (_m_parent != NULL)
    {
        if (is_empty_string(_m_parent->getIdentifier()))
        {
            tmp_result = _m_parent->saveRecord();
            if (tmp_result == NOERR)
                setValueForKey(_m_parent->getIdentifier(), TESTS_HIERARCHY_PARENT_TEST_ID);
            else
                return tmp_result;
        }
        else
            setValueForKey(_m_parent->getIdentifier(), TESTS_HIERARCHY_PARENT_TEST_ID);

        setValueForKey(_m_parent->getValueForKey(TESTS_HIERARCHY_PROJECT_ID), TESTS_HIERARCHY_PROJECT_ID);
        setValueForKey(_m_parent->getValueForKey(TESTS_HIERARCHY_VERSION), TESTS_HIERARCHY_VERSION);
    }
    else
        setValueForKey(NULL, TESTS_HIERARCHY_PARENT_TEST_ID);

    if (_m_is_a_copy == false)
    {
        if (_m_project_version != NULL)
        {
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), TESTS_HIERARCHY_PROJECT_ID);
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), TESTS_HIERARCHY_VERSION);
        }

        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_ID), TESTS_TABLE_ORIGINAL_TEST_ID);
        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_PARENT_TEST_ID), TESTS_TABLE_PARENT_TEST_ID);
        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_PREVIOUS_TEST_ID), TESTS_TABLE_PREVIOUS_TEST_ID);
        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_PROJECT_ID), TESTS_TABLE_PROJECT_ID);
        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID), TESTS_TABLE_TEST_CONTENT_ID);
        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_VERSION), TESTS_TABLE_VERSION);

        if (is_empty_string(getIdentifier()))
        {
            tmp_result = tmp_test_record->insertRecord();
            if (tmp_result == NOERR)
            {
                setValueForKey(tmp_test_record->getIdentifier(), TESTS_HIERARCHY_TEST_ID);
                cloneColumns();
            }
        }
        else
        {
            tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_TEST_ID), TESTS_TABLE_TEST_ID);
            tmp_result = tmp_test_record->saveRecord();
            if (tmp_result == NOERR)
            {
                cloneColumns();
            }
        }
    }

    return tmp_result;
}


/**
  Enregistrer les tests enfants
**/
int TestHierarchy::saveChildsRecords()
{
    int			    tmp_index = 0;
    int			    tmp_result = NOERR;
    const char		    *tmp_previous_child_id = NULL;

    for(tmp_index = 0; tmp_result == NOERR && tmp_index < _m_childs.count(); tmp_index++)
    {
        _m_childs[tmp_index]->setValueForKey(tmp_previous_child_id, TESTS_HIERARCHY_PREVIOUS_TEST_ID);
        tmp_result = _m_childs[tmp_index]->saveRecord();
        if (tmp_result == NOERR)
            tmp_result = _m_childs[tmp_index]->saveChildsRecords();

        tmp_previous_child_id = _m_childs[tmp_index]->getIdentifier();
    }

    return tmp_result;
}

void TestHierarchy::searchFieldWithValue(QList<Record*> *in_found_list, const char* in_field_name, const char* in_field_value, bool in_recursive, int in_comparison_value)
{
    int		tmp_comparison = 0;

    tmp_comparison = compare_values(getValueForKey(in_field_name), in_field_value);
    if ((tmp_comparison == 0 && (in_comparison_value == EqualTo || in_comparison_value == LowerOrEqualTo || in_comparison_value == UpperOrEqualTo))
    || (tmp_comparison < 0 && (in_comparison_value == LowerThan || in_comparison_value == LowerOrEqualTo))
    || (tmp_comparison > 0 && (in_comparison_value == UpperThan || in_comparison_value == UpperOrEqualTo)))
    {
        in_found_list->append(this);
    }

    if (in_recursive)
    {
        foreach(TestHierarchy *tmp_child, _m_childs)
        {
            tmp_child->searchFieldWithValue(in_found_list, in_field_name, in_field_value, in_recursive, in_comparison_value);
        }
    }
}


Test* TestHierarchy::cloneTestRecord()
{
    Test		*tmp_test_record = new Test();

    tmp_test_record->setValueForKey(getOriginalValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_ID), TESTS_TABLE_ORIGINAL_TEST_ID);
    tmp_test_record->setValueForKey(getOriginalValueForKey(TESTS_HIERARCHY_PARENT_TEST_ID), TESTS_TABLE_PARENT_TEST_ID);
    tmp_test_record->setValueForKey(getOriginalValueForKey(TESTS_HIERARCHY_PREVIOUS_TEST_ID), TESTS_TABLE_PREVIOUS_TEST_ID);
    tmp_test_record->setValueForKey(getOriginalValueForKey(TESTS_HIERARCHY_PROJECT_ID), TESTS_TABLE_PROJECT_ID);
    tmp_test_record->setValueForKey(getOriginalValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID), TESTS_TABLE_TEST_CONTENT_ID);
    tmp_test_record->setValueForKey(getOriginalValueForKey(TESTS_HIERARCHY_VERSION), TESTS_TABLE_VERSION);
    tmp_test_record->cloneColumns();

    return tmp_test_record;
}


int TestHierarchy::loadChilds()
{
    TestHierarchy			**tmp_childs_tests = NULL;
    unsigned long			tmp_tests_count = 0;
    net_session				*tmp_session = CLIENT_SESSION;
    unsigned long			tmp_index = 0;

    sprintf(tmp_session->m_last_query, "%s=%s", TESTS_HIERARCHY_PARENT_TEST_ID, getIdentifier());

    tmp_childs_tests = Record::loadRecords<TestHierarchy>(tmp_session, &tests_hierarchy_def, tmp_session->m_last_query, NULL, &tmp_tests_count);
    if (tmp_childs_tests != NULL)
    {
        if (tmp_tests_count > 0)
        {
            for(tmp_index = 0; tmp_index < tmp_tests_count; tmp_index++)
            {
                tmp_childs_tests[tmp_index]->_m_parent = this;
                tmp_childs_tests[tmp_index]->_m_project_version = _m_project_version;
                tmp_childs_tests[tmp_index]->loadChilds();
            }
            _m_childs = ProjectVersion::orderedProjectTests(_m_project_version, tmp_childs_tests, tmp_tests_count, getIdentifier());
        }

        Record::freeRecords<TestHierarchy>(tmp_childs_tests, 0);
    }

    return NOERR;
}


Hierarchy* TestHierarchy::findItemWithId(const char* in_item_id, int in_child_type, bool in_recursive)
{
    if (in_child_type == TESTS_HIERARCHY_SIG_ID)
        return findTestWithId(_m_childs, in_item_id, in_recursive);

    return NULL;
}


void TestHierarchy::writeXml(QXmlStreamWriter & in_xml_writer)
{
    in_xml_writer.writeStartElement("test");

    if (_m_original != NULL)
    {
        in_xml_writer.writeAttribute("id", getIdentifier());
        in_xml_writer.writeAttribute("originalId", _m_original->getIdentifier());
        in_xml_writer.writeAttribute("testContentId", _m_original->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID));
    }
    else
    {
        in_xml_writer.writeAttribute("id", getIdentifier());
        in_xml_writer.writeAttribute("testContentId", getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID));

        if (_m_childs.count() > 0)
        {
            in_xml_writer.writeStartElement("tests");
            foreach(TestHierarchy *tmp_test, _m_childs)
            {
                tmp_test->writeXml(in_xml_writer);
            }
            in_xml_writer.writeEndElement();
        }

    }

    in_xml_writer.writeEndElement();
}


bool TestHierarchy::readXml(QXmlStreamReader & in_xml_reader)
{
    TestHierarchy		*tmp_test = NULL;

    QString			tmp_id = in_xml_reader.attributes().value("id").toString();
    QString			tmp_original_id = in_xml_reader.attributes().value("originalId").toString();
    QString			tmp_test_content_id = in_xml_reader.attributes().value("testContentId").toString();

    const char			*tmp_previous_test_id = NULL;

    setValueForKey(tmp_id.toStdString().c_str(), TESTS_HIERARCHY_TEST_ID);
    setValueForKey(tmp_original_id.toStdString().c_str(), TESTS_HIERARCHY_ORIGINAL_TEST_ID);
    setValueForKey(tmp_test_content_id.toStdString().c_str(), TESTS_HIERARCHY_TEST_CONTENT_ID);

    while (in_xml_reader.readNextStartElement())
    {
        if (in_xml_reader.name() == "tests")
        {
            while (in_xml_reader.readNextStartElement())
            {
                if (in_xml_reader.name() == "test")
                {
                    tmp_test = new TestHierarchy(this);
                    tmp_test->setValueForKey(tmp_id.toStdString().c_str(), TESTS_HIERARCHY_PARENT_TEST_ID);
                    tmp_test->setValueForKey(tmp_previous_test_id, TESTS_HIERARCHY_PREVIOUS_TEST_ID);
                    tmp_test->readXml(in_xml_reader);
                    tmp_previous_test_id = tmp_test->getIdentifier();
                    _m_childs.append(tmp_test);
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


int TestHierarchy::saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas)
{
    int			tmp_result = NOERR;
    QString		tmp_test_id = QString(getIdentifier());
    const char		*tmp_parent_test_id = getValueForKey(TESTS_HIERARCHY_PARENT_TEST_ID);
    const char		*tmp_original_test_id = getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_ID);
    const char		*tmp_previous_test_id = getValueForKey(TESTS_HIERARCHY_PREVIOUS_TEST_ID);
    const char		*tmp_test_content_id = getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID);
    Test		*tmp_test_record = NULL;

    if (is_empty_string(getOriginalValueForKey(TESTS_HIERARCHY_TEST_ID)) == TRUE)
    {
        setValueForKey(NULL, TESTS_HIERARCHY_TEST_ID);

        if (_m_project_version != NULL)
        {
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), TESTS_HIERARCHY_PROJECT_ID);
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), TESTS_HIERARCHY_VERSION);
        }

        if (is_empty_string(tmp_parent_test_id) == FALSE)
            setValueForKey(in_xml_datas.getNewTestHierarchyIdentifier(_m_project_version, tmp_parent_test_id), TESTS_HIERARCHY_PARENT_TEST_ID);

        if (is_empty_string(tmp_original_test_id) == FALSE)
            setValueForKey(in_xml_datas.getNewTestHierarchyIdentifier(_m_project_version, tmp_original_test_id), TESTS_HIERARCHY_ORIGINAL_TEST_ID);

        if (is_empty_string(tmp_previous_test_id) == FALSE)
            setValueForKey(in_xml_datas.getNewTestHierarchyIdentifier(_m_project_version, tmp_previous_test_id), TESTS_HIERARCHY_PREVIOUS_TEST_ID);

        setValueForKey(in_xml_datas.getNewTestContentIdentifier(tmp_test_content_id), TESTS_HIERARCHY_TEST_CONTENT_ID);

        tmp_test_record = new Test();
        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_ID), TESTS_TABLE_ORIGINAL_TEST_ID);
        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_PARENT_TEST_ID), TESTS_TABLE_PARENT_TEST_ID);
        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_PREVIOUS_TEST_ID), TESTS_TABLE_PREVIOUS_TEST_ID);
        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_PROJECT_ID), TESTS_TABLE_PROJECT_ID);
        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID), TESTS_TABLE_TEST_CONTENT_ID);
        tmp_test_record->setValueForKey(getValueForKey(TESTS_HIERARCHY_VERSION), TESTS_TABLE_VERSION);

        tmp_result = tmp_test_record->insertRecord();
        if (tmp_result == NOERR)
        {
            in_xml_datas.m_tests_dict.insert(tmp_test_id, this);

            setValueForKey(tmp_test_record->getIdentifier(), TESTS_HIERARCHY_TEST_ID);
            cloneColumns();

            foreach(TestHierarchy *tmp_test, _m_childs)
            {
                tmp_test->saveFromXmlProjectDatas(in_xml_datas);
            }
        }
    }


    return tmp_result;
}


QList<Bug*> TestHierarchy::loadBugs()
{
    Bug					**tmp_bugs = NULL;
    unsigned long			tmp_bugs_count = 0;
    net_session				*tmp_session = CLIENT_SESSION;
    unsigned long			tmp_index = 0;
    QList<Bug*>				tmp_bugs_list;

    sprintf(tmp_session->m_last_query, "%s IN (SELECT %s FROM %s WHERE %s=%s)",
    BUGS_TABLE_EXECUTION_TEST_ID,
    EXECUTIONS_TESTS_TABLE_EXECUTION_TEST_ID,
    EXECUTIONS_TESTS_TABLE_SIG,
    EXECUTIONS_TESTS_TABLE_TEST_ID,
    getIdentifier());

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


QList<CustomTestField*> TestHierarchy::loadCustomFields()
{
    sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s", CUSTOM_TEST_FIELDS_TABLE_TEST_CONTENT_ID, getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID));
    return CustomTestField::loadRecordsList(CLIENT_SESSION->m_where_clause_buffer);
}


void TestHierarchy::setDataFromTestContent(TestContent* in_test_content)
{
    if (in_test_content != NULL)
    {
        setValueForKey(in_test_content->getIdentifier(), TESTS_HIERARCHY_TEST_CONTENT_ID);
        setValueForKey(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_SHORT_NAME), TESTS_HIERARCHY_SHORT_NAME);
        setValueForKey(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_CATEGORY_ID), TESTS_HIERARCHY_CATEGORY_ID);
        setValueForKey(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_PRIORITY_LEVEL), TESTS_HIERARCHY_PRIORITY_LEVEL);
        setValueForKey(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_VERSION), TESTS_HIERARCHY_CONTENT_VERSION);
        setValueForKey(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_STATUS), TESTS_HIERARCHY_STATUS);
        setValueForKey(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID), TESTS_HIERARCHY_ORIGINAL_TEST_CONTENT_ID);
        setValueForKey(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_TYPE), TESTS_HIERARCHY_TEST_CONTENT_TYPE);
        setValueForKey(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATED), TESTS_HIERARCHY_TEST_CONTENT_AUTOMATED);
    }
}



bool TestHierarchy::isAutomatedTest() const
{
    return (compare_values(getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_AUTOMATED), YES) == 0);
}

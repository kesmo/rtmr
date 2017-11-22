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

#include "action.h"
#include "utilities.h"

/**
  Constructeur
**/
Action::Action() : Record(getEntityDef())
{
    _m_test_content = NULL;
}


/**
  Constructeur
**/
Action::Action(TestContent *in_test_content) : Record(getEntityDef())
{
    _m_test_content = NULL;
    setTestContent(in_test_content);
}

Action::~Action()
{
    delete _m_test_content;
    destroy();
}

/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* Action::getEntityDef() const
{
    return &actions_table_def;
}


Action* Action::copy(TestContent *in_test_content)
{
    Action  *tmp_new_action = NULL;

    if (in_test_content == NULL)
        tmp_new_action = new Action(_m_test_content);
    else
        tmp_new_action = new Action(in_test_content);

    tmp_new_action->setValueForKey(getValueForKey(ACTIONS_TABLE_SHORT_NAME), ACTIONS_TABLE_SHORT_NAME);
    tmp_new_action->setValueForKey(getValueForKey(ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID), ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID);
    tmp_new_action->setValueForKey(getValueForKey(ACTIONS_TABLE_DESCRIPTION), ACTIONS_TABLE_DESCRIPTION);
    tmp_new_action->setValueForKey(getValueForKey(ACTIONS_TABLE_WAIT_RESULT), ACTIONS_TABLE_WAIT_RESULT);

    return tmp_new_action;
}


int Action::saveRecord()
{
    int tmp_return = NOERR;

    if (is_empty_string(getIdentifier()))
    {
        if (_m_test_content != NULL)
            setValueForKey(_m_test_content->getIdentifier(), ACTIONS_TABLE_TEST_CONTENT_ID);

        tmp_return = Record::saveRecord();
    }
    else
        tmp_return = Record::saveRecord();

    return tmp_return;
}


void Action::setTestContent(TestContent *in_test_content)
{
    if (in_test_content != NULL)
    {
        delete _m_test_content;
        _m_test_content = in_test_content->clone<TestContent>(false);
        setValueForKey(_m_test_content->getIdentifier(), ACTIONS_TABLE_TEST_CONTENT_ID);
    }
}


bool Action::loadAssociatedActionsForVersion(const char *in_project_version, const char *in_original_test_content_id )
{
    QList<Action*>  tmp_associated_actions_list;
    const char      *tmp_original_test_content_id = NULL;

    if (is_empty_string(in_original_test_content_id) == FALSE)
        tmp_original_test_content_id = in_original_test_content_id;
    else if (_m_test_content != NULL)
        tmp_original_test_content_id = _m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID);

    if (is_empty_string(getValueForKey(ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID)) == FALSE && is_empty_string(tmp_original_test_content_id) == FALSE)
    {
        _m_associated_test_actions.clear();

        tmp_associated_actions_list = loadAssociatedTestActionsForVersion(in_project_version);
        if (tmp_associated_actions_list.count() > 0)
        {
            foreach (Action *tmp_associated_action, tmp_associated_actions_list)
            {
                if (!tmp_associated_action->appendAssociatedTestActionsToAction(_m_associated_test_actions, tmp_original_test_content_id, in_project_version))
                {
                    _m_associated_test_actions.clear();
                    return false;
                }
            }
        }
    }

    return true;
}


QList<Action*> Action::loadAssociatedTestActionsForVersion(const char *in_project_version)
{
    QList<Action*>       tmp_actions;

    TestContent		*tmp_test_content = TestContent::loadLastTestContentForVersion(getValueForKey(ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID), in_project_version);

    if (tmp_test_content != NULL)
    {
        tmp_actions = TestContent::loadActions(tmp_test_content->getIdentifier());
        delete tmp_test_content;
    }

    return tmp_actions;
}


bool Action::setAssociatedTest(TestHierarchy *in_test)
{
    QList<Action*>  tmp_associated_actions_list;
    TestContent	    *tmp_test_content = NULL;

    if (in_test != NULL &&
            compare_values(in_test->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID), getValueForKey(ACTIONS_TABLE_TEST_CONTENT_ID)) != 0 &&
            (_m_test_content == NULL || compare_values(in_test->getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_CONTENT_ID), _m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID)) != 0))
    {
        _m_associated_test_actions.clear();
        setValueForKey(in_test->getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_CONTENT_ID), ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID);
        setValueForKey(in_test->getValueForKey(TESTS_HIERARCHY_SHORT_NAME), ACTIONS_TABLE_SHORT_NAME);

        tmp_test_content = new TestContent(in_test->projectVersion());
        if (tmp_test_content->loadRecord(in_test->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID)) == NOERR)
        {
            tmp_associated_actions_list = tmp_test_content->loadActions();
            if (tmp_associated_actions_list.count() > 0)
            {
                foreach (Action *tmp_associated_action, tmp_associated_actions_list)
                {
                    if (!tmp_associated_action->appendAssociatedTestActionsToAction(_m_associated_test_actions, _m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID), in_test->getValueForKey(TESTS_HIERARCHY_VERSION)))
                    {
                        _m_associated_test_actions.clear();
                        setValueForKey(NULL, ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID);
                        return false;
                    }
                }
            }
        }
    }
    else
        return false;

    return true;
}


bool Action::appendAssociatedTestActionsToAction(QList<Action*> & in_actions_list, const char *in_original_test_id, const char *in_project_version)
{
    QList<Action*>  tmp_associated_actions_list;

    if (_m_test_content == NULL || compare_values(in_original_test_id, _m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID)) != 0)
    {
        if (is_empty_string(getValueForKey(ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID)) == FALSE)
        {
            if (compare_values(in_original_test_id, getValueForKey(ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID)) != 0)
            {
                tmp_associated_actions_list = loadAssociatedTestActionsForVersion(in_project_version);
                if (tmp_associated_actions_list.count() > 0)
                {
                    foreach (Action *tmp_associated_action, tmp_associated_actions_list)
                    {
                        if (!tmp_associated_action->appendAssociatedTestActionsToAction(in_actions_list, in_original_test_id, in_project_version))
                            return false;
                    }
                }
            }
            else
            {
                return false;
            }
        }
        else
            in_actions_list.append(this);
    }
    else
        return false;

    return true;
}

QList<Action*> Action::associatedTestActions()
{
    return _m_associated_test_actions;
}

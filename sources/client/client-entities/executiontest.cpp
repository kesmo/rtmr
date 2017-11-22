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

#include "executiontest.h"
#include "executionaction.h"
#include "executioncampaignparameter.h"
#include "executiontestparameter.h"

#include <QTextDocument>

QColor ExecutionTest::OK_COLOR;
QColor ExecutionTest::KO_COLOR;
QColor ExecutionTest::INCOMPLETED_COLOR;
QColor ExecutionTest::BY_PASSED_COLOR;

ExecutionTest::ExecutionTest() : Hierarchy(getEntityDef())
{
  _m_parent = NULL;
  _m_execution_campaign = NULL;
  _m_project_test = NULL;
}


ExecutionTest::ExecutionTest(ExecutionCampaign *in_execution_campaign) : Hierarchy(getEntityDef())
{
  _m_parent = NULL;
  setExecutionCampaign(in_execution_campaign);
  _m_project_test = NULL;
}

/**
  Destructeur
**/
ExecutionTest::~ExecutionTest()
{
  qDeleteAll(_m_childs);
  qDeleteAll(_m_actions);
  qDeleteAll(_m_parameters);
  destroy();
}

/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* ExecutionTest::getEntityDef() const
{
  return &executions_tests_table_def;
}


void ExecutionTest::setExecutionCampaign(ExecutionCampaign *in_execution_campaign)
{
  _m_execution_campaign = in_execution_campaign;
}


void ExecutionTest::setProjectTest(TestHierarchy *in_project_test)
{
  _m_project_test = in_project_test;
  if (_m_project_test != NULL)
    setValueForKey(_m_project_test->getIdentifier(), EXECUTIONS_TESTS_TABLE_TEST_ID);
}

void ExecutionTest::setCampaignTest(TestCampaign *in_campaign_test)
{
  ExecutionTest		*tmp_execution_test = NULL;
  ExecutionAction		*tmp_execution_action = NULL;
  QList<Action*>           tmp_actions;
  TestHierarchy               *tmp_test = NULL;

  if (in_campaign_test != NULL)
    {
      setProjectTest(in_campaign_test->projectTest());

      setValueForKey(CLIENT_MACRO_NOW, EXECUTIONS_TESTS_TABLE_EXECUTION_DATE);
      setValueForKey(EXECUTION_TEST_BYPASSED, EXECUTIONS_TESTS_TABLE_RESULT_ID);

      foreach(TestCampaign *tmp_child_test, in_campaign_test->childs())
        {
          tmp_execution_test = new ExecutionTest(_m_execution_campaign);
          tmp_execution_test->_m_parent = this;
          tmp_execution_test->setCampaignTest(tmp_child_test);
          _m_childs.append(tmp_execution_test);
        }

      if (_m_project_test != NULL)
        {
          if (_m_project_test->original() != NULL)
            tmp_test = _m_project_test->original();
          else
            tmp_test = _m_project_test;

          tmp_actions = TestContent::loadActions(tmp_test->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID));

          foreach(Action *tmp_action, tmp_actions)
            {
              tmp_action->loadAssociatedActionsForVersion(tmp_test->getValueForKey(TESTS_HIERARCHY_VERSION), tmp_test->getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_CONTENT_ID));
              if (tmp_action->associatedTestActions().count() > 0)
                {
                  foreach (Action *tmp_associated_action, tmp_action->associatedTestActions())
                    {
                      tmp_execution_action = new ExecutionAction(tmp_associated_action);
                      tmp_execution_action->setValueForKey(EXECUTION_ACTION_BYPASSED, EXECUTIONS_ACTIONS_TABLE_RESULT_ID);
                      _m_actions.append(tmp_execution_action);
                    }
                }
              else
                {
                  tmp_execution_action = new ExecutionAction(tmp_action);
                  tmp_execution_action->setValueForKey(EXECUTION_ACTION_BYPASSED, EXECUTIONS_ACTIONS_TABLE_RESULT_ID);
                  _m_actions.append(tmp_execution_action);
                }
            }

          qDeleteAll(tmp_actions);
        }
    }
}



void ExecutionTest::loadExecutionsActions()
{
  ExecutionAction              **tmp_executions_actions = NULL;

  unsigned long         tmp_actions_count = 0;

  _m_actions.clear();

  sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s", EXECUTIONS_ACTIONS_TABLE_EXECUTION_TEST_ID, getIdentifier());

  tmp_executions_actions = Record::loadRecords<ExecutionAction>(CLIENT_SESSION, &executions_actions_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_actions_count);
  if (tmp_executions_actions != NULL)
    {
      if (tmp_actions_count > 0)
        _m_actions = orderedTestActions(tmp_executions_actions, tmp_actions_count);

      Record::freeRecords<ExecutionAction>(tmp_executions_actions, 0);
    }
}



/**
  Ordonner la liste des actions
**/
QList < ExecutionAction* > ExecutionTest::orderedTestActions(ExecutionAction **in_test_actions, unsigned long in_actions_count)
{
  unsigned long                  tmp_actions_index = 0;
  ExecutionAction             *tmp_execution_action = NULL;
  QList < ExecutionAction* >  tmp_actions_list;
  Action			   *tmp_action = NULL;

  const char			    *tmp_previous_id = NULL;
  int				    tmp_load_result = NOERR;


  if (in_test_actions != NULL && in_actions_count > 0)
    {
      do
        {
          for (tmp_actions_index = 0; tmp_actions_index < in_actions_count; tmp_actions_index++)
            {
              tmp_execution_action = in_test_actions[tmp_actions_index];
              if (tmp_execution_action != NULL
                  && compare_values(tmp_previous_id, tmp_execution_action->getValueForKey(EXECUTIONS_ACTIONS_TABLE_PREVIOUS_EXECUTION_ACTION_ID)) == 0)
                {
                  /* Associer l'action */
                  tmp_action = new Action();
                  tmp_load_result = tmp_action->loadRecord(tmp_execution_action->getValueForKey(EXECUTIONS_ACTIONS_TABLE_ACTION_ID));
                  if (tmp_load_result != NOERR)
                    {
                      tmp_action->setValueForKey("-- ACTION INCONNUE --", ACTIONS_TABLE_DESCRIPTION);
                      tmp_action->setValueForKey("-- ACTION INCONNUE --", ACTIONS_TABLE_WAIT_RESULT);
                    }
                  else
                    tmp_execution_action->setAction(tmp_action);

                  tmp_actions_list.append(tmp_execution_action);
                  tmp_previous_id = tmp_execution_action->getIdentifier();

                  delete tmp_action;
                  break;
                }
            }
        }
      while (is_empty_string(tmp_previous_id) == FALSE && !(tmp_actions_index >= in_actions_count));
    }

  return tmp_actions_list;
}



bool ExecutionTest::setChilds(QList < ExecutionTest * > in_tests_list)
{
  _m_childs = in_tests_list;

  foreach(ExecutionTest *tmp_test, _m_childs)
    {
      tmp_test->_m_parent = this;
    }

  return true;
}


bool ExecutionTest::insertCopyOfChildren(int /* in_index */, int /* count */, GenericRecord* /* in_item */, int /* in_child_type */)
{
  return false;
}


bool ExecutionTest::insertChildren(int /* position */, int /* count */, Hierarchy* /* in_test */, int /* in_child_type */)
{
  return false;
}


bool ExecutionTest::removeChildren(int /* position */, int /* count */, bool /* in_move_indic */, int /* in_child_type */)
{
  return false;
}

bool ExecutionTest::isWritable(int /* in_child_type */)
{
  return false;
}

QVariant ExecutionTest::data(int column, int role)
{
  float	tmp_test_execution_coverage = executionCoverageRate() * 100;
  float	tmp_test_execution_validated = executionValidatedRate() * 100;
  float	tmp_test_execution_invalidated = executionInValidatedRate() * 100;
  float	tmp_test_execution_bypassed = executionBypassedRate() * 100;

  QString     tmp_float_string;

  if (role == Qt::ToolTipRole)
    {
      return QVariant();
    }
  else if (role == Qt::TextColorRole)
    {
      // Vert
      if (tmp_test_execution_validated == 100.0)
        {
          return QVariant(OK_COLOR);
        }
      else
        {
          if (tmp_test_execution_invalidated > 0.0)
            {
              return QVariant(KO_COLOR);
            }
          else if (tmp_test_execution_coverage == 0.0)
            {
              return QVariant(BY_PASSED_COLOR);
            }
          else
            {
              return QVariant(INCOMPLETED_COLOR);
            }
        }
    }
  else if (role == Qt::DisplayRole)
    {
      if (_m_project_test != NULL)
        {
          switch (column)
            {
            case 0:
              if (role == Qt::DisplayRole && _m_actions.count() >= 1)
                return QVariant(QString(_m_project_test->data(column, role).toString() + " (" + QString::number(_m_actions.count()) + ")"));
              else
                return _m_project_test->data(column, role);
              break;

            case 1:
              if (tmp_test_execution_coverage > 0){
                  tmp_float_string.setNum(tmp_test_execution_coverage, 'f', 0);
                  return QVariant(tmp_float_string + " %");
                }
              break;

            case 2:
              if (tmp_test_execution_validated > 0)
                {
                  tmp_float_string.setNum(tmp_test_execution_validated, 'f', 0);
                  return QVariant(tmp_float_string + " %");
                }
              break;

            case 3:
              if (tmp_test_execution_invalidated > 0)
                {
                  tmp_float_string.setNum(tmp_test_execution_invalidated, 'f', 0);
                  return QVariant(tmp_float_string + " %");
                }
              break;

            case 4:
              if (tmp_test_execution_bypassed > 0)
                {
                  tmp_float_string.setNum(tmp_test_execution_bypassed, 'f', 0);
                  return QVariant(tmp_float_string + " %");
                }
              break;
            }
        }
    }

  return QVariant();
}

Hierarchy* ExecutionTest::copy()
{
  return NULL;
}

Hierarchy* ExecutionTest::cloneForMove(bool /* in_recursivly */)
{
  return NULL;
}


Hierarchy* ExecutionTest::parent()
{
  if (_m_parent != NULL)
    return _m_parent;
  else
    return _m_execution_campaign;
}

int ExecutionTest::row() const
{
  if (_m_parent)
    return _m_parent->_m_childs.indexOf(const_cast<ExecutionTest*>(this));
  else
    return _m_execution_campaign->testsList().indexOf(const_cast<ExecutionTest*>(this));
}


Hierarchy* ExecutionTest::child(int number, int /* in_child_type */)
{
  return _m_childs[number];
}


int ExecutionTest::childCount(int /* in_child_type */)
{
  return _m_childs.count();
}


/**
  Enregistrer les actions
**/
int ExecutionTest::saveExecutionsActions()
{
  int                 tmp_result = NOERR;
  ExecutionAction   *tmp_previous_action = NULL;
  ExecutionAction   *tmp_action = NULL;

  for (int tmp_index = 0; tmp_index < _m_actions.count() && tmp_result == NOERR; tmp_index++)
    {
      tmp_action = _m_actions[tmp_index];
      if (tmp_action != NULL)
        {
          tmp_action->setValueForKey(getIdentifier(), EXECUTIONS_ACTIONS_TABLE_EXECUTION_TEST_ID);

          if (tmp_previous_action != NULL)
            tmp_action->setValueForKey(tmp_previous_action->getIdentifier(), EXECUTIONS_ACTIONS_TABLE_PREVIOUS_EXECUTION_ACTION_ID);
          else
            tmp_action->setValueForKey(NULL, EXECUTIONS_ACTIONS_TABLE_PREVIOUS_EXECUTION_ACTION_ID);

          tmp_result = tmp_action->saveRecord();
        }

      tmp_previous_action = tmp_action;
    }

  return tmp_result;
}



/**
  Enregistrer les paramètres d'exécutions
**/
int ExecutionTest::saveExecutionsParameters()
{
  int			    tmp_result = NOERR;
  ExecutionTestParameter   *tmp_parameter = NULL;

  for (int tmp_index = 0; tmp_index < _m_parameters.count() && tmp_result == NOERR; tmp_index++)
    {
      tmp_parameter = _m_parameters[tmp_index];
      if (tmp_parameter != NULL)
        {
          tmp_parameter->setValueForKey(getIdentifier(), EXECUTIONS_TESTS_PARAMETERS_TABLE_EXECUTION_TEST_ID);
          tmp_result = tmp_parameter->saveRecord();
        }
    }

  return tmp_result;
}

void ExecutionTest::updateTestResult(bool in_recursive)
{
  if(executionBypassedRate() == 1.0)
    setValueForKey(EXECUTION_TEST_BYPASSED, EXECUTIONS_TESTS_TABLE_RESULT_ID);
  else if(executionValidatedRate() == 1.0)
    setValueForKey(EXECUTION_TEST_VALIDATED, EXECUTIONS_TESTS_TABLE_RESULT_ID);
  else if(executionInValidatedRate() > 0.0)
    setValueForKey(EXECUTION_TEST_INVALIDATED, EXECUTIONS_TESTS_TABLE_RESULT_ID);
  else if(executionCoverageRate() < 1.0)
    setValueForKey(EXECUTION_TEST_INCOMPLETED, EXECUTIONS_TESTS_TABLE_RESULT_ID);

  if (in_recursive)
    {
      foreach(ExecutionTest* tmp_child_execution_test, _m_childs)
        {
          tmp_child_execution_test->updateTestResult(in_recursive);
        }
    }
}


int ExecutionTest::saveRecord()
{
  int tmp_result = NOERR;

  if (is_empty_string(getIdentifier()))
    {
      // Associer le parent
      if (_m_parent != NULL)
        {
          if (is_empty_string(_m_parent->getIdentifier()))
            {
              tmp_result = _m_parent->saveRecord();
              if (tmp_result == NOERR)
                setValueForKey(_m_parent->getIdentifier(), EXECUTIONS_TESTS_TABLE_PARENT_EXECUTION_TEST_ID);
              else
                return tmp_result;
            }
          else
            setValueForKey(_m_parent->getIdentifier(), EXECUTIONS_TESTS_TABLE_PARENT_EXECUTION_TEST_ID);
        }
      else
        setValueForKey(NULL, EXECUTIONS_TESTS_TABLE_PARENT_EXECUTION_TEST_ID);

      // Associer le test du projet
      if (_m_project_test != NULL)
        {
          if (is_empty_string(_m_project_test->getIdentifier()))
            {
              tmp_result = _m_project_test->saveRecord();
              if (tmp_result == NOERR)
                setValueForKey(_m_project_test->getIdentifier(), EXECUTIONS_TESTS_TABLE_TEST_ID);
              else
                return tmp_result;
            }
          else
            setValueForKey(_m_project_test->getIdentifier(), EXECUTIONS_TESTS_TABLE_TEST_ID);
        }
      else
        setValueForKey(NULL, EXECUTIONS_TESTS_TABLE_TEST_ID);

      //Associer l'execution de la campagne
      if (_m_execution_campaign != NULL)
        {
          setValueForKey(_m_execution_campaign->getIdentifier(), EXECUTIONS_TESTS_TABLE_EXECUTION_CAMPAIGN_ID);
        }

      if (tmp_result == NOERR)
        tmp_result = Record::insertRecord();
    }
  else
    tmp_result = Record::saveRecord();

  if (tmp_result == NOERR)
    {
      // Sauvegarder les test enfants
      tmp_result = saveHierarchicalList(_m_childs, EXECUTIONS_TESTS_TABLE_PREVIOUS_EXECUTION_TEST_ID);

      if (tmp_result == NOERR)
        {
          // Sauvegarder les actions
          tmp_result = saveExecutionsActions();
          if (tmp_result == NOERR)
            {
              // Sauvegarder les paramètres d'exécutions
              tmp_result = saveExecutionsParameters();
            }

        }

    }

  return tmp_result;
}


float ExecutionTest::executionCoverageRate()
{
  return (1.0 - executionBypassedRate());
}


float ExecutionTest::executionRateForResults(const char* in_test_result_id, const char* in_action_result_id)
{
  float	tmp_count = 0;

  float	tmp_childs_result = 0;
  float	tmp_actions_result = 0;

  float	tmp_actions_executions_count = 0;

  float	tmp_result = 0;

  const char	*tmp_result_id = NULL;

  if (_m_childs.isEmpty() && _m_actions.isEmpty())
    {
      tmp_result_id = getValueForKey(EXECUTIONS_TESTS_TABLE_RESULT_ID);
      if (is_empty_string(tmp_result_id) == FALSE && compare_values(tmp_result_id, in_test_result_id) == 0)
        return 1;
      else
        return 0;
    }

  if (_m_childs.isEmpty() == false)
    {
      tmp_count += 1;
      foreach(ExecutionTest *tmp_child_execution_test, _m_childs)
        {
          tmp_childs_result += tmp_child_execution_test->executionRateForResults(in_test_result_id, in_action_result_id);
        }

      tmp_childs_result = tmp_childs_result / _m_childs.count();
    }

  if (_m_actions.isEmpty() == false)
    {
      tmp_count += 1;
      foreach(ExecutionAction *tmp_execution_action, _m_actions)
        {
          tmp_result_id = tmp_execution_action->getValueForKey(EXECUTIONS_ACTIONS_TABLE_RESULT_ID);
          if (is_empty_string(tmp_result_id) == FALSE && compare_values(tmp_result_id, in_action_result_id) == 0)
            {
              tmp_actions_executions_count++;
            }
        }
      tmp_actions_result = (tmp_actions_executions_count/_m_actions.count());
    }

  tmp_result = (tmp_childs_result + tmp_actions_result) / tmp_count;

  return tmp_result;

}




float ExecutionTest::executionValidatedRate()
{
  return executionRateForResults(EXECUTION_TEST_VALIDATED, EXECUTION_ACTION_VALIDATED);
}


float ExecutionTest::executionInValidatedRate()
{
  return executionRateForResults(EXECUTION_TEST_INVALIDATED, EXECUTION_ACTION_INVALIDATED);
}


float ExecutionTest::executionBypassedRate()
{
  return executionRateForResults(EXECUTION_TEST_BYPASSED, EXECUTION_ACTION_BYPASSED);
}


float ExecutionTest::executionIncompleteRate()
{
  return executionRateForResults(EXECUTION_TEST_INCOMPLETED, EXECUTION_ACTION_BYPASSED);
}


int ExecutionTest::executionCountForResult(const char *in_result)
{
  int tmp_count = 0;

  if (_m_childs.isEmpty() == false)
    {
      foreach(ExecutionTest *tmp_child_execution_test, _m_childs)
        {
          tmp_count += tmp_child_execution_test->executionCountForResult(in_result);
        }
    }
  else
    {
      if (compare_values(getValueForKey(EXECUTIONS_TESTS_TABLE_RESULT_ID), in_result) == 0)
        tmp_count = 1;
    }

  return tmp_count;
}


QString ExecutionTest::toFragmentHtml(QString suffix)
{
  QString		tmp_html_content = QString();
  int			tmp_index = 0;
  QString		tmp_coverage_rate = "", validated_rate = "", invalidated_rate = "", bypassed_rate = "";

  float	tmp_test_execution_coverage = executionCoverageRate() * 100;
  float	tmp_test_execution_validated = executionValidatedRate() * 100;
  float	tmp_test_execution_invalidated = executionInValidatedRate() * 100;
  float	tmp_test_execution_bypassed = executionBypassedRate() * 100;

  if (tmp_test_execution_coverage > 0)
    tmp_coverage_rate = QString::number(tmp_test_execution_coverage, 'f', 0) + " %";

  if (tmp_test_execution_validated > 0)
    validated_rate = QString::number(tmp_test_execution_validated, 'f', 0) + " %";

  if (tmp_test_execution_invalidated > 0)
    invalidated_rate = QString::number(tmp_test_execution_invalidated, 'f', 0) + " %";

  if (tmp_test_execution_bypassed > 0)
    bypassed_rate = QString::number(tmp_test_execution_bypassed, 'f', 0) + " %";

  tmp_html_content += "<tr><td>" + suffix + " " + QString(_m_project_test->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + "</td>";
  tmp_html_content += "<td align=\"right\">" + tmp_coverage_rate + "</td>";
  tmp_html_content += "<td align=\"right\">" + validated_rate + "</td>";
  tmp_html_content += "<td align=\"right\">" + invalidated_rate + "</td>";
  tmp_html_content += "<td align=\"right\">" + bypassed_rate + "</td></tr>";

  foreach(ExecutionTest *tmp_execution_test, _m_childs)
    {
      tmp_index++;
      tmp_html_content += tmp_execution_test->toFragmentHtml(suffix + QString::number(tmp_index) + ".");
    }

  return tmp_html_content;
}


Hierarchy* ExecutionTest::findItemWithId(const char* in_item_id, int in_child_type, bool in_recursive)
{
  ExecutionTest *tmp_found_test = NULL;

  if (in_child_type == EXECUTIONS_TESTS_TABLE_SIG_ID || in_child_type == 0)
    {
      foreach(ExecutionTest *tmp_test, _m_childs)
        {
          if (compare_values(tmp_test->getIdentifier(), in_item_id) == 0)
            return tmp_test;

          if (in_recursive)
            {
              tmp_found_test = (ExecutionTest*)tmp_test->findItemWithId(in_item_id, in_child_type, in_recursive);
              if (tmp_found_test != NULL)
                return tmp_found_test;
            }
        }
    }

  return NULL;
}


QList<Bug*> ExecutionTest::loadBugs()
{
  Bug					**tmp_bugs = NULL;
  unsigned long			tmp_bugs_count = 0;
  unsigned long			tmp_index = 0;
  QList<Bug*>				tmp_bugs_list;

  sprintf(CLIENT_SESSION->m_last_query, "%s=%s", BUGS_TABLE_EXECUTION_TEST_ID, getIdentifier());

  tmp_bugs = Record::loadRecords<Bug>(CLIENT_SESSION, &bugs_table_def, CLIENT_SESSION->m_last_query, BUGS_TABLE_CREATION_DATE, &tmp_bugs_count);
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


void ExecutionTest::synchronizeFromTestDatas(QList<TestCampaign*> in_tests_campaign_list)
{
  ExecutionAction        			*tmp_execution_action = NULL;
  int								tmp_test_action_index = 0;
  int								tmp_execution_action_index = 0;
  QList<Action*>					tmp_actions;
  QList<Action*>					tmp_all_actions;

  ExecutionTest        			*tmp_execution_test = NULL;
  int								tmp_test_index = 0;
  int								tmp_execution_test_index = 0;
  TestHierarchy               *tmp_test = NULL;

  if (_m_project_test != NULL)
    {
      if (_m_project_test->original() != NULL)
        tmp_test = _m_project_test->original();
      else
        tmp_test = _m_project_test;

      tmp_actions = TestContent::loadActions(tmp_test->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID));
      foreach(Action *tmp_action, tmp_actions)
        {
          tmp_action->loadAssociatedActionsForVersion(tmp_test->getValueForKey(TESTS_HIERARCHY_VERSION), tmp_test->getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_CONTENT_ID));
          if (tmp_action->associatedTestActions().count() > 0)
            {
              foreach (Action *tmp_associated_action, tmp_action->associatedTestActions())
                {
                  tmp_all_actions.append(tmp_associated_action);
                }
            }
          else
            {
              tmp_all_actions.append(tmp_action);
            }
        }
    }

  for(tmp_test_action_index = 0; tmp_test_action_index < tmp_all_actions.count(); tmp_test_action_index++)
    {
      tmp_execution_action_index = indexForActionWithValueForKey(tmp_all_actions[tmp_test_action_index]->getValueForKey(ACTIONS_TABLE_ACTION_ID), EXECUTIONS_ACTIONS_TABLE_ACTION_ID);
      if (tmp_execution_action_index < 0)
        {
          tmp_execution_action = new ExecutionAction(tmp_all_actions[tmp_test_action_index]);
          tmp_execution_action->setValueForKey(EXECUTION_ACTION_BYPASSED, EXECUTIONS_ACTIONS_TABLE_RESULT_ID);
          _m_actions.insert(tmp_test_action_index, tmp_execution_action);
        }
      else if (tmp_execution_action_index != tmp_test_action_index)
        {
          _m_actions.move(tmp_execution_action_index, tmp_test_action_index);
        }
    }

  for(tmp_test_action_index = tmp_all_actions.count(); tmp_test_action_index < _m_actions.count();)
    {
      tmp_execution_action = _m_actions.takeAt(tmp_test_action_index);
      tmp_execution_action->deleteRecord();
      delete tmp_execution_action;
    }

  for(tmp_test_index = 0; tmp_test_index < in_tests_campaign_list.count(); tmp_test_index++)
    {
      tmp_execution_test_index = indexForItemWithValueForKey(in_tests_campaign_list[tmp_test_index]->getValueForKey(TESTS_CAMPAIGNS_TABLE_TEST_ID), EXECUTIONS_TESTS_TABLE_TEST_ID, EXECUTIONS_TESTS_TABLE_SIG_ID);
      if (tmp_execution_test_index < 0)
        {
          tmp_execution_test = new ExecutionTest(_m_execution_campaign);
          tmp_execution_test->setCampaignTest(in_tests_campaign_list[tmp_test_index]);
          tmp_execution_test->_m_parent = this;
          _m_childs.insert(tmp_test_index, tmp_execution_test);
        }
      else if (tmp_execution_test_index != tmp_test_index)
        {
          _m_childs.move(tmp_execution_test_index, tmp_test_index);
          _m_childs[tmp_test_index]->synchronizeFromTestDatas(in_tests_campaign_list[tmp_test_index]->childs());
        }
      else
        _m_childs[tmp_test_index]->synchronizeFromTestDatas(in_tests_campaign_list[tmp_test_index]->childs());
    }


  for(tmp_test_index = in_tests_campaign_list.count(); tmp_test_index < _m_childs.count();)
    {
      tmp_execution_test = _m_childs.takeAt(tmp_test_index);
      tmp_execution_test->deleteRecord();
      delete tmp_execution_test;
    }
}



int ExecutionTest::indexForActionWithValueForKey(const char *in_value, const char *in_key)
{
  int		tmp_index = 0;

  for(tmp_index = 0; tmp_index < _m_actions.count(); tmp_index++)
    {
      if (compare_values(_m_actions[tmp_index]->getValueForKey(in_key), in_value) == 0)
        return tmp_index;
    }

  return -1;
}


void ExecutionTest::loadExecutionsParameters()
{
  ExecutionTestParameter		**tmp_executions_parameters = NULL;

  unsigned long			tmp_parameters_count = 0;

  _m_parameters.clear();

  sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s", EXECUTIONS_TESTS_PARAMETERS_TABLE_EXECUTION_TEST_ID, getIdentifier());

  tmp_executions_parameters = Record::loadRecords<ExecutionTestParameter>(CLIENT_SESSION, &executions_tests_parameters_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_parameters_count);
  if (tmp_executions_parameters != NULL)
    {
      for(unsigned long tmp_index = 0; tmp_index < tmp_parameters_count; tmp_index++)
        {
          _m_parameters.append(tmp_executions_parameters[tmp_index]);
        }

      Record::freeRecords<ExecutionTestParameter>(tmp_executions_parameters, 0);

    }
}


QList<ExecutionTestParameter*> ExecutionTest::inheritedParameters()
{
  QList<ExecutionTestParameter*>	tmp_params_list = QList<ExecutionTestParameter*>(_m_parameters);
  ExecutionTest					*tmp_parent_test = this;
  ExecutionTestParameter			*tmp_param = NULL;

  while (tmp_parent_test->_m_parent != NULL)
    {
      tmp_parent_test = tmp_parent_test->_m_parent;
      foreach (ExecutionTestParameter *tmp_parent_param, tmp_parent_test->_m_parameters)
        {
          tmp_param = Parameter::parameterForParamName<ExecutionTestParameter>(tmp_params_list, tmp_parent_param->name());
          if (tmp_param == NULL)
            tmp_params_list.append(tmp_parent_param);
        }
    }

  return tmp_params_list;
}


bool ExecutionTest::hasChangedValues() const
{
    if (!Record::hasChangedValues()){
      foreach (ExecutionTest *tmp_test, _m_childs){
          if (tmp_test->hasChangedValues())
            return true;
        }

      foreach (ExecutionAction *tmp_action, _m_actions){
          if (tmp_action->hasChangedValues())
            return true;
        }

      foreach (ExecutionTestParameter *tmp_param, _m_parameters){
          if (tmp_param->hasChangedValues())
            return true;
        }

      return false;
    }

  return true;
}

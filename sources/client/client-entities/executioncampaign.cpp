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

#include "projectparameter.h"
#include "project.h"
#include "projectversion.h"
#include "campaign.h"
#include "executioncampaign.h"
#include "executioncampaignparameter.h"
#include "executiontest.h"

ExecutionCampaign::ExecutionCampaign() : Hierarchy(getEntityDef())
{
    _m_campaign = NULL;
}


ExecutionCampaign::ExecutionCampaign(Campaign *in_campaign) : Hierarchy(getEntityDef())
{
	setCampaign(in_campaign);
}


/**
  Destructeur
**/
ExecutionCampaign::~ExecutionCampaign()
{
    qDeleteAll(_m_tests_list);
    qDeleteAll(_m_parameters_list);
    destroy();
}

/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* ExecutionCampaign::getEntityDef() const
{
    return &executions_campaigns_table_def;
}


ExecutionCampaign* ExecutionCampaign::create(Campaign *in_campaign)
{
    ExecutionTest        			*tmp_execution_test = NULL;
    ExecutionCampaign    			*tmp_execution_campaign = new ExecutionCampaign(in_campaign);
    ExecutionCampaignParameter      *tmp_execution_param = NULL;

    tmp_execution_campaign->setValueForKey(CLIENT_MACRO_NOW, EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_DATE);

    foreach(TestCampaign *tmp_test_campaign, in_campaign->testsList())
    {
        tmp_execution_test = new ExecutionTest(tmp_execution_campaign);
        tmp_execution_test->setCampaignTest(tmp_test_campaign);
        tmp_execution_campaign->_m_tests_list.append(tmp_execution_test);
    }

    if (in_campaign != NULL && in_campaign->projectVersion() != NULL && in_campaign->projectVersion()->project() != NULL)
    {
        foreach(ProjectParameter *tmp_project_param, in_campaign->projectVersion()->project()->parametersList())
        {
        	tmp_execution_param = new ExecutionCampaignParameter();
            tmp_execution_param->setValueForKey(tmp_project_param->getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME), EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_NAME);
            tmp_execution_param->setValueForKey(tmp_project_param->getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE), EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_VALUE);
        	tmp_execution_campaign->_m_parameters_list.append(tmp_execution_param);
        }
    }

    return tmp_execution_campaign;
}



void ExecutionCampaign::setCampaign(Campaign *in_campaign)
{
    _m_campaign = in_campaign;
}


void ExecutionCampaign::addParameter(ExecutionCampaignParameter *in_parameter)
{
    _m_parameters_list.append(in_parameter);
}

void ExecutionCampaign::removeParameter(ExecutionCampaignParameter *in_parameter)
{
    for(int tmp_index = 0; tmp_index < _m_parameters_list.count(); tmp_index++)
    {
	if (in_parameter == _m_parameters_list[tmp_index])
	{
	    delete _m_parameters_list.takeAt(tmp_index);
	}
    }
}



void ExecutionCampaign::loadExecutionsDatas()
{
    loadExecutionsTests();
    loadExecutionsParameters();
}


void ExecutionCampaign::loadExecutionsTests()
{
    ExecutionTest	    **tmp_executions_tests = NULL;
    ExecutionTest	    *tmp_test = NULL;

    unsigned long	    tmp_tests_count = 0;
    char		    tmp_where_clause[128];

    qDeleteAll(_m_tests_list);
    _m_tests_list.clear();

    sprintf(tmp_where_clause, "%s=%s", EXECUTIONS_TESTS_TABLE_EXECUTION_CAMPAIGN_ID, getIdentifier());

    tmp_executions_tests = Record::loadRecords<ExecutionTest>(CLIENT_SESSION, &executions_tests_table_def, tmp_where_clause, NULL, &tmp_tests_count);
    if (tmp_executions_tests != NULL)
    {
        if (tmp_tests_count > 0)
	{
            _m_tests_list = ExecutionCampaign::orderedExecutionsTests(this, tmp_executions_tests, tmp_tests_count, NULL);

	    for (unsigned long tmp_tests_index = 0; tmp_tests_index < tmp_tests_count; tmp_tests_index++)
	    {
		tmp_test = tmp_executions_tests[tmp_tests_index];
		if (findItemWithId(tmp_test->getIdentifier()) == NULL)
		    delete tmp_test;
	    }
	}

        Record::freeRecords<ExecutionTest>(tmp_executions_tests, 0);
    }
}


void ExecutionCampaign::loadExecutionsParameters()
{
    ExecutionCampaignParameter                **tmp_parameters = NULL;

    unsigned long         tmp_parameters_count = 0;
    unsigned long         tmp_parameter_index = 0;
    char                  tmp_where_clause[128];

    qDeleteAll(_m_parameters_list);
    _m_parameters_list.clear();

    sprintf(tmp_where_clause, "%s=%s", EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_EXECUTION_CAMPAIGN_ID, getIdentifier());

    tmp_parameters = Record::loadRecords<ExecutionCampaignParameter>(CLIENT_SESSION, &executions_campaigns_parameters_table_def, tmp_where_clause, NULL, &tmp_parameters_count);
    if (tmp_parameters != NULL)
    {
    	for (tmp_parameter_index = 0; tmp_parameter_index < tmp_parameters_count; tmp_parameter_index++)
    	{
    		_m_parameters_list.append(tmp_parameters[tmp_parameter_index]);
    	}

        Record::freeRecords<ExecutionCampaignParameter>(tmp_parameters, 0);
    }
}



QList < ExecutionTest* > ExecutionCampaign::orderedExecutionsTests(ExecutionCampaign *in_execution_campaign, ExecutionTest **in_executions_tests, unsigned long in_tests_count, const char *in_parent_id)
{
    ExecutionTest               *tmp_test = NULL;
    QList < ExecutionTest* >    tmp_tests_list;
    QList < ExecutionTest* >    tmp_tests_childs_list;

    const char                            *tmp_previous_id = NULL;

    unsigned long                   tmp_tests_index = 0;

    if (in_execution_campaign != NULL && in_tests_count > 0)
    {
        do
        {
            for (tmp_tests_index = 0; tmp_tests_index < in_tests_count; tmp_tests_index++)
            {
                tmp_test = in_executions_tests[tmp_tests_index];
                if (tmp_test != NULL
                    && compare_values(in_parent_id, tmp_test->getValueForKey(EXECUTIONS_TESTS_TABLE_PARENT_EXECUTION_TEST_ID)) == 0
                    && compare_values(tmp_previous_id, tmp_test->getValueForKey(EXECUTIONS_TESTS_TABLE_PREVIOUS_EXECUTION_TEST_ID)) == 0)
                {
                    // Associer a la campagne
                    tmp_test->setExecutionCampaign(in_execution_campaign);

                    // Associer le test du projet
            tmp_test->setProjectTest(TestHierarchy::findTestWithId(in_execution_campaign->campaign()->projectVersion()->testsHierarchy(), tmp_test->getValueForKey(EXECUTIONS_TESTS_TABLE_TEST_ID)));

                    // Charger les actions
                    tmp_test->loadExecutionsActions();

		    // Charger les paramètres d'exécutions
		    tmp_test->loadExecutionsParameters();

		    tmp_tests_list.append(tmp_test);
                    tmp_previous_id = tmp_test->getIdentifier();

                    // Appel recursif : Charger les tests enfants
                    tmp_tests_childs_list = orderedExecutionsTests(in_execution_campaign, in_executions_tests, in_tests_count, tmp_test->getIdentifier());
                    if (tmp_tests_childs_list.count() > 0)
                        tmp_test->setChilds(tmp_tests_childs_list);

                    break;
                }
            }
        }
        while (is_empty_string(tmp_previous_id) == FALSE && !(tmp_tests_index >= in_tests_count));
    }

    return tmp_tests_list;
}



bool ExecutionCampaign::insertChildren(int /* position */, int /* count */, Hierarchy* /* in_test */, int /* in_child_type */)
{
    return false;
}


bool ExecutionCampaign::insertCopyOfChildren(int /* position */, int /* count */, GenericRecord* /* in_item */, int /* in_child_type */)
{
    return false;
}


bool ExecutionCampaign::removeChildren(int /* position */, int /* count */, bool /* in_move_indic */, int /* in_child_type */)
{
    return false;
}

bool ExecutionCampaign::isWritable(int /* in_child_type */)
{
    return true;
}

QVariant ExecutionCampaign::data(int /* column */, int /* role */)
{
    return QVariant();
}

Hierarchy* ExecutionCampaign::copy()
{
    return NULL;
}

Hierarchy* ExecutionCampaign::cloneForMove(bool /* in_recursivly */)
{
    return NULL;
}


Hierarchy* ExecutionCampaign::parent()
{
    return NULL;
}

int ExecutionCampaign::row() const
{
    return 0;
}


Hierarchy* ExecutionCampaign::child(int number, int /* in_child_type */)
{
    return _m_tests_list[number];
}


int ExecutionCampaign::childCount(int /* in_child_type */)
{
    return _m_tests_list.count();
}


int ExecutionCampaign::saveRecord()
{
    int     						tmp_result = NOERR;
    ExecutionCampaignParameter*		tmp_param = NULL;

    setValueForKey(_m_campaign->getIdentifier(), EXECUTIONS_CAMPAIGNS_TABLE_CAMPAIGN_ID);

    tmp_result = Record::saveRecord();
    if (tmp_result == NOERR)
    {
    	// Mettre à jour les resultats des tests
	foreach(ExecutionTest* tmp_child_execution_test, _m_tests_list)
	{
	    tmp_child_execution_test->updateTestResult();
	}

    	// Sauver la liste des tests
	tmp_result = Hierarchy::saveHierarchicalList(_m_tests_list, EXECUTIONS_TESTS_TABLE_PREVIOUS_EXECUTION_TEST_ID);
        if (tmp_result == NOERR)
        {
        	// Sauver la liste des parametres d'executions
        	for (int tmp_index = 0; tmp_index < _m_parameters_list.count() && tmp_result == NOERR; tmp_index++)
        	{
        		tmp_param = _m_parameters_list[tmp_index];
        		if (tmp_param != NULL)
        		{
                    tmp_param->setValueForKey(getIdentifier(), EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_EXECUTION_CAMPAIGN_ID);
        			tmp_result = tmp_param->saveRecord();
        		}
        	}
        }
    }

    return tmp_result;
}


bool ExecutionCampaign::hasChangedValues() const
{
  if (!Record::hasChangedValues()){
      foreach(ExecutionTest* tmp_child_execution_test, _m_tests_list){
          if (tmp_child_execution_test->hasChangedValues())
            return true;
        }

      foreach(ExecutionCampaignParameter* tmp_param, _m_parameters_list){
          if (tmp_param->hasChangedValues())
            return true;
        }
      return false;
    }

  return true;
}

float ExecutionCampaign::executionCoverageRate()
{
	float	tmp_result = 0;

	if (_m_tests_list.count() > 0)
	{
		foreach(ExecutionTest *tmp_execution_test, _m_tests_list)
		{
			tmp_result += tmp_execution_test->executionCoverageRate();
		}
		tmp_result = tmp_result / _m_tests_list.count();
	}

	return tmp_result;
}

float ExecutionCampaign::executionValidatedRate()
{
	float	tmp_result = 0;

	if (_m_tests_list.count() > 0)
	{
		foreach(ExecutionTest *tmp_execution_test, _m_tests_list)
		{
			tmp_result += tmp_execution_test->executionValidatedRate();
		}
		tmp_result = tmp_result / _m_tests_list.count();
	}

	return tmp_result;
}

float ExecutionCampaign::executionInValidatedRate()
{
	float	tmp_result = 0;

	if (_m_tests_list.count() > 0)
	{
		foreach(ExecutionTest *tmp_execution_test, _m_tests_list)
		{
			tmp_result += tmp_execution_test->executionInValidatedRate();
		}
		tmp_result = tmp_result / _m_tests_list.count();
	}

	return tmp_result;
}

float ExecutionCampaign::executionBypassedRate()
{
	float	tmp_result = 0;

	if (_m_tests_list.count() > 0)
	{
		foreach(ExecutionTest *tmp_execution_test, _m_tests_list)
		{
			tmp_result += tmp_execution_test->executionBypassedRate();
		}
		tmp_result = tmp_result / _m_tests_list.count();
	}

	return tmp_result;
}


int ExecutionCampaign::executionCountForResult(const char *in_result)
{
    int tmp_count = 0;

    if (_m_tests_list.isEmpty() == false)
    {
	foreach(ExecutionTest *tmp_execution_test, _m_tests_list)
	{
	    tmp_count += tmp_execution_test->executionCountForResult(in_result);
	}
    }

    return tmp_count;
}


Hierarchy* ExecutionCampaign::findItemWithId(const char* in_item_id , int /* in_child_type */, bool in_recursive)
{
    ExecutionTest *tmp_found_test = NULL;

    foreach(ExecutionTest *tmp_test, _m_tests_list)
    {
	if (compare_values(tmp_test->getIdentifier(), in_item_id) == 0)
	    return tmp_test;

	if (in_recursive)
	{
	    tmp_found_test = (ExecutionTest*)tmp_test->findItemWithId(in_item_id);
	    if (tmp_found_test != NULL)
		return tmp_found_test;
	}
    }

    return NULL;
}

void ExecutionCampaign::synchronizeFromCampaignDatas()
{
    ExecutionTest        			*tmp_execution_test = NULL;
    int						tmp_test_campaign_index = 0;
    int						tmp_execution_test_index = 0;

    QList < TestCampaign* >			tmp_tests_campaign_list = _m_campaign->testsList();

    QMap< QString, int >			tmp_map;
    QString					tmp_id;

    for(tmp_test_campaign_index = 0; tmp_test_campaign_index < tmp_tests_campaign_list.count(); tmp_test_campaign_index++)
    {
    tmp_id = QString(tmp_tests_campaign_list[tmp_test_campaign_index]->getValueForKey(TESTS_CAMPAIGNS_TABLE_TEST_ID));
	tmp_map[tmp_id] = 0;
    }

    for(tmp_test_campaign_index = 0; tmp_test_campaign_index < tmp_tests_campaign_list.count(); tmp_test_campaign_index++)
    {
    tmp_id = QString(tmp_tests_campaign_list[tmp_test_campaign_index]->getValueForKey(TESTS_CAMPAIGNS_TABLE_TEST_ID));

        tmp_execution_test_index = indexForItemWithValueForKeyAtPosition(tmp_tests_campaign_list[tmp_test_campaign_index]->getValueForKey(TESTS_CAMPAIGNS_TABLE_TEST_ID), EXECUTIONS_TESTS_TABLE_TEST_ID, tmp_map[tmp_id], EXECUTIONS_TESTS_TABLE_SIG_ID);
    	if (tmp_execution_test_index < 0)
    	{
	    tmp_execution_test = new ExecutionTest(this);
	    tmp_execution_test->setCampaignTest(tmp_tests_campaign_list[tmp_test_campaign_index]);
	    _m_tests_list.insert(tmp_test_campaign_index, tmp_execution_test);
    	}
	else
	{
	    tmp_map[tmp_id] = tmp_map[tmp_id]+1;

	    if (tmp_execution_test_index != tmp_test_campaign_index)
	    {
		_m_tests_list.move(tmp_execution_test_index, tmp_test_campaign_index);
		_m_tests_list[tmp_test_campaign_index]->synchronizeFromTestDatas(tmp_tests_campaign_list[tmp_test_campaign_index]->childs());
	    }
	    else
	    {
		_m_tests_list[tmp_test_campaign_index]->synchronizeFromTestDatas(tmp_tests_campaign_list[tmp_test_campaign_index]->childs());
	    }
	}
    }

    for(tmp_test_campaign_index = tmp_tests_campaign_list.count(); tmp_test_campaign_index < _m_tests_list.count();)
    {
	tmp_execution_test = _m_tests_list.takeAt(tmp_test_campaign_index);
	tmp_execution_test->deleteRecord();
	delete tmp_execution_test;
    }

    synchronizeFromProjectParameters();
}


void ExecutionCampaign::synchronizeFromProjectParameters()
{
    int								tmp_parameter_index = 0;
    int								tmp_execution_parameter_index = 0;

    QList < ProjectParameter* >		tmp_parameters = _m_campaign->projectVersion()->project()->parametersList();
    ExecutionCampaignParameter*		tmp_parameter = NULL;

    for(tmp_parameter_index = 0; tmp_parameter_index < tmp_parameters.count(); tmp_parameter_index++)
    {
        tmp_execution_parameter_index = indexForParameterWithValueForKey(tmp_parameters[tmp_parameter_index]->getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME), EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_NAME);
    	if (tmp_execution_parameter_index < 0)
    	{
		tmp_parameter = new ExecutionCampaignParameter();
        tmp_parameter->setValueForKey(tmp_parameters[tmp_parameter_index]->getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME), EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_NAME);
        tmp_parameter->setValueForKey(tmp_parameters[tmp_parameter_index]->getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE), EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_VALUE);
		_m_parameters_list.insert(tmp_parameter_index, tmp_parameter);
    	}
    }
}

int ExecutionCampaign::indexForParameterWithValueForKey(const char *in_value, const char *in_key)
{
	int		tmp_index = 0;

	for(tmp_index = 0; tmp_index < _m_parameters_list.count(); tmp_index++)
	{
        if (compare_values(_m_parameters_list[tmp_index]->getValueForKey(in_key), in_value) == 0)
		    return tmp_index;
	}

	return -1;
}



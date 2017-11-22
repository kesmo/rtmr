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

#include "automatedaction.h"
#include "automatedactionvalidation.h"

AutomatedAction::AutomatedAction() : Entity<AutomatedAction>()
{
  _m_test_content = NULL;
}


/**
  Constructeur
**/
AutomatedAction::AutomatedAction(TestContent *in_test_content) : Entity<AutomatedAction>()
{
  _m_test_content = NULL;
  setTestContent(in_test_content);
}

AutomatedAction::~AutomatedAction()
{
  delete _m_test_content;
  destroy();
}


AutomatedAction* AutomatedAction::copy(TestContent *in_test_content)
{
  AutomatedAction  *tmp_new_action = NULL;

  if (in_test_content == NULL)
    tmp_new_action = new AutomatedAction(_m_test_content);
  else
    tmp_new_action = new AutomatedAction(in_test_content);

  tmp_new_action->setValueForKey(getValueForKey(AUTOMATED_ACTIONS_TABLE_WINDOW_ID), AUTOMATED_ACTIONS_TABLE_WINDOW_ID);
  tmp_new_action->setValueForKey(getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA), AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA);
  tmp_new_action->setValueForKey(getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE), AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE);
  tmp_new_action->setValueForKey(getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY), AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY);

  return tmp_new_action;
}


int AutomatedAction::saveRecord()
{
  int tmp_return = NOERR;

  if (is_empty_string(getIdentifier()))
    {
      if (_m_test_content != NULL)
        setValueForKey(_m_test_content->getIdentifier(), AUTOMATED_ACTIONS_TABLE_TEST_CONTENT_ID);

      tmp_return = Record::saveRecord();
    }
  else
    tmp_return = Record::saveRecord();

  if (tmp_return == NOERR){
      foreach(AutomatedActionValidation* tmp_validation, Parent<AutomatedActionValidation>::_m_childs){
          tmp_validation->setAutomatedAction(this);
        }

      tmp_return = saveChilds(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_PREVIOUS_VALIDATION_ID);
    }

  return tmp_return;
}


void AutomatedAction::setTestContent(TestContent *in_test_content)
{
  if (in_test_content != NULL)
    {
      delete _m_test_content;
      _m_test_content = in_test_content->clone<TestContent>(false);
      setValueForKey(_m_test_content->getIdentifier(), AUTOMATED_ACTIONS_TABLE_TEST_CONTENT_ID);
    }
}



QList<AutomatedActionValidation*> AutomatedAction::loadChilds()
{
  QList<AutomatedActionValidation*>   tmp_validations;

  sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s", AUTOMATED_ACTIONS_VALIDATIONS_TABLE_AUTOMATED_ACTION_ID, getIdentifier());
  tmp_validations = AutomatedActionValidation::loadRecordsList(CLIENT_SESSION->m_where_clause_buffer);
  foreach(AutomatedActionValidation* tmp_validation, tmp_validations){
      tmp_validation->setAutomatedAction(this);
    }

  return tmp_validations;
}

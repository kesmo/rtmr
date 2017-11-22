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

#include "form_execution_test.h"
#include "testhierarchy.h"
#include "projectgrant.h"
#include "executioncampaignparameter.h"
#include "executiontestparameter.h"
#include "ui_Form_Execution_Test.h"
#include "form_execution_bugs.h"
#include "session.h"

#include <QMessageBox>
#include <QInputDialog>
#include <QDomDocument>

Form_Execution_Test::Form_Execution_Test(QWidget *parent) : QWidget(parent), _m_ui(new Ui::Form_Execution_Test)
{
  _m_execution_test = NULL;
  _m_execution_action = NULL;
  _m_test_content = NULL;
  _m_ui->setupUi(this);

  _m_ui->test_description->addTextToolBar(RecordTextEditToolBar::Small);
  _m_ui->action_description->addTextToolBar(RecordTextEditToolBar::Small);
  _m_ui->action_result->addTextToolBar(RecordTextEditToolBar::Small);
  _m_ui->comments->addTextToolBar(RecordTextEditToolBar::Small);

  _m_ui->action_group_box->setVisible(false);

  connect(_m_ui->save_comments, SIGNAL(clicked()), this, SLOT(saveComments()));
  connect(_m_ui->save_test_description, SIGNAL(clicked()), this, SLOT(saveTestDescription()));
  connect(_m_ui->ok_button, SIGNAL(clicked()), this, SLOT(validateResult()));
  connect(_m_ui->ko_button, SIGNAL(clicked()), this, SLOT(inValidateResult()));
  connect(_m_ui->bypass_button, SIGNAL(clicked()), this, SLOT(bypass()));
  connect(_m_ui->next_button, SIGNAL(clicked()), this, SLOT(selectNext()));
  connect(_m_ui->previous_button, SIGNAL(clicked()), this, SLOT(selectPrevious()));
  connect(_m_ui->manage_bugs_button, SIGNAL(clicked()), this, SLOT(manageBugs()));
  connect(_m_ui->modify_button, SIGNAL(clicked()), this, SLOT(modifyAction()));
  connect(_m_ui->modify_test_button, SIGNAL(clicked()), this, SLOT(modifyTest()));
  connect(_m_ui->reinit_parameters_button, SIGNAL(clicked()), this, SLOT(reinitExecutionsTestsParameters()));
}

Form_Execution_Test::~Form_Execution_Test()
{
  delete _m_test_content;
  delete _m_ui;
}

void Form_Execution_Test::changeEvent(QEvent *e)
{
  QWidget::changeEvent(e);
  switch (e->type()) {
    case QEvent::LanguageChange:
      _m_ui->retranslateUi(this);
      break;
    default:
      break;
    }
}


/**
  Charger un test
**/
void Form_Execution_Test::loadTest(ExecutionTest *in_execution_test, ExecutionAction *in_execution_action)
{
  TestHierarchy	*tmp_test = NULL;
  const char		*tmp_test_content_id = NULL;
  int			tmp_result = NOERR;

  _m_execution_test = in_execution_test;
  _m_execution_action = in_execution_action;

  if (_m_execution_test != NULL)
    {
      tmp_test = in_execution_test->projectTest();
      if (tmp_test != NULL)
        {
          if (tmp_test->original() != NULL)
            tmp_test_content_id = tmp_test->original()->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID);
          else
            tmp_test_content_id = tmp_test->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID);

          // Supprimer les données du précédent contenu de test
          if (_m_test_content != NULL && compare_values(_m_test_content->getIdentifier(), tmp_test_content_id) != 0)
            {
              destroyAttachments();

              delete _m_test_content;
              _m_test_content = NULL;
            }

          // Charger les données du contenu de test
          if (_m_test_content == NULL)
            {
              _m_test_content = new TestContent();
              tmp_result = _m_test_content->loadRecord(tmp_test_content_id);
              if (tmp_result == NOERR)
                {
                  // Charger les pièces jointes
                  _m_files = _m_test_content->loadFiles();
                }
            }
        }
    }


  initLayout();
}


/**
  Initialiser l'affichage
**/
void Form_Execution_Test::initLayout()
{
  Action		*tmp_action = NULL;
  bool                tmp_modifiable = true;
  bool		tmp_original_action_modifiable = false;

  QPixmap		tmp_test_image_status;
  QString		tmp_image_status_url = QString::fromUtf8(":/images/warning.png");
  QString		tmp_image_status_label = tr("Non passé");

  QList<QString>	tmp_parameters_list;

  QList<QString>	tmp_parameters_names_list;

  const char		*tmp_result_id = NULL;

  _m_ui->action_group_box->setTitle(tr("Action"));

  _m_current_parameters.clear();

  if (_m_execution_test != NULL)
    {
      if (_m_execution_test->executionCampaign() != NULL)
        tmp_parameters_names_list = Parameter::parametersNamesList<ExecutionCampaignParameter>(_m_execution_test->executionCampaign()->parametersList());

      _m_ui->test_description->textEditor()->setCompletionFromList(tmp_parameters_names_list);
      _m_ui->action_description->textEditor()->setCompletionFromList(tmp_parameters_names_list);
      _m_ui->action_result->textEditor()->setCompletionFromList(tmp_parameters_names_list);

      if (_m_execution_test->projectTest())
        {
          tmp_modifiable = _m_execution_test->executionCampaign() != NULL
              && _m_execution_test->executionCampaign()->campaign() != NULL
              && _m_execution_test->executionCampaign()->campaign()->projectVersion() != NULL
              && _m_execution_test->executionCampaign()->campaign()->projectVersion()->project() != NULL
              && _m_execution_test->executionCampaign()->campaign()->projectVersion()->project()->projectGrants() != NULL
              && compare_values(_m_execution_test->executionCampaign()->campaign()->projectVersion()->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC), PROJECT_GRANT_WRITE) == 0;

          tmp_original_action_modifiable = tmp_modifiable &&
              compare_values(_m_execution_test->executionCampaign()->campaign()->projectVersion()->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC), PROJECT_GRANT_WRITE) == 0;

          _m_ui->test_name->setText(_m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_SHORT_NAME));
          tmp_parameters_list = RecordTextEdit::parametersList(_m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION));
          if (tmp_parameters_list.isEmpty())
            {
              _m_ui->test_description->textEditor()->setHtml(_m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION));
            }
          else
            {
              _m_current_parameters.append(tmp_parameters_list);
              synchronizeExecutionsTestsParameters(tmp_parameters_list);

              _m_ui->test_description->textEditor()->setHtml(
                    RecordTextEdit::toHtmlWithParametersValues<ExecutionTestParameter>(
                      _m_execution_test->inheritedParameters(),
                      _m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION)));
            }
        }

      if (_m_execution_action != NULL)
        {
          _m_ui->action_group_box->setVisible(true);
          tmp_result_id = _m_execution_action->getValueForKey(EXECUTIONS_ACTIONS_TABLE_RESULT_ID);
          //tmp_modifiable = (is_empty_string(_m_execution_action->getIdentifier()) || is_empty_string(tmp_result_id));
          _m_ui->action_group_box->setTitle(tr("Action %1 sur %2").arg(QString::number(_m_execution_test->actions().indexOf(_m_execution_action) + 1)).arg(QString::number(_m_execution_test->actions().count())));

          if (is_empty_string(tmp_result_id) == FALSE)
            {
              if (compare_values(_m_execution_action->getValueForKey(EXECUTIONS_ACTIONS_TABLE_RESULT_ID), EXECUTION_ACTION_VALIDATED) == 0)
                {
                  tmp_image_status_url = QString::fromUtf8(":/images/ok.png");
                  tmp_image_status_label = tr("Ok");
                }
              else if (compare_values(_m_execution_action->getValueForKey(EXECUTIONS_ACTIONS_TABLE_RESULT_ID), EXECUTION_ACTION_INVALIDATED) == 0)
                {
                  tmp_image_status_url = QString::fromUtf8(":/images/ko.png");
                  tmp_image_status_label = tr("Ko");
                }
            }

          tmp_test_image_status = QPixmap(tmp_image_status_url);
          _m_ui->test_image_status->setPixmap(tmp_test_image_status);
          _m_ui->test_label_status->setText(tmp_image_status_label);

          _m_ui->comments->textEditor()->setHtml(_m_execution_action->getValueForKey(EXECUTIONS_ACTIONS_TABLE_COMMENTS));

          if (_m_execution_action->action() != NULL)
            {
              tmp_action = _m_execution_action->action();

              // Appliquer les paramètres d'exécutions pour le champ description de l'action
              tmp_parameters_list = RecordTextEdit::parametersList(tmp_action->getValueForKey(ACTIONS_TABLE_DESCRIPTION));
              if (tmp_parameters_list.isEmpty())
                {
                  _m_ui->action_description->textEditor()->setHtml(tmp_action->getValueForKey(ACTIONS_TABLE_DESCRIPTION));
                }
              else
                {
                  _m_current_parameters.append(tmp_parameters_list);
                  synchronizeExecutionsTestsParameters(tmp_parameters_list);

                  _m_ui->action_description->textEditor()->setHtml(
                        RecordTextEdit::toHtmlWithParametersValues<ExecutionTestParameter>(
                          _m_execution_test->inheritedParameters(),
                          tmp_action->getValueForKey(ACTIONS_TABLE_DESCRIPTION)));
                }
              loadTextEditAttachments(_m_ui->action_description->textEditor());

              // Appliquer les paramètres d'exécutions pour le champ résultat attendu de l'action
              tmp_parameters_list = RecordTextEdit::parametersList(tmp_action->getValueForKey(ACTIONS_TABLE_WAIT_RESULT));
              if (tmp_parameters_list.isEmpty())
                {
                  _m_ui->action_result->textEditor()->setHtml(tmp_action->getValueForKey(ACTIONS_TABLE_WAIT_RESULT));
                }
              else
                {
                  _m_current_parameters.append(tmp_parameters_list);
                  synchronizeExecutionsTestsParameters(tmp_parameters_list);

                  _m_ui->action_result->textEditor()->setHtml(
                        RecordTextEdit::toHtmlWithParametersValues<ExecutionTestParameter>(
                          _m_execution_test->inheritedParameters(),
                          tmp_action->getValueForKey(ACTIONS_TABLE_WAIT_RESULT)));
                }
              loadTextEditAttachments(_m_ui->action_result->textEditor());

            }
          else
            tmp_original_action_modifiable = false;

        }
      else
        {
          _m_ui->action_group_box->setVisible(false);
          tmp_original_action_modifiable = false;
        }

      _m_ui->save_comments->setVisible(tmp_modifiable);
      _m_ui->save_test_description->setVisible(tmp_modifiable);
      _m_ui->reinit_parameters_button->setVisible(tmp_modifiable);
      _m_ui->ok_button->setVisible(tmp_modifiable);
      _m_ui->ko_button->setVisible(tmp_modifiable);
      _m_ui->bypass_button->setVisible(tmp_modifiable);
      _m_ui->modify_button->setVisible(tmp_original_action_modifiable);
      _m_ui->comments->toolBar()->setVisible(tmp_modifiable);
      _m_ui->comments->textEditor()->setReadOnly(!tmp_modifiable);

      _m_ui->next_button->setEnabled(true);
      _m_ui->previous_button->setEnabled(true);
      _m_ui->modify_test_button->setVisible(tmp_modifiable);
    }

  _m_ui->test_description->textEditor()->setReadOnly(true);
  _m_ui->test_description->toolBar()->hide();

  _m_ui->action_description->textEditor()->setReadOnly(true);
  _m_ui->action_description->toolBar()->hide();

  _m_ui->action_result->textEditor()->setReadOnly(true);
  _m_ui->action_result->toolBar()->hide();

  _m_ui->reinit_parameters_button->setEnabled(!_m_current_parameters.isEmpty());

  initManageBugsButtonLabel();
}



void Form_Execution_Test::setExecutionTestData(const char *in_result_id)
{
  if (_m_execution_test != NULL)
    {
      _m_execution_test->setValueForKey(NOW, EXECUTIONS_TESTS_TABLE_EXECUTION_DATE);
      _m_execution_test->setValueForKey(in_result_id, EXECUTIONS_TESTS_TABLE_RESULT_ID);
    }
}


void Form_Execution_Test::setExecutionActionData(const char *in_result_id)
{
  if (_m_execution_action != NULL)
    {
      _m_execution_action->setValueForKey(in_result_id, EXECUTIONS_ACTIONS_TABLE_RESULT_ID);
      _m_execution_action->setValueForKey(_m_ui->comments->textEditor()->toHtml().toStdString().c_str(), EXECUTIONS_ACTIONS_TABLE_COMMENTS);

      if (_m_execution_test != NULL)
        {
          if(_m_execution_test->executionBypassedRate() == 1.0)
            _m_execution_test->setValueForKey(EXECUTION_TEST_BYPASSED, EXECUTIONS_TESTS_TABLE_RESULT_ID);
          else if(_m_execution_test->executionValidatedRate() == 1.0)
            _m_execution_test->setValueForKey(EXECUTION_TEST_VALIDATED, EXECUTIONS_TESTS_TABLE_RESULT_ID);
          else if(_m_execution_test->executionInValidatedRate() > 0.0)
            _m_execution_test->setValueForKey(EXECUTION_TEST_INVALIDATED, EXECUTIONS_TESTS_TABLE_RESULT_ID);
          else if(_m_execution_test->executionCoverageRate() < 1.0)
            _m_execution_test->setValueForKey(EXECUTION_TEST_INCOMPLETED, EXECUTIONS_TESTS_TABLE_RESULT_ID);
        }

      saveOriginalAction();
    }
}


void Form_Execution_Test::validateResult()
{
  if (_m_execution_action != NULL)
    setExecutionActionData(EXECUTION_ACTION_VALIDATED);
  else if (_m_execution_test != NULL)
    setExecutionTestData(EXECUTION_TEST_VALIDATED);

  emit resultValidated();
}


void Form_Execution_Test::saveComments()
{
  if (_m_execution_action != NULL)
    {
      _m_execution_action->setValueForKey(_m_ui->comments->textEditor()->toHtml().toStdString().c_str(), EXECUTIONS_ACTIONS_TABLE_COMMENTS);
      saveOriginalAction();
      initLayout();
    }
}



void Form_Execution_Test::saveTestDescription()
{
  int tmp_save_result = NOERR;
  if (_m_test_content != NULL)
    {
      _m_test_content->setValueForKey(_m_ui->test_description->textEditor()->toHtml().toStdString().c_str(), TESTS_CONTENTS_TABLE_DESCRIPTION);
      tmp_save_result = _m_test_content->saveRecord();
      if (tmp_save_result != NOERR)
        {
          QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_save_result));
        }

      initLayout();
    }
}

void Form_Execution_Test::inValidateResult()
{
  if (_m_execution_action != NULL)
    setExecutionActionData(EXECUTION_ACTION_INVALIDATED);
  else if (_m_execution_test != NULL)
    setExecutionTestData(EXECUTION_TEST_INVALIDATED);

  emit resultInValidated();
}


void Form_Execution_Test::bypass()
{
  if (_m_execution_action != NULL)
    setExecutionActionData(EXECUTION_ACTION_BYPASSED);
  else if (_m_execution_test != NULL)
    setExecutionTestData(EXECUTION_TEST_BYPASSED);

  emit bypassed();
}


void Form_Execution_Test::selectNext()
{
  emit nextSelected();
}


void Form_Execution_Test::selectPrevious()
{
  emit previousSelected();
}


void Form_Execution_Test::manageBugs()
{
  Form_Execution_Bugs		*tmp_form_bug = new Form_Execution_Bugs(_m_execution_test, _m_execution_action, this);

  connect(tmp_form_bug, SIGNAL(accepted()), this, SLOT(initManageBugsButtonLabel()));

  tmp_form_bug->show();
}


void Form_Execution_Test::initManageBugsButtonLabel()
{
  char				***tmp_results = NULL;
  unsigned long			tmp_rows_count = 0, tmp_columns_count = 0;
  net_session				*tmp_session = Session::instance()->getClientSession();

  _m_ui->manage_bugs_button->setText(tr("Anomalies"));

  if (_m_execution_action != NULL)
    sprintf(tmp_session->m_last_query, "SELECT count(*) FROM %s WHERE %s=%s;", BUGS_TABLE_SIG, BUGS_TABLE_EXECUTION_ACTION_ID, _m_execution_action->getIdentifier());
  else if (_m_execution_test != NULL)
    sprintf(tmp_session->m_last_query, "SELECT count(*) FROM %s WHERE %s=%s;", BUGS_TABLE_SIG, BUGS_TABLE_EXECUTION_TEST_ID, _m_execution_test->getIdentifier());
  else
    return;

  tmp_results = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
  if (tmp_results != NULL)
    {
      if (tmp_rows_count > 0 && tmp_columns_count > 0 && compare_values(tmp_results[0][0], "0") != 0)
        {
          _m_ui->manage_bugs_button->setText(tr("Anomalies (%1)").arg(tmp_results[0][0]));
        }

      cl_free_rows_columns_array(&tmp_results, tmp_rows_count, tmp_columns_count);
    }
}


void Form_Execution_Test::modifyAction()
{
  Action  *tmp_action = _m_execution_action->action();

  if (tmp_action != NULL)
    {
      _m_ui->action_description->textEditor()->setReadOnly(false);
      _m_ui->action_description->toolBar()->show();

      _m_ui->action_result->textEditor()->setReadOnly(false);
      _m_ui->action_result->toolBar()->show();

      _m_ui->action_description->textEditor()->setHtml(tmp_action->getValueForKey(ACTIONS_TABLE_DESCRIPTION));
      _m_ui->action_result->textEditor()->setHtml(tmp_action->getValueForKey(ACTIONS_TABLE_WAIT_RESULT));
    }
}


void Form_Execution_Test::modifyTest()
{
  if (_m_test_content != NULL)
    {
      _m_ui->test_description->textEditor()->setReadOnly(false);
      _m_ui->test_description->toolBar()->show();

      _m_ui->test_description->textEditor()->setHtml(_m_test_content->getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION));
    }
}


void Form_Execution_Test::saveOriginalAction()
{
  Action  *tmp_action = NULL;
  int	    tmp_save_result = NOERR;

  if (_m_execution_action != NULL)
    {
      tmp_action = _m_execution_action->action();
      if (tmp_action != NULL && _m_ui->action_description->textEditor()->isReadOnly() == false)
        {
          tmp_action->setValueForKey(_m_ui->action_description->textEditor()->toHtml().toStdString().c_str(), ACTIONS_TABLE_DESCRIPTION);
          tmp_action->setValueForKey(_m_ui->action_result->textEditor()->toHtml().toStdString().c_str(), ACTIONS_TABLE_WAIT_RESULT);
          tmp_save_result = tmp_action->saveRecord();
          if (tmp_save_result != NOERR)
            {
              QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_save_result));
            }
        }
    }
}



void Form_Execution_Test::synchronizeExecutionsTestsParameters(QList<QString> in_parameters_list)
{
  ExecutionTestParameter	    *tmp_test_param = NULL;
  const char			    *tmp_campaign_param_value = NULL;
  ExecutionTest			*tmp_parent_test = NULL;

  foreach(QString tmp_param_name, in_parameters_list)
    {
      tmp_test_param = Parameter::parameterForParamName<ExecutionTestParameter>(_m_execution_test->parameters(), tmp_param_name.toStdString().c_str());
      if (tmp_test_param == NULL || is_empty_string(tmp_test_param->getValueForKey(EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_VALUE)))
        {
          tmp_parent_test = _m_execution_test;
          while (tmp_test_param == NULL && tmp_parent_test->parent() != NULL && tmp_parent_test->parent() != _m_execution_test->executionCampaign())
            {
              tmp_parent_test = (ExecutionTest*)tmp_parent_test->parent();
              tmp_test_param = Parameter::parameterForParamName<ExecutionTestParameter>(tmp_parent_test->parameters(), tmp_param_name.toStdString().c_str());
            }

          if (tmp_test_param == NULL || is_empty_string(tmp_test_param->getValueForKey(EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_VALUE)))
            {
              bool	tmp_ok = false;
              QString	tmp_param_value;

              tmp_campaign_param_value = Parameter::paramValueForParamName<ExecutionCampaignParameter>(tmp_parent_test->executionCampaign()->parametersList(), tmp_param_name.toStdString());
              tmp_param_value = QInputDialog::getText(this, tr("Paramètre d'exécution..."), tr("Veuillez saisir la valeur du paramètre ${%1} :").arg(tmp_param_name.toStdString().c_str()), QLineEdit::Normal, tmp_campaign_param_value, &tmp_ok);
              if (tmp_ok)
                {
                  // Ajout du paramètre d'execution
                  if (tmp_test_param == NULL)
                    {
                      tmp_test_param = new ExecutionTestParameter();
                      tmp_test_param->setValueForKey(tmp_param_name.toStdString().c_str(), EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_NAME);
                      tmp_test_param->setValueForKey(tmp_param_value.toStdString().c_str(), EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_VALUE);
                      tmp_parent_test->addExecutionParameter(tmp_test_param);
                    }
                  // Mise à jour du paramètre d'execution existant
                  else
                    {
                      tmp_test_param->setValueForKey(tmp_param_value.toStdString().c_str(), EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_VALUE);
                    }
                }
            }
        }
    }

}



void Form_Execution_Test::reinitExecutionsTestsParameters()
{
  ExecutionTestParameter 				*tmp_test_param = NULL;
  QList<ExecutionTestParameter*>		tmp_inherited_parameters = _m_execution_test->inheritedParameters();

  foreach(QString tmp_param_name, _m_current_parameters)
    {
      tmp_test_param = Parameter::parameterForParamName<ExecutionTestParameter>(tmp_inherited_parameters, tmp_param_name.toStdString().c_str());
      if (tmp_test_param != NULL)
        tmp_test_param->setValueForKey(NULL, EXECUTIONS_TESTS_PARAMETERS_TABLE_PARAMETER_VALUE);
    }

  initLayout();
}

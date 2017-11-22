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

#ifndef FORM_EXECUTION_CAMPAIGN_H
#define FORM_EXECUTION_CAMPAIGN_H

#include <QtGui/QDialog>
#include "gui/components/records_tree_model.h"
#include "gui/components/records_tree_view.h"
#include "campaign.h"
#include "executioncampaign.h"
#include "gui/components/test_action_attachments_manager.h"

#include "form_execution_test.h"

#include "objects/Trinome.h"

namespace Ui {
  class Form_Execution_Campaign;
}

class Form_Execution_Campaign :
    public QDialog,
    public TestActionAttachmentsManager
{
  Q_OBJECT
  Q_DISABLE_COPY(Form_Execution_Campaign)
public:
  explicit Form_Execution_Campaign(ExecutionCampaign *in_execution_campaign, QWidget *parent = 0);
  virtual ~Form_Execution_Campaign();

  void showTestInfosForTestWithId(const char *in_execution_test_id);

public slots:
  void invalidateAction();
  void addBug();
  void showTestInfos();
  void showNextActionInfos();
  void showPreviousActionInfos();
  bool saveExecution();
  void cancelExecution();
  void print();
  void saveAs();
  void parameterItemChanged(QTableWidgetItem *in_widget_item);
  void reloadFromCampaignDatas();
  void insertParameter();
  void removeSelectedParameter();
  void revisionChanged(QString);

signals:
  void executionCampaignSaved();

protected:
  void showTestInfosAtIndex(QModelIndex in_model_index);
  void showNextActionInfosForTestAtIndex(QModelIndex in_model_index);
  void showPreviousActionInfosForTestAtIndex(QModelIndex in_model_index);
  QTextDocument* generateDocument(bool include_image = true, QFileInfo file_info = QFileInfo());
  virtual void closeEvent(QCloseEvent *in_event);
  bool maybeClose();

private:
  Ui::Form_Execution_Campaign *_m_ui;

  ExecutionCampaign        *_m_execution_campaign;

  RecordsTreeView           *_m_tests_tree_view;
  RecordsTreeModel          *_m_tests_tree_model;

  Form_Execution_Test         *_m_execution_test_view;

  QTableWidget			*_m_parameters_table_widget;
  QAction					*insertParameterAction;
  QAction					*removeParameterAction;

  int							_m_current_action_index;
  bool						_m_index_changed_programmatically;

  QList<Trinome*>             _m_trinomes_array;

  QLineEdit		*_m_revision_edit;

  void setParameterAtIndex(ExecutionCampaignParameter *in_parameter, int in_index);

  QString executionTestToHtml(QTextDocument *in_doc, ExecutionTest *in_execution_test, QString suffix, int level = 0, QString images_folder = QString());
};

#endif // FORM_EXECUTION_CAMPAIGN_H

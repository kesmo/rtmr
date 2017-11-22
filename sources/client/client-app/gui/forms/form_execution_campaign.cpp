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

#include "form_execution_campaign.h"
#include "ui_Form_Execution_Campaign.h"
#include "session.h"
#include "executioncampaignparameter.h"
#include "projectgrant.h"
#include <QDialogButtonBox>
#include "imaging/Chart.h"
#include <QMessageBox>
#include "form_bug.h"
#include "gui/components/record_text_edit.h"
#include "executiontestparameter.h"


Form_Execution_Campaign::Form_Execution_Campaign(ExecutionCampaign *in_execution_campaign, QWidget *parent) : QDialog(parent), _m_ui(new Ui::Form_Execution_Campaign)
{
  QSplitter               *tmp_horizontal_splitter = NULL;
  QSplitter               *tmp_vertical_splitter = NULL;
  QStringList     		tmp_parameters_list_headers;
  QDialogButtonBox		*tmp_button_box = NULL;
  QWidget					*tmp_execution_campaign_widget = new QWidget(this);
  QWidget					*tmp_parameters_widget = new QWidget(this);
  QWidget					*tmp_left_widget = new QWidget(this);
  QLabel					*tmp_label = NULL;
  QStringList           tmp_headers;
  QGridLayout		*tmp_grid_layout = NULL;
  QHBoxLayout		*tmp_h_layout = NULL;

  setAttribute(Qt::WA_DeleteOnClose);

  _m_current_action_index = -1;
  _m_index_changed_programmatically = false;
  _m_execution_campaign = in_execution_campaign;

  if (_m_execution_campaign != NULL && is_empty_string(_m_execution_campaign->getIdentifier()))
    {
      _m_execution_campaign->saveRecord();
    }

  _m_ui->setupUi(this);

  _m_tests_tree_view = new RecordsTreeView;
  _m_tests_tree_view->setSelectionBehavior(QAbstractItemView::SelectRows);
  _m_tests_tree_view->setSelectionMode(QAbstractItemView::SingleSelection);
  _m_tests_tree_view->setDragEnabled(false);
  _m_tests_tree_view->setAcceptDrops(false);

  _m_tests_tree_model = new RecordsTreeModel(_m_execution_campaign, EXECUTIONS_TESTS_TABLE_SIG_ID);
  _m_tests_tree_model->setColumnsCount(5);
  _m_tests_tree_view->setModel(_m_tests_tree_model);
  _m_tests_tree_view->expandAll();

  tmp_headers << tr("Description") << tr("Avancement") << tr("Réussite") << tr("Echec") << tr("Non passé");
  QStandardItemModel *tmp_header_model = new QStandardItemModel();
  tmp_header_model->setHorizontalHeaderLabels(tmp_headers);
  _m_tests_tree_view->header()->setModel(tmp_header_model);
  _m_tests_tree_view->header()->resizeSection(0, 290);
  _m_tests_tree_view->header()->resizeSection(1, 90);
  _m_tests_tree_view->header()->resizeSection(2, 60);
  _m_tests_tree_view->header()->resizeSection(3, 60);
  _m_tests_tree_view->header()->resizeSection(4, 60);

  //_m_tests_tree_view->setMinimumWidth(700);
  _m_tests_tree_view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  _m_execution_test_view = new Form_Execution_Test(this);
  _m_execution_test_view->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  _m_parameters_table_widget = new QTableWidget(this);
  _m_parameters_table_widget->setSelectionMode(QAbstractItemView::SingleSelection);
  _m_parameters_table_widget->setColumnCount(2);
  _m_parameters_table_widget->verticalHeader()->setVisible(false);
  _m_parameters_table_widget->verticalHeader()->setDefaultSectionSize(18);
  _m_parameters_table_widget->horizontalHeader()->setVisible(true);
  _m_parameters_table_widget->horizontalHeader()->setMinimumSectionSize(50);
  tmp_parameters_list_headers << tr("Nom") << tr("Valeur");
  _m_parameters_table_widget->setHorizontalHeaderLabels(tmp_parameters_list_headers);
  _m_parameters_table_widget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
  _m_parameters_table_widget->sortByColumn(0, Qt::AscendingOrder);
  _m_parameters_table_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  connect(_m_parameters_table_widget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(parameterItemChanged(QTableWidgetItem*)));

  // Parametres
  _m_parameters_table_widget->setRowCount(0);
  foreach(ExecutionCampaignParameter *tmp_parameter, _m_execution_campaign->parametersList())
    {
      _m_parameters_table_widget->insertRow(_m_parameters_table_widget->rowCount());
      setParameterAtIndex(tmp_parameter, _m_parameters_table_widget->rowCount() - 1);
    }

  tmp_left_widget->setLayout(new QVBoxLayout);
  tmp_left_widget->layout()->setSpacing(0);
  tmp_left_widget->layout()->setMargin(0);

  tmp_h_layout = new QHBoxLayout();
  //tmp_h_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
  tmp_execution_campaign_widget->setLayout(tmp_h_layout);
  tmp_execution_campaign_widget->layout()->setSpacing(0);
  tmp_execution_campaign_widget->layout()->setContentsMargins(0, 0, 0, 10);
  tmp_label = new QLabel("<b><u>" + tr("Numéro de révision (build number)") + "</u></b> : ", this);
  tmp_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  _m_revision_edit = new QLineEdit(this);
  _m_revision_edit->setObjectName("revision");
  _m_revision_edit->setMaximumSize(100, 20);
  tmp_execution_campaign_widget->layout()->addWidget(tmp_label);
  tmp_execution_campaign_widget->layout()->addWidget(_m_revision_edit);
  tmp_execution_campaign_widget->layout()->addItem(new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Preferred));
  tmp_execution_campaign_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  tmp_grid_layout = new QGridLayout();
  //tmp_grid_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
  tmp_parameters_widget->setLayout(tmp_grid_layout);
  tmp_label = new QLabel(tr("Paramètres d'exécutions de la campagne"), this);
  tmp_label->setObjectName("parameter_dock_widget_title");
  tmp_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  tmp_parameters_widget->layout()->addWidget(tmp_label);
  tmp_parameters_widget->layout()->addWidget(_m_parameters_table_widget);
  tmp_parameters_widget->layout()->setSpacing(0);
  tmp_parameters_widget->layout()->setMargin(0);
  tmp_parameters_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

  tmp_vertical_splitter = new QSplitter(Qt::Vertical);
  tmp_vertical_splitter->addWidget(_m_tests_tree_view);
  tmp_vertical_splitter->addWidget(tmp_parameters_widget);
  tmp_vertical_splitter->setStretchFactor(0, 10);
  tmp_vertical_splitter->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

  tmp_left_widget->layout()->addWidget(tmp_execution_campaign_widget);
  tmp_left_widget->layout()->addWidget(tmp_vertical_splitter);

  tmp_horizontal_splitter = new QSplitter(Qt::Horizontal);
  tmp_horizontal_splitter->addWidget(tmp_left_widget);
  tmp_horizontal_splitter->addWidget(_m_execution_test_view);
  _m_ui->execution_campaign_widget->layout()->addWidget(tmp_horizontal_splitter);
  _m_ui->execution_campaign_widget->setObjectName("swidget_content");

  insertParameterAction = new QAction(QIcon(":/images/22x22/config_plus.png"), tr("&Nouveau paramètre"), this);
  insertParameterAction->setStatusTip(tr("Ajouter un nouveau paramètre"));
  connect(insertParameterAction, SIGNAL(triggered()), this, SLOT(insertParameter()));

  removeParameterAction = new QAction(QIcon(":/images/22x22/config_minus.png"), tr("&Supprimer le paramètre sélectionné"), this);
  removeParameterAction->setStatusTip(tr("Supprimer le paramètre sélectionné"));
  connect(removeParameterAction, SIGNAL(triggered()), this, SLOT(removeSelectedParameter()));

  tmp_button_box = _m_ui->buttonBox;
  if (_m_execution_campaign != NULL
      && _m_execution_campaign->campaign() != NULL
      && _m_execution_campaign->campaign()->projectVersion() != NULL
      && _m_execution_campaign->campaign()->projectVersion()->project() != NULL
      && _m_execution_campaign->campaign()->projectVersion()->project()->projectGrants() != NULL
      && compare_values(_m_execution_campaign->campaign()->projectVersion()->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC), PROJECT_GRANT_WRITE) != 0)
    {
      tmp_button_box->button(QDialogButtonBox::SaveAll)->hide();
      _m_ui->reload_button->hide();
      _m_revision_edit->setReadOnly(true);
      insertParameterAction->setEnabled(false);
      removeParameterAction->setEnabled(false);
      _m_parameters_table_widget->setEnabled(false);
    }

  connect(tmp_button_box, SIGNAL(accepted()), this, SLOT(saveExecution()));
  connect(tmp_button_box, SIGNAL(rejected()), this, SLOT(cancelExecution()));
  connect(_m_ui->print_button, SIGNAL(clicked()), this, SLOT(print()));
  connect(_m_ui->export_button, SIGNAL(clicked()), this, SLOT(saveAs()));
  connect(_m_ui->reload_button, SIGNAL(clicked()), this, SLOT(reloadFromCampaignDatas()));

  connect(_m_tests_tree_view->selectionModel(), SIGNAL(currentChanged( const QModelIndex &, const QModelIndex &)), this, SLOT(showTestInfos()));
  connect(_m_execution_test_view, SIGNAL(resultValidated()), this, SLOT(showNextActionInfos()));
  connect(_m_execution_test_view, SIGNAL(resultInValidated()), this, SLOT(invalidateAction()));
  connect(_m_execution_test_view, SIGNAL(bypassed()), this, SLOT(showNextActionInfos()));
  connect(_m_execution_test_view, SIGNAL(nextSelected()), this, SLOT(showNextActionInfos()));
  connect(_m_execution_test_view, SIGNAL(previousSelected()), this, SLOT(showPreviousActionInfos()));

  setWindowTitle(tr("Exécution de la campagne %1").arg(_m_execution_campaign->campaign()->getValueForKey(CAMPAIGNS_TABLE_SHORT_NAME)));

  _m_parameters_table_widget->addAction(insertParameterAction);
  _m_parameters_table_widget->addAction(removeParameterAction);
  _m_parameters_table_widget->setContextMenuPolicy(Qt::ActionsContextMenu);

  _m_revision_edit->setText(_m_execution_campaign->getValueForKey(EXECUTIONS_CAMPAIGNS_TABLE_REVISION));
  connect(_m_revision_edit, SIGNAL(textChanged(QString)), this, SLOT(revisionChanged(QString)));

  _m_tests_tree_view->setFocus();
}

Form_Execution_Campaign::~Form_Execution_Campaign()
{
  delete _m_tests_tree_view;
  delete _m_execution_test_view;
  delete _m_tests_tree_model;
  delete _m_parameters_table_widget;
  delete _m_ui;

  qDeleteAll(_m_trinomes_array);
  _m_trinomes_array.clear();
}

void Form_Execution_Campaign::setParameterAtIndex(ExecutionCampaignParameter *in_parameter, int in_index)
{
  QTableWidgetItem        *tmp_widget_item = NULL;

  tmp_widget_item = new QTableWidgetItem;
  tmp_widget_item->setText(in_parameter->getValueForKey(EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_NAME));
  tmp_widget_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_parameter));
  _m_parameters_table_widget->setItem(in_index, 0, tmp_widget_item);

  tmp_widget_item = new QTableWidgetItem;
  tmp_widget_item->setText(in_parameter->getValueForKey(EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_VALUE));
  tmp_widget_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_parameter));
  _m_parameters_table_widget->setItem(in_index, 1, tmp_widget_item);
}


/**
 * Modification d'un parametre
 */
void Form_Execution_Campaign::parameterItemChanged(QTableWidgetItem *in_widget_item)
{
  ExecutionCampaignParameter		*tmp_parameter = NULL;
  std::string				tmp_param_column_str;
  const char				*tmp_param_column = NULL;

  if (in_widget_item != NULL)
    {
      tmp_parameter = (ExecutionCampaignParameter*)in_widget_item->data(Qt::UserRole).value<void*>();
      if (tmp_parameter != NULL)
        {
          tmp_param_column_str = in_widget_item->text().toStdString();
          tmp_param_column = tmp_param_column_str.c_str();
          if (is_empty_string(tmp_param_column) == FALSE)
            {
              if (in_widget_item->column() == 0)
                {
                  tmp_parameter->setValueForKey(tmp_param_column, EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_NAME);
                }
              else if (in_widget_item->column() == 1)
                {
                  tmp_parameter->setValueForKey(tmp_param_column, EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_PARAMETER_VALUE);
                }
            }
        }
    }
}


/**
  Afficher les informations du test selectionne
**/
void Form_Execution_Campaign::showTestInfos()
{
  showTestInfosAtIndex(_m_tests_tree_view->selectionModel()->currentIndex());
}

/**
  Afficher les informations du test selectionne
**/
void Form_Execution_Campaign::showTestInfosAtIndex(QModelIndex in_model_index)
{
  ExecutionTest        *tmp_selected_execution_test = NULL;

  if (!_m_index_changed_programmatically)
    _m_current_action_index = -1;

  if (in_model_index.isValid())
    {
      _m_tests_tree_view->selectionModel()->select(in_model_index, QItemSelectionModel::ClearAndSelect);

      tmp_selected_execution_test = (ExecutionTest*)(_m_tests_tree_model->getItem(in_model_index));
      if (tmp_selected_execution_test != NULL && _m_execution_test_view != NULL)
        {
          if (tmp_selected_execution_test->actions().count() == 0)
            {
              _m_execution_test_view->loadTest(tmp_selected_execution_test, NULL);
              _m_execution_test_view->show();
            }
          else if (_m_current_action_index >= 0 && _m_current_action_index < tmp_selected_execution_test->actions().count())
            {
              _m_execution_test_view->loadTest(tmp_selected_execution_test, tmp_selected_execution_test->actionAtIndex(_m_current_action_index));
              _m_execution_test_view->show();
            }
          else
            showNextActionInfos();
        }

    }
  else if (_m_execution_test_view != NULL)
    _m_execution_test_view->hide();

  _m_index_changed_programmatically = false;
}


/**
  Afficher les informations du test passé en paramètre
**/
void Form_Execution_Campaign::showTestInfosForTestWithId(const char *in_execution_test_id)
{
  QModelIndexList	    tmp_model_indexes = _m_tests_tree_model->modelIndexesForItemWithValueForKey(in_execution_test_id, EXECUTIONS_TESTS_TABLE_EXECUTION_TEST_ID);

  if(!tmp_model_indexes.isEmpty())
    {
      _m_current_action_index = 0;
      _m_index_changed_programmatically = true;
      showTestInfosAtIndex(tmp_model_indexes[0]);
    }
}


/**
  Afficher les informations de l'action suivante dans le test courant
**/
void Form_Execution_Campaign::showNextActionInfos()
{
  showNextActionInfosForTestAtIndex(_m_tests_tree_view->selectionModel()->currentIndex());
}


/**
  Afficher les informations de l'action précédente dans le test courant
**/
void Form_Execution_Campaign::showPreviousActionInfos()
{
  showPreviousActionInfosForTestAtIndex(_m_tests_tree_view->selectionModel()->currentIndex());
}


/**
  Afficher les informations de l'action précédente dans le test à l'index passé en paramètre
**/
void Form_Execution_Campaign::showPreviousActionInfosForTestAtIndex(QModelIndex in_model_index)
{
  QModelIndex             tmp_next_index;
  ExecutionTest	    *tmp_selected_execution_test = NULL;

  if (in_model_index.isValid())
    {
      tmp_selected_execution_test = (ExecutionTest*)(_m_tests_tree_model->getItem(in_model_index));
      if (tmp_selected_execution_test != NULL && _m_execution_test_view != NULL)
        {
          _m_current_action_index--;
          if (_m_current_action_index >= 0 && _m_current_action_index < tmp_selected_execution_test->actions().count())
            {
              _m_execution_test_view->loadTest(tmp_selected_execution_test, tmp_selected_execution_test->actionAtIndex(_m_current_action_index));
              _m_execution_test_view->show();
            }
          else
            {
              tmp_next_index = _m_tests_tree_view->indexAbove(in_model_index);
              if (tmp_next_index.isValid())
                {
                  tmp_selected_execution_test = (ExecutionTest*)(_m_tests_tree_model->getItem(tmp_next_index));
                  if (tmp_selected_execution_test != NULL)
                    _m_current_action_index = tmp_selected_execution_test->actions().count() - 1;

                  _m_index_changed_programmatically = true;
                  _m_tests_tree_view->setCurrentIndex(tmp_next_index);
                }
            }
        }

    }
  else if (_m_execution_test_view != NULL)
    _m_execution_test_view->hide();
}

/**
  Afficher les informations de l'action suivante dans le test à l'index passé en paramètre
**/
void Form_Execution_Campaign::showNextActionInfosForTestAtIndex(QModelIndex in_model_index)
{
  QModelIndex             tmp_next_index;
  ExecutionTest        *tmp_selected_execution_test = NULL;

  if (in_model_index.isValid())
    {
      tmp_selected_execution_test = (ExecutionTest*)(_m_tests_tree_model->getItem(in_model_index));
      if (tmp_selected_execution_test != NULL && _m_execution_test_view != NULL)
        {
          _m_current_action_index++;
          if (_m_current_action_index >= 0 && _m_current_action_index < tmp_selected_execution_test->actions().count())
            {
              _m_execution_test_view->loadTest(tmp_selected_execution_test, tmp_selected_execution_test->actionAtIndex(_m_current_action_index));
              _m_execution_test_view->show();
            }
          else
            {
              if (_m_tests_tree_model->rowCount(in_model_index) > 0)
                {
                  tmp_next_index = in_model_index.child(0, 0);
                }
              else
                {
                  tmp_next_index = _m_tests_tree_view->indexBelow(in_model_index);
                }

              if (tmp_next_index.isValid())
                {
                  _m_tests_tree_view->setCurrentIndex(tmp_next_index);
                }
            }
        }

    }
  else if (_m_execution_test_view != NULL)
    _m_execution_test_view->hide();
}



bool Form_Execution_Campaign::saveExecution()
{
  int     tmp_result = NOERR;

  tmp_result = cl_transaction_start(Session::instance()->getClientSession());
  if (tmp_result == NOERR)
    {
      _m_execution_campaign->setValueForKey(_m_revision_edit->text().toStdString().c_str(), EXECUTIONS_CAMPAIGNS_TABLE_REVISION);
      _m_execution_campaign->setValueForKey(CLIENT_MACRO_NOW, EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_DATE);

      tmp_result = _m_execution_campaign->saveRecord();
      if (tmp_result == NOERR)
        tmp_result = cl_transaction_commit(Session::instance()->getClientSession());
    }
  if (tmp_result == NOERR)
    {
      return true;
    }
  else
    {
      cl_transaction_rollback(Session::instance()->getClientSession());
      QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_result));
      return false;
    }
}


void Form_Execution_Campaign::cancelExecution()
{
  close();
}

void Form_Execution_Campaign::print()
{
  QPrinter printer(QPrinter::HighResolution);
  QPageSetupDialog printer_setup(&printer);

  if (printer_setup.exec() == QDialog::Accepted)
    {
      QString fileName = QFileDialog::getSaveFileName(this, "Export PDF", QString(), "*.pdf");
      if (!fileName.isEmpty())
        {
          if (QFileInfo(fileName).suffix().isEmpty())
            fileName.append(".pdf");

          printer.setOutputFormat(QPrinter::PdfFormat);
          printer.setOutputFileName(fileName);
          generateDocument()->print(&printer);
        }
    }
}


void Form_Execution_Campaign::saveAs()
{
  QString fileName = QFileDialog::getSaveFileName(this, "Export html", QString(), "*.html");
  if (!fileName.isEmpty())
    {
      QFile tmp_file(fileName);
      if (tmp_file.open(QIODevice::WriteOnly))
        {
          tmp_file.write(generateDocument(false, QFileInfo(tmp_file))->toHtml("utf-8").toAscii());
          tmp_file.close();
        }
      else
        {
          QMessageBox::critical(this, tr("Fichier non créé"), tr("L'ouverture du fichier en écriture est impossible (%1).").arg(tmp_file.errorString()));
        }
    }
}


QTextDocument* Form_Execution_Campaign::generateDocument(bool include_image, QFileInfo file_info)
{
  QString			tmp_doc_content = QString("<html><head></head><body>");
  QTextDocument               *tmp_doc = new QTextDocument();
  int				tmp_index = 0;
  QString			tmp_coverage_rate, validated_rate, invalidated_rate, bypassed_rate;

  Chart                       *tmp_chart = new Chart(800, 600);
  QDateTime                   tmp_date_time;
  vector<QColor>              tmp_colors;

  QTextDocument		tmp_description_doc;

  QString         tmp_images_folder_absolute_path;
  QString         tmp_images_folder_name;

  qDeleteAll(_m_trinomes_array);
  _m_trinomes_array.clear();

  tmp_date_time = QDateTime::fromString(QString(_m_execution_campaign->getValueForKey(EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_DATE)).left(16), "yyyy-MM-dd hh:mm");

  tmp_chart->setRenderingForQuality();
  tmp_chart->m_show_legend = true;
  tmp_chart->setType(Chart::Camembert);

  _m_trinomes_array.append(new Trinome(new String(tr("Ko")), new Double(_m_execution_campaign->executionInValidatedRate())));
  _m_trinomes_array.append(new Trinome(new String(tr("Non passé")), new Double(_m_execution_campaign->executionBypassedRate())));
  _m_trinomes_array.append(new Trinome(new String(tr("Ok")), new Double(_m_execution_campaign->executionValidatedRate())));

  tmp_chart->setTitle(tr("Graphe de couverture"));
  tmp_chart->setData(TrinomeArray(_m_trinomes_array));
  tmp_colors.push_back(ExecutionTest::koColor());
  tmp_colors.push_back(ExecutionTest::byPassedColor());
  tmp_colors.push_back(ExecutionTest::okColor());
  tmp_chart->setGraphColors(tmp_colors);
  tmp_chart->draw();

  if (include_image)
    {
      tmp_doc->addResource(QTextDocument::ImageResource, QUrl("images://graphe.png"), QVariant(tmp_chart->getImageData()));
    }
  else
    {
      if (file_info.isFile())
        {
          tmp_images_folder_absolute_path = file_info.canonicalFilePath()+"_images";
          QDir tmp_images_dir(tmp_images_folder_absolute_path);
          tmp_images_folder_name = tmp_images_dir.dirName();
          if (!tmp_images_dir.exists())
            file_info.absoluteDir().mkdir(tmp_images_folder_name);

          tmp_chart->saveAs(tmp_images_folder_absolute_path+"/graphe.png");
        }
    }

  tmp_coverage_rate.setNum(_m_execution_campaign->executionCoverageRate() * 100, 'f', 0);
  validated_rate.setNum(_m_execution_campaign->executionValidatedRate() * 100, 'f', 0);
  invalidated_rate.setNum(_m_execution_campaign->executionInValidatedRate() * 100, 'f', 0);
  bypassed_rate.setNum(_m_execution_campaign->executionBypassedRate() * 100, 'f', 0);

  tmp_doc_content += "<h1>" + QString(_m_execution_campaign->campaign()->getValueForKey(CAMPAIGNS_TABLE_SHORT_NAME)) + "</h1>";
  if (is_empty_string(_m_execution_campaign->getValueForKey(EXECUTIONS_CAMPAIGNS_TABLE_REVISION)) == FALSE)
    tmp_doc_content += tr(" Révision ") + QString(_m_execution_campaign->getValueForKey(EXECUTIONS_CAMPAIGNS_TABLE_REVISION)) + "<br>";

  tmp_doc_content += "<u>"+tr("Exécution du")+"</u> : " + tmp_date_time.toString("dd/MM/yyyy-hh:mm");
  tmp_doc_content += "&nbsp;<u>" + tr("Couverture") + "</u> : " + tmp_coverage_rate + " %";
  tmp_doc_content += "&nbsp;<u>" + tr("Réussite") + "</u> : " + validated_rate + " %";
  tmp_doc_content += "&nbsp;<u>" + tr("Echec") + "</u> : " + invalidated_rate + " %";
  tmp_doc_content += "&nbsp;<u>" + tr("Non passé") + "</u> : " + bypassed_rate + " %<br>";
  tmp_description_doc.setHtml(_m_execution_campaign->campaign()->getValueForKey(CAMPAIGNS_TABLE_DESCRIPTION));
  if (!tmp_description_doc.toPlainText().isEmpty())
    tmp_doc_content += "<u>" + tr("Description") + "</u> : " + QString(_m_execution_campaign->campaign()->getValueForKey(CAMPAIGNS_TABLE_DESCRIPTION)) + "<br/>";

  if (include_image)
    tmp_doc_content += "<p align=\"center\"><img border=\"1\" src=\"images://graphe.png\" width=\"480\" height=\"360\"/></p>";
  else
    tmp_doc_content += "<p align=\"center\"><img border=\"1\" src=\""+ tmp_images_folder_name+"/graphe.png" + "\" width=\"480\" height=\"360\"/></p>";

  tmp_doc_content += "<u>" + tr("Liste des scénarios et cas de tests") + "</u> :";
  tmp_doc_content += "<table border=\"1\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\"><tr><th>" + tr("Nom") + "</th><th>" + tr("Couverture") + "</th><th>" + tr("Réussite") + "</th><th>" + tr("Echec") + "</th><th>" + tr("Non passé") + "</th></tr>";

  foreach(ExecutionTest *tmp_execution_test, _m_execution_campaign->testsList())
    {
      tmp_index++;
      tmp_doc_content += QString(tmp_execution_test->toFragmentHtml(QString::number(tmp_index) + "."));
    }
  tmp_doc_content += "</table>";

  tmp_index = 0;
  foreach(ExecutionTest *tmp_execution_test, _m_execution_campaign->testsList())
    {
      tmp_index++;
      tmp_doc_content += executionTestToHtml(tmp_doc, tmp_execution_test, QString::number(tmp_index) + ".", 0, tmp_images_folder_absolute_path);
    }

  tmp_doc_content += "</body></html>";

  tmp_doc->setHtml(tmp_doc_content);


  return tmp_doc;
}


void Form_Execution_Campaign::reloadFromCampaignDatas()
{
  QMessageBox			*tmp_msg_box = new QMessageBox(this);
  QPushButton         *tmp_sync_campaign_button;
  QPushButton         *tmp_sync_test_button;
  QPushButton         *tmp_sync_params_button;
  QPushButton         *tmp_cancel_button;

  ExecutionTest        *tmp_selected_execution_test = NULL;
  QModelIndex			tmp_selected_index;

  TestCampaign		*tmp_test_campaign = NULL;

  tmp_msg_box->setIcon(QMessageBox::Question);
  tmp_msg_box->setWindowTitle(tr("Confirmation..."));
  tmp_msg_box->setText(tr("Cette action permet de synchroniser les tests en cours d'exécutions avec les tests éventuellement mis à jour au niveau du projet ou de la campagne" \
                          " ainsi que les paramètres d'exécutions avec les paramètres du projet.") + "<br><br>" + \
                       tr("Que voulez-vous synchroniser ?") + "<ul><li>" + tr("tout : les tests en cours d'exécution et les paramètres") + "</il>" \
                       "<li>" + tr("le test sélectionné seulement") + "</li>" \
                       "<li>" + tr("les paramètres seulement") + "</li>");
  tmp_sync_campaign_button = tmp_msg_box->addButton(tr("Synchroniser tout"), QMessageBox::NoRole);
  tmp_sync_test_button = tmp_msg_box->addButton(tr("Synchroniser le test sélectionné"), QMessageBox::NoRole);
  tmp_sync_params_button = tmp_msg_box->addButton(tr("Synchroniser les paramètres"), QMessageBox::NoRole);
  tmp_cancel_button = tmp_msg_box->addButton(tr("Annuler"), QMessageBox::RejectRole);

  tmp_msg_box->exec();
  if (tmp_msg_box->clickedButton() == tmp_sync_campaign_button)
    {
      _m_tests_tree_view->reset();
      _m_execution_campaign->synchronizeFromCampaignDatas();
    }
  else if (tmp_msg_box->clickedButton() == tmp_sync_test_button)
    {
      tmp_selected_index = _m_tests_tree_view->selectionModel()->currentIndex();
      if (tmp_selected_index.isValid())
        {
          tmp_selected_execution_test = (ExecutionTest*)(_m_tests_tree_model->getItem(tmp_selected_index));
          if (tmp_selected_execution_test != NULL && _m_execution_test_view != NULL)
            {
              tmp_test_campaign = (TestCampaign*)_m_execution_campaign->campaign()->findItemWithValueForKey(tmp_selected_execution_test->getValueForKey(EXECUTIONS_TESTS_TABLE_TEST_ID), TESTS_CAMPAIGNS_TABLE_TEST_ID, TESTS_CAMPAIGNS_TABLE_SIG_ID);
              if (tmp_test_campaign != NULL)
                {
                  tmp_selected_execution_test->synchronizeFromTestDatas(tmp_test_campaign->childs());
                  showTestInfos();
                }
            }
        }
    }
  else if  (tmp_msg_box->clickedButton() == tmp_sync_params_button)
    {
      _m_execution_campaign->synchronizeFromProjectParameters();
    }

  // Parametres
  _m_parameters_table_widget->setRowCount(0);
  foreach(ExecutionCampaignParameter *tmp_parameter, _m_execution_campaign->parametersList())
    {
      _m_parameters_table_widget->insertRow(_m_parameters_table_widget->rowCount());
      setParameterAtIndex(tmp_parameter, _m_parameters_table_widget->rowCount() - 1);
    }

  _m_tests_tree_view->collapseAll();
  _m_tests_tree_view->expandAll();
  _m_tests_tree_view->update();
  _m_tests_tree_view->setFocus();
}


void Form_Execution_Campaign::insertParameter()
{
  ExecutionCampaignParameter     *tmp_parameter = new ExecutionCampaignParameter();

  tmp_parameter->setValueForKey(_m_execution_campaign->getIdentifier(), EXECUTIONS_CAMPAIGNS_PARAMETERS_TABLE_EXECUTION_CAMPAIGN_ID);
  _m_parameters_table_widget->insertRow(0);
  _m_execution_campaign->addParameter(tmp_parameter);

  setParameterAtIndex(tmp_parameter, 0);
}


void Form_Execution_Campaign::removeSelectedParameter()
{
  QModelIndex 			tmp_select_row = _m_parameters_table_widget->selectionModel()->currentIndex();
  int         			tmp_row_index = 0;
  QTableWidgetItem			*tmp_item = NULL;
  ExecutionCampaignParameter		*tmp_parameter = NULL;

  if (tmp_select_row.isValid())
    {
      tmp_row_index = tmp_select_row.row();
      tmp_item = _m_parameters_table_widget->item(tmp_row_index, 0);
      if (tmp_item != NULL)
        {
          tmp_parameter = (ExecutionCampaignParameter*)tmp_item->data(Qt::UserRole).value<void*>();
          if (tmp_parameter != NULL)
            {
              if (tmp_parameter->deleteRecord() == NOERR)
                {
                  _m_parameters_table_widget->removeRow(tmp_row_index);
                  _m_execution_campaign->removeParameter(tmp_parameter);
                }
            }
        }
    }

}



void Form_Execution_Campaign::revisionChanged(QString revision)
{
  _m_execution_campaign->setValueForKey(revision.toStdString().c_str(), EXECUTIONS_CAMPAIGNS_TABLE_REVISION);
}




void Form_Execution_Campaign::invalidateAction()
{
  if (QMessageBox::question(
        this,
        tr("Confirmation..."),
        tr("Voulez-vous créer une anomalie ?"),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No) == QMessageBox::Yes)
    {
      addBug();
    }

  showNextActionInfos();
}



void Form_Execution_Campaign::addBug()
{
  Form_Bug			*tmp_form_bug = NULL;
  ExecutionTest       *tmp_selected_execution_test = NULL;

  tmp_selected_execution_test = (ExecutionTest*)(_m_tests_tree_model->getItem(_m_tests_tree_view->selectionModel()->currentIndex()));
  if (tmp_selected_execution_test != NULL && _m_execution_test_view != NULL)
    {
      if (tmp_selected_execution_test->actions().count() == 0)
        {
          tmp_form_bug = new Form_Bug(NULL, tmp_selected_execution_test, NULL, NULL, this);
          tmp_form_bug->show();
        }
      else if (_m_current_action_index >= 0 && _m_current_action_index < tmp_selected_execution_test->actions().count())
        {
          tmp_form_bug = new Form_Bug(NULL, tmp_selected_execution_test, tmp_selected_execution_test->actionAtIndex(_m_current_action_index), NULL, this);
          tmp_form_bug->show();
        }
    }
}



QString Form_Execution_Campaign::executionTestToHtml(QTextDocument *in_doc, ExecutionTest *in_execution_test, QString suffix, int level, QString images_folder)
{
  QString		tmp_html_content = QString();
  QString		tmp_html_action_content;
  int			tmp_index = 0;
  const char		*tmp_action_result = NULL;
  const char		*tmp_action_result_label = NULL;
  QString		tmp_coverage_rate = "", validated_rate = "", invalidated_rate = "", bypassed_rate = "";
  TestContent		*tmp_test_content = new TestContent();
  QTextDocument	tmp_doc;
  QList<TestContentFile*> tmp_files;

  if (in_execution_test != NULL && in_execution_test->projectTest() != NULL)
    {
      tmp_test_content->loadRecord(in_execution_test->projectTest()->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID));

      tmp_files = tmp_test_content->loadFiles();
      _m_files.append(tmp_files);

      float	tmp_test_execution_coverage = in_execution_test->executionCoverageRate() * 100;
      float	tmp_test_execution_validated = in_execution_test->executionValidatedRate() * 100;
      float	tmp_test_execution_invalidated = in_execution_test->executionInValidatedRate() * 100;
      float	tmp_test_execution_bypassed = in_execution_test->executionBypassedRate() * 100;

      if (tmp_test_execution_coverage > 0)
        tmp_coverage_rate = QString::number(tmp_test_execution_coverage, 'f', 0) + " %";

      if (tmp_test_execution_validated > 0)
        validated_rate = QString::number(tmp_test_execution_validated, 'f', 0) + " %";

      if (tmp_test_execution_invalidated > 0)
        invalidated_rate = QString::number(tmp_test_execution_invalidated, 'f', 0) + " %";

      if (tmp_test_execution_bypassed > 0)
        bypassed_rate = QString::number(tmp_test_execution_bypassed, 'f', 0) + " %";

      switch (level)
        {
        case 0:
          tmp_html_content += "<h2 style=\"page-break-before: always;\">" + suffix + " " + QString(in_execution_test->projectTest()->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + "</h2>";
          break;
        case 1:
          tmp_html_content += "<h3>" + suffix + " " + QString(in_execution_test->projectTest()->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + "</h3>";
          break;
        case 2:
          tmp_html_content += "<h4>" + suffix + " " + QString(in_execution_test->projectTest()->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + "</h4>";
          break;
        case 3:
          tmp_html_content += "<h5>" + suffix + " " + QString(in_execution_test->projectTest()->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + "</h5>";
          break;
        default:
          tmp_html_content += "<h6>" + suffix + " " + QString(in_execution_test->projectTest()->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + "</h6>";
          break;
        }

      tmp_html_content += "<u>"+TR_CUSTOM_MESSAGE("Couverture")+"</u> : " + tmp_coverage_rate;
      tmp_html_content += "&nbsp;<u>"+TR_CUSTOM_MESSAGE("Réussite")+"</u> : " + validated_rate;
      tmp_html_content += "&nbsp;<u>"+TR_CUSTOM_MESSAGE("Echec")+"</u> : " + invalidated_rate;
      tmp_html_content += "&nbsp;<u>"+TR_CUSTOM_MESSAGE("Non passé")+"</u> : " + bypassed_rate + "<br/>";
      tmp_doc.setHtml(tmp_test_content->getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION));
      if (!tmp_doc.toPlainText().isEmpty())
        tmp_html_content += "<u>"+TR_CUSTOM_MESSAGE("Description")+"</u> : " + RecordTextEdit::toHtmlWithParametersValues<ExecutionTestParameter>(
              in_execution_test->inheritedParameters(),
              QString(tmp_test_content->getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION))) + "<br/>";

      delete tmp_test_content;

      if (in_execution_test->actions().count() > 0)
        {
          tmp_html_content += "<u>"+TR_CUSTOM_MESSAGE("Liste des actions")+"</u><table border=\"1\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">";
          tmp_html_content += "<tr><th>"+TR_CUSTOM_MESSAGE("Description")+"</th>";
          tmp_html_content += "<th>"+TR_CUSTOM_MESSAGE("Résultat attendu")+"</th>";
          tmp_html_content += "<th>"+TR_CUSTOM_MESSAGE("Résultat")+"</th>";
          tmp_html_content += "<th>"+TR_CUSTOM_MESSAGE("Commentaires")+"</th></tr>";

          foreach(ExecutionAction *tmp_execution_action, in_execution_test->actions())
            {
              tmp_action_result = tmp_execution_action->getValueForKey(EXECUTIONS_ACTIONS_TABLE_RESULT_ID);
              tmp_action_result_label = Record::matchingValueInRecordsList(Session::instance()->actionsResults(),
                                                                           EXECUTIONS_ACTIONS_TABLE_RESULT_ID,
                                                                           tmp_action_result,
                                                                           ACTIONS_RESULTS_TABLE_DESCRIPTION);

              tmp_html_action_content = RecordTextEdit::toHtmlWithParametersValues<ExecutionTestParameter>(in_execution_test->inheritedParameters(), QString(tmp_execution_action->action()->getValueForKey(ACTIONS_TABLE_DESCRIPTION)));
              loadTextEditAttachments(tmp_html_action_content, in_doc, images_folder);
              tmp_html_content += "<tr><td>" + tmp_html_action_content + "</td>";

              tmp_html_action_content = RecordTextEdit::toHtmlWithParametersValues<ExecutionTestParameter>(in_execution_test->inheritedParameters(), QString(tmp_execution_action->action()->getValueForKey(ACTIONS_TABLE_WAIT_RESULT)));
              loadTextEditAttachments(tmp_html_action_content, in_doc, images_folder);
              tmp_html_content += "<td>" + tmp_html_action_content + "</td>";

              if (compare_values(tmp_action_result, EXECUTION_ACTION_VALIDATED) == 0)
                tmp_html_content += "<td>" + TR_CUSTOM_MESSAGE(tmp_action_result_label) + "</td>";
              else if (compare_values(tmp_action_result, EXECUTION_ACTION_INVALIDATED) == 0)
                tmp_html_content += "<td><span style=\"color:red\">" + TR_CUSTOM_MESSAGE(tmp_action_result_label) + "</span></td>";
              else
                tmp_html_content += "<td><span style=\"color:orange\">" + TR_CUSTOM_MESSAGE(tmp_action_result_label) + "</span></td>";

              tmp_html_content += "<td>" + QString(tmp_execution_action->getValueForKey(EXECUTIONS_ACTIONS_TABLE_COMMENTS)) + "</td></tr>";
            }
          tmp_html_content += "</table>";
        }

      tmp_html_content += "<br/>";

      foreach(ExecutionTest *tmp_execution_test, in_execution_test->childs())
        {
          tmp_index++;
          tmp_html_content += executionTestToHtml(in_doc, tmp_execution_test, suffix + QString::number(tmp_index) + ".", level + 1, images_folder);
        }
    }
  return tmp_html_content;
}


void Form_Execution_Campaign::closeEvent(QCloseEvent *in_event)
{
  if (maybeClose())
    in_event->accept();
  else
    in_event->ignore();
}


bool Form_Execution_Campaign::maybeClose()
{
  int		tmp_confirm_choice = 0;

  if (_m_execution_campaign != NULL)
    {
      if (_m_execution_campaign->hasChangedValues())
        {
          tmp_confirm_choice = QMessageBox::question(
                this,
                tr("Confirmation..."),
                tr("L'exécution de campagne a été modifiée. Voulez-vous enregistrer les modifications ?"),
                QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                QMessageBox::Cancel);

          if (tmp_confirm_choice == QMessageBox::Yes)
            return saveExecution();
          else if (tmp_confirm_choice == QMessageBox::Cancel)
            return false;
        }

      _m_execution_campaign->unlockRecord();
    }

  return true;
}


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

#include "session.h"
#include "form_campaign.h"
#include "ui_Form_Campaign.h"
#include "form_execution_campaign.h"
#include "testcontent.h"
#include "projectgrant.h"


/**
  Constructeur
**/
Form_Campaign::Form_Campaign(QWidget *parent) :
    AbstractProjectWidget(parent),
    _m_ui(new Ui::Form_Campaign),
    _m_tests_tree_model(NULL),
    _m_campaign(NULL),
    _m_original_campaign(NULL)

{
    QStringList           tmp_headers;

    setAttribute(Qt::WA_DeleteOnClose);

    _m_campaign = NULL;

    _m_ui->setupUi(this);
    _m_tests_tree_view = new RecordsTreeView;
    _m_tests_tree_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    _m_tests_tree_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _m_tests_tree_view->setDragEnabled(true);
    _m_tests_tree_view->setAcceptDrops(true);
    _m_tests_tree_view->setDropIndicatorShown(true);
    _m_tests_tree_view->setHeaderHidden(true);
    _m_tests_tree_view->setExpandsOnDoubleClick(false);

    _m_ui->tests_campaign_frame->layout()->addWidget(_m_tests_tree_view);
    _m_ui->campaign_description->addTextToolBar(RecordTextEditToolBar::Small);

    // Liste des executions
    tmp_headers << tr("Date") << tr("Révision") << tr("Couverture")  << tr("Réussite") << tr("Echec")<< tr("Non passé");
    _m_ui->executions_campaigns_list->setColumnCount(tmp_headers.count());
    _m_ui->executions_campaigns_list->setHorizontalHeaderLabels(tmp_headers);
    _m_ui->executions_campaigns_list->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _m_ui->executions_campaigns_list->setRowCount(0);
    _m_ui->executions_campaigns_list->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

    connect(_m_tests_tree_view, SIGNAL(delKeyPressed(QList<Hierarchy*>)), this, SLOT(deleteTestsCampaignList(QList<Hierarchy*>)));

    connect(_m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(save()));
    connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));

    connect(_m_ui->execute_button, SIGNAL(clicked()), this, SLOT(execute()));
    connect(_m_ui->button_continue_execution, SIGNAL(clicked()), this, SLOT(executeSelectedExecutionCampaign()));
    connect(_m_ui->executions_campaigns_list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(open(QModelIndex)));
    connect(_m_ui->executions_campaigns_list->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(manageExecutionsSelection()));

    connect(_m_ui->button_delete_execution, SIGNAL(clicked()), this, SLOT(deleteSelectedExecutionCampaign()));

    connect(_m_ui->campaign_name, SIGNAL(textChanged(const QString &)), this, SLOT(setModified()));
    connect(_m_ui->campaign_description->textEditor(), SIGNAL(textChanged()), this, SLOT(setModified()));

    manageExecutionsSelection();
}


/**
  Destructeur
**/
Form_Campaign::~Form_Campaign()
{
    destroyPluginsViews();

    delete _m_ui;

    delete _m_tests_tree_view;
    delete _m_tests_tree_model;

    qDeleteAll(_m_executions_list);
    delete _m_campaign;

    // Ne pas supprimer la campagne : son destructeur est appelé par le destructeur de project_version
    // delete _m_original_campaign;
}


void Form_Campaign::reloadCampaign()
{
    loadCampaign(_m_original_campaign);
}


void Form_Campaign::loadCampaign(Campaign *in_campaign)
{
    int                   tmp_execution_index = 0;
    bool		   tmp_modifiable = false;
    bool		   tmp_executable = false;
    net_session		*tmp_session = Session::instance()->getClientSession();

    _m_ui->lock_widget->setVisible(false);

    if (in_campaign != NULL)
    {
        _m_original_campaign = in_campaign;
        if (_m_campaign != NULL)
            delete _m_campaign;

        _m_campaign = _m_original_campaign->duplicate();

        _m_campaign->loadTests();
        loadCampaignExecutions();

        tmp_modifiable = _m_campaign->isWritable();
        if (tmp_modifiable)
            _m_campaign->lockRecord(true);

        tmp_modifiable = tmp_modifiable && _m_campaign->lockRecordStatus() == RECORD_STATUS_OWN_LOCK;

        if (_m_campaign->lockRecordStatus() == RECORD_STATUS_LOCKED)
        {
            _m_ui->lock_widget->setVisible(true);
            net_get_field(NET_MESSAGE_TYPE_INDEX+1, Session::instance()->getClientSession()->m_response, Session::instance()->getClientSession()->m_column_buffer, SEPARATOR_CHAR);
            _m_ui->label_lock_by->setText(tr("Verrouillé par %1").arg(Session::instance()->getClientSession()->m_column_buffer));
        }

        tmp_executable = _m_campaign->projectVersion() != NULL && _m_campaign->projectVersion()->project() != NULL && _m_campaign->projectVersion()->project()->projectGrants() != NULL
                && compare_values(_m_campaign->projectVersion()->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC), PROJECT_GRANT_WRITE) == 0;

        _m_ui->execute_button->setEnabled(tmp_executable);
        _m_ui->button_delete_execution->setVisible(cl_has_user_this_role(tmp_session, cl_current_user(tmp_session), "admin_role"));

        setWindowTitle(tr("Campagne : %1").arg(_m_campaign->getValueForKey(CAMPAIGNS_TABLE_SHORT_NAME)));
        _m_ui->title->setText(tr("Campagne : %1").arg(_m_campaign->getValueForKey(CAMPAIGNS_TABLE_SHORT_NAME)));

        delete _m_tests_tree_model;

        _m_tests_tree_model = new RecordsTreeModel(_m_campaign, tests_campaigns_table_def.m_entity_signature_id);
        _m_tests_tree_model->setSupportedDropActions(Qt::CopyAction|Qt::MoveAction);
        _m_tests_tree_view->setModel(_m_tests_tree_model);

        _m_ui->campaign_name->setText(_m_original_campaign->getValueForKey(CAMPAIGNS_TABLE_SHORT_NAME));
        _m_ui->campaign_description->textEditor()->setHtml(_m_original_campaign->getValueForKey(CAMPAIGNS_TABLE_DESCRIPTION));

        _m_ui->campaign_name->setReadOnly(!tmp_modifiable);
        _m_ui->campaign_description->textEditor()->setReadOnly(!tmp_modifiable);
        if (tmp_modifiable)
            _m_ui->campaign_description->toolBar()->show();
        else
            _m_ui->campaign_description->toolBar()->hide();

        _m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(tmp_modifiable);

        _m_ui->executions_campaigns_list->setRowCount(_m_executions_list.count());
        for (tmp_execution_index = 0; tmp_execution_index < _m_executions_list.count(); tmp_execution_index++)
        {
            setExecutionCampaignForRow(_m_executions_list.at(tmp_execution_index), tmp_execution_index);
        }
        _m_ui->executions_campaigns_list->resizeColumnsToContents();

        destroyPluginsViews();
        loadPluginsViews();
    }

    _m_modified = false;
}


void Form_Campaign::changeEvent(QEvent *e)
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
  Enregistrer les modifications
**/
bool Form_Campaign::saveCampaign()
{
    _m_original_campaign->setValueForKey(_m_ui->campaign_name->text().toStdString().c_str(), CAMPAIGNS_TABLE_SHORT_NAME);
    _m_original_campaign->setValueForKey(_m_ui->campaign_description->textEditor()->toHtml().toStdString().c_str(), CAMPAIGNS_TABLE_DESCRIPTION);
    return (_m_original_campaign->saveRecord() == NOERR);
}


/**
  Valider la saisie
**/
void Form_Campaign::save()
{
    if (saveCampaign())
    {
        savePluginsDatas();
        _m_modified = false;
        emit campaignSaved();
    }
}


void Form_Campaign::cancel()
{
    QWidget::close();
}


void Form_Campaign::deleteTestsCampaignList(QList<Hierarchy*> in_records_list)
{
    QModelIndex         tmp_model_index;
    QMessageBox         *tmp_msg_box = NULL;
    QPushButton         *tmp_del_button = NULL;
    QPushButton         *tmp_keep_button = NULL;
    QPushButton         *tmp_del_all_button = NULL;
    QPushButton         *tmp_cancel_button = NULL;
    bool                tmp_delete_all = false;
    bool                tmp_delete_current = false;

    QList<Hierarchy*>	tmp_records_list;

    if (_m_campaign->isWritable())
    {
        foreach(Hierarchy *tmp_item, in_records_list)
        {
            if (tmp_item->parent() == _m_campaign)
                tmp_records_list.append(tmp_item);
        }

        if (tmp_records_list.count() > 0)
        {
            tmp_msg_box = new QMessageBox(this);
            tmp_msg_box->setIcon(QMessageBox::Question);
            tmp_msg_box->setWindowTitle(tr("Confirmation..."));

            tmp_del_button = tmp_msg_box->addButton(tr("Supprimer le test"), QMessageBox::YesRole);
            tmp_keep_button = tmp_msg_box->addButton(tr("Conserver le test"), QMessageBox::YesRole);
            tmp_cancel_button = tmp_msg_box->addButton(tr("Annuler"), QMessageBox::RejectRole);

            if (tmp_records_list.count() > 1)
                tmp_del_all_button = tmp_msg_box->addButton(tr("Supprimer tous les tests"), QMessageBox::NoRole);

            foreach(Hierarchy *tmp_item, tmp_records_list)
            {
                tmp_delete_current = tmp_delete_all;

                if (tmp_delete_all == false)
                {
                    tmp_msg_box->setText(tr("Etes-vous sûr(e) de vouloir supprimer le test \"%1\" ?").arg(((TestCampaign*)tmp_item)->projectTest()->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)));
                    tmp_msg_box->exec();
                    if (tmp_msg_box->clickedButton() == tmp_del_button)
                    {
                        tmp_delete_current = true;
                    }
                    else if (tmp_msg_box->clickedButton() == tmp_del_all_button)
                    {
                        tmp_delete_current = true;
                        tmp_delete_all = true;
                    }
                    else if (tmp_msg_box->clickedButton() == tmp_cancel_button)
                        break;
                }

                tmp_model_index = _m_tests_tree_model->modelIndexForItem(tmp_item);
                if (tmp_delete_current && tmp_model_index.isValid())
                    _m_tests_tree_model->removeItem(tmp_model_index.row(), tmp_model_index.parent());
            }
        }
    }
}



void Form_Campaign::setExecutionCampaignForRow(ExecutionCampaign *in_execution_campaign, int in_row)
{
    QTableWidgetItem    *tmp_column_item = NULL;
    QString				tmp_float_string;
    QDateTime			tmp_date_time;
    QProgressBar		*tmp_progress_bar = NULL;

    float		tmp_tests_coverage_rate = 0;
    float		tmp_tests_validated_rate = 0;
    float		tmp_tests_invalidated_rate = 0;
    float		tmp_tests_bypassed_rate = 0;

    if (in_execution_campaign != NULL)
    {
        in_execution_campaign->loadExecutionsTests();
        tmp_tests_coverage_rate = in_execution_campaign->executionCoverageRate() * 100;
        tmp_tests_validated_rate = in_execution_campaign->executionValidatedRate() * 100;
        tmp_tests_invalidated_rate = in_execution_campaign->executionInValidatedRate() * 100;
        tmp_tests_bypassed_rate = in_execution_campaign->executionBypassedRate() * 100;

        // Premiere colonne (date)
        tmp_date_time = QDateTime::fromString(QString(in_execution_campaign->getValueForKey(EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_DATE)).left(16), "yyyy-MM-dd hh:mm");
        tmp_column_item = new QTableWidgetItem;
        tmp_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_execution_campaign));
        tmp_column_item->setText(tmp_date_time.toString("dddd dd MMMM yyyy à hh:mm"));
        _m_ui->executions_campaigns_list->setItem(in_row, 0, tmp_column_item);

        // Deuxieme colonne (Révision)
        tmp_column_item = new QTableWidgetItem;
        tmp_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_execution_campaign));
        tmp_column_item->setText(in_execution_campaign->getValueForKey(EXECUTIONS_CAMPAIGNS_TABLE_REVISION));
        _m_ui->executions_campaigns_list->setItem(in_row, 1, tmp_column_item);

        // Troisième colonne (Couverture)
        tmp_column_item = new QTableWidgetItem;
        tmp_progress_bar = new QProgressBar(this);
        tmp_progress_bar->setMinimum(0);
        tmp_progress_bar->setMaximum(100);
        tmp_progress_bar->setValue(tmp_tests_coverage_rate);
        tmp_progress_bar->setStyleSheet("QProgressBar::chunk {background: #c4c2c3};");
        _m_ui->executions_campaigns_list->setCellWidget(in_row, 2, tmp_progress_bar);
        tmp_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_execution_campaign));
        tmp_float_string.setNum(tmp_tests_coverage_rate, 'f', 2);
        //tmp_column_item->setText(tmp_float_string + " %");
        _m_ui->executions_campaigns_list->setItem(in_row, 2, tmp_column_item);


        // Quatrième colonne (réussite)
        tmp_column_item = new QTableWidgetItem;
        tmp_progress_bar = new QProgressBar(this);
        tmp_progress_bar->setMinimum(0);
        tmp_progress_bar->setMaximum(100);
        tmp_progress_bar->setValue(tmp_tests_validated_rate);
        tmp_progress_bar->setStyleSheet("QProgressBar::chunk {background: #77d57a};");
        _m_ui->executions_campaigns_list->setCellWidget(in_row, 3, tmp_progress_bar);
        tmp_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_execution_campaign));
        tmp_float_string.setNum(tmp_tests_validated_rate, 'f', 2);
        //tmp_column_item->setText(tmp_float_string + " %");
        _m_ui->executions_campaigns_list->setItem(in_row, 3, tmp_column_item);

        // Cinquième colonne (echec)
        tmp_column_item = new QTableWidgetItem;
        tmp_progress_bar = new QProgressBar(this);
        tmp_progress_bar->setMinimum(0);
        tmp_progress_bar->setMaximum(100);
        tmp_progress_bar->setValue(tmp_tests_invalidated_rate);
        tmp_progress_bar->setStyleSheet("QProgressBar::chunk {background: #d56060};");
        _m_ui->executions_campaigns_list->setCellWidget(in_row, 4, tmp_progress_bar);
        tmp_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_execution_campaign));
        tmp_float_string.setNum(tmp_tests_invalidated_rate, 'f', 2);
        //tmp_column_item->setText(tmp_float_string + " %");
        _m_ui->executions_campaigns_list->setItem(in_row, 4, tmp_column_item);

        // Sixième colonne (non passé)
        tmp_column_item = new QTableWidgetItem;
        tmp_progress_bar = new QProgressBar(this);
        tmp_progress_bar->setMinimum(0);
        tmp_progress_bar->setMaximum(100);
        tmp_progress_bar->setValue(tmp_tests_bypassed_rate);
        tmp_progress_bar->setStyleSheet("QProgressBar::chunk {background: #c4c2c3};");
        _m_ui->executions_campaigns_list->setCellWidget(in_row, 5, tmp_progress_bar);
        tmp_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_execution_campaign));
        tmp_float_string.setNum(tmp_tests_bypassed_rate, 'f', 2);
        //tmp_column_item->setText(tmp_float_string + " %");
        _m_ui->executions_campaigns_list->setItem(in_row, 5, tmp_column_item);

    }
}



/**
  Lancer une execution de la campagne
**/
void Form_Campaign::execute()
{
    ExecutionCampaign        *tmp_execution_campaign = NULL;
    Form_Execution_Campaign     *tmp_dialog = NULL;

    tmp_execution_campaign = ExecutionCampaign::create(_m_campaign);
    tmp_dialog = new Form_Execution_Campaign(tmp_execution_campaign, this);
    tmp_dialog->showMaximized();
    connect(tmp_dialog, SIGNAL(destroyed()), this, SLOT(reloadCampaign()));
}


/**
  Ouvrir une execution de la campagne
**/
void Form_Campaign::open(QModelIndex in_index)
{
    ExecutionCampaign        *tmp_execution_campaign = NULL;
    QTableWidgetItem            *tmp_column_item = NULL;
    Form_Execution_Campaign     *tmp_dialog = NULL;

    if (in_index.isValid())
    {
        tmp_column_item = _m_ui->executions_campaigns_list->item(in_index.row(), 0);
        if (tmp_column_item != NULL)
        {
            tmp_execution_campaign = (ExecutionCampaign*)tmp_column_item->data(Qt::UserRole).value<void*>();
            if (tmp_execution_campaign != NULL)
            {
                tmp_execution_campaign->loadExecutionsDatas();
                tmp_dialog = new Form_Execution_Campaign(tmp_execution_campaign, this);
                tmp_dialog->showMaximized();
                connect(tmp_dialog, SIGNAL(destroyed()), this, SLOT(reloadCampaign()));
            }
        }
    }
}


void Form_Campaign::closeEvent(QCloseEvent *in_event)
{
    int		tmp_confirm_choice = 0;

    if (_m_campaign != NULL)
    {
        if (_m_ui->campaign_name->isModified() || _m_ui->campaign_description->textEditor()->document()->isModified())
        {
            tmp_confirm_choice = QMessageBox::question(
                        this,
                        tr("Confirmation..."),
                        tr("La campagne a été modifiée voules-vous enregistrer les modifications ?"),
                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                        QMessageBox::Cancel);

            if (tmp_confirm_choice == QMessageBox::Yes)
            {
                if (saveCampaign() == false)
                {
                    in_event->ignore();
                    return;
                }
            }
            else if (tmp_confirm_choice == QMessageBox::Cancel)
            {
                in_event->ignore();
                return;
            }
        }

        _m_campaign->unlockRecord();
    }

    in_event->accept();
}


int Form_Campaign::saveExecutionsCampaign()
{
    int             tmp_result = NOERR;

    foreach(ExecutionCampaign *tmp_execution, _m_executions_list)
    {
        tmp_result = tmp_execution->saveRecord();
        if (tmp_result != NOERR)
            return tmp_result;
    }

    return tmp_result;
}



void Form_Campaign::loadCampaignExecutions()
{
    ExecutionCampaign          **tmp_executions = NULL;

    unsigned long                 tmp_execution_count = 0;
    net_session                    *tmp_session = Session::instance()->getClientSession();
    char                          *tmp_where_clause = tmp_session->m_last_query;

    qDeleteAll(_m_executions_list);
    _m_executions_list.clear();

    sprintf(tmp_where_clause, "%s=%s", EXECUTIONS_CAMPAIGNS_TABLE_CAMPAIGN_ID, _m_campaign->getIdentifier());

    tmp_executions = Record::loadRecords<ExecutionCampaign>(tmp_session, &executions_campaigns_table_def, tmp_where_clause, EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_DATE, &tmp_execution_count);
    if (tmp_executions != NULL)
    {
        for (unsigned long tmp_index = 0; tmp_index < tmp_execution_count; tmp_index++)
        {
            tmp_executions[tmp_index]->setCampaign(_m_campaign);
            _m_executions_list.append(tmp_executions[tmp_index]);
        }
        tmp_executions = Record::freeRecords<ExecutionCampaign>(tmp_executions, 0);
    }
}

void Form_Campaign::manageExecutionsSelection()
{
    if (!_m_ui->executions_campaigns_list->selectedItems().isEmpty())
    {
        _m_ui->button_continue_execution->setEnabled(true);
        _m_ui->button_delete_execution->setEnabled(true);
    }
    else
    {
        _m_ui->button_continue_execution->setDisabled(true);
        _m_ui->button_delete_execution->setDisabled(true);
    }
}


void Form_Campaign::executeSelectedExecutionCampaign()
{
    QModelIndex 			tmp_select_row = _m_ui->executions_campaigns_list->selectionModel()->currentIndex();

    if (tmp_select_row.isValid())
    {
        open(tmp_select_row);
    }
}

void Form_Campaign::deleteSelectedExecutionCampaign()
{
    QModelIndex 			tmp_select_row = _m_ui->executions_campaigns_list->selectionModel()->currentIndex();
    ExecutionCampaign        *tmp_execution_campaign = NULL;
    QTableWidgetItem            *tmp_column_item = NULL;
    int         			tmp_row_index = 0;

    if (tmp_select_row.isValid())
    {
        tmp_row_index = tmp_select_row.row();
        tmp_column_item = _m_ui->executions_campaigns_list->item(tmp_row_index, 0);
        if (tmp_column_item != NULL)
        {
            tmp_execution_campaign = (ExecutionCampaign*)tmp_column_item->data(Qt::UserRole).value<void*>();
            if (tmp_execution_campaign != NULL)
            {
                if (QMessageBox::question(
                            this,
                            tr("Confirmation..."),
                            tr("Etes-vous sûr(e) de vouloir supprimer l'exécution de la campagne du %1 ?")
                            .arg(QDateTime::fromString(QString(tmp_execution_campaign->getValueForKey(EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_DATE)).left(16), "yyyy-MM-dd hh:mm").toString("dddd dd MMMM yyyy à hh:mm")),
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::Yes)
                {
                    _m_ui->executions_campaigns_list->removeRow(tmp_row_index);
                    _m_executions_list.removeAt(tmp_row_index);
                    tmp_execution_campaign->deleteRecord();
                    delete tmp_execution_campaign;
                }
            }
        }
    }
}


void Form_Campaign::setModified()
{
    _m_modified = true;
}


void Form_Campaign::loadPluginsViews()
{
    QMap < QString, ClientModule*>	tmp_modules_map = Session::instance()->externalsModules().value(ClientModule::CampaignPlugin);

    CampaignModule   *tmp_campaign_module = NULL;

    foreach(ClientModule *tmp_module, tmp_modules_map)
    {
        tmp_campaign_module = static_cast<CampaignModule*>(tmp_module);

        tmp_campaign_module->loadCampaignModuleDatas(_m_campaign);

        QWidget	    *tmp_module_view = tmp_campaign_module->createView(this);
        if (tmp_module_view != NULL)
        {
            _m_views_modules_map[tmp_campaign_module] = tmp_module_view;
            _m_ui->toolBox->addItem(tmp_module_view, tmp_module->getModuleName());
        }
    }
}


void Form_Campaign::savePluginsDatas()
{
    QMap<CampaignModule*, QWidget*>::iterator tmp_module_iterator;

    for(tmp_module_iterator = _m_views_modules_map.begin(); tmp_module_iterator != _m_views_modules_map.end(); tmp_module_iterator++)
    {
        tmp_module_iterator.key()->saveCampaignModuleDatas();
    }
}


void Form_Campaign::destroyPluginsViews()
{
    QMap<CampaignModule*, QWidget*>::iterator tmp_module_iterator;

    for(tmp_module_iterator = _m_views_modules_map.begin(); tmp_module_iterator != _m_views_modules_map.end(); tmp_module_iterator++)
    {
        tmp_module_iterator.key()->destroyView(tmp_module_iterator.value());
    }
}

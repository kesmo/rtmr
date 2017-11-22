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

#include "frame_project.h"

#include <QtGui>
#include "records/record.h"
#include "records/test.h"
#include "records/testcontent.h"
#include "records/projectparameter.h"
#include "records/projectgrant.h"
#include "gui/forms/form_project.h"
#include "utilities.h"
#include "entities.h"
#include "application.h"
#include "records/hierarchy.h"
#include "gui/forms/form_campaign_wizard.h"
#include "gui/forms/form_data_import.h"
#include "gui/forms/form_data_export.h"
#include "gui/forms/form_search_project.h"

#define TEST_IMPORT_SHORT_NAME		"test_name"
#define TEST_IMPORT_DESCRIPTION	"test_description"
#define TEST_IMPORT_CATEGORY_ID	"test_category"
#define TEST_IMPORT_PRIORITY_LEVEL "test_priority"
#define ACTION_IMPORT_DESCRIPTION	"action_description"
#define ACTION_IMPORT_WAIT_RESULT	"action_result"

static const char *TEST_COLUMNS_IMPORT[] = {TEST_IMPORT_SHORT_NAME, TEST_IMPORT_DESCRIPTION, TEST_IMPORT_CATEGORY_ID, TEST_IMPORT_PRIORITY_LEVEL, ACTION_IMPORT_DESCRIPTION, ACTION_IMPORT_WAIT_RESULT};
static const entity_def test_entity_import = {0, 0, 0, 0, TEST_COLUMNS_IMPORT, 0, 0, sizeof(TEST_COLUMNS_IMPORT)/sizeof(char*)};

#define REQUIREMENT_IMPORT_SHORT_NAME		"requirement_name"
#define REQUIREMENT_IMPORT_DESCRIPTION	"requirement_description"
#define REQUIREMENT_IMPORT_CATEGORY_ID	"requirement_category"
#define REQUIREMENT_IMPORT_PRIORITY_LEVEL "requirement_priority"

static const char *REQUIREMENT_COLUMNS_IMPORT[] = {REQUIREMENT_IMPORT_SHORT_NAME, REQUIREMENT_IMPORT_DESCRIPTION, REQUIREMENT_IMPORT_CATEGORY_ID, REQUIREMENT_IMPORT_PRIORITY_LEVEL};
static const entity_def requirement_entity_import = {0, 0, 0, 0, REQUIREMENT_COLUMNS_IMPORT, 0, 0, sizeof(REQUIREMENT_COLUMNS_IMPORT)/sizeof(char*)};

/**
  Constructeur
**/
Frame_Project::Frame_Project(ProjectVersion *in_project_version, MainWindow *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);

    _m_campaigns_table_widget = NULL;
    _m_dock_campaigns_widget = NULL;
    _m_dock_parameters_widget = NULL;
    _m_dock_requirements_widget = NULL;
    _m_dock_search_widget = NULL;
    _m_dock_tests_widget = NULL;
    _m_executions_reports_form = NULL;
    _m_form_campaign = NULL;
    _m_form_project_version = NULL;
    _m_form_requirement = NULL;
    _m_form_test = NULL;
    _m_grid_layout = NULL;
    _m_parameters_table_widget = NULL;
    _m_project_version = NULL;
    _m_requirements_project_tree_model = NULL;
    _m_requirements_tree_view = NULL;
    _m_search_table_widget = NULL;
    _m_tests_project_tree_model = NULL;
    _m_tests_tree_view = NULL;
    _m_bugs_form = NULL;
    _m_last_imported_test = NULL;

    _m_history_index = 0;

    _m_project_version = in_project_version;
    _m_project_version->loadProjectVersionDatas(&updateProgressBar);

    changeWindowTitle();
    createPanels();
    createMenus(parent);
}


/**
  Destructeur
**/
Frame_Project::~Frame_Project()
{

    delete _m_campaigns_table_widget;
    delete _m_dock_campaigns_widget;

    delete _m_parameters_table_widget;
    delete _m_dock_parameters_widget;

    delete _m_requirements_project_tree_model;
    delete _m_requirements_tree_view;
    delete _m_dock_requirements_widget;

    delete _m_search_table_widget;
    delete _m_dock_search_widget;

    delete _m_tests_project_tree_model;
    delete _m_tests_tree_view;
    delete _m_dock_tests_widget;

    delete _m_executions_reports_form;
    delete _m_form_campaign;
    delete _m_form_project_version;
    delete _m_form_requirement;
    delete _m_form_test;
    delete _m_grid_layout;

    delete _m_bugs_form;

    delete _m_project_version;
}


bool Frame_Project::hideAllForms()
{
    if(_m_form_project_version != NULL)
    {
        if(_m_form_project_version->close())
        {
            _m_form_project_version = NULL;
        }
        else
            return false;
    }

    if(_m_form_test != NULL)
    {
        if(_m_form_test->close())
        {
            _m_form_test = NULL;
        }
        else
            return false;
    }

    if(_m_form_campaign != NULL)
    {
        if(_m_form_campaign->close())
        {
            _m_form_campaign = NULL;
        }
        else
            return false;
    }

    if(_m_form_requirement != NULL)
    {
        if(_m_form_requirement->close())
        {
            _m_form_requirement = NULL;
        }
        else
            return false;
    }

    if (_m_executions_reports_form != NULL)
    {
        if(_m_executions_reports_form->close())
            _m_executions_reports_form = NULL;
        else
            return false;
    }

    if (_m_bugs_form != NULL)
    {
        if(_m_bugs_form->close())
            _m_bugs_form = NULL;
        else
            return false;
    }

    return true;
}



/**
  Changer le titre de la fenetre
**/
void Frame_Project::changeWindowTitle()
{
    setWindowTitle(windowTitle());
}


QString Frame_Project::windowTitle()
{
    return QString(_m_project_version->project()->valueForKey(PROJECTS_TABLE_SHORT_NAME)) +
    QString(" - ") +
    ProjectVersion::formatProjectVersionNumber(_m_project_version->valueForKey(PROJECTS_VERSIONS_TABLE_VERSION));
}


/**
  Afficher les informations du test selectionne
**/
void Frame_Project::showTestInfos(TestHierarchy *in_test, bool in_update_history)
{
    Qt::KeyboardModifiers tmp_modifiers = QApplication::keyboardModifiers();

    if (!(tmp_modifiers == Qt::ShiftModifier || tmp_modifiers == Qt::ControlModifier || tmp_modifiers == (Qt::ControlModifier|Qt::ShiftModifier)))
    {
        if (hideAllForms())
        {
            parentWidget()->setWindowIcon(QIcon(QPixmap(":/images/22x22/pellicule.png")));

            if (_m_form_test == NULL)
            {
                _m_form_test = new Form_Test(this);
                connect(_m_form_test, SIGNAL(testSaved(TestHierarchy*)), this, SLOT(testSaved(TestHierarchy*)));
                connect(_m_form_test, SIGNAL(canceled()), this, SLOT(showProjectVersionInfos()));
                connect(_m_form_test, SIGNAL(showOriginalTestInfos(TestHierarchy*)), this, SLOT(showOriginalTestInfos(TestHierarchy*)));
                connect(_m_form_test, SIGNAL(showRequirementWithOriginalContentId(const char*)), this, SLOT(showRequirementWithOriginalContentId(const char*)));
                _m_grid_layout->addWidget(_m_form_test);
            }

            if (in_test != NULL)
            {
                if (in_update_history)
                {
                    _m_history.append(in_test);
                    _m_history_index = _m_history.count() - 1;
                }
                _m_form_test->loadTest(in_test);
                if (_m_form_test != NULL)
                    _m_form_test->show();
            }
        }
    }
}


void Frame_Project::showOriginalTestInfos(TestHierarchy *in_test_hierarchy)
{
    QModelIndex	tmp_model_index;

    if (in_test_hierarchy != NULL && in_test_hierarchy->original() != NULL)
    {
        _m_tests_tree_view->selectionModel()->select(_m_tests_tree_view->selectionModel()->selection(), QItemSelectionModel::Deselect);
        tmp_model_index = _m_tests_project_tree_model->modelIndexForItem(in_test_hierarchy->original());
        if (tmp_model_index.isValid())
        {
            _m_tests_tree_view->selectionModel()->select(tmp_model_index, QItemSelectionModel::Select);
            _m_tests_tree_view->expandIndex(tmp_model_index);
            showTestInfos(in_test_hierarchy->original());
            updateMenusActionsAndToolbar();
        }
    }
}

/**
  Afficher les informations de l'exigence selectionnee
**/
void Frame_Project::showRequirementInfos(RequirementHierarchy *in_requirement, bool in_update_history)
{
    Qt::KeyboardModifiers tmp_modifiers = QApplication::keyboardModifiers();

    if (!(tmp_modifiers == Qt::ShiftModifier || tmp_modifiers == Qt::ControlModifier || tmp_modifiers == (Qt::ControlModifier|Qt::ShiftModifier)))
    {
        if (hideAllForms())
        {
            parentWidget()->setWindowIcon(QIcon(QPixmap(":/images/notes.png")));

            if (_m_form_requirement == NULL)
            {
                _m_form_requirement = new Form_Requirement(this);
                connect(_m_form_requirement, SIGNAL(requirementSaved(RequirementHierarchy*)), this, SLOT(requirementSaved(RequirementHierarchy*)));
                connect(_m_form_requirement, SIGNAL(canceled()), this, SLOT(showProjectVersionInfos()));
                connect(_m_form_requirement, SIGNAL(showTestWithContentId(const char*)), this, SLOT(showTestWithContentId(const char*)));

                _m_grid_layout->addWidget(_m_form_requirement);
            }

            if (in_requirement != NULL)
            {
                if (in_update_history)
                {
                    _m_history.append(in_requirement);
                    _m_history_index = _m_history.count() - 1;
                }
                _m_form_requirement->loadRequirement(in_requirement);
                if (_m_form_requirement != NULL)
                    _m_form_requirement->show();
            }
        }
    }
}



void Frame_Project::showTestWithContentId(const char *in_test_content_id)
{
    QModelIndex				tmp_model_index;
    TestHierarchy	*tmp_test = NULL;
    TestHierarchy	**tmp_tests_array = NULL;
    unsigned long			tmp_rows_count = 0;

    net_session             *tmp_session = (dynamic_cast<Application*>qApp)->m_session;
    char                    *tmp_statement = tmp_session->m_where_clause_buffer;

    if (is_empty_string(in_test_content_id) == FALSE)
    {
        _m_tests_tree_view->selectionModel()->select(_m_tests_tree_view->selectionModel()->selection(), QItemSelectionModel::Deselect);

        /* Chercher le test dans l'arborescence */
        tmp_test = (TestHierarchy*)_m_project_version->findItemWithValueForKey(in_test_content_id, TESTS_HIERARCHY_TEST_CONTENT_ID, TESTS_HIERARCHY_SIG_ID);
        if (tmp_test != NULL)
        {
            tmp_model_index = _m_tests_project_tree_model->modelIndexForItem(tmp_test);
            if (tmp_model_index.isValid())
            {
                _m_tests_tree_view->selectionModel()->select(tmp_model_index, QItemSelectionModel::Select);
                _m_tests_tree_view->expandIndex(tmp_model_index);
                showTestInfos(tmp_model_index);
                updateMenusActionsAndToolbar();
            }
        }
        /* Chercher le test en base (ex: si le test est dans une autre version du projet) */
        else
        {
            sprintf(tmp_statement, "%s=%s", TESTS_HIERARCHY_TEST_CONTENT_ID, in_test_content_id);

            tmp_tests_array = Record::loadRecords<TestHierarchy>(tmp_session, &tests_hierarchy_def, tmp_statement, NULL, &tmp_rows_count);
            if (tmp_tests_array != NULL)
            {
                if (tmp_rows_count == 1)
                {
                    showTestInfos(tmp_tests_array[0]);
                    updateMenusActionsAndToolbar();
                    Record::freeRecords<TestHierarchy>(tmp_tests_array, 0);
                }
                else
                {
                    Record::freeRecords<TestHierarchy>(tmp_tests_array, tmp_rows_count);
                }
            }

        }
    }

}

void Frame_Project::showRequirementWithOriginalContentId(const char *in_original_requirement_content_id)
{
    QModelIndex				tmp_model_index;
    RequirementHierarchy	*tmp_requirement = NULL;
    RequirementHierarchy	**tmp_requirements_array = NULL;
    unsigned long			tmp_rows_count = 0;

    net_session             *tmp_session = (dynamic_cast<Application*>qApp)->m_session;
    char                    *tmp_statement = tmp_session->m_where_clause_buffer;

    if (is_empty_string(in_original_requirement_content_id) == FALSE)
    {
        _m_requirements_tree_view->selectionModel()->select(_m_requirements_tree_view->selectionModel()->selection(), QItemSelectionModel::Deselect);

        /* Chercher l'exigence dans l'arborescence */
        tmp_requirement = RequirementHierarchy::findRequirementWithValueForKey(_m_project_version->requirementsHierarchy(), in_original_requirement_content_id, REQUIREMENTS_HIERARCHY_ORIGINAL_REQUIREMENT_CONTENT_ID);
        if (tmp_requirement != NULL)
        {
            tmp_model_index = _m_requirements_project_tree_model->modelIndexForItem(tmp_requirement);
            if (tmp_model_index.isValid())
            {
                _m_requirements_tree_view->selectionModel()->select(tmp_model_index, QItemSelectionModel::Select);
                _m_requirements_tree_view->expandIndex(tmp_model_index);
                showRequirementInfos(tmp_model_index);
                updateMenusActionsAndToolbar();
            }
        }
        /* Chercher l'exigence en base (ex: si l'exigence est dans une autre version du projet) */
        else
        {
            sprintf(tmp_statement, "%s=%s", REQUIREMENTS_HIERARCHY_ORIGINAL_REQUIREMENT_CONTENT_ID, in_original_requirement_content_id);

            tmp_requirements_array = Record::loadRecords<RequirementHierarchy>(tmp_session, &requirements_hierarchy_def, tmp_statement, NULL, &tmp_rows_count);
            if (tmp_requirements_array != NULL)
            {
                if (tmp_rows_count == 1)
                {
                    showRequirementInfos(tmp_requirements_array[0]);
                    updateMenusActionsAndToolbar();
                    Record::freeRecords<RequirementHierarchy>(tmp_requirements_array, 0);
                }
                else
                {
                    Record::freeRecords<RequirementHierarchy>(tmp_requirements_array, tmp_rows_count);
                }
            }

        }
    }
}


/**
  Afficher les informations de la campagne selectionnee
**/
void Frame_Project::showCampaignInfos(Campaign *in_campaign, bool in_update_history)
{
    if (hideAllForms())
    {
        parentWidget()->setWindowIcon(QIcon(QPixmap(":/images/package.png")));

        if (_m_form_campaign == NULL)
        {
            _m_form_campaign = new Form_Campaign(this);
            connect(_m_form_campaign, SIGNAL(campaignSaved()), this, SLOT(showProjectVersionInfos()));
            connect(_m_form_campaign, SIGNAL(canceled()), this, SLOT(showProjectVersionInfos()));
            _m_grid_layout->addWidget(_m_form_campaign);
        }

        if (in_campaign != NULL)
        {
            if (in_update_history)
            {
                _m_history.append(in_campaign);
                _m_history_index = _m_history.count() - 1;
            }
            _m_form_campaign->loadCampaign(in_campaign);
            if (_m_form_campaign != NULL)
                _m_form_campaign->show();
        }
    }
}


/**
  Afficher les informations de la version du projet
**/
void Frame_Project::showProjectVersionInfos(bool in_update_history)
{
    if (hideAllForms())
    {
        parentWidget()->setWindowIcon(QIcon(QPixmap(":/images/box.png")));

        if (_m_form_project_version == NULL)
        {
            _m_form_project_version = new Form_Project_Version(_m_project_version, this);
            _m_grid_layout->addWidget(_m_form_project_version);
        }

        if (in_update_history)
        {
            _m_history.append(_m_project_version);
            _m_history_index = _m_history.count() - 1;
        }
        _m_form_project_version->show();
        changeWindowTitle();
        updateMenusActionsAndToolbar();
    }
}


/**
  Afficher les statistiques d'executions du projet
**/
void Frame_Project::showExecutionsStats()
{
    if (hideAllForms())
    {
        parentWidget()->setWindowIcon(QIcon(QPixmap(":/images/22x22/stats.png")));

        if (_m_executions_reports_form == NULL)
        {
            _m_executions_reports_form = new Form_Executions_Reports(_m_project_version, this);
            connect(_m_executions_reports_form, SIGNAL(canceled()), this, SLOT(showProjectVersionInfos()));
            _m_grid_layout->addWidget(_m_executions_reports_form);
        }

        _m_executions_reports_form->show();
        changeWindowTitle();
    }
}



/**
  Afficher les anomalies du projet
**/
void Frame_Project::showBugs()
{
    if (hideAllForms())
    {
        parentWidget()->setWindowIcon(QIcon(QPixmap(":/images/bug.png")));

        if (_m_bugs_form == NULL)
        {
            _m_bugs_form = new Form_Project_Bugs(_m_project_version, this);
            connect(_m_bugs_form, SIGNAL(closed()), this, SLOT(showProjectVersionInfos()));
            _m_grid_layout->addWidget(_m_bugs_form);
        }

        _m_bugs_form->show();
        changeWindowTitle();
    }
}



void Frame_Project::testSaved(TestHierarchy *in_test)
{
    QModelIndex	tmp_model_index = _m_tests_project_tree_model->modelIndexForItem(in_test);
    if (tmp_model_index.isValid())
        _m_tests_tree_view->update(tmp_model_index);

    //showProjectVersionInfos();
}


void Frame_Project::requirementSaved(RequirementHierarchy *in_requirement)
{
    QModelIndex	tmp_model_index = _m_requirements_project_tree_model->modelIndexForItem(in_requirement);
    if (tmp_model_index.isValid())
        _m_requirements_tree_view->update(tmp_model_index);

    //showProjectVersionInfos();
}


int Frame_Project::saveProjectVersion()
{
    QWidget	*tmp_focus_widget = NULL;
    int tmp_return = _m_project_version->project()->saveRecord();

    // Mettre à jour les vues
    changeWindowTitle();
    tmp_focus_widget = focusWidget();
    _m_tests_tree_view->setFocus();
    _m_requirements_tree_view->setFocus();
    tmp_focus_widget->setFocus();

    return tmp_return;
}


Record* Frame_Project::nextRecordInHistory()
{
    if (_m_history_index < _m_history.count() - 1)
    {
        _m_history_index++;
        return _m_history[_m_history_index];
    }

    return NULL;
}


Record* Frame_Project::previousRecordInHistory()
{
    if (_m_history_index > 0)
    {
        _m_history_index--;
        return _m_history[_m_history_index];
    }

    return NULL;
}


bool Frame_Project::historyHasNextRecord()
{
    return _m_history_index < _m_history.count() - 1;
}


bool Frame_Project::historyHasPreviousRecord()
{
    return _m_history_index > 0;
}


void Frame_Project::createPanels()
{
    QStringList     tmp_parameters_list_headers;

    QVBoxLayout     *tmp_vertical_layout = NULL;
    QLabel          *tmp_title = NULL;
    QSplitter       *tmp_horizontal_splitter1 = new QSplitter(Qt::Horizontal);
    QSplitter       *tmp_horizontal_splitter2 = new QSplitter(Qt::Horizontal);

    QSplitter       *tmp_vertical_splitter1 = new QSplitter(Qt::Vertical);
    QSplitter       *tmp_vertical_splitter2 = new QSplitter(Qt::Vertical);
    QSplitter       *tmp_vertical_splitter3 = new QSplitter(Qt::Vertical);

    QWidget         *tmp_central_widget = new QWidget;

    QGridLayout     *tmp_grid_layout = new QGridLayout(this);
    tmp_grid_layout->setSpacing(0);
    tmp_grid_layout->setMargin(0);
    setLayout(tmp_grid_layout);

    // Exigences
    createRequirementsTreeView();

    // Tests
    createTestsTreeView();

    // Campagnes
    _m_campaigns_table_widget = new QTableWidget();
    _m_campaigns_table_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
    _m_campaigns_table_widget->setSelectionMode(QAbstractItemView::SingleSelection);
    _m_campaigns_table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _m_campaigns_table_widget->setColumnCount(1);
    _m_campaigns_table_widget->verticalHeader()->setVisible(false);
    _m_campaigns_table_widget->verticalHeader()->setDefaultSectionSize(18);
    _m_campaigns_table_widget->horizontalHeader()->setVisible(false);
    _m_campaigns_table_widget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    _m_campaigns_table_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    //    connect(_m_campaigns_table_widget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showCampaignInfos(QModelIndex)));
    connect(_m_campaigns_table_widget, SIGNAL(clicked(QModelIndex)), this, SLOT(showCampaignInfos(QModelIndex)));

    // Parametres
    _m_parameters_table_widget = new QTableWidget();
    _m_parameters_table_widget->setSelectionMode(QAbstractItemView::SingleSelection);
    _m_parameters_table_widget->setColumnCount(2);
    _m_parameters_table_widget->verticalHeader()->setVisible(false);
    _m_parameters_table_widget->verticalHeader()->setDefaultSectionSize(18);
    _m_parameters_table_widget->horizontalHeader()->setVisible(true);
    _m_parameters_table_widget->horizontalHeader()->setMinimumSectionSize(50);
    tmp_parameters_list_headers << "Nom" << "Valeur";
    _m_parameters_table_widget->setHorizontalHeaderLabels(tmp_parameters_list_headers);
    _m_parameters_table_widget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    _m_parameters_table_widget->sortByColumn(0, Qt::AscendingOrder);
    _m_parameters_table_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    connect(_m_parameters_table_widget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(parameterItemChanged(QTableWidgetItem*)));

    // Recherche
    _m_search_table_widget = new QTableWidget();
    _m_search_table_widget->setColumnCount(2);
    _m_search_table_widget->setColumnWidth(0, 24);
    _m_search_table_widget->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
    _m_search_table_widget->setSelectionBehavior(QAbstractItemView::SelectRows);
    _m_search_table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _m_search_table_widget->horizontalHeader()->setVisible(false);
    _m_search_table_widget->verticalHeader()->setVisible(false);
    _m_search_table_widget->verticalHeader()->setDefaultSectionSize(18);
    _m_search_table_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    connect(_m_search_table_widget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showSearchItem(QModelIndex)));
    connect(_m_search_table_widget, SIGNAL(itemSelectionChanged()), this, SLOT(searchItemsSelectionChanged()));

    // Tests
    _m_tests_project_tree_model = new RecordsTreeModel(_m_project_version, TESTS_HIERARCHY_SIG_ID);
    _m_tests_tree_view->setModel(_m_tests_project_tree_model);
    //connect(_m_tests_tree_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showTestInfos(QModelIndex)));
    connect(_m_tests_tree_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateMenusActionsAndToolbar()));
    connect(_m_tests_project_tree_model, SIGNAL(cyclicRedundancy()), this, SLOT(showCylicRedundancyAlert()));

    // Exigences
    _m_requirements_project_tree_model = new RecordsTreeModel(_m_project_version, REQUIREMENTS_HIERARCHY_SIG_ID);
    _m_requirements_tree_view->setModel(_m_requirements_project_tree_model);
    //connect(_m_requirements_tree_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showRequirementInfos(QModelIndex)));
    connect(_m_requirements_tree_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateMenusActionsAndToolbar()));

    // Parametres
    _m_parameters_table_widget->setRowCount(0);
    foreach(ProjectParameter *tmp_project_parameter, _m_project_version->project()->parametersList())
    {
        _m_parameters_table_widget->insertRow(_m_parameters_table_widget->rowCount());
        setParameterAtIndex(tmp_project_parameter, _m_parameters_table_widget->rowCount() - 1);
    }
    //connect(_m_parameters_table_widget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(parameterItemChanged(QTableWidgetItem*)));
    connect(_m_parameters_table_widget->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateMenusActionsAndToolbar()));

    // Campagnes
    _m_campaigns_table_widget->setRowCount(0);
    foreach(Campaign *tmp_campaign, _m_project_version->campaignsList())
    {
        _m_campaigns_table_widget->insertRow(_m_campaigns_table_widget->rowCount());
        setCampaignAtIndex(tmp_campaign, _m_campaigns_table_widget->rowCount() - 1);
    }
    //connect(_m_campaigns_table_widget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showCampaignInfos(QModelIndex)));
    connect(_m_campaigns_table_widget->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateMenusActionsAndToolbar()));

    // Recherche
    _m_search_table_widget->setRowCount(0);
    //connect(_m_search_table_widget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showSearchItem(QModelIndex)));
    //connect(_m_search_table_widget, SIGNAL(itemSelectionChanged()), this, SLOT(searchItemsSelectionChanged()));

    _m_grid_layout = new QGridLayout();
    _m_grid_layout->setSpacing(0);
    _m_grid_layout->setMargin(0);
    tmp_central_widget->setLayout(_m_grid_layout);
    tmp_central_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    _m_dock_requirements_widget = new QWidget;
    _m_dock_requirements_widget->setObjectName("requirement_dock_widget");
    tmp_vertical_layout = new QVBoxLayout();
    tmp_vertical_layout->setSpacing(0);
    tmp_vertical_layout->setMargin(0);
    tmp_title = new QLabel(tr("Exigences"));
    tmp_title->setObjectName("requirement_dock_widget_title");
    tmp_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tmp_vertical_layout->addWidget(tmp_title);
    tmp_vertical_layout->addWidget(_m_requirements_tree_view);
    _m_dock_requirements_widget->setLayout(tmp_vertical_layout);
    _m_dock_requirements_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    tmp_vertical_splitter1->addWidget(_m_dock_requirements_widget);

    _m_dock_campaigns_widget = new QWidget;
    _m_dock_campaigns_widget->setObjectName("campaign_dock_widget");
    tmp_vertical_layout = new QVBoxLayout();
    tmp_vertical_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    tmp_vertical_layout->setSpacing(0);
    tmp_vertical_layout->setMargin(0);
    tmp_title = new QLabel(tr("Campagnes"));
    tmp_title->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    tmp_title->setObjectName("campaign_dock_widget_title");
    tmp_vertical_layout->addWidget(tmp_title);
    tmp_vertical_layout->addWidget(_m_campaigns_table_widget);
    _m_dock_campaigns_widget->setLayout(tmp_vertical_layout);
    _m_dock_campaigns_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    tmp_vertical_splitter1->addWidget(_m_dock_campaigns_widget);
    tmp_vertical_splitter1->setStretchFactor(0, 4);

    tmp_vertical_splitter1->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    _m_dock_tests_widget = new QWidget;
    _m_dock_tests_widget->setObjectName("test_dock_widget");
    tmp_vertical_layout = new QVBoxLayout();
    tmp_vertical_layout->setSpacing(0);
    tmp_vertical_layout->setMargin(0);
    tmp_title = new QLabel(tr("Scénarios et cas de test"));
    tmp_title->setObjectName("test_dock_widget_title");
    tmp_title->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    tmp_vertical_layout->addWidget(tmp_title);
    tmp_vertical_layout->addWidget(_m_tests_tree_view);
    _m_dock_tests_widget->setLayout(tmp_vertical_layout);
    _m_dock_tests_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    tmp_vertical_splitter2->addWidget(_m_dock_tests_widget);

    tmp_vertical_splitter2->addWidget(tmp_vertical_splitter3);
    tmp_vertical_splitter2->setStretchFactor(0, 4);

    _m_dock_parameters_widget = new QWidget;
    _m_dock_parameters_widget->setObjectName("parameter_dock_widget");
    tmp_vertical_layout = new QVBoxLayout();
    tmp_vertical_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    tmp_vertical_layout->setSpacing(0);
    tmp_vertical_layout->setMargin(0);
    tmp_title = new QLabel(tr("Paramètres de projet"));
    tmp_title->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    tmp_title->setObjectName("parameter_dock_widget_title");
    tmp_vertical_layout->addWidget(tmp_title);
    tmp_vertical_layout->addWidget(_m_parameters_table_widget);
    _m_dock_parameters_widget->setLayout(tmp_vertical_layout);
    _m_dock_parameters_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    tmp_vertical_splitter3->addWidget(_m_dock_parameters_widget);

    _m_dock_search_widget = new QWidget;
    _m_dock_search_widget->setObjectName("search_dock_widget");
    tmp_vertical_layout = new QVBoxLayout();
    tmp_vertical_layout->setSpacing(0);
    tmp_vertical_layout->setMargin(0);
    tmp_title = new QLabel(tr("Résultats de la recherche"));
    tmp_title->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    tmp_title->setObjectName("search_dock_widget_title");
    tmp_vertical_layout->addWidget(tmp_title);
    tmp_vertical_layout->addWidget(_m_search_table_widget);
    _m_dock_search_widget->setLayout(tmp_vertical_layout);
    tmp_vertical_splitter3->addWidget(_m_dock_search_widget);
    _m_dock_search_widget->hide();

    tmp_horizontal_splitter1->addWidget(tmp_vertical_splitter1);
    tmp_horizontal_splitter1->addWidget(tmp_horizontal_splitter2);
    tmp_horizontal_splitter1->setStretchFactor(1, 2);

    tmp_horizontal_splitter2->addWidget(tmp_central_widget);
    tmp_horizontal_splitter2->addWidget(tmp_vertical_splitter2);
    tmp_horizontal_splitter2->setStretchFactor(0, 2);

    tmp_grid_layout->addWidget(tmp_horizontal_splitter1);
}

void Frame_Project::createMenus(MainWindow *parent)
{
    /*
    // Actions du menu projet
    insertParameterAction = parent->insertParameterAction;
    removeParameterAction = parent->removeParameterAction;
    showProjectPropertiesAction = parent->showProjectPropertiesAction;
    executionsReportsAction = parent->executionsReportsAction;
    searchProjectAction = parent->searchProjectAction;

    // Actions de tests
    insertTestAction = parent->insertTestAction;
#if defined(AUTOMATION_LIB) && (defined(_WINDOWS) || defined (WIN32))
    insertAutomatedTestAction = parent->insertAutomatedTestAction;
#endif
    removeTestAction = parent->removeTestAction;
    insertChildTestAction = parent->insertChildTestAction;
    expandAllTestsAction = parent->expandAllTestsAction;
    expandSelectedTestsAction = parent->expandSelectedTestsAction;
    collapseAllTestsAction = parent->collapseAllTestsAction;
    collapseSelectedTestsAction = parent->collapseSelectedTestsAction;
    selectLinkTestsAction = parent->selectLinkTestsAction;
    importTestsAction = parent->importTestsAction;
    exportTestsAction = parent->exportTestsAction;

    // Actions des exigences
    insertRequirementAction = parent->insertRequirementAction;
    removeRequirementAction = parent->removeRequirementAction;
    insertChildRequirementAction = parent->insertChildRequirementAction;
    expandAllRequirementsAction = parent->expandAllRequirementsAction;
    expandSelectedRequirementsAction = parent->expandSelectedRequirementsAction;
    collapseAllRequirementsAction = parent->collapseAllRequirementsAction;
    collapseSelectedRequirementsAction = parent->collapseSelectedRequirementsAction;
    selectDependantsTestsAction = parent->selectDependantsTestsAction;
    importRequirementsAction = parent->importRequirementsAction;
    exportRequirementsAction = parent->exportRequirementsAction;

    // Actions des campagnes
    insertCampaignAction = parent->insertCampaignAction;
    removeCampaignAction = parent->removeCampaignAction;

    historyPreviousAction = parent->historyPreviousAction;
    historyNextAction = parent->historyNextAction;

    _m_tests_tree_view->addAction(insertTestAction);
#if defined(AUTOMATION_LIB) && (defined(_WINDOWS) || defined (WIN32))
    _m_tests_tree_view->addAction(insertAutomatedTestAction);
#endif
    _m_tests_tree_view->addAction(removeTestAction);
    _m_tests_tree_view->addAction(insertChildTestAction);
    _m_tests_tree_view->addAction(expandAllTestsAction);
    _m_tests_tree_view->addAction(expandSelectedTestsAction);
    _m_tests_tree_view->addAction(collapseAllTestsAction);
    _m_tests_tree_view->addAction(collapseSelectedTestsAction);
    _m_tests_tree_view->addAction(selectLinkTestsAction);
    _m_tests_tree_view->addAction(importTestsAction);
    _m_tests_tree_view->addAction(exportTestsAction);
    _m_tests_tree_view->setContextMenuPolicy(Qt::ActionsContextMenu);

    _m_requirements_tree_view->addAction(insertRequirementAction);
    _m_requirements_tree_view->addAction(removeRequirementAction);
    _m_requirements_tree_view->addAction(insertChildRequirementAction);
    _m_requirements_tree_view->addAction(expandAllRequirementsAction);
    _m_requirements_tree_view->addAction(expandSelectedRequirementsAction);
    _m_requirements_tree_view->addAction(collapseAllRequirementsAction);
    _m_requirements_tree_view->addAction(collapseSelectedRequirementsAction);
    _m_requirements_tree_view->addAction(selectDependantsTestsAction);
    _m_requirements_tree_view->addAction(importRequirementsAction);
    _m_requirements_tree_view->addAction(exportRequirementsAction);
    _m_requirements_tree_view->setContextMenuPolicy(Qt::ActionsContextMenu);

    _m_campaigns_table_widget->addAction(insertCampaignAction);
    _m_campaigns_table_widget->addAction(removeCampaignAction);
    _m_campaigns_table_widget->setContextMenuPolicy(Qt::ActionsContextMenu);

    _m_parameters_table_widget->addAction(insertParameterAction);
    _m_parameters_table_widget->addAction(removeParameterAction);
    _m_parameters_table_widget->setContextMenuPolicy(Qt::ActionsContextMenu);
    */

}

void Frame_Project::searchItemsSelectionChanged()
{
    QList<Hierarchy*>     	tmp_list;
    Record             	*tmp_record = NULL;
    QModelIndex				tmp_model_index;

    _m_tests_tree_view->selectionModel()->select(_m_tests_tree_view->selectionModel()->selection(), QItemSelectionModel::Deselect);
    _m_requirements_tree_view->selectionModel()->select(_m_requirements_tree_view->selectionModel()->selection(), QItemSelectionModel::Deselect);

    foreach(QTableWidgetItem *tmp_item, _m_search_table_widget->selectedItems())
    {
        tmp_record = (Record*)tmp_item->data(Qt::UserRole).value<void*>();
        if (tmp_record->getEntityDefSignatureId() == TESTS_HIERARCHY_SIG_ID)
        {
            tmp_model_index = _m_tests_project_tree_model->modelIndexForItem((Hierarchy*)tmp_record);
            if (tmp_model_index.isValid())
            {
                _m_tests_tree_view->selectionModel()->select(tmp_model_index, QItemSelectionModel::Select);
                _m_tests_tree_view->expandIndex(tmp_model_index);
            }
        }
        else if (tmp_record->getEntityDefSignatureId() == REQUIREMENTS_HIERARCHY_SIG_ID)
        {
            tmp_model_index = _m_requirements_project_tree_model->modelIndexForItem((Hierarchy*)tmp_record);
            if (tmp_model_index.isValid())
            {
                _m_requirements_tree_view->selectionModel()->select(tmp_model_index, QItemSelectionModel::Select);
                _m_requirements_tree_view->expandIndex(tmp_model_index);
            }
        }
    }
}


/**
  Afficher les informations de l'item sélectionné dans la liste de recherche
**/
void Frame_Project::showSearchItem(QModelIndex in_model_index)
{
    QTableWidgetItem		*tmp_item = NULL;

    if (in_model_index.isValid())
    {
        tmp_item = _m_search_table_widget->item(in_model_index.row(), in_model_index.column());
        if (tmp_item != NULL)
        {
            showItemProject((Record*)tmp_item->data(Qt::UserRole).value<void*>());
            updateMenusActionsAndToolbar();
        }
    }
}


void Frame_Project::showItemProject(Record *in_record, bool in_update_project_history)
{
    QModelIndex                 tmp_model_index;
    QTableWidgetItem			*tmp_item = NULL;
    Campaign                 *tmp_campaign = NULL;

    if (in_record != NULL)
    {
        // Sélectionner l'enregistrement
        if (in_record->getEntityDefSignatureId() == TESTS_HIERARCHY_SIG_ID)
        {
            tmp_model_index = _m_tests_project_tree_model->modelIndexForItem((Hierarchy*)in_record);
            if (tmp_model_index.isValid())
            {
                _m_tests_tree_view->setCurrentIndex(tmp_model_index);
                _m_tests_tree_view->expandIndex(tmp_model_index);
                showTestInfos((TestHierarchy*)in_record, in_update_project_history);
                return;
            }
        }
        else if (in_record->getEntityDefSignatureId() == REQUIREMENTS_HIERARCHY_SIG_ID)
        {
            tmp_model_index = _m_requirements_project_tree_model->modelIndexForItem((Hierarchy*)in_record);
            if (tmp_model_index.isValid())
            {
                _m_requirements_tree_view->setCurrentIndex(tmp_model_index);
                _m_requirements_tree_view->expandIndex(tmp_model_index);
                showRequirementInfos((RequirementHierarchy*)in_record, in_update_project_history);
                return;
            }
        }
        else if (in_record->getEntityDefSignatureId() == projects_versions_table_def.m_entity_signature_id)
        {
            showProjectVersionInfos(in_update_project_history);
        }
        else if (in_record->getEntityDefSignatureId() == campaigns_table_def.m_entity_signature_id)
        {
            for (int tmp_item_index = 0; tmp_item_index < _m_campaigns_table_widget->rowCount(); tmp_item_index++)
            {
                tmp_item = _m_campaigns_table_widget->item(tmp_item_index, 0);
                if (tmp_item != NULL)
                {
                    tmp_campaign = (Campaign*)tmp_item->data(Qt::UserRole).value<void*>();
                    if (tmp_campaign != NULL && tmp_campaign == in_record)
                    {
                        _m_campaigns_table_widget->selectRow(tmp_item_index);
                        showCampaignInfos(tmp_campaign, in_update_project_history);
                        return;
                    }
                }
            }
        }

        updateMenusActionsAndToolbar();

    }
}


/**
  Afficher les informations d'une campagne
**/
void Frame_Project::showCampaignInfos(QModelIndex in_model_index)
{
    QTableWidgetItem			*tmp_item = NULL;
    Campaign                 *tmp_campaign = NULL;

    if (in_model_index.isValid())
    {
        tmp_item = _m_campaigns_table_widget->item(in_model_index.row(), in_model_index.column());
        if (tmp_item != NULL)
        {
            tmp_campaign = (Campaign*)tmp_item->data(Qt::UserRole).value<void*>();
            if (tmp_campaign != NULL)
            {
                showCampaignInfos(tmp_campaign);
                updateMenusActionsAndToolbar();
            }
        }
    }
}


/**
  Afficher les informations d'une exigence
**/
void Frame_Project::showRequirementInfos(QModelIndex in_model_index)
{
    RequirementHierarchy     	*tmp_selected_requirement = NULL;

    if (in_model_index.isValid())
    {
        tmp_selected_requirement = (RequirementHierarchy*)(_m_requirements_project_tree_model->getItem(in_model_index));
        showRequirementInfos(tmp_selected_requirement);
        updateMenusActionsAndToolbar();
    }
}



/**
  Afficher les informations du test selectionne
**/
void Frame_Project::showTestInfos(QModelIndex in_model_index)
{
    TestHierarchy     *tmp_selected_test = NULL;

    if (in_model_index.isValid())
    {
        tmp_selected_test = (TestHierarchy*)(_m_tests_project_tree_model->getItem(in_model_index));
        showTestInfos(tmp_selected_test);
        updateMenusActionsAndToolbar();
    }
}


/**
  Inserer un test
**/
void Frame_Project::insertRequirementAtIndex(QModelIndex index)
{
    _m_requirements_project_tree_model->insertRows(index.row()+1, 1, index.parent());

    _m_requirements_tree_view->selectionModel()->setCurrentIndex(_m_requirements_project_tree_model->index(index.row()+1, 0, index.parent()), QItemSelectionModel::ClearAndSelect);
    showRequirementInfos(_m_requirements_tree_view->selectionModel()->currentIndex());
}



/**
  Inserer un test
**/
void Frame_Project::insertTestAtIndex(QModelIndex index, bool in_automated_test)
{
    QModelIndex tmp_new_test_index;

    if (in_automated_test)
    {
        TestHierarchy* tmp_test = NULL;

        TestContent* tmp_test_content = new TestContent(_m_project_version);
        tmp_test_content->takeValueForKey("", TESTS_CONTENTS_TABLE_SHORT_NAME);
#ifdef AUTOMATION_LIB
        tmp_test_content->takeValueForKey(TEST_CONTENT_TYPE_AUTOMATED, TESTS_CONTENTS_TABLE_TYPE);
#endif
        if (tmp_test_content->saveRecord() == NOERR)
        {
            tmp_test = new TestHierarchy(_m_project_version);
            tmp_test->setDataFromTestContent(tmp_test_content);
        }
        delete tmp_test_content;

        if (tmp_test)
            _m_tests_project_tree_model->insertItem(index.row()+1, index.parent(), tmp_test);
    }
    else
        _m_tests_project_tree_model->insertRows(index.row()+1, 1, index.parent());

    tmp_new_test_index = _m_tests_project_tree_model->index(index.row()+1, 0, index.parent());
    if (tmp_new_test_index.isValid())
    {
        _m_tests_tree_view->selectionModel()->setCurrentIndex(tmp_new_test_index, QItemSelectionModel::ClearAndSelect);
        showTestInfos(_m_tests_tree_view->selectionModel()->currentIndex());
    }
}


/**
  Inserer un test
**/
void Frame_Project::insertTest()
{
    insertTestAtIndex(_m_tests_tree_view->selectionModel()->currentIndex());
}


/**
  Inserer un test automatisé
**/
void Frame_Project::insertAutomatedTest()
{
    insertTestAtIndex(_m_tests_tree_view->selectionModel()->currentIndex(), true);
}


/**
  Inserer un test enfant
**/
void Frame_Project::insertChildTest()
{
    QModelIndex		index = _m_tests_tree_view->selectionModel()->currentIndex();
    Hierarchy		*tmp_item = NULL;
    int				tmp_row = 0;

    if (index.isValid())
    {
        tmp_item = _m_tests_project_tree_model->getItem(index);

        if (tmp_item != NULL)
            tmp_row = tmp_item->childCount(TESTS_HIERARCHY_SIG_ID);

        if (!_m_tests_project_tree_model->insertRow(tmp_row, index))
            return;

        _m_tests_tree_view->selectionModel()->setCurrentIndex(_m_tests_project_tree_model->index(tmp_row, 0, index), QItemSelectionModel::ClearAndSelect);
        showTestInfos(_m_tests_tree_view->selectionModel()->currentIndex());
    }
}

/**
  Supprimer les tests sélectionnés
**/
void Frame_Project::removeSelectedTests()
{
    removeTestsList(_m_tests_tree_view->selectedRecords());
}


/**
  Supprimer la liste des tests
**/
void Frame_Project::removeTestsList(QList<Hierarchy*> in_records_list)
{
    QModelIndex         tmp_model_index;
    QMessageBox         *tmp_msg_box = NULL;
    QPushButton         *tmp_del_button = NULL;
    QPushButton         *tmp_del_all_button = NULL;
    QPushButton         *tmp_cancel_button = NULL;
    bool                tmp_delete_all = false;
    bool                tmp_delete_current = false;

    if (_m_project_version->isWritable(TESTS_HIERARCHY_SIG_ID) && in_records_list.count() > 0)
    {
        tmp_msg_box = new QMessageBox(this);
        tmp_msg_box->setIcon(QMessageBox::Question);
        tmp_msg_box->setWindowTitle("Confirmation...");

        if (in_records_list.count() > 1)
            tmp_del_all_button = tmp_msg_box->addButton(tr("Supprimer tous les tests"), QMessageBox::NoRole);

        tmp_del_button = tmp_msg_box->addButton(tr("Supprimer le test"), QMessageBox::YesRole);
        tmp_cancel_button = tmp_msg_box->addButton(tr("Annuler"), QMessageBox::RejectRole);

        tmp_msg_box->setDefaultButton(tmp_cancel_button);

        foreach(Hierarchy *tmp_item, in_records_list)
        {
            tmp_delete_current = tmp_delete_all;

            if (tmp_delete_all == false)
            {
                tmp_msg_box->setText(tr("Etes-vous sûr(e) de vouloir supprimer le test \"%1\" ?").arg(((TestHierarchy*)tmp_item)->valueForKey(TESTS_HIERARCHY_SHORT_NAME)));
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

            tmp_model_index = _m_tests_project_tree_model->modelIndexForItem(tmp_item);
            if (tmp_delete_current && tmp_model_index.isValid())
                _m_tests_project_tree_model->removeItem(tmp_model_index.row(), tmp_model_index.parent());
        }
    }
}


void Frame_Project::selectDependantsTestsFromSelectedRequirements()
{
    QModelIndexList tmp_indexes_list = dependantsTestsIndexesFromRequirementsList(_m_requirements_tree_view->selectedRecords());

    foreach(QModelIndex tmp_model_index, tmp_indexes_list)
    {
        _m_tests_tree_view->selectionModel()->select(tmp_model_index, QItemSelectionModel::Select);
        _m_tests_tree_view->expandIndex(tmp_model_index);
    }
}


QModelIndexList Frame_Project::dependantsTestsIndexesFromRequirementsList(QList<Hierarchy*> in_records_list)
{
    net_session         *tmp_session = (dynamic_cast<Application*>qApp)->m_session;
    char                *tmp_query = tmp_session->m_last_query;
    char                ***tmp_query_results = NULL;
    unsigned long       tmp_rows_count = 0;
    unsigned long       tmp_columns_count = 0;

    TestHierarchy       *tmp_test = NULL;
    QModelIndexList     tmp_final_indexes_list;
    QModelIndexList     tmp_indexes_list;

    if (in_records_list.count() > 0)
    {
        _m_tests_tree_view->selectionModel()->select(_m_tests_tree_view->selectionModel()->selection(), QItemSelectionModel::Deselect);

        tmp_query += sprintf(tmp_query, "SELECT %s FROM %s WHERE %s IN("
        , TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID
        , TESTS_REQUIREMENTS_TABLE_SIG
        , TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID
        );
        for(int tmp_index = 0; tmp_index < in_records_list.count(); tmp_index++)
        {
            tmp_query += sprintf(tmp_query, "%s", in_records_list[tmp_index]->valueForKey(REQUIREMENTS_HIERARCHY_ORIGINAL_REQUIREMENT_CONTENT_ID));
            if (tmp_index + 1 < in_records_list.count())
                tmp_query += sprintf(tmp_query, ",");
        }
        tmp_query += sprintf(tmp_query, ");");

        tmp_query_results = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
        if (tmp_query_results != NULL)
        {
            for(unsigned long tmp_index = 0; tmp_index < tmp_rows_count; tmp_index++)
            {
                tmp_indexes_list = _m_tests_project_tree_model->modelIndexesForItemWithValueForKey(tmp_query_results[tmp_index][0], TESTS_HIERARCHY_TEST_CONTENT_ID);
                foreach (QModelIndex tmp_model_index, tmp_indexes_list)
                {
                    if (tmp_model_index.isValid())
                    {
                        tmp_test = (TestHierarchy*)_m_tests_project_tree_model->getItem(tmp_model_index);
                        if (tmp_test != NULL)
                        {
                            while (tmp_test->original() != NULL && tmp_test->parent() != _m_project_version)
                            {
                                tmp_test = (TestHierarchy*)tmp_test->parent();
                                tmp_model_index = tmp_model_index.parent();
                            }

                            tmp_final_indexes_list.append(tmp_model_index);

                            foreach (TestHierarchy *tmp_copy, tmp_test->copies())
                            {
                                tmp_model_index = _m_tests_project_tree_model->modelIndexForItem(tmp_copy);

                                while (tmp_copy->original() != NULL && tmp_copy->parent() != _m_project_version)
                                {
                                    tmp_copy = (TestHierarchy*)tmp_copy->parent();
                                    tmp_model_index = tmp_model_index.parent();
                                }

                                tmp_final_indexes_list.append(tmp_model_index);
                            }

                        }
                    }
                }
            }
            cl_free_rows_columns_array(&tmp_query_results, tmp_rows_count, tmp_columns_count);
        }
    }

    return tmp_final_indexes_list;
}


void Frame_Project::selectLinkTests()
{
    selectLinkTests(_m_tests_tree_view->selectedRecords());
}

void Frame_Project::selectLinkTests(QList<Hierarchy*> in_records_list)
{
    QModelIndex         tmp_model_index;
    QList<TestHierarchy*>		tmp_link_tests_list;
    TestHierarchy		*tmp_test = NULL;

    if (in_records_list.count() > 0)
    {
        _m_tests_tree_view->selectionModel()->select(_m_tests_tree_view->selectionModel()->selection(), QItemSelectionModel::Deselect);

        foreach(Hierarchy *tmp_item, in_records_list)
        {
            tmp_link_tests_list = ((TestHierarchy*)tmp_item)->copies();

            foreach(TestHierarchy *tmp_link_test, tmp_link_tests_list)
            {
                if (tmp_link_test->parent() == _m_project_version)
                {
                    tmp_model_index = _m_tests_project_tree_model->modelIndexForItem(tmp_link_test);
                }
                else
                {
                    tmp_test = (TestHierarchy*)tmp_link_test->parent();
                    while (tmp_test->original() != NULL)
                    {
                        if (tmp_test->parent() == _m_project_version)
                            break;

                        tmp_test = (TestHierarchy*)tmp_test->parent();
                    }

                    tmp_model_index = _m_tests_project_tree_model->modelIndexForItem(tmp_test);
                }

                if (tmp_model_index.isValid())
                {
                    _m_tests_tree_view->selectionModel()->select(tmp_model_index, QItemSelectionModel::Select);
                    _m_tests_tree_view->expandIndex(tmp_model_index);
                }
            }
        }
    }
}

/**
  Tout derouler
**/
void Frame_Project::expandTestsTree()
{
    _m_tests_tree_view->expandAll();
}


/**
  Derouler les tests sélectionnés
**/
void Frame_Project::expandSelectedTestsTree()
{
    foreach(QModelIndex tmp_index, _m_tests_tree_view->selectionModel()->selectedIndexes())
    {
        _m_tests_tree_view->setExpandedIndex(tmp_index, true);
    }
}


/**
  Tout enrouler
**/
void Frame_Project::collapseTestsTree()
{
    _m_tests_tree_view->collapseAll();
}


/**
  Enrouler les tests sélectionnés
**/
void Frame_Project::collapseSelectedTestsTree()
{
    foreach(QModelIndex tmp_index, _m_tests_tree_view->selectionModel()->selectedIndexes())
    {
        _m_tests_tree_view->setExpandedIndex(tmp_index, false);
    }
}



/**
  Importer des tests
**/
void Frame_Project::importTests()
{
    QStringList		tmp_columns_names;
    Form_Data_Import	*tmp_import_form = NULL;
    QList<Hierarchy*>	tmp_records_list = _m_tests_tree_view->selectedRecords();
    Hierarchy		*tmp_test = NULL;
    TestHierarchy	*tmp_parent_test = NULL;

    _m_last_imported_test = NULL;

    if (tmp_records_list.isEmpty())
    {
        if (_m_project_version->testsHierarchy().isEmpty() == false)
        {
            tmp_test = _m_project_version->child(_m_project_version->childCount(TESTS_HIERARCHY_SIG_ID) - 1, TESTS_HIERARCHY_SIG_ID);
        }
    }
    else
    {
        tmp_test = tmp_records_list[tmp_records_list.count() - 1];
    }

    if (tmp_test != NULL)
    {
        QMessageBox::information(this, tr("Information"), tr("Les tests seront importés après le test \"%1\".").arg(tmp_test->valueForKey(TESTS_HIERARCHY_SHORT_NAME)));

        if (tmp_test->parent() != _m_project_version)
            tmp_parent_test = new TestHierarchy((TestHierarchy*)tmp_test->parent());
        else
            tmp_parent_test = new TestHierarchy(_m_project_version);
    }
    else
        tmp_parent_test = new TestHierarchy(_m_project_version);

    tmp_columns_names << tr("Nom du test") << tr("Description du test") << tr("Catégorie du test") << tr("Priorité du test") << tr("Description de l'action") << tr("Résultat attendu de l'action");
    tmp_import_form = new Form_Data_Import(tmp_parent_test, tmp_columns_names, &test_entity_import, this);
    connect(tmp_import_form, SIGNAL(startImport(Hierarchy*)), this, SLOT(startImportTests(Hierarchy*)));
    connect(tmp_import_form, SIGNAL(importRecord(Hierarchy*, GenericRecord*, bool)), this, SLOT(importTest(Hierarchy*, GenericRecord*, bool)));
    tmp_import_form->show();
}


void Frame_Project::startImportTests(Hierarchy *in_parent)
{
    TestContent		*tmp_test_content = NULL;
    int			tmp_save_result = NOERR;

    TestHierarchy	*tmp_test_parent = NULL;

    tmp_test_parent = (TestHierarchy*)in_parent;

    cl_transaction_start(mainApp->m_session);

    tmp_test_content = new TestContent(_m_project_version);

    tmp_test_content->takeValueForKey(tr("Import de tests").toStdString().c_str(), TESTS_CONTENTS_TABLE_SHORT_NAME);
    tmp_test_content->takeValueForKey(tr("Import de tests").toStdString().c_str(), TESTS_CONTENTS_TABLE_DESCRIPTION);
    tmp_save_result = tmp_test_content->saveRecord();
    if (tmp_save_result == NOERR)
    {
        tmp_test_parent->setDataFromTestContent(tmp_test_content);
        tmp_save_result = tmp_test_parent->saveRecord();
    }

    if (tmp_save_result != NOERR)
    {
        cl_transaction_rollback(mainApp->m_session);

        disconnect(this, SLOT(startImportTests(Hierarchy*)));
        disconnect(this, SLOT(importTest(Hierarchy*, GenericRecord*, bool)));
    }
}


void Frame_Project::endImportTests(Hierarchy *in_parent)
{
    QList<Hierarchy*>	tmp_records_list = _m_tests_tree_view->selectedRecords();
    Hierarchy		*tmp_test = NULL;
    Hierarchy		*tmp_parent = NULL;
    QModelIndex		tmp_model_index;
    int			tmp_index = 0;

    TestHierarchy	*tmp_test_parent = NULL;

    int			tmp_tr_result = NOERR;

    tmp_test_parent = (TestHierarchy*)in_parent;

    if (tmp_records_list.isEmpty())
    {
        if (_m_project_version->testsHierarchy().isEmpty() == false)
        {
            tmp_test = _m_project_version->child(_m_project_version->childCount(TESTS_HIERARCHY_SIG_ID) - 1, TESTS_HIERARCHY_SIG_ID);
        }
    }
    else
    {
        tmp_test = tmp_records_list[tmp_records_list.count() - 1];
    }

    if (tmp_test != NULL)
    {
        tmp_model_index = _m_tests_project_tree_model->modelIndexForItem(tmp_test);
        if (tmp_model_index.isValid())
        {
            tmp_index = tmp_model_index.row() + 1;
            tmp_model_index = tmp_model_index.parent();
        }

        tmp_parent = tmp_test->parent();
    }
    else
    {
        tmp_index = _m_project_version->childCount(TESTS_HIERARCHY_SIG_ID);
        tmp_parent = _m_project_version;
        tmp_model_index  = QModelIndex();
    }

    if (_m_tests_project_tree_model->insertItem(tmp_index, tmp_model_index, tmp_test_parent))
    {
        tmp_tr_result = cl_transaction_commit(mainApp->m_session);
        if (tmp_tr_result != NOERR)
            QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), TR_SERVER_MESSAGE(cl_get_error_message(mainApp->m_session, tmp_tr_result)));

    }
    else
    {
        cl_transaction_rollback(mainApp->m_session);
    }
}


void Frame_Project::importTest(Hierarchy *in_parent, GenericRecord *in_record, bool in_last_record)
{
    TestContent		*tmp_test_content = NULL;
    Action		*tmp_action = NULL;
    int			tmp_save_result = NOERR;

    TestHierarchy	*tmp_test = NULL;

    TestHierarchy	*tmp_existing_test = NULL;

    int			tmp_chils_count = 0;

    bool		tmp_create_action = false;

    TestHierarchy	*tmp_test_parent = NULL;

    QStringList         tmp_str_list;

    if (in_parent != NULL && in_record != NULL)
    {
        tmp_test_parent = (TestHierarchy*)in_parent;

        tmp_create_action = is_empty_string(in_record->valueForKey(ACTION_IMPORT_DESCRIPTION)) == FALSE || is_empty_string(in_record->valueForKey(ACTION_IMPORT_WAIT_RESULT)) == FALSE;

        tmp_str_list = QString(in_record->valueForKey(TEST_IMPORT_SHORT_NAME)).split(PARENT_HIERARCHY_SEPARATOR, QString::SkipEmptyParts);
        if (tmp_str_list.isEmpty())
        {
            tmp_test_parent = _m_last_imported_test;
        }
        else
        {
            foreach(QString tmp_str_name, tmp_str_list)
            {
                tmp_existing_test = (TestHierarchy*)tmp_test_parent->findItemWithValueForKey(tmp_str_name.toStdString().c_str(), TESTS_HIERARCHY_SHORT_NAME, TESTS_HIERARCHY_SIG_ID, false);
                if (tmp_existing_test == NULL)
                {
                    tmp_test_content = new TestContent(_m_project_version);
                    if (tmp_test_content != NULL)
                    {
                        tmp_test_content->takeValueForKey(tmp_str_name.toStdString().c_str(), TESTS_CONTENTS_TABLE_SHORT_NAME);
                        tmp_test_content->takeValueForKey(in_record->valueForKey(TEST_IMPORT_DESCRIPTION), TESTS_CONTENTS_TABLE_DESCRIPTION);
                        tmp_test_content->takeValueForKey(in_record->valueForKey(TEST_IMPORT_CATEGORY_ID), TESTS_CONTENTS_TABLE_CATEGORY_ID);
                        tmp_test_content->takeValueForKey(in_record->valueForKey(TEST_IMPORT_PRIORITY_LEVEL), TESTS_CONTENTS_TABLE_PRIORITY_LEVEL);
                        tmp_save_result = tmp_test_content->saveRecord();
                        if (tmp_save_result == NOERR)
                        {
                            tmp_test = new TestHierarchy(_m_project_version);
                            if (tmp_test != NULL)
                            {
                                tmp_chils_count = tmp_test_parent->childCount(TESTS_HIERARCHY_SIG_ID);
                                tmp_test->setDataFromTestContent(tmp_test_content);

                                tmp_test_parent->insertChildren(tmp_chils_count, 0, tmp_test);

                                tmp_test_parent = tmp_test;
                            }
                        }

                        delete tmp_test_content;
                    }
                }
                else
                    tmp_test_parent = tmp_existing_test;
            }
        }

        if (tmp_test_parent != NULL)
        {
            _m_last_imported_test = tmp_test_parent;

            if (tmp_create_action)
            {
                tmp_action = new Action();
                tmp_action->takeValueForKey(tmp_test_parent->valueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID), ACTIONS_TABLE_TEST_CONTENT_ID);
                tmp_action->takeValueForKey(in_record->valueForKey(ACTION_IMPORT_DESCRIPTION), ACTIONS_TABLE_DESCRIPTION);
                tmp_action->takeValueForKey(in_record->valueForKey(ACTION_IMPORT_WAIT_RESULT), ACTIONS_TABLE_WAIT_RESULT);
                tmp_save_result = tmp_action->saveRecord();
            }
        }
    }

    if (tmp_save_result != NOERR)
    {
        cl_transaction_rollback(mainApp->m_session);

        disconnect(this, SLOT(startImportTests(Hierarchy*)));
        disconnect(this, SLOT(importTest(Hierarchy*, GenericRecord*, bool)));

        QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), TR_SERVER_MESSAGE(cl_get_error_message(mainApp->m_session, tmp_save_result)));

    }
    else if (in_last_record)
    {
        endImportTests(in_parent);
    }
}


/**
  Exporter des tests
**/
void Frame_Project::exportTests()
{
    Form_Data_Export	*tmp_export_form = NULL;

    tmp_export_form = new Form_Data_Export(this);
    connect(tmp_export_form, SIGNAL(startExport(QString,QByteArray,QByteArray, QByteArray)), this, SLOT(startExportTests(QString,QByteArray,QByteArray, QByteArray)));
    tmp_export_form->show();
}



void Frame_Project::startExportTests(QString in_filepath, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char)
{
    QList<Hierarchy*>       tmp_records_list = Hierarchy::parentRecordsFromRecordsList(_m_tests_tree_view->selectedRecords());

    QFile                   tmp_file;

    tmp_file.setFileName(in_filepath);
    if (tmp_file.open(QIODevice::WriteOnly))
    {
        // Entête
        tmp_file.write(QString(in_field_enclosing_char+tr("Nom")+in_field_enclosing_char+in_field_separator+
        in_field_enclosing_char+tr("Description")+in_field_enclosing_char+in_field_separator+
        in_field_enclosing_char+tr("Catégorie")+in_field_enclosing_char+in_field_separator+
        in_field_enclosing_char+tr("Priorité")+in_field_enclosing_char+in_field_separator+
        in_field_enclosing_char+tr("Action")+in_field_enclosing_char+in_field_separator+
        in_field_enclosing_char+tr("Résultat attendu")+in_field_enclosing_char+in_record_separator).toStdString().c_str());

        writeTestsListToExportFile(tmp_file, castHierarchyList<TestHierarchy>(tmp_records_list), in_field_separator, in_record_separator, in_field_enclosing_char);

        tmp_file.close();
    }
    else
    {
        QMessageBox::critical(this, tr("Fichier non créé"), tr("L'ouverture du fichier en écriture est impossible (%1).").arg(tmp_file.errorString()));
    }
}


void Frame_Project::writeTestsListToExportFile(QFile & in_file, QList<TestHierarchy*> in_records_list, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char)
{
    TestContent             *tmp_test_content = NULL;
    QList<Action*>          tmp_test_actions;

    foreach(Hierarchy *tmp_record, in_records_list)
    {
        tmp_test_content = new TestContent(_m_project_version);
        if (tmp_test_content != NULL)
        {
            if (tmp_test_content->loadRecord(tmp_record->valueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID)) == NOERR)
            {
                // Actions
                tmp_test_actions = tmp_test_content->loadActions();
                if (tmp_test_actions.isEmpty())
                {
                    writeTestToExportFile(in_file, (TestHierarchy*)tmp_record, tmp_test_content, NULL, in_field_separator, in_record_separator, in_field_enclosing_char);
                }
                else
                {
                    foreach(Action *tmp_action, tmp_test_actions)
                    {
                        writeTestToExportFile(in_file, (TestHierarchy*)tmp_record, tmp_test_content, tmp_action, in_field_separator, in_record_separator, in_field_enclosing_char);
                    }
                }

                // Tests enfants
                writeTestsListToExportFile(in_file, ((TestHierarchy*)tmp_record)->childs(), in_field_separator, in_record_separator, in_field_enclosing_char);
            }

            delete tmp_test_content;
        }
    }

}


void Frame_Project::writeTestToExportFile(QFile & in_file, TestHierarchy *in_test, TestContent *in_test_content, Action *in_action, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char)
{
    QString                 tmp_str;

    QTextDocument           tmp_doc;

    Hierarchy               *tmp_parent = NULL;

    qint64                  tmp_bytes_write = 0;

    // Action
    if (in_action != NULL)
    {
        in_action->loadAssociatedActionsForVersion(in_test->valueForKey(TESTS_HIERARCHY_VERSION));
        if (!in_action->associatedTestActions().isEmpty())
        {
            foreach (Action *tmp_associated_action, in_action->associatedTestActions())
            {
                writeTestToExportFile(in_file, in_test, in_test_content, tmp_associated_action, in_field_separator, in_record_separator, in_field_enclosing_char);
            }
            return;
        }
    }

    // Nom du test
    tmp_str = "";
    tmp_parent = in_test;
    while (tmp_parent->parent() != NULL && tmp_parent->parent() != _m_project_version)
    {
        tmp_parent = tmp_parent->parent();
        tmp_str = QString(tmp_parent->valueForKey(TESTS_HIERARCHY_SHORT_NAME)) + PARENT_HIERARCHY_SEPARATOR + tmp_str;
    }
    tmp_str += QString(in_test_content->valueForKey(TESTS_CONTENTS_TABLE_SHORT_NAME));

    tmp_str = in_field_enclosing_char + tmp_str.replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Description du test
    tmp_doc.setHtml(in_test_content->valueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION));
    tmp_str = in_field_enclosing_char + tmp_doc.toPlainText().replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Catégorie du test
    tmp_str = in_field_enclosing_char + QString(in_test_content->valueForKey(TESTS_CONTENTS_TABLE_CATEGORY_ID)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Priorité du test
    tmp_str = in_field_enclosing_char + QString(in_test_content->valueForKey(TESTS_CONTENTS_TABLE_PRIORITY_LEVEL)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Action
    if (in_action != NULL)
    {
        // Separateur de champs
        tmp_bytes_write = in_file.write(in_field_separator);
        if (tmp_bytes_write < 0)    return;

        // Description de l'action
        tmp_doc.setHtml(in_action->valueForKey(ACTIONS_TABLE_DESCRIPTION));
        tmp_str = in_field_enclosing_char + tmp_doc.toPlainText().replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
        tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
        if (tmp_bytes_write < 0)    return;

        // Separateur de champs
        tmp_bytes_write = in_file.write(in_field_separator);
        if (tmp_bytes_write < 0)    return;

        // Résultat attendu
        tmp_doc.setHtml(in_action->valueForKey(ACTIONS_TABLE_WAIT_RESULT));
        tmp_str = in_field_enclosing_char + tmp_doc.toPlainText().replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
        tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
        if (tmp_bytes_write < 0)    return;
    }

    // Separateur d'enregistrements
    tmp_bytes_write = in_file.write(in_record_separator);
}

void Frame_Project::insertCampaign()
{
    Form_Campaign_Wizard    *tmp_campaign_wizard = new Form_Campaign_Wizard(_m_project_version, this);

    connect(tmp_campaign_wizard, SIGNAL(campaignCreated(Campaign*)), this, SLOT(insertCampaign(Campaign*)));

    tmp_campaign_wizard->show();
}


/**
  Ajout d'une campagne
**/
void Frame_Project::insertCampaign(Campaign *in_campaign)
{
    _m_campaigns_table_widget->insertRow(_m_campaigns_table_widget->rowCount());
    _m_project_version->addCampaign(in_campaign);

    setCampaignAtIndex(in_campaign, _m_campaigns_table_widget->rowCount() - 1);
    _m_campaigns_table_widget->selectRow(_m_campaigns_table_widget->rowCount() - 1);
    showCampaignInfos(_m_campaigns_table_widget->selectionModel()->currentIndex());
    updateMenusActionsAndToolbar();
}


void Frame_Project::createRequirementsTreeView()
{
    _m_requirements_tree_view = new RecordsTreeView();
    _m_requirements_tree_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    _m_requirements_tree_view->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    _m_requirements_tree_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _m_requirements_tree_view->setDragEnabled(true);
    _m_requirements_tree_view->setAcceptDrops(true);
    _m_requirements_tree_view->setDropIndicatorShown(true);
    _m_requirements_tree_view->setHeaderHidden(true);
    _m_requirements_tree_view->setExpandsOnDoubleClick(false);

    //connect(_m_requirements_tree_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showRequirementInfos(QModelIndex)));
    connect(_m_requirements_tree_view, SIGNAL(clicked(QModelIndex)), this, SLOT(showRequirementInfos(QModelIndex)));
    connect(_m_requirements_tree_view, SIGNAL(delKeyPressed(QList<Hierarchy*>)), this, SLOT(removeRequirementsList(QList<Hierarchy*>)));
}


void Frame_Project::createTestsTreeView()
{
    _m_tests_tree_view = new RecordsTreeView();
    _m_tests_tree_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    _m_tests_tree_view->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    _m_tests_tree_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _m_tests_tree_view->setDragEnabled(true);
    _m_tests_tree_view->setAcceptDrops(true);
    _m_tests_tree_view->setDropIndicatorShown(true);
    _m_tests_tree_view->setHeaderHidden(true);
    _m_tests_tree_view->setExpandsOnDoubleClick(false);

    connect(_m_tests_tree_view, SIGNAL(delKeyPressed(QList<Hierarchy*>)), this, SLOT(removeTestsList(QList<Hierarchy*>)));
    //connect(_m_tests_tree_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showTestInfos(QModelIndex)));
    connect(_m_tests_tree_view, SIGNAL(clicked(QModelIndex)), this, SLOT(showTestInfos(QModelIndex)));
}



/**
  Affichage de la fenetre de recherche
**/
void Frame_Project::searchProject()
{
    Form_Search_Project *tmp_form = new Form_Search_Project(_m_project_version, this);

    connect(tmp_form, SIGNAL(foundRecords(const QList<Record*> &)), this, SLOT(showSearchResults(const QList<Record*>&)));
    tmp_form->show();
}



void Frame_Project::showSearchResults(const QList<Record*> & out_records_list)
{
    int	tmp_index = 0;

    _m_dock_search_widget->show();
    _m_search_table_widget->setRowCount(out_records_list.count());

    foreach(Record *tmp_record, out_records_list)
    {
        setSearchRecordAtIndex(tmp_record, tmp_index++);
    }
}



void Frame_Project::setParameterAtIndex(ProjectParameter *in_parameter, int in_index)
{
    QTableWidgetItem        *tmp_widget_item = NULL;

    tmp_widget_item = new QTableWidgetItem;
    tmp_widget_item->setText(in_parameter->valueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME));
    tmp_widget_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_parameter));
    _m_parameters_table_widget->setItem(in_index, 0, tmp_widget_item);

    tmp_widget_item = new QTableWidgetItem;
    tmp_widget_item->setText(in_parameter->valueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE));
    tmp_widget_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_parameter));
    _m_parameters_table_widget->setItem(in_index, 1, tmp_widget_item);
}


void Frame_Project::setCampaignAtIndex(Campaign *in_campaign, int in_index)
{
    QTableWidgetItem        *tmp_widget_item = NULL;

    tmp_widget_item = new QTableWidgetItem;
    tmp_widget_item->setText(in_campaign->valueForKey(CAMPAIGNS_TABLE_SHORT_NAME));
    tmp_widget_item->setToolTip(in_campaign->valueForKey(CAMPAIGNS_TABLE_DESCRIPTION));
    tmp_widget_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_campaign));
    _m_campaigns_table_widget->setItem(in_index, 0, tmp_widget_item);
}


void Frame_Project::setSearchRecordAtIndex(Record *in_record, int in_index)
{
    QTableWidgetItem        *tmp_icon_widget_item = NULL;
    QTableWidgetItem        *tmp_widget_item = NULL;

    tmp_icon_widget_item = new QTableWidgetItem;
    tmp_widget_item = new QTableWidgetItem;

    if (in_record != NULL)
    {
        if (in_record->getEntityDefSignatureId() == TESTS_HIERARCHY_SIG_ID)
        {
            tmp_icon_widget_item->setIcon(QIcon(":/images/22x22/pellicule.png"));
            tmp_widget_item->setText(((TestHierarchy*)in_record)->valueForKey(TESTS_HIERARCHY_SHORT_NAME));
        }
        else if (in_record->getEntityDefSignatureId() == REQUIREMENTS_HIERARCHY_SIG_ID)
        {
            tmp_icon_widget_item->setIcon(QIcon(":/images/22x22/notes.png"));
            tmp_widget_item->setText(((RequirementHierarchy*)in_record)->valueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME));
        }
    }
    else
    {
        tmp_widget_item->setText(tr("Aucune données trouvées."));
    }

    tmp_icon_widget_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_record));
    _m_search_table_widget->setItem(in_index, 0, tmp_icon_widget_item);

    tmp_widget_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_record));
    _m_search_table_widget->setItem(in_index, 1, tmp_widget_item);
}



/**
  Ajout d'un parametre
**/
void Frame_Project::insertParameter()
{
    ProjectParameter     *tmp_parameter = new ProjectParameter(_m_project_version->project());

    _m_parameters_table_widget->insertRow(0);
    _m_project_version->project()->addParameter(tmp_parameter);

    setParameterAtIndex(tmp_parameter, 0);
    updateMenusActionsAndToolbar();
}



/**
  Suppression d'un parametre
**/
void Frame_Project::removeSelectedParameter()
{
    QModelIndex 			tmp_select_row = _m_parameters_table_widget->selectionModel()->currentIndex();
    int         			tmp_row_index = 0;
    QTableWidgetItem		*tmp_item = NULL;
    ProjectParameter     *tmp_parameter = NULL;

    if (tmp_select_row.isValid())
    {
        tmp_row_index = tmp_select_row.row();
        tmp_item = _m_parameters_table_widget->item(tmp_row_index, 0);
        if (tmp_item != NULL)
        {
            tmp_parameter = (ProjectParameter*)tmp_item->data(Qt::UserRole).value<void*>();
            if (tmp_parameter != NULL)
            {
                if (tmp_parameter->deleteRecord() == NOERR)
                {
                    _m_parameters_table_widget->removeRow(tmp_row_index);
                    _m_project_version->project()->removeParameter(tmp_parameter);
                }
            }
        }
    }
}



/**
 * Modification d'un parametre
 */
void Frame_Project::parameterItemChanged(QTableWidgetItem *in_widget_item)
{
    ProjectParameter     *tmp_parameter = NULL;
    const char				*tmp_param_column = NULL;

    if (in_widget_item != NULL)
    {
        tmp_parameter = (ProjectParameter*)in_widget_item->data(Qt::UserRole).value<void*>();
        if (tmp_parameter != NULL)
        {
            tmp_param_column = in_widget_item->text().toStdString().c_str();
            if (in_widget_item->column() == 0)
            {
                tmp_parameter->takeValueForKey(tmp_param_column, PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME);
                if (tmp_parameter->saveRecord() == NOERR)
                {
                    _m_project_version->project()->updateParametersNamesList();
                }
            }
            else if (in_widget_item->column() == 1)
            {
                tmp_parameter->takeValueForKey(tmp_param_column, PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE);
                tmp_parameter->saveRecord();
            }
        }
    }
}


/**
  Ajout d'une exigence fille
**/
void Frame_Project::insertChildRequirement()
{
    QModelIndex		index = _m_requirements_tree_view->selectionModel()->currentIndex();
    Hierarchy		*tmp_item = NULL;
    int				tmp_row = 0;

    if (index.isValid())
    {
        tmp_item = _m_requirements_project_tree_model->getItem(index);

        if (tmp_item != NULL)
            tmp_row = tmp_item->childCount(REQUIREMENTS_HIERARCHY_SIG_ID);

        if (!_m_requirements_project_tree_model->insertRow(tmp_row, index))
            return;

        _m_requirements_tree_view->selectionModel()->setCurrentIndex(_m_requirements_project_tree_model->index(tmp_row, 0, index), QItemSelectionModel::ClearAndSelect);
        showRequirementInfos(_m_requirements_tree_view->selectionModel()->currentIndex());
    }
}


/**
  Dérouler l'arbre des exigences
**/
void Frame_Project::expandRequirementsTree()
{
    _m_requirements_tree_view->expandAll();
}


/**
  Dérouler les exigences sélectionnées
**/
void Frame_Project::expandSelectedRequirementsTree()
{
    foreach(QModelIndex tmp_index, _m_requirements_tree_view->selectionModel()->selectedIndexes())
    {
        _m_requirements_tree_view->setExpandedIndex(tmp_index, true);
    }
}

/**
  Enrouler l'arbre des exigences
**/
void Frame_Project::collapseRequirementsTree()
{
    _m_requirements_tree_view->collapseAll();
}


/**
  Enrouler les exigences sélectionnées
**/
void Frame_Project::collapseSelectedRequirementsTree()
{
    foreach(QModelIndex tmp_index, _m_requirements_tree_view->selectionModel()->selectedIndexes())
    {
        _m_requirements_tree_view->setExpandedIndex(tmp_index, false);
    }
}



/**
  Ajout d'une exigence
**/
void Frame_Project::insertRequirement()
{
    insertRequirementAtIndex(_m_requirements_tree_view->selectionModel()->currentIndex());
}


/**
  Supprimer les exigences sélectionnées
**/
void Frame_Project::removeSelectedRequirements()
{
    removeRequirementsList(_m_requirements_tree_view->selectedRecords());
}



/**
  Suppression de la campagne sélectionnée
**/
void Frame_Project::removeSelectedCampaign()
{
    QModelIndex 			tmp_select_row = _m_campaigns_table_widget->selectionModel()->currentIndex();
    int         			tmp_row_index = 0;
    QTableWidgetItem		*tmp_item = NULL;
    Campaign			    *tmp_campaign = NULL;

    if (tmp_select_row.isValid())
    {
        tmp_row_index = tmp_select_row.row();
        tmp_item = _m_campaigns_table_widget->item(tmp_row_index, 0);
        if (tmp_item != NULL)
        {
            tmp_campaign = (Campaign*)tmp_item->data(Qt::UserRole).value<void*>();
            if (tmp_campaign != NULL)
            {
                if (QMessageBox::question(
                this,
                tr("Confirmation..."),
                tr("Etes-vous sûr(e) de vouloir supprimer la campagne \"%1\" ?\nToutes les exécutions associées seront supprimées.").arg(tmp_campaign->valueForKey(CAMPAIGNS_TABLE_SHORT_NAME)),
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No) == QMessageBox::Yes)
                {
                    _m_campaigns_table_widget->removeRow(tmp_row_index);
                    _m_project_version->removeCampaign(tmp_campaign);
                }
            }
        }
    }
}



/**
  Importer des exigences
**/
void Frame_Project::importRequirements()
{
    QStringList		tmp_columns_names;
    Form_Data_Import	*tmp_import_form = NULL;
    QList<Hierarchy*> tmp_records_list = _m_requirements_tree_view->selectedRecords();
    Hierarchy *tmp_requirement = NULL;

    RequirementHierarchy *tmp_parent_requirement = NULL;

    if (tmp_records_list.isEmpty())
    {
        if (_m_project_version->requirementsHierarchy().isEmpty() == false)
        {
            tmp_requirement = _m_project_version->child(_m_project_version->childCount(REQUIREMENTS_HIERARCHY_SIG_ID) - 1, REQUIREMENTS_HIERARCHY_SIG_ID);
        }
    }
    else
    {
        tmp_requirement = tmp_records_list[tmp_records_list.count() - 1];
    }

    if (tmp_requirement != NULL)
    {
        QMessageBox::information(this, tr("Information"), tr("Les exigences seront importées après l'exigence' \"%1\".").arg(tmp_requirement->valueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)));

        if (tmp_requirement->parent() != _m_project_version)
            tmp_parent_requirement = new RequirementHierarchy((RequirementHierarchy*)tmp_requirement->parent());
        else
            tmp_parent_requirement = new RequirementHierarchy(_m_project_version);
    }
    else
        tmp_parent_requirement = new RequirementHierarchy(_m_project_version);

    tmp_columns_names << tr("Nom de l'exigence") << tr("Description de l'exigence") << tr("Catégorie de l'exigence") << tr("Priorité de l'exigence");
    tmp_import_form = new Form_Data_Import(tmp_parent_requirement, tmp_columns_names, &requirement_entity_import, this);
    connect(tmp_import_form, SIGNAL(startImport(Hierarchy*)), this, SLOT(startImportRequirements(Hierarchy*)));
    connect(tmp_import_form, SIGNAL(importRecord(Hierarchy*, GenericRecord*, bool)), this, SLOT(importRequirement(Hierarchy*, GenericRecord*, bool)));

    tmp_import_form->show();
}



void Frame_Project::startImportRequirements(Hierarchy *in_parent)
{
    RequirementContent		*tmp_requirement_content = NULL;
    int				tmp_save_result = NOERR;

    RequirementHierarchy	*tmp_requirement_parent = NULL;

    tmp_requirement_parent = (RequirementHierarchy*)in_parent;

    cl_transaction_start(mainApp->m_session);

    tmp_requirement_content = new RequirementContent(_m_project_version);

    tmp_requirement_content->takeValueForKey(tr("Import d'exigences").toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME);
    tmp_requirement_content->takeValueForKey(tr("Import d'exigences").toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION);
    tmp_save_result = tmp_requirement_content->saveRecord();
    if (tmp_save_result == NOERR)
    {
        tmp_requirement_parent->setDataFromRequirementContent(tmp_requirement_content);
        tmp_save_result = tmp_requirement_parent->saveRecord();
    }

    if (tmp_save_result != NOERR)
    {
        cl_transaction_rollback(mainApp->m_session);

        disconnect(this, SLOT(startImportRequirements(Hierarchy*)));
        disconnect(this, SLOT(importRequirement(Hierarchy*, GenericRecord*, bool)));
    }
}


void Frame_Project::endImportRequirements(Hierarchy *in_parent)
{
    QList<Hierarchy*>	tmp_records_list = _m_requirements_tree_view->selectedRecords();
    Hierarchy		*tmp_requirement = NULL;
    Hierarchy		*tmp_parent = NULL;
    QModelIndex		tmp_model_index;
    int			tmp_index = 0;

    int			tmp_tr_result = NOERR;

    RequirementHierarchy	*tmp_requirement_parent = NULL;

    tmp_requirement_parent = (RequirementHierarchy*)in_parent;

    if (tmp_records_list.isEmpty())
    {
        if (_m_project_version->requirementsHierarchy().isEmpty() == false)
        {
            tmp_requirement = _m_project_version->child(_m_project_version->childCount(REQUIREMENTS_HIERARCHY_SIG_ID) - 1, REQUIREMENTS_HIERARCHY_SIG_ID);
        }
    }
    else
    {
        tmp_requirement = tmp_records_list[tmp_records_list.count() - 1];
    }

    if (tmp_requirement != NULL)
    {
        tmp_model_index = _m_requirements_project_tree_model->modelIndexForItem(tmp_requirement);
        if (tmp_model_index.isValid())
        {
            tmp_index = tmp_model_index.row() + 1;
            tmp_model_index = tmp_model_index.parent();
        }

        tmp_parent = tmp_requirement->parent();
    }
    else
    {
        tmp_index = _m_project_version->childCount(REQUIREMENTS_HIERARCHY_SIG_ID);
        tmp_parent = _m_project_version;
        tmp_model_index  = QModelIndex();
    }

    if (_m_requirements_project_tree_model->insertItem(tmp_index, tmp_model_index, tmp_requirement_parent))
    {
        tmp_tr_result = cl_transaction_commit(mainApp->m_session);
        if (tmp_tr_result != NOERR)
            QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), TR_SERVER_MESSAGE(cl_get_error_message(mainApp->m_session, tmp_tr_result)));
    }
    else
    {
        cl_transaction_rollback(mainApp->m_session);
    }
}



void Frame_Project::importRequirement(Hierarchy *in_parent, GenericRecord *in_record, bool in_last_record)
{
    RequirementContent		*tmp_requirement_content = NULL;
    int			tmp_save_result = NOERR;

    int			tmp_chils_count = 0;

    RequirementHierarchy	*tmp_requirement_parent = NULL;
    RequirementHierarchy	*tmp_requirement = NULL;

    RequirementHierarchy	*tmp_existing_requirement = NULL;

    QStringList                 tmp_str_list;

    if (in_parent != NULL && in_record != NULL)
    {
        tmp_requirement_parent = (RequirementHierarchy*)in_parent;

        if (is_empty_string(in_record->valueForKey(REQUIREMENT_IMPORT_SHORT_NAME)) == FALSE)
        {
            tmp_str_list = QString(in_record->valueForKey(REQUIREMENT_IMPORT_SHORT_NAME)).split(PARENT_HIERARCHY_SEPARATOR, QString::SkipEmptyParts);

            foreach(QString tmp_str_name, tmp_str_list)
            {
                if (!tmp_str_name.isEmpty())
                {
                    tmp_existing_requirement = (RequirementHierarchy*)tmp_requirement_parent->findItemWithValueForKey(tmp_str_name.toStdString().c_str(), REQUIREMENTS_HIERARCHY_SHORT_NAME, REQUIREMENTS_HIERARCHY_SIG_ID, false);
                    if (tmp_existing_requirement == NULL)
                    {
                        // Creation de l'exigence importée
                        tmp_requirement_content = new RequirementContent(_m_project_version);
                        if (tmp_requirement_content != NULL)
                        {
                            tmp_requirement_content->takeValueForKey(tmp_str_name.toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME);
                            tmp_requirement_content->takeValueForKey(in_record->valueForKey(REQUIREMENT_IMPORT_DESCRIPTION), REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION);
                            tmp_requirement_content->takeValueForKey(in_record->valueForKey(REQUIREMENT_IMPORT_CATEGORY_ID), REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID);
                            tmp_requirement_content->takeValueForKey(in_record->valueForKey(REQUIREMENT_IMPORT_PRIORITY_LEVEL), REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL);
                            tmp_save_result = tmp_requirement_content->saveRecord();
                            if (tmp_save_result == NOERR)
                            {
                                // Ajout de l'exigence à l'arborescence
                                tmp_requirement = new RequirementHierarchy(_m_project_version);
                                if (tmp_requirement != NULL)
                                {
                                    tmp_chils_count = tmp_requirement_parent->childCount(REQUIREMENTS_HIERARCHY_SIG_ID);

                                    tmp_requirement->setDataFromRequirementContent(tmp_requirement_content);

                                    tmp_requirement_parent->insertChildren(tmp_chils_count, 0, tmp_requirement);

                                    tmp_requirement_parent = tmp_requirement;
                                }
                            }

                            delete tmp_requirement_content;
                        }
                    }
                    else
                        tmp_requirement_parent = tmp_existing_requirement;
                }
            }
        }
    }

    if (tmp_save_result != NOERR)
    {
        cl_transaction_rollback(mainApp->m_session);

        disconnect(this, SLOT(startImportRequirements(Hierarchy*)));
        disconnect(this, SLOT(importRequirement(Hierarchy*, GenericRecord*, bool)));

        QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), TR_SERVER_MESSAGE(cl_get_error_message(mainApp->m_session, tmp_save_result)));

    }
    else if (in_last_record)
    {
        endImportRequirements(in_parent);
    }

}


/**
  Exporter des exigences
**/
void Frame_Project::exportRequirements()
{
    Form_Data_Export	*tmp_export_form = NULL;

    tmp_export_form = new Form_Data_Export(this);
    connect(tmp_export_form, SIGNAL(startExport(QString,QByteArray,QByteArray, QByteArray)), this, SLOT(startExportRequirements(QString,QByteArray,QByteArray, QByteArray)));
    tmp_export_form->show();
}



void Frame_Project::startExportRequirements(QString in_filepath, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char)
{
    QList<Hierarchy*>       tmp_records_list = Hierarchy::parentRecordsFromRecordsList(_m_requirements_tree_view->selectedRecords());

    QFile                   tmp_file;

    tmp_file.setFileName(in_filepath);
    if (tmp_file.open(QIODevice::WriteOnly))
    {
        // Entête
        tmp_file.write(QString(in_field_enclosing_char+tr("Nom")+in_field_enclosing_char+in_field_separator+
        in_field_enclosing_char+tr("Description")+in_field_enclosing_char+in_field_separator+
        in_field_enclosing_char+tr("Catégorie")+in_field_enclosing_char+in_field_separator+
        in_field_enclosing_char+tr("Priorité")+in_field_enclosing_char+in_record_separator).toStdString().c_str());

        writeRequirementsListToExportFile(tmp_file, castHierarchyList<RequirementHierarchy>(tmp_records_list), in_field_separator, in_record_separator, in_field_enclosing_char);

        tmp_file.close();
    }
    else
    {
        QMessageBox::critical(this, tr("Fichier non créé"), tr("L'ouverture du fichier en écriture est impossible (%1).").arg(tmp_file.errorString()));
    }
}



void Frame_Project::writeRequirementsListToExportFile(QFile & in_file, QList<RequirementHierarchy*> in_records_list, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char)
{
    QString                 tmp_str;

    QTextDocument           tmp_doc;

    Hierarchy               *tmp_parent = NULL;
    RequirementContent      *tmp_requirement_content = NULL;

    qint64                  tmp_bytes_write = 0;

    foreach(Hierarchy *tmp_record, in_records_list)
    {
        tmp_requirement_content = new RequirementContent(_m_project_version);
        if (tmp_requirement_content != NULL)
        {
            if (tmp_requirement_content->loadRecord(tmp_record->valueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID)) == NOERR)
            {
                // Nom de l'exigence
                tmp_str = "";
                tmp_parent = tmp_record;
                while (tmp_parent->parent() != NULL && tmp_parent->parent() != _m_project_version)
                {
                    tmp_parent = tmp_parent->parent();
                    tmp_str = QString(tmp_parent->valueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)) + PARENT_HIERARCHY_SEPARATOR + tmp_str;
                }
                tmp_str += QString(tmp_requirement_content->valueForKey(REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME));

                tmp_str = in_field_enclosing_char + tmp_str.replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
                tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
                if (tmp_bytes_write < 0)    break;

                // Separateur de champs
                tmp_bytes_write = in_file.write(in_field_separator);
                if (tmp_bytes_write < 0)    break;

                // Description de l'exigence
                tmp_doc.setHtml(tmp_requirement_content->valueForKey(REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION));
                tmp_str = in_field_enclosing_char + tmp_doc.toPlainText().replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
                tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
                if (tmp_bytes_write < 0)    break;

                // Separateur de champs
                tmp_bytes_write = in_file.write(in_field_separator);
                if (tmp_bytes_write < 0)    break;

                // Catégorie de l'exigence
                tmp_str = in_field_enclosing_char + QString(tmp_requirement_content->valueForKey(REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
                tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
                if (tmp_bytes_write < 0)    break;

                // Separateur de champs
                tmp_bytes_write = in_file.write(in_field_separator);
                if (tmp_bytes_write < 0)    break;

                // Priorité de l'exigence
                tmp_str = in_field_enclosing_char + QString(tmp_requirement_content->valueForKey(REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
                tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
                if (tmp_bytes_write < 0)    break;

                // Separateur d'enregistrements
                tmp_bytes_write = in_file.write(in_record_separator);

                // Exigences filles
                writeRequirementsListToExportFile(in_file, ((RequirementHierarchy*)tmp_record)->childs(), in_field_separator, in_record_separator, in_field_enclosing_char);
            }

            delete tmp_requirement_content;
        }
    }

}

void Frame_Project::updateMenusActionsAndToolbar()
{
    int		    tmp_requirements_selection_count = 0;
    int		    tmp_tests_selection_count = 0;
    int		    tmp_campaigns_selection_count = 0;
    int		    tmp_parameters_selection_count = 0;

    bool	    tmp_tests_writable = false;
    bool	    tmp_requirements_writable = false;
    bool	    tmp_campaigns_writable = false;

    tmp_requirements_selection_count = _m_requirements_tree_view->selectionModel()->selection().count();
    tmp_tests_selection_count = _m_tests_tree_view->selectionModel()->selection().count();
    tmp_campaigns_selection_count = _m_campaigns_table_widget->selectionModel()->selection().count();
    tmp_parameters_selection_count = _m_parameters_table_widget->selectionModel()->selection().count();

    tmp_tests_writable = this->projectVersion() != NULL && this->projectVersion()->project() != NULL && this->projectVersion()->project()->projectGrants() != NULL &&
    compare_values(this->projectVersion()->project()->projectGrants()->valueForKey(PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC), PROJECT_GRANT_WRITE) == 0;

    tmp_requirements_writable = this->projectVersion() != NULL && this->projectVersion()->project() != NULL && this->projectVersion()->project()->projectGrants() != NULL &&
    compare_values(this->projectVersion()->project()->projectGrants()->valueForKey(PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC), PROJECT_GRANT_WRITE) == 0;

    tmp_campaigns_writable = this->projectVersion() != NULL && this->projectVersion()->project() != NULL && this->projectVersion()->project()->projectGrants() != NULL &&
    compare_values(this->projectVersion()->project()->projectGrants()->valueForKey(PROJECTS_GRANTS_TABLE_MANAGE_CAMPAIGNS_INDIC), PROJECT_GRANT_WRITE) == 0;

    insertRequirementAction->setEnabled(tmp_requirements_writable);
    insertChildRequirementAction->setEnabled(tmp_requirements_selection_count == 1 && tmp_requirements_writable);
    importRequirementsAction->setEnabled(tmp_requirements_writable);
    exportRequirementsAction->setEnabled(tmp_requirements_selection_count > 0);
    expandSelectedRequirementsAction->setEnabled(tmp_requirements_selection_count > 0);
    collapseSelectedRequirementsAction->setEnabled(tmp_requirements_selection_count > 0);
    removeRequirementAction->setEnabled(tmp_requirements_selection_count > 0 && tmp_requirements_writable);
    selectDependantsTestsAction->setEnabled(tmp_requirements_selection_count > 0);

    insertTestAction->setEnabled(tmp_tests_writable);
#if defined(AUTOMATION_LIB) && (defined(_WINDOWS) || defined (WIN32))
    insertAutomatedTestAction->setEnabled(tmp_tests_writable);
#endif
    insertChildTestAction->setEnabled(tmp_tests_selection_count == 1 && tmp_tests_writable);
    importTestsAction->setEnabled(tmp_tests_writable);
    exportTestsAction->setEnabled(tmp_tests_selection_count > 0);
    expandSelectedTestsAction->setEnabled(tmp_tests_selection_count > 0);
    collapseSelectedTestsAction->setEnabled(tmp_tests_selection_count > 0);
    removeTestAction->setEnabled(tmp_tests_selection_count > 0 && tmp_tests_writable);
    selectLinkTestsAction->setEnabled(tmp_tests_selection_count > 0);

    insertCampaignAction->setEnabled(tmp_campaigns_writable);
    removeCampaignAction->setEnabled(tmp_campaigns_selection_count > 0 && tmp_campaigns_writable);

    insertParameterAction->setEnabled(tmp_tests_writable);
    removeParameterAction->setEnabled(tmp_tests_writable && tmp_parameters_selection_count > 0);
    _m_parameters_table_widget->setEnabled(tmp_tests_writable);

    historyNextAction->setEnabled(this->historyHasNextRecord());
    historyPreviousAction->setEnabled(this->historyHasPreviousRecord());

}



/**
  Supprimer la liste des exigences passée en paramètre
**/
void Frame_Project::removeRequirementsList(QList<Hierarchy*> in_records_list)
{
    QModelIndex         tmp_model_index;
    QMessageBox         *tmp_msg_box = NULL;
    QPushButton         *tmp_del_button = NULL;
    QPushButton         *tmp_del_all_button = NULL;
    QPushButton         *tmp_cancel_button = NULL;
    bool                tmp_delete_all = false;
    bool                tmp_delete_current = false;

    if (_m_project_version->isWritable(REQUIREMENTS_HIERARCHY_SIG_ID) && in_records_list.count() > 0)
    {
        tmp_msg_box = new QMessageBox(this);
        tmp_msg_box->setIcon(QMessageBox::Question);
        tmp_msg_box->setWindowTitle(tr("Confirmation..."));

        if (in_records_list.count() > 1)
            tmp_del_all_button = tmp_msg_box->addButton(tr("Supprimer tous les exigences"), QMessageBox::NoRole);

        tmp_del_button = tmp_msg_box->addButton(tr("Supprimer l'exigence"), QMessageBox::YesRole);
        tmp_cancel_button = tmp_msg_box->addButton(tr("Annuler"), QMessageBox::RejectRole);

        tmp_msg_box->setDefaultButton(tmp_cancel_button);

        foreach(Hierarchy *tmp_item, in_records_list)
        {
            tmp_delete_current = tmp_delete_all;

            if (tmp_delete_all == false)
            {
                tmp_msg_box->setText(tr("Etes-vous sûr(e) de vouloir supprimer l'exigence \"%1\" ?").arg(((RequirementHierarchy*)tmp_item)->valueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)));
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

            tmp_model_index = _m_requirements_project_tree_model->modelIndexForItem(tmp_item);
            if (tmp_delete_current && tmp_model_index.isValid())
                _m_requirements_project_tree_model->removeItem(tmp_model_index.row(), tmp_model_index.parent());
        }
    }
}


void Frame_Project::closeEvent(QCloseEvent *in_event)
{
    if (hideAllForms())
        in_event->accept();
    else
        in_event->ignore();
}


bool Frame_Project::isRequirementsPanelVisible()
{
    return _m_dock_requirements_widget->isVisible();
}


bool Frame_Project::isTestsPanelVisible()
{
    return _m_dock_tests_widget->isVisible();
}


bool Frame_Project::isCampaignsPanelVisible()
{
    return _m_dock_campaigns_widget->isVisible();
}


bool Frame_Project::isParametersPanelVisible()
{
    return _m_dock_parameters_widget->isVisible();
}


bool Frame_Project::isSearchPanelVisible()
{
    return _m_dock_search_widget->isVisible();
}


void Frame_Project::toogleRequirementsPanel()
{
    if (_m_dock_requirements_widget->isVisible())
        _m_dock_requirements_widget->hide();
    else
        _m_dock_requirements_widget->show();
}


void Frame_Project::toogleTestsPanel()
{
    if (_m_dock_tests_widget->isVisible())
        _m_dock_tests_widget->hide();
    else
        _m_dock_tests_widget->show();
}


void Frame_Project::toogleCampaignsPanel()
{
    if (_m_dock_campaigns_widget->isVisible())
        _m_dock_campaigns_widget->hide();
    else
        _m_dock_campaigns_widget->show();
}


void Frame_Project::toogleParametersPanel()
{
    if (_m_dock_parameters_widget->isVisible())
        _m_dock_parameters_widget->hide();
    else
        _m_dock_parameters_widget->show();
}


void Frame_Project::toogleSearchPanel()
{
    if (_m_dock_search_widget->isVisible())
        _m_dock_search_widget->hide();
    else
        _m_dock_search_widget->show();
}


void Frame_Project::showCylicRedundancyAlert()
{
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this, tr("Redondance cyclique..."),
    tr("Un des scénarios que vous voulez insérer :") \
    + "<ul><li>" + tr("est un lien vers un autre scénario") + "</li>" \
    + "<li>" + tr("fait directement ou indirectement référence au scénario de destination.") + "</li></ul>");
}

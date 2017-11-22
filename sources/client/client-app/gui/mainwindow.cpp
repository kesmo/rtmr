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

#include <QtGui>
#include <QNetworkProxy>

#include "mainwindow.h"
#include "gui/forms/form_logon.h"
#include "gui/forms/form_project.h"
#include "gui/forms/form_project_selection.h"
#include "gui/forms/form_manage_users.h"
#include "gui/forms/form_requirement.h"
#include "gui/forms/form_campaign.h"
#include "gui/forms/form_campaign_wizard.h"
#include "gui/forms/form_projects_reports.h"
#include "gui/forms/form_search_project.h"
#include "gui/forms/form_change_password.h"
#include "gui/forms/form_options.h"
#include "gui/forms/form_data_import.h"
#include "gui/forms/form_data_export.h"
#include "gui/forms/form_manage_customfields.h"

#include "projectgrant.h"

#include "client.h"
#include "session.h"

#include "projectparameter.h"


#include "../client-launcher/process_utils.h"

#define TEST_IMPORT_SHORT_NAME		"test_name"
#define TEST_IMPORT_DESCRIPTION	"test_description"
#define TEST_IMPORT_CATEGORY_ID	"test_category"
#define TEST_IMPORT_PRIORITY_LEVEL "test_priority"
#define ACTION_IMPORT_DESCRIPTION	"action_description"
#define ACTION_IMPORT_WAIT_RESULT	"action_result"

static const char *TEST_COLUMNS_IMPORT[] = {TEST_IMPORT_SHORT_NAME, TEST_IMPORT_DESCRIPTION, TEST_IMPORT_CATEGORY_ID, TEST_IMPORT_PRIORITY_LEVEL,
                                            ACTION_IMPORT_DESCRIPTION, ACTION_IMPORT_WAIT_RESULT,
                                            TESTS_CONTENTS_TABLE_STATUS,
                                            TESTS_CONTENTS_TABLE_AUTOMATED,
                                            TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND,
                                            TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND_PARAMETERS,
                                            TESTS_CONTENTS_TABLE_TYPE,
                                            TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE,
                                            AUTOMATED_ACTIONS_TABLE_WINDOW_ID,
                                            AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE,
                                            AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA,
                                            AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY};
static const entity_def test_entity_import = {0, 0, 0, 0, TEST_COLUMNS_IMPORT, 0, 0, sizeof(TEST_COLUMNS_IMPORT)/sizeof(char*)};

#define REQUIREMENT_IMPORT_SHORT_NAME		"requirement_name"
#define REQUIREMENT_IMPORT_DESCRIPTION	"requirement_description"
#define REQUIREMENT_IMPORT_CATEGORY_ID	"requirement_category"
#define REQUIREMENT_IMPORT_PRIORITY_LEVEL "requirement_priority"

static const char *REQUIREMENT_COLUMNS_IMPORT[] = {REQUIREMENT_IMPORT_SHORT_NAME, REQUIREMENT_IMPORT_DESCRIPTION, REQUIREMENT_IMPORT_CATEGORY_ID, REQUIREMENT_IMPORT_PRIORITY_LEVEL};
static const entity_def requirement_entity_import = {0, 0, 0, 0, REQUIREMENT_COLUMNS_IMPORT, 0, 0, sizeof(REQUIREMENT_COLUMNS_IMPORT)/sizeof(char*)};

QProgressDialog	*m_progress_dialog = NULL;

void updateProgressBar(unsigned long long int in_index, unsigned long long int in_count, unsigned long long int in_step, const char* in_message)
{
    if (m_progress_dialog == NULL)
    {
        m_progress_dialog = new QProgressDialog("Loading...", "Cancel", 0, 100);
        QProgressBar	*tmp_progress_bar = new QProgressBar(m_progress_dialog);
        tmp_progress_bar->setFormat("%v/%m");
        m_progress_dialog->setWindowModality(Qt::ApplicationModal);
        m_progress_dialog->setBar(tmp_progress_bar);
        //m_progress_dialog->setStyleSheet("QProgressBar {border:1px solid #3a3e69}; QProgressBar::chunk {background: #8aa4d5};");
        m_progress_dialog->show();
    }

    m_progress_dialog->setLabelText(in_message);

    if (in_count > 0)
        m_progress_dialog->setMaximum(in_count);

    if (in_step > 0)
        m_progress_dialog->setValue(m_progress_dialog->value() + in_step);
    else
        m_progress_dialog->setValue(in_index);

    qApp->processEvents();

    if (m_progress_dialog->maximum() > 0
            && m_progress_dialog->value() >= m_progress_dialog->maximum())
    {
        m_progress_dialog->close();
        delete m_progress_dialog;
        m_progress_dialog = NULL;
    }
}

/**
  Constructeur
*/
MainWindow::MainWindow() : QMainWindow(),
    _m_keepalive_timer_id(0),
    _m_dock_tests_widget(NULL),
    _m_dock_requirements_widget(NULL),
    _m_dock_campaigns_widget(NULL),
    _m_dock_parameters_widget(NULL),
    _m_dock_search_widget(NULL),
    _m_project_version(NULL),
    _m_tests_project_tree_model(NULL),
    _m_tests_tree_view(NULL),
    _m_requirements_project_tree_model(NULL),
    _m_requirements_tree_view(NULL),
    _m_campaigns_table_widget(NULL),
    _m_parameters_table_widget(NULL),
    _m_search_table_widget(NULL)
{
#ifndef Q_WS_MAC
#if defined(NETSCAPE_PLUGIN)
    loadStyleSheet(false);
#else
    loadStyleSheet(true);
#endif
#endif

    setWindowIcon(QIcon(QPixmap(QString::fromUtf8(":/images/client.png"))));

    mdiArea = new QMdiArea(this);
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mdiArea);
    windowMapper = new QSignalMapper(this);
    connect(windowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveSubWindow(QWidget*)));

    setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);

    setWindowTitle("R.T.M.R");

    createMenus();
    createToolBar();
    createStatusBar();

    readSettings();

    updateMenus();

#ifdef Q_WS_MAC
    setUnifiedTitleAndToolBarOnMac(true);
#endif

    _m_previous_ping_status = NOERR;

    // Lecture de la configuration
    Session::instance()->initialize();
}


MainWindow::~MainWindow()
{
    closeProject();
    delete mdiArea;
    delete windowMapper;
}


void MainWindow::setActiveSubWindow(QWidget *in_window)
{
    QMdiSubWindow* tmp_child_window = dynamic_cast<QMdiSubWindow*>(in_window);

    if (tmp_child_window){
        mdiArea->setActiveSubWindow(tmp_child_window);
    }
}

void MainWindow::loadStyleSheet(bool loadFile)
{
    QFile	    tmp_css_file( "standard.css" );


    // Gestion de la feuille de style
    if ( loadFile && tmp_css_file.open( QIODevice::ReadOnly | QIODevice::Text ))
    {
        qApp->setStyleSheet( tmp_css_file.readAll());
        tmp_css_file.close( );
    }
    else
    {
        tmp_css_file.setFileName( ":/standard.css" );
        if ( tmp_css_file.open( QIODevice::ReadOnly | QIODevice::Text ))
        {
            qApp->setStyleSheet( tmp_css_file.readAll());
            tmp_css_file.close( );
        }
    }

}

void MainWindow::createDockWidgets()
{
    QSettings settings("rtmr", "");

    delete _m_dock_tests_widget;
    delete _m_dock_requirements_widget;
    delete _m_dock_campaigns_widget;
    delete _m_dock_parameters_widget;
    delete _m_dock_search_widget;

    _m_dock_tests_widget = new QDockWidget(tr("Scénarios et cas de test"), this);
    _m_dock_requirements_widget = new QDockWidget(tr("Exigences"), this);
    _m_dock_campaigns_widget = new QDockWidget(tr("Campagnes"), this);
    _m_dock_parameters_widget = new QDockWidget(tr("Paramètres de projet"), this);
    _m_dock_search_widget = new QDockWidget(tr("Résultats de la recherche"), this);

    addDockWidget((Qt::DockWidgetArea)settings.value("dock_tests_area", Qt::RightDockWidgetArea).toInt(), _m_dock_tests_widget);
    addDockWidget((Qt::DockWidgetArea)settings.value("dock_parameters_area", Qt::RightDockWidgetArea).toInt(), _m_dock_parameters_widget);
    addDockWidget((Qt::DockWidgetArea)settings.value("dock_requirements_area", Qt::LeftDockWidgetArea).toInt(), _m_dock_requirements_widget);
    addDockWidget((Qt::DockWidgetArea)settings.value("dock_campaigns_area", Qt::LeftDockWidgetArea).toInt(), _m_dock_campaigns_widget);
    addDockWidget((Qt::DockWidgetArea)settings.value("dock_search_area", Qt::LeftDockWidgetArea).toInt(), _m_dock_search_widget);

    createPanels();
    createContextualMenus();
}

void MainWindow::loadPannelProjectDatas()
{
    _m_project_version->loadProjectVersionDatas(&updateProgressBar);
    if (m_progress_dialog){
        delete m_progress_dialog;
        m_progress_dialog = NULL;
    }

    // Tests
    delete _m_tests_project_tree_model;

    _m_tests_project_tree_model = new RecordsTreeModel(_m_project_version, TESTS_HIERARCHY_SIG_ID);
    _m_tests_tree_view->setModel(_m_tests_project_tree_model);
    connect(_m_tests_tree_view, SIGNAL(userEnterIndex(QModelIndex)), this, SLOT(showTestInfos(QModelIndex)));
    connect(_m_tests_tree_view, SIGNAL(delKeyPressed(QList<Hierarchy*>)), this, SLOT(removeTestsList(QList<Hierarchy*>)));
    connect(_m_tests_tree_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateMenusActionsAndToolbar()));
    connect(_m_tests_project_tree_model, SIGNAL(cyclicRedundancy()), this, SLOT(showCylicRedundancyAlert()));

    // Exigences
    delete _m_requirements_project_tree_model;

    _m_requirements_project_tree_model = new RecordsTreeModel(_m_project_version, REQUIREMENTS_HIERARCHY_SIG_ID);
    _m_requirements_tree_view->setModel(_m_requirements_project_tree_model);
    connect(_m_requirements_tree_view, SIGNAL(userEnterIndex(QModelIndex)), this, SLOT(showRequirementInfos(QModelIndex)));
    connect(_m_requirements_tree_view, SIGNAL(delKeyPressed(QList<Hierarchy*>)), this, SLOT(removeRequirementsList(QList<Hierarchy*>)));
    connect(_m_requirements_tree_view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateMenusActionsAndToolbar()));

    // Parametres
    _m_parameters_table_widget->setRowCount(0);
    foreach(ProjectParameter *tmp_project_parameter, _m_project_version->project()->parametersList())
    {
        _m_parameters_table_widget->insertRow(_m_parameters_table_widget->rowCount());
        setParameterAtIndex(tmp_project_parameter, _m_parameters_table_widget->rowCount() - 1);
    }
    connect(_m_parameters_table_widget->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateMenusActionsAndToolbar()));

    // Campagnes
    _m_campaigns_table_widget->setRowCount(0);
    foreach(Campaign *tmp_campaign, _m_project_version->campaignsList())
    {
        _m_campaigns_table_widget->insertRow(_m_campaigns_table_widget->rowCount());
        setCampaignAtIndex(tmp_campaign, _m_campaigns_table_widget->rowCount() - 1);
    }
    connect(_m_campaigns_table_widget->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(updateMenusActionsAndToolbar()));

    // Recherche
    _m_search_table_widget->setRowCount(0);
}

void MainWindow::createPanels()
{
    QStringList     tmp_parameters_list_headers;

    // Exigences
    createRequirementsTreeView();

    // Tests
    createTestsTreeView();

    // Campagnes
    delete _m_campaigns_table_widget;

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
    delete _m_parameters_table_widget;

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
    delete _m_search_table_widget;

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


    _m_dock_requirements_widget->setObjectName("requirement_dock_widget");
    _m_dock_requirements_widget->setWindowTitle(tr("Exigences"));
    _m_dock_requirements_widget->setWidget(_m_requirements_tree_view);

    _m_dock_campaigns_widget->setObjectName("campaign_dock_widget");
    _m_dock_campaigns_widget->setWindowTitle(tr("Campagnes"));
    _m_dock_campaigns_widget->setWidget(_m_campaigns_table_widget);

    _m_dock_tests_widget->setObjectName("test_dock_widget");
    _m_dock_tests_widget->setWindowTitle(tr("Scénarios et cas de test"));
    _m_dock_tests_widget->setWidget(_m_tests_tree_view);

    _m_dock_parameters_widget->setObjectName("parameter_dock_widget");
    _m_dock_parameters_widget->setWindowTitle(tr("Paramètres de projet"));
    _m_dock_parameters_widget->setWidget(_m_parameters_table_widget);

    _m_dock_search_widget->setObjectName("search_dock_widget");
    _m_dock_search_widget->setWindowTitle(tr("Résultats de la recherche"));
    _m_dock_search_widget->setWidget(_m_search_table_widget);
}



void MainWindow::createRequirementsTreeView()
{
    delete _m_requirements_tree_view;

    _m_requirements_tree_view = new RecordsTreeView();
    _m_requirements_tree_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    _m_requirements_tree_view->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    _m_requirements_tree_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _m_requirements_tree_view->setDragEnabled(true);
    _m_requirements_tree_view->setAcceptDrops(true);
    _m_requirements_tree_view->setDropIndicatorShown(true);
    _m_requirements_tree_view->setHeaderHidden(true);
    _m_requirements_tree_view->setExpandsOnDoubleClick(false);
}


void MainWindow::createTestsTreeView()
{
    delete _m_tests_tree_view;

    _m_tests_tree_view = new RecordsTreeView();
    _m_tests_tree_view->setSelectionBehavior(QAbstractItemView::SelectRows);
    _m_tests_tree_view->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred));
    _m_tests_tree_view->setSelectionMode(QAbstractItemView::ExtendedSelection);
    _m_tests_tree_view->setDragEnabled(true);
    _m_tests_tree_view->setAcceptDrops(true);
    _m_tests_tree_view->setDropIndicatorShown(true);
    _m_tests_tree_view->setHeaderHidden(true);
    _m_tests_tree_view->setExpandsOnDoubleClick(false);
}


void MainWindow::setParameterAtIndex(ProjectParameter *in_parameter, int in_index)
{
    QTableWidgetItem        *tmp_widget_item = NULL;

    tmp_widget_item = new QTableWidgetItem;
    tmp_widget_item->setText(in_parameter->getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME));
    tmp_widget_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_parameter));
    _m_parameters_table_widget->setItem(in_index, 0, tmp_widget_item);

    tmp_widget_item = new QTableWidgetItem;
    tmp_widget_item->setText(in_parameter->getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE));
    tmp_widget_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_parameter));
    _m_parameters_table_widget->setItem(in_index, 1, tmp_widget_item);
}


void MainWindow::setCampaignAtIndex(Campaign *in_campaign, int in_index)
{
    QTableWidgetItem        *tmp_widget_item = NULL;

    tmp_widget_item = new QTableWidgetItem;
    tmp_widget_item->setText(in_campaign->getValueForKey(CAMPAIGNS_TABLE_SHORT_NAME));
    tmp_widget_item->setToolTip(in_campaign->getValueForKey(CAMPAIGNS_TABLE_DESCRIPTION));
    tmp_widget_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_campaign));
    _m_campaigns_table_widget->setItem(in_index, 0, tmp_widget_item);
}


void MainWindow::createToolBar()
{
    QList<QAction*>		tmp_menu_connection;
    QList<QAction*>		tmp_menu_fichier;
    QList<QAction*>		tmp_menu_test = menuTests->actions();
    QList<QAction*>		tmp_menu_requirements = menuRequirements->actions();

    mainToolBar = new QToolBar();
    mainToolBar->setObjectName("mainToolBar");

    tmp_menu_connection.append(actionConnection);
    tmp_menu_connection.append(actionDeconnection);

    tmp_menu_fichier.append(actionEnregistrerFenetreCourante);

    tmp_menu_test.removeOne(importTestsAction);
    tmp_menu_test.removeOne(exportTestsAction);
    tmp_menu_test.removeOne(printTestsAction);
    tmp_menu_test.removeOne(saveAsHtmlTestsAction);

    tmp_menu_requirements.removeOne(importRequirementsAction);
    tmp_menu_requirements.removeOne(exportRequirementsAction);
    tmp_menu_requirements.removeOne(printRequirementsAction);
    tmp_menu_requirements.removeOne(saveAsHtmlRequirementsAction);

    mainToolBar->addActions(tmp_menu_connection);
    mainToolBar->addSeparator();
    mainToolBar->addActions(tmp_menu_fichier);
    mainToolBar->addSeparator();

    mainToolBar->addActions(tmp_menu_test);
    mainToolBar->addSeparator();

    mainToolBar->addActions(tmp_menu_requirements);
    mainToolBar->addSeparator();

    mainToolBar->addActions(menuCampaigns->actions());
    mainToolBar->addSeparator();

    mainToolBar->addActions(menuProjet->actions());
    mainToolBar->addSeparator();

    mainToolBar->addActions(menuHistory->actions());
    mainToolBar->addSeparator();

    toolbarSeparatorAboutAction = new QAction(this);
    toolbarSeparatorAboutAction->setSeparator(true);

    mainToolBar->addActions(menuHelp->actions());

    addToolBar(mainToolBar);
}


void MainWindow::createStatusBar()
{
    statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
}

void MainWindow::createContextualMenus()
{
    _m_tests_tree_view->addAction(insertTestAction);
#ifdef AUTOMATION_ACTIVATED
    _m_tests_tree_view->addAction(insertAutomatedTestAction);
#endif
    _m_tests_tree_view->addAction(removeTestAction);
    _m_tests_tree_view->addAction(insertChildTestAction);
    _m_tests_tree_view->addAction(expandAllTestsAction);
    _m_tests_tree_view->addAction(expandSelectedTestsAction);
    _m_tests_tree_view->addAction(collapseAllTestsAction);
    _m_tests_tree_view->addAction(collapseSelectedTestsAction);
    _m_tests_tree_view->addAction(selectLinkTestsAction);

    _m_tests_tree_view->addAction(importExportTestsActions);
    importExportTestsActions->setMenu(new QMenu);
    importExportTestsActions->menu()->addAction(importTestsAction);
    importExportTestsActions->menu()->addAction(exportTestsAction);
    importExportTestsActions->menu()->addAction(printTestsAction);
    importExportTestsActions->menu()->addAction(saveAsHtmlTestsAction);
    _m_tests_tree_view->setContextMenuPolicy(Qt::ActionsContextMenu);

    _m_requirements_tree_view->addAction(insertRequirementAction);
    _m_requirements_tree_view->addAction(removeRequirementAction);
    _m_requirements_tree_view->addAction(insertChildRequirementAction);
    _m_requirements_tree_view->addAction(expandAllRequirementsAction);
    _m_requirements_tree_view->addAction(expandSelectedRequirementsAction);
    _m_requirements_tree_view->addAction(collapseAllRequirementsAction);
    _m_requirements_tree_view->addAction(collapseSelectedRequirementsAction);
    _m_requirements_tree_view->addAction(selectDependantsTestsAction);

    _m_requirements_tree_view->addAction(importExportRequirementsActions);
    importExportRequirementsActions->setMenu(new QMenu);
    importExportRequirementsActions->menu()->addAction(importRequirementsAction);
    importExportRequirementsActions->menu()->addAction(exportRequirementsAction);
    importExportRequirementsActions->menu()->addAction(printRequirementsAction);
    importExportRequirementsActions->menu()->addAction(saveAsHtmlRequirementsAction);
    _m_requirements_tree_view->setContextMenuPolicy(Qt::ActionsContextMenu);

    _m_campaigns_table_widget->addAction(insertCampaignAction);
    _m_campaigns_table_widget->addAction(removeCampaignAction);
    _m_campaigns_table_widget->setContextMenuPolicy(Qt::ActionsContextMenu);

    _m_parameters_table_widget->addAction(insertParameterAction);
    _m_parameters_table_widget->addAction(removeParameterAction);
    _m_parameters_table_widget->setContextMenuPolicy(Qt::ActionsContextMenu);
}


void MainWindow::createMenus()
{
#ifdef Q_WS_MAC
    menuBar = new QMenuBar(0);
#else
    menuBar = new QMenuBar(this);
#endif

    /* Menu fichier */
    actionConnection = new QAction(this);
    connect(actionConnection, SIGNAL(triggered()), this, SLOT(logon()));

    actionChangePassword = new QAction(this);
    connect(actionChangePassword, SIGNAL(triggered()), this, SLOT(changePassword()));
    actionChangePassword->setEnabled(false);

    actionDeconnection = new QAction(this);
    actionDeconnection->setEnabled(false);
    connect(actionDeconnection, SIGNAL(triggered()), this, SLOT(logoff()));

    actionEnregistrerFenetreCourante = new QAction(this);
    actionEnregistrerFenetreCourante->setEnabled(false);
    connect(actionEnregistrerFenetreCourante, SIGNAL(triggered()), this, SLOT(saveCurrentChildWindow()));

    actionFermerFenetreCourante = new QAction(this);
    actionFermerFenetreCourante->setEnabled(false);
    connect(actionFermerFenetreCourante, SIGNAL(triggered()), mdiArea, SLOT(closeActiveSubWindow()));

    actionFermerFenetres = new QAction(this);
    actionFermerFenetres->setEnabled(false);
    connect(actionFermerFenetres, SIGNAL(triggered()), mdiArea, SLOT(closeAllSubWindows()));

    actionNouveau_projet = new QAction(this);
    actionNouveau_projet->setEnabled(false);
    connect(actionNouveau_projet, SIGNAL(triggered()), this, SLOT(newProject()));

    actionOuvrir_projet = new QAction(this);
    actionOuvrir_projet->setEnabled(false);
    connect(actionOuvrir_projet, SIGNAL(triggered()), this, SLOT(openProject()));

    actionExporter_projet = new QAction(this);
    actionExporter_projet->setEnabled(false);
    connect(actionExporter_projet, SIGNAL(triggered()), this, SLOT(exportProject()));

    actionImporter_projet = new QAction(this);
    actionImporter_projet->setEnabled(false);
    connect(actionImporter_projet, SIGNAL(triggered()), this, SLOT(importProject()));

    actionFermer_projet = new QAction(this);
    actionFermer_projet->setEnabled(false);
    connect(actionFermer_projet, SIGNAL(triggered()), this, SLOT(closeProject()));

    actionQuitter = new QAction(this);
    connect(actionQuitter, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));

    menuFichier = new QMenu(menuBar);
    menuFichier->addAction(actionConnection);
    menuFichier->addAction(actionChangePassword);
    menuFichier->addAction(actionDeconnection);
    menuFichier->addSeparator();
    menuFichier->addAction(actionEnregistrerFenetreCourante);
    menuFichier->addAction(actionFermerFenetreCourante);
    menuFichier->addAction(actionFermerFenetres);
    menuFichier->addSeparator();
    menuFichier->addAction(actionNouveau_projet);
    menuFichier->addAction(actionOuvrir_projet);
    menuFichier->addAction(actionExporter_projet);
    menuFichier->addAction(actionImporter_projet);
    menuFichier->addAction(actionFermer_projet);
    menuFichier->addSeparator();
    menuFichier->addAction(actionQuitter);

    // Menu des tests
    insertTestAction = new QAction(this);
    connect(insertTestAction, SIGNAL(triggered()), this, SLOT(insertTest()));

#ifdef AUTOMATION_ACTIVATED
    insertAutomatedTestAction = new QAction(this);
    connect(insertAutomatedTestAction, SIGNAL(triggered()), this, SLOT(insertAutomatedTest()));
#endif
    removeTestAction = new QAction(this);
    connect(removeTestAction, SIGNAL(triggered()), this, SLOT(removeSelectedTests()));

    insertChildTestAction = new QAction(this);
    connect(insertChildTestAction, SIGNAL(triggered()), this, SLOT(insertChildTest()));

    expandAllTestsAction = new QAction(this);
    connect(expandAllTestsAction, SIGNAL(triggered()), this, SLOT(expandTestsTree()));

    expandSelectedTestsAction = new QAction(this);
    connect(expandSelectedTestsAction, SIGNAL(triggered()), this, SLOT(expandSelectedTestsTree()));

    collapseAllTestsAction = new QAction(this);
    connect(collapseAllTestsAction, SIGNAL(triggered()), this, SLOT(collapseTestsTree()));

    collapseSelectedTestsAction = new QAction(this);
    connect(collapseSelectedTestsAction, SIGNAL(triggered()), this, SLOT(collapseSelectedTestsTree()));

    selectLinkTestsAction = new QAction(this);
    connect(selectLinkTestsAction, SIGNAL(triggered()), this, SLOT(selectLinkTests()));

    importExportTestsActions = new QAction(this);
    importTestsAction = new QAction(this);
    connect(importTestsAction, SIGNAL(triggered()), this, SLOT(importTests()));

    exportTestsAction = new QAction(this);
    connect(exportTestsAction, SIGNAL(triggered()), this, SLOT(exportTests()));

    printTestsAction = new QAction(this);
    connect(printTestsAction, SIGNAL(triggered()), this, SLOT(printTests()));

    saveAsHtmlTestsAction = new QAction(this);
    connect(saveAsHtmlTestsAction, SIGNAL(triggered()), this, SLOT(saveTestsAsHtml()));

    menuTests = new QMenu(menuBar);
    menuTests->addAction(insertTestAction);
#ifdef AUTOMATION_ACTIVATED
    menuTests->addAction(insertAutomatedTestAction);
#endif
    menuTests->addAction(removeTestAction);
    menuTests->addAction(insertChildTestAction);
    menuTests->addAction(expandAllTestsAction);
    menuTests->addAction(expandSelectedTestsAction);
    menuTests->addAction(collapseAllTestsAction);
    menuTests->addAction(collapseSelectedTestsAction);
    menuTests->addAction(selectLinkTestsAction);
    menuTests->addAction(importTestsAction);
    menuTests->addAction(exportTestsAction);
    menuTests->addAction(printTestsAction);
    menuTests->addAction(saveAsHtmlTestsAction);

    // Menu des exigences
    insertRequirementAction = new QAction(this);
    connect(insertRequirementAction, SIGNAL(triggered()), this, SLOT(insertRequirement()));

    removeRequirementAction = new QAction(this);
    connect(removeRequirementAction, SIGNAL(triggered()), this, SLOT(removeSelectedRequirements()));

    insertChildRequirementAction = new QAction(this);
    connect(insertChildRequirementAction, SIGNAL(triggered()), this, SLOT(insertChildRequirement()));

    expandAllRequirementsAction = new QAction(this);
    connect(expandAllRequirementsAction, SIGNAL(triggered()), this, SLOT(expandRequirementsTree()));

    expandSelectedRequirementsAction = new QAction(this);
    connect(expandSelectedRequirementsAction, SIGNAL(triggered()), this, SLOT(expandSelectedRequirementsTree()));

    collapseAllRequirementsAction = new QAction(this);
    connect(collapseAllRequirementsAction, SIGNAL(triggered()), this, SLOT(collapseRequirementsTree()));

    collapseSelectedRequirementsAction = new QAction(this);
    connect(collapseSelectedRequirementsAction, SIGNAL(triggered()), this, SLOT(collapseSelectedRequirementsTree()));

    selectDependantsTestsAction = new QAction(this);
    connect(selectDependantsTestsAction, SIGNAL(triggered()), this, SLOT(selectDependantsTestsFromSelectedRequirements()));

    importExportRequirementsActions = new QAction(this);
    importRequirementsAction = new QAction(this);
    connect(importRequirementsAction, SIGNAL(triggered()), this, SLOT(importRequirements()));

    exportRequirementsAction = new QAction(this);
    connect(exportRequirementsAction, SIGNAL(triggered()), this, SLOT(exportRequirements()));

    printRequirementsAction = new QAction(this);
    connect(printRequirementsAction, SIGNAL(triggered()), this, SLOT(printRequirements()));

    saveAsHtmlRequirementsAction = new QAction(this);
    connect(saveAsHtmlRequirementsAction, SIGNAL(triggered()), this, SLOT(saveRequirementsAsHtml()));

    menuRequirements = new QMenu(menuBar);
    menuRequirements->addAction(insertRequirementAction);
    menuRequirements->addAction(removeRequirementAction);
    menuRequirements->addAction(insertChildRequirementAction);
    menuRequirements->addAction(expandAllRequirementsAction);
    menuRequirements->addAction(expandSelectedRequirementsAction);
    menuRequirements->addAction(collapseAllRequirementsAction);
    menuRequirements->addAction(collapseSelectedRequirementsAction);
    menuRequirements->addAction(selectDependantsTestsAction);
    menuRequirements->addAction(importRequirementsAction);
    menuRequirements->addAction(exportRequirementsAction);
    menuRequirements->addAction(printRequirementsAction);
    menuRequirements->addAction(saveAsHtmlRequirementsAction);

    // Menu des campagnes
    insertCampaignAction = new QAction(this);
    connect(insertCampaignAction, SIGNAL(triggered()), this, SLOT(insertCampaign()));

    removeCampaignAction = new QAction(this);
    connect(removeCampaignAction, SIGNAL(triggered()), this, SLOT(removeSelectedCampaign()));

    menuCampaigns = new QMenu(menuBar);
    menuCampaigns->addAction(insertCampaignAction);
    menuCampaigns->addAction(removeCampaignAction);

    // Menu projet
    insertParameterAction = new QAction(this);
    connect(insertParameterAction, SIGNAL(triggered()), this, SLOT(insertParameter()));

    removeParameterAction = new QAction(this);
    connect(removeParameterAction, SIGNAL(triggered()), this, SLOT(removeSelectedParameter()));

    showProjectPropertiesAction = new QAction(this);
    connect(showProjectPropertiesAction, SIGNAL(triggered()), this, SLOT(showProjectProperties()));

    executionsReportsAction = new QAction(this);
    connect(executionsReportsAction, SIGNAL(triggered()), this, SLOT(executionReports()));

    showProjectBugsAction = new QAction(this);
    connect(showProjectBugsAction, SIGNAL(triggered()), this, SLOT(showProjectBugs()));

    searchProjectAction = new QAction(this);
    connect(searchProjectAction, SIGNAL(triggered()), this, SLOT(searchProject()));

    menuProjet = new QMenu(menuBar);
    menuProjet->setEnabled(false);

    menuProjet->addAction(insertParameterAction);
    menuProjet->addAction(removeParameterAction);
    menuProjet->addAction(showProjectPropertiesAction);
    menuProjet->addAction(executionsReportsAction);
    menuProjet->addAction(showProjectBugsAction);
    menuProjet->addAction(searchProjectAction);

    /* Menu Administration */
    manageUsersAction = new QAction(this);
    connect(manageUsersAction, SIGNAL(triggered()), this, SLOT(manageUsers()));

    projectsReportsAction = new QAction(this);
    connect(projectsReportsAction, SIGNAL(triggered()), this, SLOT(projectsReports()));

    manageCustomFieldsMenu = new QMenu(this);

    manageCustomTestsFieldsAction = new QAction(manageCustomFieldsMenu);
    manageCustomRequirementsFieldsAction = new QAction(manageCustomFieldsMenu);

    manageCustomFieldsMenu->addAction(manageCustomTestsFieldsAction);
    manageCustomFieldsMenu->addAction(manageCustomRequirementsFieldsAction);

    connect(manageCustomTestsFieldsAction, SIGNAL(triggered()), this, SLOT(manageCustomTestsFields()));
    connect(manageCustomRequirementsFieldsAction, SIGNAL(triggered()), this, SLOT(manageCustomRequirementsFields()));

    menuAdministration = new QMenu(menuBar);
    menuAdministration->setEnabled(false);
    menuAdministration->addAction(manageUsersAction);
    menuAdministration->addAction(projectsReportsAction);
    menuAdministration->addMenu(manageCustomFieldsMenu);

    /* Menu Historique */
    historyHomeAction = new QAction(this);
    connect(historyHomeAction, SIGNAL(triggered()), this, SLOT(showProjectProperties()));

    historyPreviousAction = new QAction(this);
    connect(historyPreviousAction, SIGNAL(triggered()), this, SLOT(showPreviousItemInProjectHistory()));

    historyNextAction = new QAction(this);
    connect(historyNextAction, SIGNAL(triggered()), this, SLOT(showNextItemInProjectHistory()));

    menuHistory = new QMenu(menuBar);
    menuHistory->addAction(historyHomeAction);
    menuHistory->addAction(historyPreviousAction);
    menuHistory->addAction(historyNextAction);

    // Menu outils
    actionShowOptions = new QAction(this);
    connect(actionShowOptions, SIGNAL(triggered()), this, SLOT(showOption()));

    menuSelectLanguage = new QMenu(this);
    createLanguageMenu();

    menuOutils = new QMenu(menuBar);
    menuOutils->addAction(actionShowOptions);
    menuOutils->addMenu(menuSelectLanguage);
    //menuOutils->setEnabled(false);

    // Fenetres
    menuChildsWindows = new QMenu(menuBar);
    connect(menuChildsWindows, SIGNAL(aboutToShow()), this, SLOT(menuChildsWindowsAboutToShow()));

    closeWindowAction = new QAction(this);
    connect(closeWindowAction, SIGNAL(triggered()), mdiArea, SLOT(closeActiveSubWindow()));

    closeAllWindowsAction = new QAction(this);
    connect(closeAllWindowsAction, SIGNAL(triggered()), mdiArea, SLOT(closeAllSubWindows()));

    tileWindowAction = new QAction(this);
    connect(tileWindowAction, SIGNAL(triggered()), mdiArea, SLOT(tileSubWindows()));

    cascadeWindowsAction = new QAction(this);
    connect(cascadeWindowsAction, SIGNAL(triggered()), mdiArea, SLOT(cascadeSubWindows()));

    nextWindowAction = new QAction(this);
    connect(nextWindowAction, SIGNAL(triggered()), mdiArea, SLOT(activateNextSubWindow()));

    previousWindowAction = new QAction(this);
    connect(previousWindowAction, SIGNAL(triggered()), mdiArea, SLOT(activatePreviousSubWindow()));

    separatorWindowAction = new QAction(this);
    separatorWindowAction->setSeparator(true);

    // Panels
    showRequirementsPanelAction = new QAction(this);
    showRequirementsPanelAction->setCheckable(true);
    connect(showRequirementsPanelAction, SIGNAL(triggered()), this, SLOT(toogleRequirementsPanel()));

    showTestsPanelAction = new QAction(this);
    showTestsPanelAction->setCheckable(true);
    connect(showTestsPanelAction, SIGNAL(triggered()), this, SLOT(toogleTestsPanel()));

    showCampaignsPanelAction = new QAction(this);
    showCampaignsPanelAction->setCheckable(true);
    connect(showCampaignsPanelAction, SIGNAL(triggered()), this, SLOT(toogleCampaignsPanel()));

    showParametersPanelAction = new QAction(this);
    showParametersPanelAction->setCheckable(true);
    connect(showParametersPanelAction, SIGNAL(triggered()), this, SLOT(toogleParametersPanel()));

    showSearchPanelAction = new QAction(this);
    showSearchPanelAction->setCheckable(true);
    connect(showSearchPanelAction, SIGNAL(triggered()), this, SLOT(toogleSearchPanel()));

    /* Menu Aide */
    aboutAction = new QAction(this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    menuHelp = new QMenu(menuBar);
    menuHelp->addAction(aboutAction);

    // Ajout des menus à la barre de menus
    menuBar->addAction(menuFichier->menuAction());
    menuBar->addAction(menuTests->menuAction());
    menuBar->addAction(menuRequirements->menuAction());
    menuBar->addAction(menuCampaigns->menuAction());
    menuBar->addAction(menuProjet->menuAction());
    menuBar->addAction(menuHistory->menuAction());
    //menuBar->addAction(menuAdministration->menuAction());
    menuBar->addAction(menuOutils->menuAction());
    menuBar->addAction(menuChildsWindows->menuAction());
    menuBar->addAction(menuHelp->menuAction());

    menuChildsWindowsAboutToShow();

    setMenuBar(menuBar);
}


void MainWindow::createLanguageMenu()
{
    QActionGroup* tmp_languages_actions = new QActionGroup(menuSelectLanguage);
    tmp_languages_actions->setExclusive(true);

    connect(tmp_languages_actions, SIGNAL(triggered(QAction *)), this, SLOT(slotLanguageChanged(QAction *)));

    // format systems language
    QString defaultLocale = QLocale::system().name();
    defaultLocale.truncate(defaultLocale.lastIndexOf('_'));

    QDir dir(":/languages");
    QStringList fileNames = dir.entryList(QStringList("client_*.qm"));

    fileNames << "client_fr.qm";

    for (int i = 0; i < fileNames.size(); ++i)
    {
        // get locale extracted by filename
        QString locale;
        locale = fileNames[i];
        locale.truncate(locale.lastIndexOf('.'));
        locale.remove(0, locale.indexOf('_') + 1);

        QString lang = QLocale::languageToString(QLocale(locale).language());

        QAction *action = new QAction(lang, this);
        action->setIcon(QIcon(":/images/22x22/" + locale + ".png"));
        action->setCheckable(true);
        action->setData(locale);

        menuSelectLanguage->addAction(action);
        tmp_languages_actions->addAction(action);

        // set default translators and language checked
        if (defaultLocale == locale)
        {
            action->setChecked(true);
        }
    }
}


void MainWindow::slotLanguageChanged(QAction* action)
{
    if(action != NULL)
    {
        // load the language dependant on the action content
        loadLanguage(action->data().toString());
    }
}

void MainWindow::retranslateUi()
{
    actionConnection->setIcon(QIcon(":/images/22x22/server.png"));
    actionConnection->setText(tr("Connexion..."));
    actionConnection->setShortcut(tr("Ctrl+K"));
    actionConnection->setStatusTip(tr("Connexion au référentiel d'exigences et de tests"));

    actionDeconnection->setIcon(QIcon(":/images/22x22/disconnect.png"));
    if (actionDeconnection->isEnabled() && Session::instance()->getClientSession() != NULL && is_empty_string(Session::instance()->getClientSession()->m_username) == FALSE)
        actionDeconnection->setText(tr("Déconn&exion") + " " + Session::instance()->getClientSession()->m_username);
    else
        actionDeconnection->setText(tr("Déconn&exion"));

    actionDeconnection->setShortcut(tr("Ctrl+E"));
    actionDeconnection->setStatusTip(tr("Déconnexion du référentiel"));

    actionEnregistrerFenetreCourante->setIcon(QIcon(":/images/22x22/save.png"));
    actionEnregistrerFenetreCourante->setText(tr("&Enregistrer"));
    actionEnregistrerFenetreCourante->setShortcut(tr("Ctrl+S"));
    actionEnregistrerFenetreCourante->setStatusTip(tr("Enregistrer"));

    actionFermerFenetreCourante->setIcon(QIcon(":/images/22x22/close.png"));
    actionFermerFenetreCourante->setText(tr("&Fermer"));
    actionFermerFenetreCourante->setStatusTip(tr("Fermer"));

    actionFermerFenetres->setIcon(QIcon(":/images/22x22/close.png"));
    actionFermerFenetres->setText(tr("&Fermer toutes les fenêtres"));
    actionFermerFenetres->setShortcut(tr("Ctrl+Shift+W"));
    actionFermerFenetres->setStatusTip(tr("Fermer toutes les fenêtres"));

    actionNouveau_projet->setIcon(QIcon(":/images/22x22/box.png"));
    actionNouveau_projet->setText(tr("&Nouveau projet..."));
    actionNouveau_projet->setShortcut(tr("Ctrl+N"));
    actionNouveau_projet->setStatusTip(tr("Créer un nouveau projet"));

    actionOuvrir_projet->setIcon(QIcon(":/images/22x22/open.png"));
    actionOuvrir_projet->setText(tr("&Ouvrir un projet..."));
    actionOuvrir_projet->setShortcut(tr("Ctrl+O"));
    actionOuvrir_projet->setStatusTip(tr("Ouvrir un projet existant"));

    actionExporter_projet->setIcon(QIcon(":/images/22x22/export.png"));
    actionExporter_projet->setText(tr("&Exporter le projet"));
    actionExporter_projet->setStatusTip(tr("Exporter le projet ouvert au format Xml"));

    actionImporter_projet->setIcon(QIcon(":/images/22x22/import.png"));
    actionImporter_projet->setText(tr("&Importer un projet"));
    actionImporter_projet->setStatusTip(tr("Importer un projet à partir d'un fichier au format Xml"));

    actionFermer_projet->setIcon(QIcon(":/images/22x22/close.png"));
    actionFermer_projet->setText(tr("Fermer le projet"));
    actionFermer_projet->setStatusTip(tr("Fermer le projet ouvert"));

    actionQuitter->setIcon(QIcon(":/images/22x22/quit.png"));
    actionQuitter->setText(tr("&Quitter"));
    actionQuitter->setShortcut(tr("Ctrl+Q"));

    menuFichier->setTitle(tr("&Fichier"));

    insertTestAction->setIcon(QIcon(":/images/22x22/pellicule_plus.png"));
    insertTestAction->setText(tr("&Nouveau test..."));
    insertTestAction->setStatusTip(tr("Ajouter un nouveau scénario/cas de test"));

#ifdef AUTOMATION_ACTIVATED
    insertAutomatedTestAction->setIcon(QIcon(":/images/22x22/pellicule_auto.png"));
    insertAutomatedTestAction->setText(tr("&Nouveau test automatisé..."));
    insertAutomatedTestAction->setStatusTip(tr("Ajouter un nouveau scénario/cas de test automatisé"));
#endif

    removeTestAction->setIcon(QIcon(":/images/22x22/pellicule_minus.png"));
    removeTestAction->setText(tr("&Supprimer le test sélectionné"));
    removeTestAction->setStatusTip(tr("Supprimer le scénario/cas de test sélectionné"));

    insertChildTestAction->setIcon(QIcon(":/images/22x22/small_pellicule_plus.png"));
    insertChildTestAction->setText(tr("&Ajouter un test enfant"));
    insertChildTestAction->setStatusTip(tr("Ajouter un test enfant au scénario/cas de test sélectionné"));

    expandAllTestsAction->setIcon(QIcon(":/images/22x22/pellicule_derouler.png"));
    expandAllTestsAction->setText(tr("&Dérouler tous les tests"));
    expandAllTestsAction->setStatusTip(tr("Dérouler toute l'arborescence des cas de tests"));
    expandAllTestsAction->setShortcut(tr("Ctrl+D"));

    expandSelectedTestsAction->setIcon(QIcon(":/images/22x22/pellicule_derouler.png"));
    expandSelectedTestsAction->setText(tr("&Dérouler les tests sélectionnés"));
    expandSelectedTestsAction->setStatusTip(tr("Dérouler les tests sélectionnés"));
    expandSelectedTestsAction->setShortcut(tr("Alt+D"));

    collapseAllTestsAction->setIcon(QIcon(":/images/22x22/pellicule_enrouler.png"));
    collapseAllTestsAction->setText(tr("&Enrouler tous les tests"));
    collapseAllTestsAction->setStatusTip(tr("Enrouler toute l'arborescence des cas de tests"));
    collapseAllTestsAction->setShortcut(tr("Ctrl+R"));

    collapseSelectedTestsAction->setIcon(QIcon(":/images/22x22/pellicule_enrouler.png"));
    collapseSelectedTestsAction->setText(tr("&Enrouler les tests sélectionnés"));
    collapseSelectedTestsAction->setStatusTip(tr("Enrouler les tests sélectionnés"));
    collapseSelectedTestsAction->setShortcut(tr("Alt+R"));

    selectLinkTestsAction->setIcon(QIcon(":/images/22x22/pellicule_dep.png"));
    selectLinkTestsAction->setText(tr("&Sélectionner les tests dépendants"));
    selectLinkTestsAction->setStatusTip(tr("Sélectionner les scénarios de test dépendants des scénarios sélectionnés"));

    importExportTestsActions->setText(tr("Import/Export"));
    importTestsAction->setIcon(QIcon(":/images/22x22/pellicule_import.png"));
    importTestsAction->setText(tr("&Importer des tests"));
    importTestsAction->setStatusTip(tr("Importer des tests depuis un fichier texte"));

    exportTestsAction->setIcon(QIcon(":/images/22x22/csv.png"));
    exportTestsAction->setText(tr("&Export texte..."));
    exportTestsAction->setStatusTip(tr("Exporter les tests sélectionnés vers un fichier texte"));

    printTestsAction->setIcon(QIcon(":/images/22x22/pdf.png"));
    printTestsAction->setText(tr("&Export PDF..."));
    printTestsAction->setStatusTip(tr("Exporter les tests sélectionnés vers un fichier pdf"));

    saveAsHtmlTestsAction->setIcon(QIcon(":/images/22x22/html.png"));
    saveAsHtmlTestsAction->setText(tr("&Export html..."));
    saveAsHtmlTestsAction->setStatusTip(tr("Exporter les tests sélectionnés vers un fichier html"));

    menuTests->setTitle(tr("&Tests"));

    insertRequirementAction->setIcon(QIcon(":/images/22x22/notes_plus.png"));
    insertRequirementAction->setText(tr("&Nouvelle exigence..."));
    insertRequirementAction->setStatusTip(tr("Ajouter une nouvelle exigence"));

    removeRequirementAction->setIcon(QIcon(":/images/22x22/notes_minus.png"));
    removeRequirementAction->setText(tr("&Supprimer l'exigence sélectionnée"));
    removeRequirementAction->setStatusTip(tr("Supprimer l'exigence sélectionnée"));

    insertChildRequirementAction->setIcon(QIcon(":/images/22x22/small_notes_plus.png"));
    insertChildRequirementAction->setText(tr("&Ajouter une exigence fille"));
    insertChildRequirementAction->setStatusTip(tr("Ajouter une exigence fille à l'exigence sélectionnée"));

    expandAllRequirementsAction->setIcon(QIcon(":/images/22x22/notes_derouler.png"));
    expandAllRequirementsAction->setText(tr("&Dérouler toutes les exigences"));
    expandAllRequirementsAction->setStatusTip(tr("Dérouler toute l'arborescence des exigences"));
    expandAllRequirementsAction->setShortcut(tr("Ctrl+Shift+D"));

    expandSelectedRequirementsAction->setIcon(QIcon(":/images/22x22/notes_derouler.png"));
    expandSelectedRequirementsAction->setText(tr("&Dérouler les exigences sélectionnées"));
    expandSelectedRequirementsAction->setStatusTip(tr("Dérouler les exigences sélectionnées"));
    expandSelectedRequirementsAction->setShortcut(tr("Alt+Shift+D"));

    collapseAllRequirementsAction->setIcon(QIcon(":/images/22x22/notes_enrouler.png"));
    collapseAllRequirementsAction->setText(tr("&Enrouler toutes les exigences"));
    collapseAllRequirementsAction->setStatusTip(tr("Enrouler toute l'arborescence des exigences"));
    collapseAllRequirementsAction->setShortcut(tr("Ctrl+Shift+R"));

    collapseSelectedRequirementsAction->setIcon(QIcon(":/images/22x22/notes_enrouler.png"));
    collapseSelectedRequirementsAction->setText(tr("&Enrouler les exigences sélectionnées"));
    collapseSelectedRequirementsAction->setStatusTip(tr("Enrouler les exigences sélectionnées"));
    collapseSelectedRequirementsAction->setShortcut(tr("Alt+Shift+R"));

    importExportRequirementsActions->setText(tr("Import/Export"));
    importRequirementsAction->setIcon(QIcon(":/images/22x22/notes_import.png"));
    importRequirementsAction->setText(tr("&Importer des exigences"));
    importRequirementsAction->setStatusTip(tr("Importer des exigence depuis un fichier texte"));

    exportRequirementsAction->setIcon(QIcon(":/images/22x22/csv.png"));
    exportRequirementsAction->setText(tr("&Export texte..."));
    exportRequirementsAction->setStatusTip(tr("Exporter les exigence sélectionnées vers un fichier texte"));

    selectDependantsTestsAction->setIcon(QIcon(":/images/22x22/notes_tests_dep.png"));
    selectDependantsTestsAction->setText(tr("&Sélectionner les tests dépendants"));
    selectDependantsTestsAction->setStatusTip(tr("Sélectionner les scénarios de test dépendants des exigences sélectionnées"));

    printRequirementsAction->setIcon(QIcon(":/images/22x22/pdf.png"));
    printRequirementsAction->setText(tr("&Export PDF..."));
    printRequirementsAction->setStatusTip(tr("Exporter les exigences sélectionnées vers un fichier pdf"));

    saveAsHtmlRequirementsAction->setIcon(QIcon(":/images/22x22/html.png"));
    saveAsHtmlRequirementsAction->setText(tr("&Export html..."));
    saveAsHtmlRequirementsAction->setStatusTip(tr("Exporter les exigences sélectionnées vers un fichier html"));

    menuRequirements->setTitle(tr("&Exigences"));

    insertCampaignAction->setIcon(QIcon(":/images/22x22/package_plus.png"));
    insertCampaignAction->setText(tr("&Nouvelle campagne..."));
    insertCampaignAction->setStatusTip(tr("Ajouter une nouvelle campagne de tests"));

    removeCampaignAction->setIcon(QIcon(":/images/22x22/package_minus.png"));
    removeCampaignAction->setText(tr("&Supprimer la campagne sélectionnée"));
    removeCampaignAction->setStatusTip(tr("Supprimer la campagne de tests sélectionnée"));

    menuCampaigns->setTitle(tr("&Campagnes"));

    insertParameterAction->setIcon(QIcon(":/images/22x22/config_plus.png"));
    insertParameterAction->setText(tr("&Nouveau paramètre"));
    insertParameterAction->setStatusTip(tr("Ajouter un nouveau paramètre au projet"));

    removeParameterAction->setIcon(QIcon(":/images/22x22/config_minus.png"));
    removeParameterAction->setText(tr("&Supprimer le paramètre sélectionné"));
    removeParameterAction->setStatusTip(tr("Supprimer le paramètre sélectionné"));

    showProjectPropertiesAction->setIcon(QIcon(":/images/22x22/properties.png"));
    showProjectPropertiesAction->setText(tr("&Propriétés du projet"));
    showProjectPropertiesAction->setStatusTip(tr("Afficher les propriétés du projet ouvert"));
    showProjectPropertiesAction->setShortcut(tr("Ctrl+P"));

    executionsReportsAction->setIcon(QIcon(":/images/22x22/stats.png"));
    executionsReportsAction->setText(tr("&Rapports d'exécutions"));
    executionsReportsAction->setStatusTip(tr("Afficher un rapport des campagnes d'exécutions du projet ouvert"));

    showProjectBugsAction->setIcon(QIcon(":/images/bug.png"));
    showProjectBugsAction->setText(tr("&Anomalies"));
    showProjectBugsAction->setStatusTip(tr("Afficher toutes les anomalies du projet ouvert"));

    searchProjectAction->setIcon(QIcon(":/images/22x22/search.png"));
    searchProjectAction->setText(tr("&Rechercher..."));
    searchProjectAction->setStatusTip(tr("Rechercher des données de scénarios, d'exigences"));
    searchProjectAction->setShortcut(tr("Ctrl+F"));

    menuProjet->setTitle(tr("&Projet"));

    manageUsersAction->setIcon(QIcon(":/images/22x22/users.png"));
    manageUsersAction->setText(tr("&Gestion des utilisateurs..."));
    manageUsersAction->setStatusTip(tr("Gestion des utilisateurs"));

    projectsReportsAction->setIcon(QIcon(":/images/22x22/stats.png"));
    projectsReportsAction->setText(tr("&Rapports de projets"));
    projectsReportsAction->setStatusTip(tr("Afficher un rapport des projets du référentiel"));

    manageCustomFieldsMenu->setIcon(QIcon(":/images/22x22/document.png"));
    manageCustomFieldsMenu->setTitle(tr("&Gestion des champs personnalisés"));
    manageCustomFieldsMenu->setStatusTip(tr("Gestion des champs personnalisés"));

    manageCustomTestsFieldsAction->setIcon(QIcon(":/images/22x22/document.png"));
    manageCustomTestsFieldsAction->setText(tr("Champs personnalisés de &tests"));
    manageCustomTestsFieldsAction->setStatusTip(tr("Gestion des champs personnalisés de tests"));

    manageCustomRequirementsFieldsAction->setIcon(QIcon(":/images/22x22/document.png"));
    manageCustomRequirementsFieldsAction->setText(tr("Champs personnalisés d'&exigences"));
    manageCustomRequirementsFieldsAction->setStatusTip(tr("Gestion des champs personnalisés d'exigences"));

    menuAdministration->setTitle(tr("&Administration"));

    historyHomeAction->setIcon(QIcon(":/images/22x22/home.png"));
    historyHomeAction->setText(tr("&Accueil"));
    historyHomeAction->setStatusTip(tr("Aller à  la page d'accueil du projet"));

    historyPreviousAction->setIcon(QIcon(":/images/22x22/previous.png"));
    historyPreviousAction->setText(tr("Page &précédente"));
    historyPreviousAction->setStatusTip(tr("Afficher la page précédente visitée"));

    historyNextAction->setIcon(QIcon(":/images/22x22/next.png"));
    historyNextAction->setText(tr("Page &suivante"));
    historyNextAction->setStatusTip(tr("Afficher la page suivante visitée"));

    menuHistory->setTitle(tr("&Navigation"));

    actionChangePassword->setIcon(QIcon(":/images/22x22/login.png"));
    actionChangePassword->setText(tr("Changer de &mot de passe"));
    actionChangePassword->setStatusTip(tr("Changer de mot de passe"));

    actionShowOptions->setIcon(QIcon(":/images/parameters.png"));
    actionShowOptions->setText(tr("&Options"));
    actionShowOptions->setStatusTip(tr("Afficher les options de l'application"));

    menuSelectLanguage->setStatusTip(tr("Choisir une nouvelle langue"));
    menuSelectLanguage->setTitle(tr("&Langues"));

    menuOutils->setTitle(tr("&Outils"));

    menuChildsWindows->setTitle(tr("&Fenêtres"));

    closeWindowAction->setText(tr("&Fermer"));
    closeWindowAction->setStatusTip(tr("Fermer la fenêtre active"));

    closeAllWindowsAction->setText(tr("Fermer &toutes les fenêtres"));
    closeAllWindowsAction->setStatusTip(tr("Fermer toutes les fenêtres"));

    tileWindowAction->setText(tr("&Côte à côte"));
    tileWindowAction->setStatusTip(tr("Disposer les fenêtres côte à côte"));

    cascadeWindowsAction->setText(tr("&Cascade"));
    cascadeWindowsAction->setStatusTip(tr("Mettre les fenêtres en cascade"));

    nextWindowAction->setText(tr("&Suivante"));
    nextWindowAction->setStatusTip(tr("Fenêtre suivante"));

    previousWindowAction->setText(tr("&Précédente"));
    previousWindowAction->setStatusTip(tr("Fenêtre précédente"));

    showRequirementsPanelAction->setText(tr("Panneau des exigences"));
    showRequirementsPanelAction->setStatusTip(tr("Afficher/masquer le panneau des exigences"));

    showTestsPanelAction->setText(tr("Panneau des scénarios"));
    showTestsPanelAction->setStatusTip(tr("Afficher/masquer le panneau des scénarios"));

    showCampaignsPanelAction->setText(tr("Panneau des campagnes"));
    showCampaignsPanelAction->setStatusTip(tr("Afficher/masquer le panneau des campagnes"));

    showParametersPanelAction->setText(tr("Panneau des paramètres"));
    showParametersPanelAction->setStatusTip(tr("Afficher/masquer le panneau des paramètres"));

    showSearchPanelAction->setText(tr("Panneau de recherche"));
    showSearchPanelAction->setStatusTip(tr("Afficher/masquer le panneau de recherche"));

    aboutAction->setIcon(QIcon(":/images/22x22/a_propos.png"));
    aboutAction->setText(tr("&A propos..."));
    aboutAction->setStatusTip(tr("A propos de R.T.M.R"));

    menuHelp->setTitle(tr("&?"));

}

void MainWindow::menuChildsWindowsAboutToShow()
{
    QList<QMdiSubWindow*>           tmp_subWindowsList = mdiArea->subWindowList();
    QAction                         *tmp_action = NULL;

    QAction							*tmpSeparatorWindowAction = new QAction(this);

    tmpSeparatorWindowAction->setSeparator(true);

    menuChildsWindows->clear();
    menuChildsWindows->addAction(closeWindowAction);
    menuChildsWindows->addAction(closeAllWindowsAction);
    menuChildsWindows->addSeparator();
    menuChildsWindows->addAction(tileWindowAction);
    menuChildsWindows->addAction(cascadeWindowsAction);
    menuChildsWindows->addSeparator();
    menuChildsWindows->addAction(nextWindowAction);
    menuChildsWindows->addAction(previousWindowAction);
    menuChildsWindows->addAction(tmpSeparatorWindowAction);

    menuChildsWindows->addAction(showRequirementsPanelAction);
    showRequirementsPanelAction->setChecked(isRequirementsPanelVisible());

    menuChildsWindows->addAction(showTestsPanelAction);
    showTestsPanelAction->setChecked(isTestsPanelVisible());

    menuChildsWindows->addAction(showCampaignsPanelAction);
    showCampaignsPanelAction->setChecked(isCampaignsPanelVisible());

    menuChildsWindows->addAction(showParametersPanelAction);
    showParametersPanelAction->setChecked(isParametersPanelVisible());

    menuChildsWindows->addAction(showSearchPanelAction);
    showSearchPanelAction->setChecked(isSearchPanelVisible());

    menuChildsWindows->addAction(separatorWindowAction);

    separatorWindowAction->setVisible(!tmp_subWindowsList.isEmpty());

    foreach (QMdiSubWindow *tmp_window, tmp_subWindowsList)
    {
        tmp_action  = menuChildsWindows->addAction(tmp_window->windowTitle());
        tmp_action->setCheckable(true);
        tmp_action->setChecked(tmp_window->isActiveWindow());
        connect(tmp_action, SIGNAL(triggered()), windowMapper, SLOT(map()));

        windowMapper->setMapping(tmp_action, tmp_window);
    }
}



/**
  Affichage de la boite de dialogue de connection
**/
void MainWindow::logon()
{
    Form_Logon      *tmp_form_logon = new Form_Logon(this);

    connect(tmp_form_logon, SIGNAL(accepted()), this, SLOT(setConnected()));

    tmp_form_logon->show();
}

/**
  Déconnexion
**/
void MainWindow::logoff()
{
    cl_disconnect(Session::instance()->getClientSessionPtr());
    Record::init(Session::instance()->getClientSession());
    setDisconnected();
    Session::instance()->unloadUserInfos();
}


/**
  Affichage de la boite de dialogue de creation d'un nouveau projet
**/
void MainWindow::newProject()
{
    Form_Project      *tmp_form_project = new Form_Project(NULL, this);

    connect(tmp_form_project, SIGNAL(projectSelected(ProjectVersion*)), this, SLOT(setCurrentProject(ProjectVersion*)));

    tmp_form_project->show();
}


/**
  Affichage de la boite de dialogue d'ouverture d'un projet
**/
void MainWindow::openProject()
{
    Form_Project_Selection *tmp_project_selection = new Form_Project_Selection(this);

    connect(tmp_project_selection, SIGNAL(projectSelected(ProjectVersion*)), this, SLOT(setCurrentProject(ProjectVersion*)));

    tmp_project_selection->show();
}


void MainWindow::setCurrentProject(ProjectVersion* in_project_version)
{
    if (closeProject())
    {
        _m_project_version = in_project_version;
        if (_m_project_version)
        {
            changeWindowTitle();
            createDockWidgets();
            loadPannelProjectDatas();
            updateMenus();
            updateMenusActionsAndToolbar();

            QSettings settings("rtmr", "");
            QByteArray window_state = settings.value("main_window_state", QByteArray()).toByteArray();
            if (!window_state.isEmpty())
                restoreState(window_state);

            _m_dock_tests_widget->setFloating(settings.value("dock_tests_floating", false).toBool());
            _m_dock_requirements_widget->setFloating(settings.value("dock_requirements_floating", false).toBool());
            _m_dock_campaigns_widget->setFloating(settings.value("dock_campaigns_floating", false).toBool());
            _m_dock_parameters_widget->setFloating(settings.value("dock_parameters_floating", false).toBool());
            _m_dock_search_widget->setFloating(settings.value("dock_search_floating", false).toBool());

            _m_dock_tests_widget->setVisible(settings.value("dock_tests_visibility", true).toBool());
            _m_dock_requirements_widget->setVisible(settings.value("dock_requirements_visibility", true).toBool());
            _m_dock_campaigns_widget->setVisible(settings.value("dock_campaigns_visibility", true).toBool());
            _m_dock_parameters_widget->setVisible(settings.value("dock_parameters_visibility", true).toBool());
            _m_dock_search_widget->setVisible(settings.value("dock_search_visibility", true).toBool());
        }
    }
}


/**
  Fermeture du projet actif
**/
bool MainWindow::closeProject()
{
    bool tmp_all_project_windows_closed = true;
    foreach (QMdiSubWindow *window, mdiArea->subWindowList())
    {
        if (window->close())
        {
            delete window->widget();
        }
        else
        {
            tmp_all_project_windows_closed = false;
            break;
        }
    }

    if (tmp_all_project_windows_closed)
    {
        QSettings settings("rtmr", "");

        if (_m_dock_tests_widget)
        {
            settings.setValue("dock_tests_floating", _m_dock_tests_widget->isFloating());
            settings.setValue("dock_tests_visibility", _m_dock_tests_widget->isVisible());
        }

        if (_m_dock_requirements_widget)
        {
            settings.setValue("dock_requirements_visibility", _m_dock_requirements_widget->isVisible());
            settings.setValue("dock_requirements_floating", _m_dock_requirements_widget->isFloating());
        }

        if (_m_dock_campaigns_widget)
        {
            settings.setValue("dock_campaigns_visibility", _m_dock_campaigns_widget->isVisible());
            settings.setValue("dock_campaigns_floating", _m_dock_campaigns_widget->isFloating());
        }

        if (_m_dock_parameters_widget)
        {
            settings.setValue("dock_parameters_visibility", _m_dock_parameters_widget->isVisible());
            settings.setValue("dock_parameters_floating", _m_dock_parameters_widget->isFloating());
        }

        if (_m_dock_search_widget)
        {
            settings.setValue("dock_search_visibility", _m_dock_search_widget->isVisible());
            settings.setValue("dock_search_floating", _m_dock_search_widget->isFloating());
        }

        settings.setValue("main_window_state", saveState());

        delete _m_campaigns_table_widget;
        _m_campaigns_table_widget = NULL;
        delete _m_parameters_table_widget;
        _m_parameters_table_widget = NULL;
        delete _m_search_table_widget;
        _m_search_table_widget = NULL;
        delete _m_requirements_tree_view;
        _m_requirements_tree_view = NULL;
        delete _m_tests_tree_view;
        _m_tests_tree_view = NULL;

        delete _m_dock_tests_widget;
        _m_dock_tests_widget = NULL;
        delete _m_dock_requirements_widget;
        _m_dock_requirements_widget = NULL;
        delete _m_dock_campaigns_widget;
        _m_dock_campaigns_widget = NULL;
        delete _m_dock_parameters_widget;
        _m_dock_parameters_widget = NULL;
        delete _m_dock_search_widget;
        _m_dock_search_widget = NULL;

        delete _m_tests_project_tree_model;
        _m_tests_project_tree_model = NULL;

        delete _m_requirements_project_tree_model;
        _m_requirements_project_tree_model = NULL;

        delete _m_project_version;
        _m_project_version = NULL;

        updateMenus();
    }

    return tmp_all_project_windows_closed;
}

/**
  Connexion active
**/
void MainWindow::setConnected()
{
    net_session		*tmp_session = Session::instance()->getClientSession();

    actionDeconnection->setText(tr("Déconn&exion") + " " +tmp_session->m_username);

    actionConnection->setEnabled(false);
    actionChangePassword->setEnabled(true);
    actionDeconnection->setEnabled(true);

    actionOuvrir_projet->setEnabled(true);

    if (cl_has_user_this_role(tmp_session, cl_current_user(tmp_session), "admin_role"))
    {
        actionNouveau_projet->setEnabled(true);
        actionImporter_projet->setEnabled(true);

        menuBar->insertAction(menuHelp->menuAction(), menuAdministration->menuAction());
        menuAdministration->setEnabled(true);

        foreach(QAction *tmp_action, menuAdministration->actions())
        {
            tmp_action->setEnabled(true);
        }
    }
    else
    {
        actionNouveau_projet->setEnabled(false);
        actionImporter_projet->setEnabled(false);

        menuAdministration->setEnabled(false);
        menuBar->removeAction(menuAdministration->menuAction());
    }

    openProject();

    _m_keepalive_timer_id = startTimer(QSettings("rtmr", "options").value("server_keepalive_interval", 10).toInt() * 1000);
}

/**
  Connection inactive
**/
void MainWindow::setDisconnected()
{
    if (_m_keepalive_timer_id > 0)
        killTimer(_m_keepalive_timer_id);

    actionDeconnection->setText(tr("Déconn&exion"));

    actionConnection->setEnabled(true);
    actionChangePassword->setEnabled(false);
    actionDeconnection->setEnabled(false);

    actionNouveau_projet->setEnabled(false);
    actionOuvrir_projet->setEnabled(false);
    actionExporter_projet->setEnabled(false);
    actionImporter_projet->setEnabled(false);
    actionFermer_projet->setEnabled(false);

    menuAdministration->setEnabled(false);
    menuBar->removeAction(menuAdministration->menuAction());

    closeProject();
}


void MainWindow::checkServerConnection()
{
    int tmp_status = cl_ping_server(Session::instance()->getClientSession());
    if (tmp_status != NOERR && _m_previous_ping_status == NOERR)
    {
        QMessageBox::critical(this, tr("Connexion avec le serveur perdue..."), tr("La connexion avec le serveur est interrompue."));
    }
    _m_previous_ping_status = tmp_status;
}


QString & formatFilename(QString & filename)
{
    return filename.replace('\\', '_')
            .replace('/', '_')
            .replace(':', '_')
            .replace('*', '_')
            .replace('?', '_')
            .replace('"', '_')
            .replace('<', '_')
            .replace('>', '_')
            .replace('|', '_');
}


/**
  Exporter le projet courant
**/
void MainWindow::exportProject()
{
    Project		*tmp_project = _m_project_version->project()->clone<Project>();
    QFile		tmp_file;
    QXmlStreamWriter	tmp_xml_writer;

    if (tmp_project != NULL)
    {
        QString tmp_filename = QString("%1.xml").arg(tmp_project->getValueForKey(PROJECTS_TABLE_SHORT_NAME));
        tmp_filename = QFileDialog::getSaveFileName(this, tr("Exporter sous..."), formatFilename(tmp_filename), "Fichier XML (*.xml)");
        if (!tmp_filename.isEmpty())
        {
            tmp_file.setFileName(tmp_filename);
            if (tmp_file.open(QIODevice::ReadWrite))
            {
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                tmp_xml_writer.setAutoFormatting(true);
                tmp_xml_writer.setDevice(&tmp_file);
                tmp_project->writeXml(tmp_xml_writer, &updateProgressBar);
                if (m_progress_dialog){
                    delete m_progress_dialog;
                    m_progress_dialog = NULL;
                }

                tmp_file.close();

                QApplication::restoreOverrideCursor();
            }
        }

        delete tmp_project;
    }
}



/**
  Importer un projet
**/
void MainWindow::importProject()
{
    Project		*tmp_project = new Project();
    QString		tmp_filename;
    QFile		tmp_file;
    QXmlStreamReader	tmp_xml_reader;

    if (tmp_project != NULL)
    {
        tmp_filename = QFileDialog::getOpenFileName(
                    this,
                    tr("Sélectionner un fichier d'export..."),
                    QString(),
                    tr("Fichier XML (*.xml)"));

        if (tmp_filename.isEmpty() == false)
        {
            tmp_file.setFileName(tmp_filename);
            if (tmp_file.open(QIODevice::ReadOnly))
            {
                QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
                tmp_xml_reader.setDevice(&tmp_file);
                if (tmp_project->readXml(tmp_xml_reader) == false)
                    QMessageBox::critical(this, tr("Erreur lors de l'import du projet"), Session::instance()->getLastErrorMessage());

                tmp_file.close();
                QApplication::restoreOverrideCursor();
            }
        }

        delete tmp_project;
    }
}



void MainWindow::showNextItemInProjectHistory()
{
    showItemProject(nextRecordInHistory(), false);
}


void MainWindow::showPreviousItemInProjectHistory()
{
    showItemProject(previousRecordInHistory(), false);
}


void MainWindow::updateActiveSubWindowMenu()
{
    if (mdiArea->activeSubWindow())
    {
        actionEnregistrerFenetreCourante->setText(tr("&Enregistrer %1").arg(mdiArea->activeSubWindow()->windowTitle()));
        actionEnregistrerFenetreCourante->setEnabled(true);

        actionFermerFenetreCourante->setText(tr("&Fermer %1").arg(mdiArea->activeSubWindow()->windowTitle()));
        actionFermerFenetreCourante->setEnabled(true);
    }
    else
    {
        actionEnregistrerFenetreCourante->setText(tr("&Enregistrer"));
        actionEnregistrerFenetreCourante->setEnabled(false);

        actionFermerFenetreCourante->setText(tr("&Fermer"));
        actionFermerFenetreCourante->setEnabled(false);
    }

    actionFermerFenetres->setEnabled(!mdiArea->subWindowList().isEmpty());
}


void MainWindow::updateMenus()
{
    bool tmp_project_opened = (_m_project_version != 0);

    actionExporter_projet->setEnabled(tmp_project_opened);
    actionFermer_projet->setEnabled(tmp_project_opened);

    menuTests->setEnabled(tmp_project_opened);
    foreach(QAction *tmp_action, menuTests->actions())
        tmp_action->setEnabled(tmp_project_opened);

    menuRequirements->setEnabled(tmp_project_opened);
    foreach(QAction *tmp_action, menuRequirements->actions())
        tmp_action->setEnabled(tmp_project_opened);

    menuCampaigns->setEnabled(tmp_project_opened);
    foreach(QAction *tmp_action, menuCampaigns->actions())
        tmp_action->setEnabled(tmp_project_opened);

    menuProjet->setEnabled(tmp_project_opened);
    foreach(QAction *tmp_action, menuProjet->actions())
        tmp_action->setEnabled(tmp_project_opened);

    menuHistory->setEnabled(tmp_project_opened);
    foreach(QAction *tmp_action, menuHistory->actions())
        tmp_action->setEnabled(tmp_project_opened);
}


void MainWindow::readSettings()
{
    QSettings			tmp_options_settings("rtmr", tr("options"));

    QSettings settings("rtmr", "");
    QPoint pos = settings.value("pos", QPoint(0, 0)).toPoint();
    QSize size = settings.value("size", QSize(800, 600)).toSize();

    QString				tmp_prefered_language(tmp_options_settings.value("prefered_language", "").toString());

    move(pos);
    resize(size);

    ProcessUtils::readAndSetApplicationProxySettings();

    // Chargement de la langue du système par défaut
    loadLanguage(QLocale::system().name().section('_', 0, 0));

    if (!tmp_prefered_language.isEmpty())
    {
        foreach(QAction *action, menuSelectLanguage->actions())
        {
            if (tmp_prefered_language.compare(action->data().toString()) == 0)
            {
                loadLanguage(tmp_prefered_language);
                action->setChecked(true);
            }
        }
    }
}

void MainWindow::writeSettings()
{
    QSettings settings("rtmr", "");

    // Main window
    settings.setValue("pos", pos());
    settings.setValue("size", size());
}


void MainWindow::timerEvent ( QTimerEvent * event )
{
    if (event->timerId() == _m_keepalive_timer_id)
    {
        checkServerConnection();
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (closeProject())
    {
        if (_m_project_version)
        {
            event->ignore();
        }
        else
        {
            writeSettings();
            event->accept();
        }
    }
    else
        event->ignore();
}


void MainWindow::changeEvent(QEvent* event)
{
    if(0 != event)
    {
        switch(event->type())
        {
        // this event is send if a translator is loaded
        case QEvent::LanguageChange:
            retranslateUi();
            break;
            // this event is send, if the system, language changes
        case QEvent::LocaleChange:
        {
            QString locale = QLocale::system().name();
            locale.truncate(locale.lastIndexOf('_'));

        }
            break;
        default:
            break;
        }
    }

    QMainWindow::changeEvent(event);
}


void MainWindow::loadLanguage(const QString& in_language)
{
    Session::instance()->loadLanguage(in_language);
    menuSelectLanguage->setIcon(QIcon(":/images/22x22/" + Session::instance()->getLangage() + ".png"));
}


/**
  Affichage de la fenetre de gestion des utilisateurs
**/
void MainWindow::manageUsers()
{
    Form_Manage_Users   *tmp_users_form = new Form_Manage_Users(this);

    tmp_users_form->show();
}


/**
  Affichage de la fenetre de rapports des projets
**/
void MainWindow::projectsReports()
{
    Form_Projects_Reports   *tmp_reports_form = new Form_Projects_Reports(this);

    tmp_reports_form->show();
}


/**
  Affichage de la fenetre de gestion des champs personnalisés
**/
void MainWindow::manageCustomTestsFields()
{
    Form_Manage_CustomFields   *tmp_customfields_form = new Form_Manage_CustomFields(CUSTOM_FIELDS_TEST, this);

    tmp_customfields_form->show();
}


void MainWindow::manageCustomRequirementsFields()
{
    Form_Manage_CustomFields   *tmp_customfields_form = new Form_Manage_CustomFields(CUSTOM_FIELDS_REQUIERMENT, this);

    tmp_customfields_form->show();
}





void MainWindow::about()
{
    char	tmp_server_version[12];
    long            tmp_api_protocol_version = cl_api_protocol_version();
    QString		tmp_about_msg = "<b>R.T.M.R</b><BR>" + tr("Version du client : ")
            + qApp->applicationVersion();

    tmp_about_msg += "<br>" + tr("Version du protocole : %1.%2.%3.%4")
            .arg(PROTOCOL_VERSION >> 24)
            .arg((PROTOCOL_VERSION >> 16) & 0xFF)
            .arg((PROTOCOL_VERSION >> 8) & 0xFF)
            .arg(PROTOCOL_VERSION & 0xFF);

#ifdef SVN_REVISION
    #define QUOTE(string)   #string
    #define TO_STRING(string)   QUOTE(string)
    tmp_about_msg += "<br>" + tr("Révision : ") + TO_STRING(SVN_REVISION);
#else
#ifdef APP_REVISION
    if (is_empty_string(APP_REVISION) == FALSE)
        tmp_about_msg += "<br>" + tr("Révision : ") + QString(APP_REVISION);
#endif
#endif

    tmp_about_msg += "<br>Qt version : ";
    tmp_about_msg += QT_VERSION_STR;

    tmp_about_msg += "<br><br><b>" + tr("Bibliothèque") + "</b><BR>" + tr("Version du protocole de l'API : %1.%2.%3.%4")
            .arg(tmp_api_protocol_version >> 24)
            .arg((tmp_api_protocol_version >> 16) & 0xFF)
            .arg((tmp_api_protocol_version >> 8) & 0xFF)
            .arg(tmp_api_protocol_version & 0xFF);

    if (Session::instance()->getClientSession() != NULL)
    {
        if (cl_get_server_infos(Session::instance()->getClientSession()) == NOERR)
        {
            net_get_field(NET_MESSAGE_TYPE_INDEX+1, Session::instance()->getClientSession()->m_response, tmp_server_version, SEPARATOR_CHAR);
            tmp_about_msg += "<br><br><b>" + tr("Serveur") + "</b><br>" + tr("Version du serveur : ") + tmp_server_version;

            net_get_field(NET_MESSAGE_TYPE_INDEX+3, Session::instance()->getClientSession()->m_response, tmp_server_version, SEPARATOR_CHAR);
            tmp_about_msg += "<br>" + tr("Version de la base de données : ") + ProjectVersion::formatProjectVersionNumber(tmp_server_version);

            net_get_field(NET_MESSAGE_TYPE_INDEX+2, Session::instance()->getClientSession()->m_response, tmp_server_version, SEPARATOR_CHAR);
            tmp_about_msg += "<br>" + tr("Version du protocole : ") + tmp_server_version;
            tmp_about_msg += "<br>" + tr("Serveur")+" : " + Session::instance()->getClientSession()->m_hostname;
            tmp_about_msg += "<br>" + tr("Port")+" : " + QString::number(Session::instance()->getClientSession()->m_host_port);

            net_get_field(NET_MESSAGE_TYPE_INDEX+4, Session::instance()->getClientSession()->m_response, tmp_server_version, SEPARATOR_CHAR);
            tmp_about_msg += "<br>" + tr("Version du serveur de données : ") + ProjectVersion::formatProjectVersionNumber(tmp_server_version);
        }
    }

    if (Session::instance()->externalsModules().count()>0)
    {
        tmp_about_msg += "<br><table><tr><th>"+tr("Module")+"</th><th>"+tr("Type")+"</th><th>"+tr("Version")+"</th><th>"+tr("Informations")+"</th></tr>";
        QMap<QString, ClientModule*> tmp_module_map;
        foreach(tmp_module_map, Session::instance()->externalsModules().values())
        {
            foreach(ClientModule* tmp_module, tmp_module_map.values())
            {
                tmp_about_msg += "<tr><td>"+tmp_module->getModuleName()+"</td><td>"+TR_CUSTOM_MESSAGE(ClientModule::getTypeDescriptionForModule(tmp_module).toStdString().c_str())+"</td>";
                tmp_about_msg += "<td align=\"center\">"+tmp_module->getModuleVersion()+"</td><td><a href=\""+tmp_module->getModuleInformationsUrl()+"\">"+tmp_module->getModuleInformationsUrl()+"</a></td></tr>";
            }
        }
        tmp_about_msg += "</table>";
    }

    QMessageBox::about(
                this,
                tr("A propos de R.T.M.R"),
                tmp_about_msg);
}


/**
  Affichage de la fenetre de changement de mot de passe
**/
void MainWindow::changePassword()
{
    QString tmp_username = cl_current_user(Session::instance()->getClientSession());
    Form_Change_Password *tmp_form = new Form_Change_Password(tmp_username, this);

    tmp_form->show();
}


/**
  Affichage de la fenetre de gestion des options de l'application
**/
void MainWindow::showOption()
{
    Form_Options *tmp_form = new Form_Options(this);

    tmp_form->show();
}


void MainWindow::saveCurrentChildWindow()
{
    QMdiSubWindow* tmp_child_window = mdiArea->currentSubWindow();
    if (tmp_child_window)
    {
        AbstractProjectWidget* tmp__widget = dynamic_cast<AbstractProjectWidget*>(tmp_child_window->widget());
        if (tmp__widget)
            tmp__widget->save();
    }
}



void MainWindow::toogleRequirementsPanel()
{
    if (_m_dock_requirements_widget)
        _m_dock_requirements_widget->setVisible(!_m_dock_requirements_widget->isVisible());
}


void MainWindow::toogleTestsPanel()
{
    if (_m_dock_tests_widget)
        _m_dock_tests_widget->setVisible(!_m_dock_tests_widget->isVisible());
}


void MainWindow::toogleCampaignsPanel()
{
    if (_m_dock_campaigns_widget)
        _m_dock_campaigns_widget->setVisible(!_m_dock_campaigns_widget->isVisible());
}


void MainWindow::toogleParametersPanel()
{
    if (_m_dock_parameters_widget)
        _m_dock_parameters_widget->setVisible(!_m_dock_parameters_widget->isVisible());
}


void MainWindow::toogleSearchPanel()
{
    if (_m_dock_search_widget)
        _m_dock_search_widget->setVisible(!_m_dock_search_widget->isVisible());
}

void MainWindow::updateMenusActionsAndToolbar()
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

    tmp_tests_writable = _m_project_version != NULL && _m_project_version->project() != NULL && _m_project_version->project()->projectGrants() != NULL &&
            compare_values(_m_project_version->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC), PROJECT_GRANT_WRITE) == 0;

    tmp_requirements_writable = _m_project_version != NULL && _m_project_version->project() != NULL && _m_project_version->project()->projectGrants() != NULL &&
            compare_values(_m_project_version->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC), PROJECT_GRANT_WRITE) == 0;

    tmp_campaigns_writable = _m_project_version != NULL && _m_project_version->project() != NULL && _m_project_version->project()->projectGrants() != NULL &&
            compare_values(_m_project_version->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_CAMPAIGNS_INDIC), PROJECT_GRANT_WRITE) == 0;

    insertRequirementAction->setEnabled(tmp_requirements_writable);
    insertChildRequirementAction->setEnabled(tmp_requirements_selection_count == 1 && tmp_requirements_writable);
    importRequirementsAction->setEnabled(tmp_requirements_writable);
    exportRequirementsAction->setEnabled(tmp_requirements_selection_count > 0);
    expandSelectedRequirementsAction->setEnabled(tmp_requirements_selection_count > 0);
    collapseSelectedRequirementsAction->setEnabled(tmp_requirements_selection_count > 0);
    removeRequirementAction->setEnabled(tmp_requirements_selection_count > 0 && tmp_requirements_writable);
    selectDependantsTestsAction->setEnabled(tmp_requirements_selection_count > 0);
    printRequirementsAction->setEnabled(tmp_requirements_selection_count > 0);
    saveAsHtmlRequirementsAction->setEnabled(tmp_requirements_selection_count > 0);

    insertTestAction->setEnabled(tmp_tests_writable);
#ifdef AUTOMATION_ACTIVATED
    insertAutomatedTestAction->setEnabled(tmp_tests_writable);
#endif
    insertChildTestAction->setEnabled(tmp_tests_selection_count == 1 && tmp_tests_writable);
    importTestsAction->setEnabled(tmp_tests_writable);
    exportTestsAction->setEnabled(tmp_tests_selection_count > 0);
    expandSelectedTestsAction->setEnabled(tmp_tests_selection_count > 0);
    collapseSelectedTestsAction->setEnabled(tmp_tests_selection_count > 0);
    removeTestAction->setEnabled(tmp_tests_selection_count > 0 && tmp_tests_writable);
    selectLinkTestsAction->setEnabled(tmp_tests_selection_count > 0);
    printTestsAction->setEnabled(tmp_tests_selection_count > 0);
    saveAsHtmlTestsAction->setEnabled(tmp_tests_selection_count > 0);

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
void MainWindow::removeRequirementsList(QList<Hierarchy*> in_records_list)
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
                tmp_msg_box->setText(tr("Etes-vous sûr(e) de vouloir supprimer l'exigence \"%1\" ?").arg(((RequirementHierarchy*)tmp_item)->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)));
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

        delete tmp_msg_box;
    }
}


void MainWindow::showCylicRedundancyAlert()
{
    QApplication::restoreOverrideCursor();
    QMessageBox::warning(this, tr("Redondance cyclique..."),
                         tr("Un des scénarios que vous voulez insérer :") \
                         + "<ul><li>" + tr("est un lien vers un autre scénario") + "</li>" \
                         + "<li>" + tr("fait directement ou indirectement référence au scénario de destination.") + "</li></ul>");
}



bool MainWindow::isRequirementsPanelVisible()
{
    return _m_dock_requirements_widget && _m_dock_requirements_widget->isVisible();
}


bool MainWindow::isTestsPanelVisible()
{
    return _m_dock_tests_widget && _m_dock_tests_widget->isVisible();
}


bool MainWindow::isCampaignsPanelVisible()
{
    return _m_dock_campaigns_widget && _m_dock_campaigns_widget->isVisible();
}


bool MainWindow::isParametersPanelVisible()
{
    return _m_dock_parameters_widget && _m_dock_parameters_widget->isVisible();
}


bool MainWindow::isSearchPanelVisible()
{
    return _m_dock_search_widget && _m_dock_search_widget->isVisible();
}


/**
  Changer le titre de la fenetre
**/
void MainWindow::changeWindowTitle()
{
    setWindowTitle(windowTitle());
}


QString MainWindow::windowTitle()
{
    return QString(_m_project_version->project()->getValueForKey(PROJECTS_TABLE_SHORT_NAME)) +
            QString(" - ") +
            ProjectVersion::formatProjectVersionNumber(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION));
}


QMdiSubWindow* MainWindow::createMdiChildForRecord(QWidget* child, Record* in_record)
{
    QMdiSubWindow* tmp_mdi_child = new QMdiSubWindow(this);
    tmp_mdi_child->setWidget(child);
    tmp_mdi_child->setAttribute(Qt::WA_DeleteOnClose);
    mdiArea->addSubWindow(tmp_mdi_child);
    _m_mdi_childs_map[tmp_mdi_child] = in_record;
    connect(child, SIGNAL(destroyed()), tmp_mdi_child, SLOT(close()));
    connect(tmp_mdi_child, SIGNAL(destroyed()), this, SLOT(updateActiveSubWindowMenu()));
    return tmp_mdi_child;
}


/**
  Afficher les informations du test selectionne
**/
void MainWindow::showTestInfos(TestHierarchy *in_test, bool in_update_history)
{
    Qt::KeyboardModifiers tmp_modifiers = QApplication::keyboardModifiers();

    if (in_test != NULL && !(tmp_modifiers == Qt::ShiftModifier || tmp_modifiers == Qt::ControlModifier || tmp_modifiers == (Qt::ControlModifier|Qt::ShiftModifier)))
    {
        QMdiSubWindow		*tmp_mdi_child = findSubWindowForRecordColumnValue<Form_Test>(TESTS_HIERARCHY_TEST_ID, in_test->getValueForKey(TESTS_HIERARCHY_TEST_ID));
        Form_Test           	*tmp_form_test = NULL;

        setWindowIcon(QIcon(QPixmap(":/images/22x22/pellicule.png")));

        if (tmp_mdi_child == NULL)
        {
            tmp_form_test = new Form_Test(this);
            tmp_form_test->loadTest(in_test);
            connect(tmp_form_test, SIGNAL(testSaved(TestHierarchy*)), this, SLOT(testSaved(TestHierarchy*)));
            connect(tmp_form_test, SIGNAL(showOriginalTestInfos(TestHierarchy*)), this, SLOT(showOriginalTestInfos(TestHierarchy*)));
            connect(tmp_form_test, SIGNAL(showRequirementWithOriginalContentId(const char*)), this, SLOT(showRequirementWithOriginalContentId(const char*)));
            tmp_mdi_child = createMdiChildForRecord(tmp_form_test, in_test);
            tmp_mdi_child->showMaximized();
        }
        else {
            tmp_form_test = dynamic_cast<Form_Test*>(tmp_mdi_child->widget());
            setActiveSubWindow(tmp_mdi_child);
        }

        if (tmp_form_test != NULL)
        {
            if (in_update_history)
            {
                _m_history.append(in_test);
                _m_history_index = _m_history.count() - 1;
            }
            updateActiveSubWindowMenu();
        }
    }
}


void MainWindow::showOriginalTestInfos(TestHierarchy *in_test_hierarchy)
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
void MainWindow::showRequirementInfos(RequirementHierarchy *in_requirement, bool in_update_history)
{
    Qt::KeyboardModifiers tmp_modifiers = QApplication::keyboardModifiers();

    if (in_requirement != NULL && !(tmp_modifiers == Qt::ShiftModifier || tmp_modifiers == Qt::ControlModifier || tmp_modifiers == (Qt::ControlModifier|Qt::ShiftModifier)))
    {
        QMdiSubWindow      	*tmp_mdi_child = findSubWindowForRecordColumnValue<Form_Requirement>(REQUIREMENTS_HIERARCHY_REQUIREMENT_ID, in_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_ID));
        Form_Requirement	*tmp_form_requirement = NULL;

        setWindowIcon(QIcon(QPixmap(":/images/notes.png")));

        if (tmp_mdi_child == NULL)
        {
            tmp_form_requirement = new Form_Requirement(this);
            tmp_form_requirement->loadRequirement(in_requirement);
            connect(tmp_form_requirement, SIGNAL(requirementSaved(RequirementHierarchy*)), this, SLOT(requirementSaved(RequirementHierarchy*)));
            connect(tmp_form_requirement, SIGNAL(showTestWithContentId(const char*)), this, SLOT(showTestWithContentId(const char*)));
            tmp_mdi_child = createMdiChildForRecord(tmp_form_requirement, in_requirement);
            tmp_mdi_child->showMaximized();
        }
        else {
            tmp_form_requirement = dynamic_cast<Form_Requirement*>(tmp_mdi_child->widget());
            setActiveSubWindow(tmp_mdi_child);
        }

        if (tmp_form_requirement != NULL)
        {
            if (in_update_history)
            {
                _m_history.append(in_requirement);
                _m_history_index = _m_history.count() - 1;
            }
            updateActiveSubWindowMenu();
        }
    }
}



void MainWindow::showTestWithContentId(const char *in_test_content_id)
{
    QModelIndex				tmp_model_index;
    TestHierarchy	*tmp_test = NULL;
    TestHierarchy	**tmp_tests_array = NULL;
    unsigned long			tmp_rows_count = 0;

    net_session             *tmp_session = Session::instance()->getClientSession();
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
                _m_tests_tree_view->expandIndex(tmp_model_index);
                showTestInfos(tmp_model_index);
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

void MainWindow::showRequirementWithOriginalContentId(const char *in_original_requirement_content_id)
{
    QModelIndex				tmp_model_index;
    RequirementHierarchy	*tmp_requirement = NULL;
    RequirementHierarchy	**tmp_requirements_array = NULL;
    unsigned long			tmp_rows_count = 0;

    net_session             *tmp_session = Session::instance()->getClientSession();
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
                _m_requirements_tree_view->expandIndex(tmp_model_index);
                showRequirementInfos(tmp_model_index);
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
void MainWindow::showCampaignInfos(Campaign *in_campaign, bool in_update_history)
{
    setWindowIcon(QIcon(QPixmap(":/images/package.png")));

    QMdiSubWindow	      	*tmp_mdi_child = findSubWindowForRecordColumnValue<Form_Campaign>(CAMPAIGNS_TABLE_CAMPAIGN_ID, in_campaign->getValueForKey(CAMPAIGNS_TABLE_CAMPAIGN_ID));
    Form_Campaign		*tmp_form_campaign = NULL;

    if (tmp_mdi_child == NULL)
    {
        tmp_form_campaign = new Form_Campaign(this);
        tmp_form_campaign->loadCampaign(in_campaign);
        tmp_mdi_child = createMdiChildForRecord(tmp_form_campaign, in_campaign);
        tmp_mdi_child->showMaximized();
    }
    else {
        tmp_form_campaign = dynamic_cast<Form_Campaign*>(tmp_mdi_child->widget());
        setActiveSubWindow(tmp_mdi_child);
    }

    if (in_campaign != NULL && tmp_form_campaign != NULL)
    {
        if (in_update_history)
        {
            _m_history.append(in_campaign);
            _m_history_index = _m_history.count() - 1;
        }
        updateActiveSubWindowMenu();
    }
}


/**
  Affichage de la boite de dialogue des proprietes du projet courant
**/
void MainWindow::showProjectProperties()
{
    if (_m_project_version != NULL)
        showProjectVersionInfos();
}


/**
  Affichage de la fenetre de rapports des executions
**/
void MainWindow::executionReports()
{
    if (_m_project_version != NULL)
        showExecutionsStats();
}


/**
  Affichage de la fenetre des anomalies
**/
void MainWindow::showProjectBugs()
{
    if (_m_project_version != NULL)
        showBugs();
}

/**
  Afficher les informations de la version du projet
**/
void MainWindow::showProjectVersionInfos(bool in_update_history)
{
    QMdiSubWindow      	*tmp_child_window = findSubWindowForProjectVersion<Form_Project_Version>(_m_project_version);
    Form_Project_Version    *tmp_form_project_version = NULL;

    setWindowIcon(QIcon(QPixmap(":/images/box.png")));

    if (tmp_child_window == NULL)
    {
        tmp_form_project_version = new Form_Project_Version(_m_project_version, this);
        tmp_child_window = createMdiChildForRecord(tmp_form_project_version, _m_project_version);
    }

    tmp_child_window->showMaximized();

    if (in_update_history)
    {
        _m_history.append(_m_project_version);
        _m_history_index = _m_history.count() - 1;
    }
    updateMenusActionsAndToolbar();
    updateActiveSubWindowMenu();
}


/**
  Afficher les statistiques d'executions du projet
**/
void MainWindow::showExecutionsStats()
{
    QMdiSubWindow	*tmp_child_window = findSubWindowForProjectVersion<Form_Executions_Reports>(_m_project_version);
    Form_Executions_Reports	*tmp_executions_reports_form = NULL;

    setWindowIcon(QIcon(QPixmap(":/images/22x22/stats.png")));

    if (tmp_child_window == NULL)
    {
        tmp_executions_reports_form = new Form_Executions_Reports(_m_project_version, this);
        tmp_child_window = createMdiChildForRecord(tmp_executions_reports_form, _m_project_version);
    }

    tmp_child_window->showMaximized();
    updateActiveSubWindowMenu();
}



/**
  Afficher les anomalies du projet
**/
void MainWindow::showBugs()
{
    QMdiSubWindow	*tmp_child_window = findSubWindowForProjectVersion<Form_Project_Bugs>(_m_project_version);
    Form_Project_Bugs	*tmp_bugs_form = NULL;

    setWindowIcon(QIcon(QPixmap(":/images/bug.png")));

    if (tmp_child_window == NULL)
    {
        tmp_bugs_form = new Form_Project_Bugs(_m_project_version, this);
        tmp_child_window = createMdiChildForRecord(tmp_bugs_form, _m_project_version);
    }

    tmp_child_window->showMaximized();
    updateActiveSubWindowMenu();
}



void MainWindow::testSaved(TestHierarchy *in_test)
{
    QModelIndex	tmp_model_index = _m_tests_project_tree_model->modelIndexForItem(in_test);
    if (tmp_model_index.isValid())
        _m_tests_tree_view->update(tmp_model_index);
}


void MainWindow::requirementSaved(RequirementHierarchy *in_requirement)
{
    QModelIndex	tmp_model_index = _m_requirements_project_tree_model->modelIndexForItem(in_requirement);
    if (tmp_model_index.isValid())
        _m_requirements_tree_view->update(tmp_model_index);
}


int MainWindow::saveProjectVersion()
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


Record* MainWindow::nextRecordInHistory()
{
    if (_m_history_index < _m_history.count() - 1)
    {
        _m_history_index++;
        return _m_history[_m_history_index];
    }

    return NULL;
}


Record* MainWindow::previousRecordInHistory()
{
    if (_m_history_index > 0)
    {
        _m_history_index--;
        return _m_history[_m_history_index];
    }

    return NULL;
}


bool MainWindow::historyHasNextRecord()
{
    return _m_history_index < _m_history.count() - 1;
}


bool MainWindow::historyHasPreviousRecord()
{
    return _m_history_index > 0;
}


void MainWindow::searchItemsSelectionChanged()
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
void MainWindow::showSearchItem(QModelIndex in_model_index)
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


void MainWindow::showItemProject(Record *in_record, bool in_update_project_history)
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
                _m_tests_tree_view->expandIndex(tmp_model_index);
                showTestInfos(tmp_model_index);
                return;
            }
        }
        else if (in_record->getEntityDefSignatureId() == REQUIREMENTS_HIERARCHY_SIG_ID)
        {
            tmp_model_index = _m_requirements_project_tree_model->modelIndexForItem((Hierarchy*)in_record);
            if (tmp_model_index.isValid())
            {
                _m_requirements_tree_view->expandIndex(tmp_model_index);
                showRequirementInfos(tmp_model_index);
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
void MainWindow::showCampaignInfos(QModelIndex in_model_index)
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
void MainWindow::showRequirementInfos(QModelIndex in_model_index)
{
    RequirementHierarchy     	*tmp_selected_requirement = NULL;

    if (in_model_index.isValid())
    {
        _m_requirements_tree_view->selectionModel()->setCurrentIndex(in_model_index, QItemSelectionModel::Current);
        tmp_selected_requirement = (RequirementHierarchy*)(_m_requirements_project_tree_model->getItem(in_model_index));
        showRequirementInfos(tmp_selected_requirement);
        updateMenusActionsAndToolbar();
    }
}



/**
  Afficher les informations du test selectionne
**/
void MainWindow::showTestInfos(QModelIndex in_model_index)
{
    TestHierarchy     *tmp_selected_test = NULL;

    if (in_model_index.isValid())
    {
        _m_tests_tree_view->selectionModel()->setCurrentIndex(in_model_index, QItemSelectionModel::Current);
        tmp_selected_test = (TestHierarchy*)(_m_tests_project_tree_model->getItem(in_model_index));
        showTestInfos(tmp_selected_test);
        updateMenusActionsAndToolbar();
    }
}


/**
  Inserer un test
**/
void MainWindow::insertRequirementAtIndex(QModelIndex index)
{
    _m_requirements_project_tree_model->insertRows(index.row()+1, 1, index.parent());

    showRequirementInfos(_m_requirements_project_tree_model->index(index.row()+1, 0, index.parent()));
}



/**
  Inserer un test
**/
void MainWindow::insertTestAtIndex(QModelIndex index, bool in_automated_test)
{
    QModelIndex tmp_new_test_index;

    if (in_automated_test)
    {
        TestHierarchy* tmp_test = NULL;

        TestContent* tmp_test_content = new TestContent(_m_project_version);
        tmp_test_content->setValueForKey("", TESTS_CONTENTS_TABLE_SHORT_NAME);
        tmp_test_content->setValueForKey(YES, TESTS_CONTENTS_TABLE_AUTOMATED);
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
        showTestInfos(tmp_new_test_index);
    }
}


/**
  Inserer un test
**/
void MainWindow::insertTest()
{
    insertTestAtIndex(_m_tests_tree_view->selectionModel()->currentIndex());
}


/**
  Inserer un test automatisé
**/
void MainWindow::insertAutomatedTest()
{
    insertTestAtIndex(_m_tests_tree_view->selectionModel()->currentIndex(), true);
}


/**
  Inserer un test enfant
**/
void MainWindow::insertChildTest()
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

        showTestInfos(_m_tests_project_tree_model->index(tmp_row, 0, index));
    }
}

/**
  Supprimer les tests sélectionnés
**/
void MainWindow::removeSelectedTests()
{
    removeTestsList(_m_tests_tree_view->selectedRecords());
}


/**
  Supprimer la liste des tests
**/
void MainWindow::removeTestsList(QList<Hierarchy*> in_records_list)
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
                tmp_msg_box->setText(tr("Etes-vous sûr(e) de vouloir supprimer le test \"%1\" ?").arg(((TestHierarchy*)tmp_item)->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)));
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

        delete tmp_msg_box;
    }
}


void MainWindow::selectDependantsTestsFromSelectedRequirements()
{
    QModelIndexList tmp_indexes_list = dependantsTestsIndexesFromRequirementsList(_m_requirements_tree_view->selectedRecords());

    foreach(QModelIndex tmp_model_index, tmp_indexes_list)
    {
        _m_tests_tree_view->selectionModel()->select(tmp_model_index, QItemSelectionModel::Select);
        _m_tests_tree_view->expandIndex(tmp_model_index);
    }
}


QModelIndexList MainWindow::dependantsTestsIndexesFromRequirementsList(QList<Hierarchy*> in_records_list)
{
    net_session         *tmp_session = Session::instance()->getClientSession();
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
            tmp_query += sprintf(tmp_query, "%s", in_records_list[tmp_index]->getValueForKey(REQUIREMENTS_HIERARCHY_ORIGINAL_REQUIREMENT_CONTENT_ID));
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

                            foreach (TestHierarchy *tmp_copy, tmp_test->links())
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


void MainWindow::selectLinkTests()
{
    selectLinkTests(_m_tests_tree_view->selectedRecords());
}

void MainWindow::selectLinkTests(QList<Hierarchy*> in_records_list)
{
    QModelIndex         tmp_model_index;
    QList<TestHierarchy*>		tmp_link_tests_list;
    TestHierarchy		*tmp_test = NULL;

    if (in_records_list.count() > 0)
    {
        _m_tests_tree_view->selectionModel()->select(_m_tests_tree_view->selectionModel()->selection(), QItemSelectionModel::Deselect);

        foreach(Hierarchy *tmp_item, in_records_list)
        {
            tmp_link_tests_list = ((TestHierarchy*)tmp_item)->links();

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
void MainWindow::expandTestsTree()
{
    _m_tests_tree_view->expandAll();
}


/**
  Derouler les tests sélectionnés
**/
void MainWindow::expandSelectedTestsTree()
{
    foreach(QModelIndex tmp_index, _m_tests_tree_view->selectionModel()->selectedIndexes())
    {
        _m_tests_tree_view->setExpandedIndex(tmp_index, true);
    }
}


/**
  Tout enrouler
**/
void MainWindow::collapseTestsTree()
{
    _m_tests_tree_view->collapseAll();
}


/**
  Enrouler les tests sélectionnés
**/
void MainWindow::collapseSelectedTestsTree()
{
    foreach(QModelIndex tmp_index, _m_tests_tree_view->selectionModel()->selectedIndexes())
    {
        _m_tests_tree_view->setExpandedIndex(tmp_index, false);
    }
}



/**
  Importer des tests
**/
void MainWindow::importTests()
{
    QStringList		tmp_columns_names = getImportExportTestColumnsNames();
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
        QMessageBox::information(this, tr("Information"), tr("Les tests seront importés après le test \"%1\".").arg(tmp_test->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)));

        if (tmp_test->parent() != _m_project_version)
            tmp_parent_test = new TestHierarchy((TestHierarchy*)tmp_test->parent());
        else
            tmp_parent_test = new TestHierarchy(_m_project_version);
    }
    else
        tmp_parent_test = new TestHierarchy(_m_project_version);


    tmp_import_form = new Form_Data_Import(tmp_parent_test, tmp_columns_names, &test_entity_import, this);
    connect(tmp_import_form, SIGNAL(startImport(Hierarchy*)), this, SLOT(startImportTests(Hierarchy*)));
    connect(tmp_import_form, SIGNAL(importRecord(Hierarchy*, GenericRecord*, bool)), this, SLOT(importTest(Hierarchy*, GenericRecord*, bool)));
    tmp_import_form->show();
}

QStringList MainWindow::getImportExportTestColumnsNames()
{
    QStringList		tmp_columns_names;

    tmp_columns_names << tr("Nom du test") << tr("Description du test") << tr("Catégorie du test") << tr("Priorité du test") << tr("Action standard : Description") << tr("Action standard : Résultat attendu");
    tmp_columns_names << tr("Status") << tr("Automatisé") << tr("Automatisation : Commande") << tr("Automatisation : Paramètres de commande") << tr("Type") <<  tr("Cas aux limites");
    tmp_columns_names << tr("Action automatisée : Id fenêtre") << tr("Action automatisée : Type de message") << tr("Action automatisée : Données") << tr("Action automatisée : Délai");

    return tmp_columns_names;
}

void MainWindow::startImportTests(Hierarchy *in_parent)
{
    TestContent		*tmp_test_content = NULL;
    int			tmp_save_result = NOERR;

    TestHierarchy	*tmp_test_parent = NULL;

    tmp_test_parent = (TestHierarchy*)in_parent;

    cl_transaction_start(Session::instance()->getClientSession());

    tmp_test_content = new TestContent(_m_project_version);

    tmp_test_content->setValueForKey(tr("Import de tests").toStdString().c_str(), TESTS_CONTENTS_TABLE_SHORT_NAME);
    tmp_test_content->setValueForKey(tr("Import de tests").toStdString().c_str(), TESTS_CONTENTS_TABLE_DESCRIPTION);
    tmp_save_result = tmp_test_content->saveRecord();
    if (tmp_save_result == NOERR)
    {
        tmp_test_parent->setDataFromTestContent(tmp_test_content);
        tmp_save_result = tmp_test_parent->saveRecord();
    }
    delete tmp_test_content;

    if (tmp_save_result != NOERR)
    {
        cl_transaction_rollback(Session::instance()->getClientSession());

        disconnect(this, SLOT(startImportTests(Hierarchy*)));
        disconnect(this, SLOT(importTest(Hierarchy*, GenericRecord*, bool)));
    }
}


void MainWindow::endImportTests(Hierarchy *in_parent)
{
    QList<Hierarchy*>	tmp_records_list = _m_tests_tree_view->selectedRecords();
    Hierarchy		*tmp_test = NULL;
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
    }
    else
    {
        tmp_index = _m_project_version->childCount(TESTS_HIERARCHY_SIG_ID);
        tmp_model_index  = QModelIndex();
    }

    if (_m_tests_project_tree_model->insertItem(tmp_index, tmp_model_index, tmp_test_parent))
    {
        tmp_tr_result = cl_transaction_commit(Session::instance()->getClientSession());
        if (tmp_tr_result != NOERR)
            QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_tr_result));

    }
    else
    {
        cl_transaction_rollback(Session::instance()->getClientSession());
    }
}


void MainWindow::importTest(Hierarchy *in_parent, GenericRecord *in_record, bool in_last_record)
{
    TestContent		*tmp_test_content = NULL;
    Action		*tmp_action = NULL;
    AutomatedAction	*tmp_automated_action = NULL;
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

        tmp_create_action = is_empty_string(in_record->getValueForKey(ACTION_IMPORT_DESCRIPTION)) == FALSE || is_empty_string(in_record->getValueForKey(ACTION_IMPORT_WAIT_RESULT)) == FALSE;

        tmp_str_list = QString(in_record->getValueForKey(TEST_IMPORT_SHORT_NAME)).split(PARENT_HIERARCHY_SEPARATOR, QString::SkipEmptyParts);
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
                        tmp_test_content->setValueForKey(tmp_str_name.toStdString().c_str(), TESTS_CONTENTS_TABLE_SHORT_NAME);
                        tmp_test_content->setValueForKey(in_record->getValueForKey(TEST_IMPORT_DESCRIPTION), TESTS_CONTENTS_TABLE_DESCRIPTION);
                        tmp_test_content->setValueForKey(in_record->getValueForKey(TEST_IMPORT_CATEGORY_ID), TESTS_CONTENTS_TABLE_CATEGORY_ID);
                        tmp_test_content->setValueForKey(in_record->getValueForKey(TEST_IMPORT_PRIORITY_LEVEL), TESTS_CONTENTS_TABLE_PRIORITY_LEVEL);
                        tmp_test_content->setValueForKey(in_record->getValueForKey(TESTS_CONTENTS_TABLE_STATUS), TESTS_CONTENTS_TABLE_STATUS);
                        tmp_test_content->setValueForKey(in_record->getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATED), TESTS_CONTENTS_TABLE_AUTOMATED);
                        tmp_test_content->setValueForKey(in_record->getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND), TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND);
                        tmp_test_content->setValueForKey(in_record->getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND_PARAMETERS), TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND_PARAMETERS);
                        tmp_test_content->setValueForKey(in_record->getValueForKey(TESTS_CONTENTS_TABLE_TYPE), TESTS_CONTENTS_TABLE_TYPE);
                        tmp_test_content->setValueForKey(in_record->getValueForKey(TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE), TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE);
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
                tmp_action->setValueForKey(tmp_test_parent->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID), ACTIONS_TABLE_TEST_CONTENT_ID);
                tmp_action->setValueForKey(in_record->getValueForKey(ACTION_IMPORT_DESCRIPTION), ACTIONS_TABLE_DESCRIPTION);
                tmp_action->setValueForKey(in_record->getValueForKey(ACTION_IMPORT_WAIT_RESULT), ACTIONS_TABLE_WAIT_RESULT);
                tmp_save_result = tmp_action->saveRecord();
                delete tmp_action;
            }
            else if (tmp_test_parent->isAutomatedTest() && is_empty_string(in_record->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE)) == FALSE)
            {
                tmp_automated_action = new AutomatedAction();
                tmp_automated_action->setValueForKey(tmp_test_parent->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID), AUTOMATED_ACTIONS_TABLE_TEST_CONTENT_ID);
                tmp_automated_action->setValueForKey(in_record->getValueForKey(AUTOMATED_ACTIONS_TABLE_WINDOW_ID), AUTOMATED_ACTIONS_TABLE_WINDOW_ID);
                tmp_automated_action->setValueForKey(in_record->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE), AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE);
                tmp_automated_action->setValueForKey(in_record->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA), AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA);
                tmp_automated_action->setValueForKey(in_record->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY), AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY);
                tmp_save_result = tmp_automated_action->saveRecord();
                delete tmp_automated_action;
            }
        }
    }

    if (tmp_save_result != NOERR)
    {
        cl_transaction_rollback(Session::instance()->getClientSession());

        disconnect(this, SLOT(startImportTests(Hierarchy*)));
        disconnect(this, SLOT(importTest(Hierarchy*, GenericRecord*, bool)));

        QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_save_result));

    }
    else if (in_last_record)
    {
        endImportTests(in_parent);
    }
}


/**
  Exporter des tests
**/
void MainWindow::exportTests()
{
    Form_Data_Export	*tmp_export_form = NULL;

    tmp_export_form = new Form_Data_Export(this);
    connect(tmp_export_form, SIGNAL(startExport(QString,QByteArray,QByteArray, QByteArray)), this, SLOT(startExportTests(QString,QByteArray,QByteArray, QByteArray)));
    tmp_export_form->show();
}



void MainWindow::startExportTests(QString in_filepath, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char)
{
    QList<Hierarchy*>       tmp_records_list = Hierarchy::parentRecordsFromRecordsList(_m_tests_tree_view->selectedRecords());

    QFile                   tmp_file;

    tmp_file.setFileName(in_filepath);
    if (tmp_file.open(QIODevice::WriteOnly))
    {
        // Entête
        QString tmp_header;
        QStringList		tmp_columns_names = getImportExportTestColumnsNames();
        for (int tmp_column_index = 0; tmp_column_index < tmp_columns_names.count() - 1; ++tmp_column_index)
        {
            tmp_header = in_field_enclosing_char + tmp_columns_names[tmp_column_index] + in_field_enclosing_char + in_field_separator;
            tmp_file.write(tmp_header.toStdString().c_str());
        }
        tmp_header = in_field_enclosing_char + tmp_columns_names[tmp_columns_names.count() - 1] + in_field_enclosing_char + in_record_separator;
        tmp_file.write(tmp_header.toStdString().c_str());

        writeTestsListToExportFile(tmp_file, castHierarchyList<TestHierarchy>(tmp_records_list), in_field_separator, in_record_separator, in_field_enclosing_char);

        tmp_file.close();
    }
    else
    {
        QMessageBox::critical(this, tr("Fichier non créé"), tr("L'ouverture du fichier en écriture est impossible (%1).").arg(tmp_file.errorString()));
    }
}


void MainWindow::writeTestsListToExportFile(QFile & in_file, QList<TestHierarchy*> in_records_list, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char)
{
    TestContent             *tmp_test_content = NULL;
    QList<Action*>          tmp_test_actions;
    QList<AutomatedAction*>  tmp_test_automated_actions;

    foreach(Hierarchy *tmp_record, in_records_list)
    {
        tmp_test_content = new TestContent(_m_project_version);
        if (tmp_test_content != NULL)
        {
            if (tmp_test_content->loadRecord(tmp_record->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID)) == NOERR)
            {
                if (tmp_test_content->isAutomatedTest())
                {
                    // Actions automatisées
                    tmp_test_automated_actions = tmp_test_content->loadAutomatedActions();
                    if (tmp_test_automated_actions.isEmpty())
                    {
                        writeTestToExportFile(in_file, (TestHierarchy*)tmp_record, tmp_test_content, NULL, NULL, in_field_separator, in_record_separator, in_field_enclosing_char);
                    }
                    else
                    {
                        foreach(AutomatedAction *tmp_automated_action, tmp_test_automated_actions)
                        {
                            writeTestToExportFile(in_file, (TestHierarchy*)tmp_record, tmp_test_content, NULL, tmp_automated_action, in_field_separator, in_record_separator, in_field_enclosing_char);
                        }
                    }
                }
                else
                {
                    // Actions
                    tmp_test_actions = tmp_test_content->loadActions();
                    if (tmp_test_actions.isEmpty())
                    {
                        writeTestToExportFile(in_file, (TestHierarchy*)tmp_record, tmp_test_content, NULL, NULL, in_field_separator, in_record_separator, in_field_enclosing_char);
                    }
                    else
                    {
                        foreach(Action *tmp_action, tmp_test_actions)
                        {
                            writeTestToExportFile(in_file, (TestHierarchy*)tmp_record, tmp_test_content, tmp_action, NULL, in_field_separator, in_record_separator, in_field_enclosing_char);
                        }
                    }
                }

                // Tests enfants
                writeTestsListToExportFile(in_file, ((TestHierarchy*)tmp_record)->childs(), in_field_separator, in_record_separator, in_field_enclosing_char);
            }

            delete tmp_test_content;
        }
    }

}


void MainWindow::writeTestToExportFile(QFile & in_file, TestHierarchy *in_test, TestContent *in_test_content, Action *in_action, AutomatedAction *in_automated_action, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char)
{
    QString                 tmp_str;

    QTextDocument           tmp_doc;

    Hierarchy               *tmp_parent = NULL;

    qint64                  tmp_bytes_write = 0;

    // Action
    if (in_action != NULL)
    {
        in_action->loadAssociatedActionsForVersion(in_test->getValueForKey(TESTS_HIERARCHY_VERSION));
        if (!in_action->associatedTestActions().isEmpty())
        {
            foreach (Action *tmp_associated_action, in_action->associatedTestActions())
            {
                writeTestToExportFile(in_file, in_test, in_test_content, tmp_associated_action, NULL, in_field_separator, in_record_separator, in_field_enclosing_char);
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
        tmp_str = QString(tmp_parent->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + PARENT_HIERARCHY_SEPARATOR + tmp_str;
    }
    tmp_str += QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_SHORT_NAME));

    tmp_str = in_field_enclosing_char + tmp_str.replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Description du test
    tmp_doc.setHtml(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION));
    tmp_str = in_field_enclosing_char + tmp_doc.toPlainText().replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Catégorie du test
    tmp_str = in_field_enclosing_char + QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_CATEGORY_ID)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Priorité du test
    tmp_str = in_field_enclosing_char + QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_PRIORITY_LEVEL)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Action
    if (in_action != NULL)
    {
        // Description de l'action
        tmp_doc.setHtml(in_action->getValueForKey(ACTIONS_TABLE_DESCRIPTION));
        tmp_str = in_field_enclosing_char + tmp_doc.toPlainText().replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
        tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
        if (tmp_bytes_write < 0)    return;
    }

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Action
    if (in_action != NULL)
    {
        // Résultat attendu
        tmp_doc.setHtml(in_action->getValueForKey(ACTIONS_TABLE_WAIT_RESULT));
        tmp_str = in_field_enclosing_char + tmp_doc.toPlainText().replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
        tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
        if (tmp_bytes_write < 0)    return;
    }

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Status
    tmp_str = in_field_enclosing_char + QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_STATUS)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Automatisé
    tmp_str = in_field_enclosing_char + QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATED)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Commande d'automatisation
    tmp_str = in_field_enclosing_char + QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Paramètres de la commande d'automatisation
    tmp_str = in_field_enclosing_char + QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND_PARAMETERS)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Type
    tmp_str = in_field_enclosing_char + QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_TYPE)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    // Cas aux limites
    tmp_str = in_field_enclosing_char + QString(in_test_content->getValueForKey(TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
    tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
    if (tmp_bytes_write < 0)    return;

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    if (in_automated_action)
    {
        // Id fenetre
        tmp_str = in_field_enclosing_char + QString(in_automated_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_WINDOW_ID)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
        tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
        if (tmp_bytes_write < 0)    return;
    }

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    if (in_automated_action)
    {
        // Type de message
        tmp_str = in_field_enclosing_char + QString(in_automated_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
        tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
        if (tmp_bytes_write < 0)    return;
    }

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    if (in_automated_action)
    {
        // Données du message
        tmp_str = in_field_enclosing_char + QString(in_automated_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
        tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
        if (tmp_bytes_write < 0)    return;
    }

    // Separateur de champs
    tmp_bytes_write = in_file.write(in_field_separator);
    if (tmp_bytes_write < 0)    return;

    if (in_automated_action)
    {
        // Delai avant message suivant
        tmp_str = in_field_enclosing_char + QString(in_automated_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
        tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
        if (tmp_bytes_write < 0)    return;
    }

    // Separateur d'enregistrements
    tmp_bytes_write = in_file.write(in_record_separator);
}

void MainWindow::insertCampaign()
{
    Form_Campaign_Wizard    *tmp_campaign_wizard = new Form_Campaign_Wizard(_m_project_version, this);

    connect(tmp_campaign_wizard, SIGNAL(campaignCreated(Campaign*)), this, SLOT(insertCampaign(Campaign*)));

    tmp_campaign_wizard->show();
}


/**
  Ajout d'une campagne
**/
void MainWindow::insertCampaign(Campaign *in_campaign)
{
    _m_campaigns_table_widget->insertRow(_m_campaigns_table_widget->rowCount());
    _m_project_version->addCampaign(in_campaign);

    setCampaignAtIndex(in_campaign, _m_campaigns_table_widget->rowCount() - 1);
    _m_campaigns_table_widget->selectRow(_m_campaigns_table_widget->rowCount() - 1);
    showCampaignInfos(_m_campaigns_table_widget->selectionModel()->currentIndex());
    updateMenusActionsAndToolbar();
}


/**
  Affichage de la fenetre de recherche
**/
void MainWindow::searchProject()
{
    Form_Search_Project *tmp_form = new Form_Search_Project(_m_project_version, this);

    connect(tmp_form, SIGNAL(foundRecords(const QList<Record*> &)), this, SLOT(showSearchResults(const QList<Record*>&)));
    tmp_form->show();
}



void MainWindow::showSearchResults(const QList<Record*> & out_records_list)
{
    int	tmp_index = 0;

    _m_dock_search_widget->show();
    _m_search_table_widget->setRowCount(out_records_list.count());

    foreach(Record *tmp_record, out_records_list)
    {
        setSearchRecordAtIndex(tmp_record, tmp_index++);
    }
}


void MainWindow::setSearchRecordAtIndex(Record *in_record, int in_index)
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
            tmp_widget_item->setText(((TestHierarchy*)in_record)->getValueForKey(TESTS_HIERARCHY_SHORT_NAME));
        }
        else if (in_record->getEntityDefSignatureId() == REQUIREMENTS_HIERARCHY_SIG_ID)
        {
            tmp_icon_widget_item->setIcon(QIcon(":/images/22x22/notes.png"));
            tmp_widget_item->setText(((RequirementHierarchy*)in_record)->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME));
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
void MainWindow::insertParameter()
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
void MainWindow::removeSelectedParameter()
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
void MainWindow::parameterItemChanged(QTableWidgetItem *in_widget_item)
{
    ProjectParameter     *tmp_parameter = NULL;
    const char				*tmp_param_column = NULL;

    if (in_widget_item != NULL)
    {
        tmp_parameter = (ProjectParameter*)in_widget_item->data(Qt::UserRole).value<void*>();
        if (tmp_parameter != NULL)
        {
            std::string tmp_std_string = in_widget_item->text().toStdString();
            tmp_param_column = tmp_std_string.c_str();
            if (in_widget_item->column() == 0)
            {
                tmp_parameter->setValueForKey(tmp_param_column, PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME);
                if (tmp_parameter->saveRecord() == NOERR)
                {
                    _m_project_version->project()->updateParametersNamesList();
                }
            }
            else if (in_widget_item->column() == 1)
            {
                tmp_parameter->setValueForKey(tmp_param_column, PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE);
                tmp_parameter->saveRecord();
            }
        }
    }
}


/**
  Ajout d'une exigence fille
**/
void MainWindow::insertChildRequirement()
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

        showRequirementInfos(_m_requirements_project_tree_model->index(tmp_row, 0, index));
    }
}


/**
  Dérouler l'arbre des exigences
**/
void MainWindow::expandRequirementsTree()
{
    _m_requirements_tree_view->expandAll();
}


/**
  Dérouler les exigences sélectionnées
**/
void MainWindow::expandSelectedRequirementsTree()
{
    foreach(QModelIndex tmp_index, _m_requirements_tree_view->selectionModel()->selectedIndexes())
    {
        _m_requirements_tree_view->setExpandedIndex(tmp_index, true);
    }
}

/**
  Enrouler l'arbre des exigences
**/
void MainWindow::collapseRequirementsTree()
{
    _m_requirements_tree_view->collapseAll();
}


/**
  Enrouler les exigences sélectionnées
**/
void MainWindow::collapseSelectedRequirementsTree()
{
    foreach(QModelIndex tmp_index, _m_requirements_tree_view->selectionModel()->selectedIndexes())
    {
        _m_requirements_tree_view->setExpandedIndex(tmp_index, false);
    }
}



/**
  Ajout d'une exigence
**/
void MainWindow::insertRequirement()
{
    insertRequirementAtIndex(_m_requirements_tree_view->selectionModel()->currentIndex());
}


/**
  Supprimer les exigences sélectionnées
**/
void MainWindow::removeSelectedRequirements()
{
    removeRequirementsList(_m_requirements_tree_view->selectedRecords());
}



/**
  Suppression de la campagne sélectionnée
**/
void MainWindow::removeSelectedCampaign()
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
                            tr("Etes-vous sûr(e) de vouloir supprimer la campagne \"%1\" ?\nToutes les exécutions associées seront supprimées.").arg(tmp_campaign->getValueForKey(CAMPAIGNS_TABLE_SHORT_NAME)),
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
void MainWindow::importRequirements()
{
    QStringList		tmp_columns_names = getImportExportRequirementColumnsNames();
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
        QMessageBox::information(this, tr("Information"), tr("Les exigences seront importées après l'exigence' \"%1\".").arg(tmp_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)));

        if (tmp_requirement->parent() != _m_project_version)
            tmp_parent_requirement = new RequirementHierarchy((RequirementHierarchy*)tmp_requirement->parent());
        else
            tmp_parent_requirement = new RequirementHierarchy(_m_project_version);
    }
    else
        tmp_parent_requirement = new RequirementHierarchy(_m_project_version);

    tmp_import_form = new Form_Data_Import(tmp_parent_requirement, tmp_columns_names, &requirement_entity_import, this);
    connect(tmp_import_form, SIGNAL(startImport(Hierarchy*)), this, SLOT(startImportRequirements(Hierarchy*)));
    connect(tmp_import_form, SIGNAL(importRecord(Hierarchy*, GenericRecord*, bool)), this, SLOT(importRequirement(Hierarchy*, GenericRecord*, bool)));

    tmp_import_form->show();
}



QStringList MainWindow::getImportExportRequirementColumnsNames()
{
    QStringList		tmp_columns_names;

    tmp_columns_names << tr("Nom de l'exigence") << tr("Description de l'exigence") << tr("Catégorie de l'exigence") << tr("Priorité de l'exigence");

    return tmp_columns_names;
}


void MainWindow::startImportRequirements(Hierarchy *in_parent)
{
    RequirementContent		*tmp_requirement_content = NULL;
    int				tmp_save_result = NOERR;

    RequirementHierarchy	*tmp_requirement_parent = NULL;

    tmp_requirement_parent = (RequirementHierarchy*)in_parent;

    cl_transaction_start(Session::instance()->getClientSession());

    tmp_requirement_content = new RequirementContent(_m_project_version);

    tmp_requirement_content->setValueForKey(tr("Import d'exigences").toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME);
    tmp_requirement_content->setValueForKey(tr("Import d'exigences").toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION);
    tmp_save_result = tmp_requirement_content->saveRecord();
    if (tmp_save_result == NOERR)
    {
        tmp_requirement_parent->setDataFromRequirementContent(tmp_requirement_content);
        tmp_save_result = tmp_requirement_parent->saveRecord();
    }
    delete tmp_requirement_content;

    if (tmp_save_result != NOERR)
    {
        cl_transaction_rollback(Session::instance()->getClientSession());

        disconnect(this, SLOT(startImportRequirements(Hierarchy*)));
        disconnect(this, SLOT(importRequirement(Hierarchy*, GenericRecord*, bool)));
    }
}


void MainWindow::endImportRequirements(Hierarchy *in_parent)
{
    QList<Hierarchy*>	tmp_records_list = _m_requirements_tree_view->selectedRecords();
    Hierarchy		*tmp_requirement = NULL;
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
    }
    else
    {
        tmp_index = _m_project_version->childCount(REQUIREMENTS_HIERARCHY_SIG_ID);
        tmp_model_index  = QModelIndex();
    }

    if (_m_requirements_project_tree_model->insertItem(tmp_index, tmp_model_index, tmp_requirement_parent))
    {
        tmp_tr_result = cl_transaction_commit(Session::instance()->getClientSession());
        if (tmp_tr_result != NOERR)
            QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_tr_result));
    }
    else
    {
        cl_transaction_rollback(Session::instance()->getClientSession());
    }
}



void MainWindow::importRequirement(Hierarchy *in_parent, GenericRecord *in_record, bool in_last_record)
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

        if (is_empty_string(in_record->getValueForKey(REQUIREMENT_IMPORT_SHORT_NAME)) == FALSE)
        {
            tmp_str_list = QString(in_record->getValueForKey(REQUIREMENT_IMPORT_SHORT_NAME)).split(PARENT_HIERARCHY_SEPARATOR, QString::SkipEmptyParts);

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
                            tmp_requirement_content->setValueForKey(tmp_str_name.toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME);
                            tmp_requirement_content->setValueForKey(in_record->getValueForKey(REQUIREMENT_IMPORT_DESCRIPTION), REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION);
                            tmp_requirement_content->setValueForKey(in_record->getValueForKey(REQUIREMENT_IMPORT_CATEGORY_ID), REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID);
                            tmp_requirement_content->setValueForKey(in_record->getValueForKey(REQUIREMENT_IMPORT_PRIORITY_LEVEL), REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL);
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
        cl_transaction_rollback(Session::instance()->getClientSession());

        disconnect(this, SLOT(startImportRequirements(Hierarchy*)));
        disconnect(this, SLOT(importRequirement(Hierarchy*, GenericRecord*, bool)));

        QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_save_result));

    }
    else if (in_last_record)
    {
        endImportRequirements(in_parent);
    }

}


/**
  Exporter des exigences
**/
void MainWindow::exportRequirements()
{
    Form_Data_Export	*tmp_export_form = NULL;

    tmp_export_form = new Form_Data_Export(this);
    connect(tmp_export_form, SIGNAL(startExport(QString,QByteArray,QByteArray, QByteArray)), this, SLOT(startExportRequirements(QString,QByteArray,QByteArray, QByteArray)));
    tmp_export_form->show();
}



void MainWindow::startExportRequirements(QString in_filepath, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char)
{
    QList<Hierarchy*>       tmp_records_list = Hierarchy::parentRecordsFromRecordsList(_m_requirements_tree_view->selectedRecords());

    QFile                   tmp_file;

    tmp_file.setFileName(in_filepath);
    if (tmp_file.open(QIODevice::WriteOnly))
    {
        // Entête
        QString tmp_header;
        QStringList		tmp_columns_names = getImportExportRequirementColumnsNames();
        for (int tmp_column_index = 0; tmp_column_index < tmp_columns_names.count() - 1; ++tmp_column_index)
        {
            tmp_header = in_field_enclosing_char + tmp_columns_names[tmp_column_index] + in_field_enclosing_char + in_field_separator;
            tmp_file.write(tmp_header.toStdString().c_str());
        }
        tmp_header = in_field_enclosing_char + tmp_columns_names[tmp_columns_names.count() - 1] + in_field_enclosing_char + in_record_separator;
        tmp_file.write(tmp_header.toStdString().c_str());

        writeRequirementsListToExportFile(tmp_file, castHierarchyList<RequirementHierarchy>(tmp_records_list), in_field_separator, in_record_separator, in_field_enclosing_char);

        tmp_file.close();
    }
    else
    {
        QMessageBox::critical(this, tr("Fichier non créé"), tr("L'ouverture du fichier en écriture est impossible (%1).").arg(tmp_file.errorString()));
    }
}



void MainWindow::writeRequirementsListToExportFile(QFile & in_file, QList<RequirementHierarchy*> in_records_list, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char)
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
            if (tmp_requirement_content->loadRecord(tmp_record->getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID)) == NOERR)
            {
                // Nom de l'exigence
                tmp_str = "";
                tmp_parent = tmp_record;
                while (tmp_parent->parent() != NULL && tmp_parent->parent() != _m_project_version)
                {
                    tmp_parent = tmp_parent->parent();
                    tmp_str = QString(tmp_parent->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)) + PARENT_HIERARCHY_SEPARATOR + tmp_str;
                }
                tmp_str += QString(tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME));

                tmp_str = in_field_enclosing_char + tmp_str.replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
                tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
                if (tmp_bytes_write < 0)    break;

                // Separateur de champs
                tmp_bytes_write = in_file.write(in_field_separator);
                if (tmp_bytes_write < 0)    break;

                // Description de l'exigence
                tmp_doc.setHtml(tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION));
                tmp_str = in_field_enclosing_char + tmp_doc.toPlainText().replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
                tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
                if (tmp_bytes_write < 0)    break;

                // Separateur de champs
                tmp_bytes_write = in_file.write(in_field_separator);
                if (tmp_bytes_write < 0)    break;

                // Catégorie de l'exigence
                tmp_str = in_field_enclosing_char + QString(tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
                tmp_bytes_write = in_file.write(tmp_str.toStdString().c_str());
                if (tmp_bytes_write < 0)    break;

                // Separateur de champs
                tmp_bytes_write = in_file.write(in_field_separator);
                if (tmp_bytes_write < 0)    break;

                // Priorité de l'exigence
                tmp_str = in_field_enclosing_char + QString(tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL)).replace(in_field_enclosing_char, QString(in_field_enclosing_char+in_field_enclosing_char)) + in_field_enclosing_char;
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



QPrinter* MainWindow::print()
{
    QPrinter *printer = new QPrinter(QPrinter::HighResolution);
    QPageSetupDialog printer_setup(printer);

    if (printer_setup.exec() == QDialog::Accepted)
    {
        QString fileName = QFileDialog::getSaveFileName(this, "Export PDF", QString(), "*.pdf");
        if (!fileName.isEmpty())
        {
            if (QFileInfo(fileName).suffix().isEmpty())
                fileName.append(".pdf");

            printer->setOutputFormat(QPrinter::PdfFormat);
            printer->setOutputFileName(fileName);
        }
    }

    return printer;
}


void MainWindow::saveTestsAsHtml()
{
    QList<TestHierarchy*> tests_list = _m_tests_tree_view->selectedRecords<TestHierarchy>();

    if (!tests_list.isEmpty())
    {
        QString fileName = QFileDialog::getSaveFileName(this, "Export html", QString(), "*.html");
        if (!fileName.isEmpty())
        {
            QFile tmp_file(fileName);
            if (tmp_file.open(QIODevice::WriteOnly))
            {
                tmp_file.write(generateTestsDocument(tests_list, false, QFileInfo(tmp_file))->toHtml("utf-8").toAscii());
                tmp_file.close();
            }
            else
            {
                QMessageBox::critical(this, tr("Fichier non créé"), tr("L'ouverture du fichier en écriture est impossible (%1).").arg(tmp_file.errorString()));
            }
        }
    }
}


void MainWindow::saveRequirementsAsHtml()
{
    QList<RequirementHierarchy*> requirements_list = _m_requirements_tree_view->selectedRecords<RequirementHierarchy>();

    if (!requirements_list.isEmpty())
    {
        QString fileName = QFileDialog::getSaveFileName(this, "Export html", QString(), "*.html");
        if (!fileName.isEmpty())
        {
            QFile tmp_file(fileName);
            if (tmp_file.open(QIODevice::WriteOnly))
            {
                tmp_file.write(generateRequirementsDocument(requirements_list, false, QFileInfo(tmp_file))->toHtml("utf-8").toAscii());
                tmp_file.close();
            }
            else
            {
                QMessageBox::critical(this, tr("Fichier non créé"), tr("L'ouverture du fichier en écriture est impossible (%1).").arg(tmp_file.errorString()));
            }
        }
    }
}


QTextDocument* MainWindow::generateTestsDocument(QList<TestHierarchy*> in_tests_list, bool include_image, QFileInfo file_info)
{
    QString			tmp_doc_content = QString("<html><head></head><body>");
    QTextDocument               *tmp_doc = new QTextDocument();
    int				tmp_index = 0;

    QString         tmp_images_folder_absolute_path;
    QString         tmp_images_folder_name;

    if (!include_image)
    {
        if (file_info.isFile())
        {
            tmp_images_folder_absolute_path = file_info.canonicalFilePath()+"_images";
            QDir tmp_images_dir(tmp_images_folder_absolute_path);
            tmp_images_folder_name = tmp_images_dir.dirName();
            if (!tmp_images_dir.exists())
                file_info.absoluteDir().mkdir(tmp_images_folder_name);
        }
    }

    tmp_index = 0;
    foreach(TestHierarchy *tmp_test, in_tests_list)
    {
        tmp_index++;
        tmp_doc_content += testToHtml(tmp_doc, tmp_test, QString::number(tmp_index) + ".", 0, tmp_images_folder_absolute_path);
    }

    tmp_doc_content += "</body></html>";

    tmp_doc->setHtml(tmp_doc_content);


    return tmp_doc;
}

QTextDocument* MainWindow::generateRequirementsDocument(QList<RequirementHierarchy*> in_requirements_list, bool include_image, QFileInfo file_info)
{
    QString			tmp_doc_content = QString("<html><head></head><body>");
    QTextDocument               *tmp_doc = new QTextDocument();
    int				tmp_index = 0;

    QString         tmp_images_folder_absolute_path;
    QString         tmp_images_folder_name;

    if (!include_image)
    {
        if (file_info.isFile())
        {
            tmp_images_folder_absolute_path = file_info.canonicalFilePath()+"_images";
            QDir tmp_images_dir(tmp_images_folder_absolute_path);
            tmp_images_folder_name = tmp_images_dir.dirName();
            if (!tmp_images_dir.exists())
                file_info.absoluteDir().mkdir(tmp_images_folder_name);
        }
    }

    tmp_index = 0;
    foreach(RequirementHierarchy *tmp_requirement, in_requirements_list)
    {
        tmp_index++;
        tmp_doc_content += requirementToHtml(tmp_doc, tmp_requirement, QString::number(tmp_index) + ".", 0, tmp_images_folder_absolute_path);
    }

    tmp_doc_content += "</body></html>";

    tmp_doc->setHtml(tmp_doc_content);


    return tmp_doc;
}

void MainWindow::printTests()
{
    QList<TestHierarchy*> tests_list = _m_tests_tree_view->selectedRecords<TestHierarchy>();

    if (!tests_list.isEmpty())
    {
        QPrinter *printer = print();
        if (printer)
        {
            if (!printer->outputFileName().isEmpty())
            {
                generateTestsDocument(tests_list)->print(printer);
            }
            qDeleteAll(_m_files);
            _m_files.clear();
            delete printer;
        }
    }
}



void MainWindow::printRequirements()
{
    QList<RequirementHierarchy*> requirements_list = _m_requirements_tree_view->selectedRecords<RequirementHierarchy>();

    if (!requirements_list.isEmpty())
    {
        QPrinter *printer = print();
        if (printer)
        {
            if (!printer->outputFileName().isEmpty())
            {
                generateRequirementsDocument(requirements_list)->print(printer);
            }
            delete printer;
        }
    }
}


QString MainWindow::testToHtml(QTextDocument *in_doc, TestHierarchy *in_test, QString suffix, int level, QString images_folder)
{
    QString		tmp_html_content = QString();
    QString		tmp_html_action_content;
    int			tmp_index = 0;
    TestContent		*tmp_test_content = new TestContent();
    QTextDocument	tmp_doc;
    QList<TestContentFile*> tmp_files;
    QList<Action*> tmp_actions;
    QList<Action*> tmp_all_actions;

    if (tmp_test_content->loadRecord(in_test->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID)) == NOERR)
    {
        tmp_files = tmp_test_content->loadFiles();
        _m_files.append(tmp_files);

        tmp_actions = tmp_test_content->loadActions();
        foreach(Action *tmp_action, tmp_actions)
        {
            tmp_action->loadAssociatedActionsForVersion(in_test->getValueForKey(TESTS_HIERARCHY_VERSION), in_test->getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_CONTENT_ID));
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

        switch (level)
        {
        case 0:
            tmp_html_content += "<h2 style=\"page-break-before: auto;\">" + suffix + " " + QString(in_test->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + "</h2>";
            break;
        case 1:
            tmp_html_content += "<h3>" + suffix + " " + QString(in_test->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + "</h3>";
            break;
        case 2:
            tmp_html_content += "<h4>" + suffix + " " + QString(in_test->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + "</h4>";
            break;
        case 3:
            tmp_html_content += "<h5>" + suffix + " " + QString(in_test->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + "</h5>";
            break;
        default:
            tmp_html_content += "<h6>" + suffix + " " + QString(in_test->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)) + "</h6>";
            break;
        }

        if (!is_empty_string(tmp_test_content->getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION)))
            tmp_html_content += "<u>"+tr("Description")+"</u> : " + QString(tmp_test_content->getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION)) + "<br/>";

        const char *test_type_label = Record::matchingValueInRecordsList(Session::instance()->testsTypes(),
                                                                         TESTS_TYPES_TABLE_TEST_TYPE_ID,
                                                                         tmp_test_content->getValueForKey(TESTS_CONTENTS_TABLE_TYPE),
                                                                         TESTS_TYPES_TABLE_TEST_TYPE_LABEL);

        const char *category_label = Record::matchingValueInRecordsList(Session::instance()->requirementsCategories(),
                                                                        REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_ID,
                                                                        tmp_test_content->getValueForKey(TESTS_CONTENTS_TABLE_CATEGORY_ID),
                                                                        REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_LABEL);

        if (!is_empty_string(test_type_label))
            tmp_html_content += "<u>"+tr("Type")+"</u> : " + QString(test_type_label) + "<br/>";

        if (!is_empty_string(category_label))
            tmp_html_content += "<u>"+tr("Categorie")+"</u> : " + QString(category_label) + "<br/>";

        if (compare_values(tmp_test_content->getValueForKey(TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE), YES) == 0)
            tmp_html_content += "<u>"+tr("Test aux limites")+"</u> : Oui<br/>";
        else
            tmp_html_content += "<u>"+tr("Test aux limites")+"</u> : Non<br/>";


        tmp_html_content += "<u>"+tr("Priorité")+"</u> : " + tmp_test_content->getPriorityLabel() + "<br/>";

        if (tmp_all_actions.count() > 0)
        {
            tmp_html_content += "<u>"+tr("Liste des actions")+"</u><table border=\"1\" width=\"100%\" cellspacing=\"0\" cellpadding=\"0\">";
            tmp_html_content += "<tr><th>"+tr("Description")+"</th>";
            tmp_html_content += "<th>"+tr("Résultat attendu")+"</th></tr>";

            foreach(Action *tmp_action, tmp_all_actions)
            {
                tmp_html_action_content = tmp_action->getValueForKey(ACTIONS_TABLE_DESCRIPTION);
                loadTextEditAttachments(tmp_html_action_content, in_doc, images_folder);
                tmp_html_content += "<tr><td>" + tmp_html_action_content + "</td>";

                tmp_html_action_content = tmp_action->getValueForKey(ACTIONS_TABLE_WAIT_RESULT);
                loadTextEditAttachments(tmp_html_action_content, in_doc, images_folder);
                tmp_html_content += "<td>" + tmp_html_action_content + "</td>";
            }
            tmp_html_content += "</table>";
        }

        tmp_html_content += "<br/>";

        foreach(TestHierarchy *tmp_test, in_test->childs())
        {
            tmp_index++;
            tmp_html_content += testToHtml(in_doc, tmp_test, suffix + QString::number(tmp_index) + ".", level + 1, images_folder);
        }
    }

    delete tmp_test_content;
    qDeleteAll(tmp_all_actions);

    return tmp_html_content;
}



QString MainWindow::requirementToHtml(QTextDocument *in_doc, RequirementHierarchy *in_requirement, QString suffix, int level, QString images_folder)
{
    QString		tmp_html_content = QString();
    int			tmp_index = 0;
    RequirementContent		*tmp_requirement_content = new RequirementContent();

    if (tmp_requirement_content->loadRecord(in_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID)) == NOERR)
    {
        switch (level)
        {
        case 0:
            tmp_html_content += "<h2 style=\"page-break-before: auto;\">" + suffix + " " + QString(in_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)) + "</h2>";
            break;
        case 1:
            tmp_html_content += "<h3>" + suffix + " " + QString(in_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)) + "</h3>";
            break;
        case 2:
            tmp_html_content += "<h4>" + suffix + " " + QString(in_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)) + "</h4>";
            break;
        case 3:
            tmp_html_content += "<h5>" + suffix + " " + QString(in_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)) + "</h5>";
            break;
        default:
            tmp_html_content += "<h6>" + suffix + " " + QString(in_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)) + "</h6>";
            break;
        }

        if (!is_empty_string(tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION)))
            tmp_html_content += "<u>"+tr("Description")+"</u> : " + QString(tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION)) + "<br/>";

        const char *category_label = Record::matchingValueInRecordsList(Session::instance()->requirementsCategories(),
                                                                        REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_ID,
                                                                        tmp_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID),
                                                                        REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_LABEL);
        if (!is_empty_string(category_label))
            tmp_html_content += "<u>"+tr("Catégorie")+"</u> : " + QString(category_label) + "<br/>";

        tmp_html_content += "<u>"+tr("Priorité")+"</u> : " + tmp_requirement_content->getPriorityLabel() + "<br/>";

        tmp_html_content += "<br/>";

        foreach(RequirementHierarchy *tmp_requirement, in_requirement->childs())
        {
            tmp_index++;
            tmp_html_content += requirementToHtml(in_doc, tmp_requirement, suffix + QString::number(tmp_index) + ".", level + 1, images_folder);
        }
    }
    delete tmp_requirement_content;

    return tmp_html_content;
}


#if defined(NETSCAPE_PLUGIN)
QTNPFACTORY_BEGIN("Qt-based RTMR Plugin", "A Qt-based NSAPI plug-in for RTMR client application");
QTNPCLASS(MainWindow)
QTNPFACTORY_END()

#ifdef QAXSERVER
#include <ActiveQt/QAxFactory>
QAXFACTORY_BEGIN("{aa3216bf-7e20-482c-84c6-06167bacb616}", "{08538ca5-eb7a-4f24-a3c4-a120c6e04dc4}")
QAXCLASS(MainWindow)
QAXFACTORY_END()
#endif

#endif


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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#if defined(NETSCAPE_PLUGIN)
#include <qtbrowserplugin.h>

#ifdef QAXSERVER
#include <ActiveQt/QAxBindable>
#include <ActiveQt/QAxFactory>
#include <qt_windows.h>
#endif

#endif

#include <QMainWindow>
#include <QtGlobal>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>


#include "projectversion.h"
#include "gui/components/record_text_edit_container.h"
#include "projectversion.h"
#include "campaign.h"
#include "gui/forms/form_test.h"
#include "gui/forms/form_requirement.h"
#include "gui/forms/form_campaign.h"
#include "gui/forms/form_project_version.h"
#include "gui/forms/form_executions_reports.h"
#include "gui/forms/form_project_bugs.h"

class QMdiArea;
class QAction;
class QToolBar;
class QMenu;
class QMenuBar;
class QStatusBar;
class QMdiSubWindow;
class Frame_Project;

void updateProgressBar(unsigned long long int in_index, unsigned long long int in_count, unsigned long long int in_step, const char* in_message);

#define     DOCK_WIDGET_TESTS               "0"
#define     DOCK_WIDGET_REQUIREMENTS        "1"
#define     DOCK_WIDGET_CAMPAIGNS           "2"
#define     DOCK_WIDGET_PARAMETERS          "3"
#define     DOCK_WIDGET_SEARCH              "4"

class MainWindow : public QMainWindow, public TestActionAttachmentsManager
{
        Q_OBJECT

        Q_CLASSINFO("MIME", "application/x-rtmr:xxx:RTMR application")

    public:
        MainWindow();
        ~MainWindow();

    protected slots:

        void closeEvent(QCloseEvent *event);
        void changeEvent(QEvent*);
        void slotLanguageChanged(QAction* action);
	void timerEvent ( QTimerEvent * event );

    public slots:

        void logon();
        void logoff();

        void saveCurrentChildWindow();

        void newProject();
        void openProject();
        void exportProject();
        void importProject();
        bool closeProject();

        void setConnected();
        void setDisconnected();

        // Tests
        void insertTest();
        void insertAutomatedTest();
        void removeSelectedTests();
        void insertChildTest();
        void expandTestsTree();
        void expandSelectedTestsTree();
        void collapseTestsTree();
        void collapseSelectedTestsTree();
        void selectLinkTests();
        void selectDependantsTestsFromSelectedRequirements();
        void importTests();
        void exportTests();

        // Exigences
        void insertRequirement();
        void removeSelectedRequirements();
        void insertChildRequirement();
        void expandRequirementsTree();
        void expandSelectedRequirementsTree();
        void collapseRequirementsTree();
        void collapseSelectedRequirementsTree();
        void importRequirements();
        void exportRequirements();

        // Parametres
        void insertParameter();
        void removeSelectedParameter();

        // Campagnes
        void insertCampaign();
        void insertCampaign(Campaign *in_campaign);
        void removeSelectedCampaign();

        void showProjectProperties();
        void updateActiveSubWindowMenu();

        void manageUsers();

        void projectsReports();
        void executionReports();
        void manageCustomTestsFields();
        void manageCustomRequirementsFields();

        void showProjectBugs();

        void searchProject();

        void showNextItemInProjectHistory();
        void showPreviousItemInProjectHistory();

        void menuChildsWindowsAboutToShow();
        void setActiveSubWindow(QWidget *in_window);

        void about();

        void changePassword();
        void showOption();

        void toogleRequirementsPanel();
        void toogleTestsPanel();
        void toogleCampaignsPanel();
        void toogleParametersPanel();
        void toogleSearchPanel();

        void loadLanguage(const QString& in_language);
        void retranslateUi();


        void checkServerConnection();

    private:
        int	_m_keepalive_timer_id;
        int 	_m_previous_ping_status;

        QMdiArea    *mdiArea;
        QSignalMapper   *windowMapper;

        /* Docks widgets */
        QDockWidget *_m_dock_tests_widget;
        QDockWidget *_m_dock_requirements_widget;
        QDockWidget *_m_dock_campaigns_widget;
        QDockWidget *_m_dock_parameters_widget;
        QDockWidget *_m_dock_search_widget;

        /* Menu ? */
        QMenu       *menuHelp;

        /* Menu des projets ouverts */
        QMenu       *menuChildsWindows;
        QAction     *closeWindowAction;
        QAction     *closeAllWindowsAction;
        QAction     *tileWindowAction;
        QAction     *cascadeWindowsAction;
        QAction     *nextWindowAction;
        QAction     *previousWindowAction;
        QAction     *separatorWindowAction;
        QAction     *showRequirementsPanelAction;
        QAction     *showTestsPanelAction;
        QAction     *showCampaignsPanelAction;
        QAction     *showParametersPanelAction;
        QAction     *showSearchPanelAction;

        QAction     *aboutAction;

        /* Menu fichier */
        QMenu       *menuFichier;
        QAction     *actionConnection;
        QAction     *actionDeconnection;

        QAction     *actionEnregistrerFenetreCourante;
        QAction     *actionFermerFenetreCourante;
        QAction     *actionFermerFenetres;

        QAction     *actionNouveau_projet;
        QAction     *actionOuvrir_projet;
        QAction     *actionExporter_projet;
        QAction     *actionImporter_projet;
        QAction     *actionFermer_projet;
        QAction     *actionQuitter;

        /* Menu projet */
        QMenu *menuProjet;

        /* Menu tests */
        QMenu *menuTests;

        /* Menu exigences */
        QMenu *menuRequirements;

        /* Menu campagnes */
        QMenu *menuCampaigns;

        /* Menu historique */
        QMenu *menuHistory;

        /* Menu outils */
        QMenu       *menuOutils;
        QAction     *actionChangePassword;
        QAction     *actionShowOptions;
        QMenu	*menuSelectLanguage;

        /* Menu administration */
        QMenu *menuAdministration;
        // Gestion des utilisateurs
        QAction *manageUsersAction;
        // Gestion des rapports
        QAction *projectsReportsAction;
        QAction *toolbarSeparatorAboutAction;
        // Champs personnalisés
        QMenu *manageCustomFieldsMenu;
        QAction *manageCustomTestsFieldsAction;
        QAction *manageCustomRequirementsFieldsAction;

        /* Barre de menus */
        QMenuBar    *menuBar;

        /* Barre d'outils */
        QToolBar    *mainToolBar;

        /* Barre de status */
        QStatusBar  *statusBar;

        /* Contenu des panneaux */
        ProjectVersion        *_m_project_version;
        RecordsTreeModel      *_m_tests_project_tree_model;
        RecordsTreeView       *_m_tests_tree_view;

        RecordsTreeModel      *_m_requirements_project_tree_model;
        RecordsTreeView       *_m_requirements_tree_view;

        QTableWidget          *_m_campaigns_table_widget;

        QTableWidget          *_m_parameters_table_widget;
        QTableWidget          *_m_search_table_widget;

        QList<Record*>		_m_history;
        int				_m_history_index;

        QMap<QMdiSubWindow*, Record*>   _m_mdi_childs_map;

        // Actions de tests
        QAction *insertTestAction;
#ifdef AUTOMATION_ACTIVATED
        QAction *insertAutomatedTestAction;
#endif
        QAction *removeTestAction;
        QAction *insertChildTestAction;
        QAction *expandAllTestsAction;
        QAction *expandSelectedTestsAction;
        QAction *collapseAllTestsAction;
        QAction *collapseSelectedTestsAction;
        QAction *selectLinkTestsAction;

        QAction *importExportTestsActions;
        QAction *importTestsAction;
        QAction *exportTestsAction;

        QAction *printTestsAction;
        QAction *saveAsHtmlTestsAction;

        // Actions du menu projet
        QAction *insertParameterAction;
        QAction *removeParameterAction;
        QAction *showProjectPropertiesAction;
        QAction *executionsReportsAction;
        QAction *showProjectBugsAction;
        QAction *searchProjectAction;

        // Actions des exigences
        QAction *insertRequirementAction;
        QAction *removeRequirementAction;
        QAction *insertChildRequirementAction;
        QAction *expandAllRequirementsAction;
        QAction *expandSelectedRequirementsAction;
        QAction *collapseAllRequirementsAction;
        QAction *collapseSelectedRequirementsAction;
        QAction *selectDependantsTestsAction;

        QAction *importExportRequirementsActions;

        QAction *importRequirementsAction;
        QAction *exportRequirementsAction;

        QAction *printRequirementsAction;
        QAction *saveAsHtmlRequirementsAction;


        // Actions des campagnes
        QAction *insertCampaignAction;
        QAction *removeCampaignAction;

        // Actions du menu navigation
        QAction *historyHomeAction;
        QAction *historyPreviousAction;
        QAction *historyNextAction;

        TestHierarchy*      _m_last_imported_test;

        void createDockWidgets();
        void createPanels();
        void createToolBar();
        void createStatusBar();

        void createContextualMenus();
        void createMenus();

        void createTestsTreeView();
        void createRequirementsTreeView();

        void loadPannelProjectDatas();
        void setParameterAtIndex(ProjectParameter *in_parameter, int in_index);
        void setCampaignAtIndex(Campaign *in_campaign, int in_index);

        void readSettings();
        void writeSettings();

        void createLanguageMenu();

        int saveProjectVersion();
        QString windowTitle();
        QMdiSubWindow* createMdiChildForRecord(QWidget* child, Record* in_record);

        Record* nextRecordInHistory();
        Record* previousRecordInHistory();

        bool historyHasNextRecord();
        bool historyHasPreviousRecord();

        bool isRequirementsPanelVisible();
        bool isTestsPanelVisible();
        bool isCampaignsPanelVisible();
        bool isParametersPanelVisible();
        bool isSearchPanelVisible();

        void loadStyleSheet(bool loadFile);

        template <class T>
        QMdiSubWindow* findSubWindowForRecordColumnValue(const char* in_record_column_name, const char* in_record_column_value)
        {
            QList< QPair<const char*, const char*> > tmp_list;
            tmp_list << QPair<const char*, const char*>(in_record_column_name, in_record_column_value);
            return findSubWindowForRecordColumnsValues<T>(tmp_list);
        }

        template <class T>
        QMdiSubWindow* findSubWindowForRecordColumnsValues(QList< QPair<const char*, const char*> > in_record_columns_values)
        {
            foreach (QMdiSubWindow *window, mdiArea->subWindowList())
            {
                T *mdiChild = qobject_cast<T*>(window->widget());
                if (mdiChild && _m_mdi_childs_map[window])
                {
                    bool tmp_found = true;
                    QPair<const char*, const char*> tmp_column_value_pair;
                    foreach(tmp_column_value_pair, in_record_columns_values)
                    {
                        tmp_found = (strcmp(_m_mdi_childs_map[window]->getValueForKey(tmp_column_value_pair.first),  tmp_column_value_pair.second) == 0);
                    }

                    if (tmp_found)
                        return window;
                }
            }

            return NULL;
        }

        template <class T>
        QMdiSubWindow* findSubWindowForProjectVersion(ProjectVersion* in_project_version)
        {
            QList< QPair<const char*, const char*> > tmp_list;
            tmp_list << QPair<const char*, const char*>(PROJECTS_VERSIONS_TABLE_PROJECT_ID, in_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID));
            tmp_list << QPair<const char*, const char*>(PROJECTS_VERSIONS_TABLE_VERSION, in_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION));
            return findSubWindowForRecordColumnsValues<T>(tmp_list);
        }

        QPrinter *print();
	QTextDocument* generateTestsDocument(QList<TestHierarchy*> in_tests_list, bool include_image = true, QFileInfo file_info = QFileInfo());
	QTextDocument* generateRequirementsDocument(QList<RequirementHierarchy*> in_requirements_list, bool include_image = true, QFileInfo file_info = QFileInfo());

        QString testToHtml(QTextDocument *in_doc, TestHierarchy *in_test, QString suffix, int level, QString images_folder);
        QString requirementToHtml(QTextDocument *in_doc, RequirementHierarchy *in_requirement, QString suffix, int level, QString images_folder);


        static QStringList getImportExportTestColumnsNames();
        static QStringList getImportExportRequirementColumnsNames();

    signals:
        void testUpdated(QModelIndex);
        void requirementUpdated(QModelIndex);

    public slots:
        void showProjectVersionInfos(bool in_update_history = true);
        void showTestWithContentId(const char *in_test_content_id);
        void showTestInfos(TestHierarchy *in_test, bool in_update_history = true);
        void showOriginalTestInfos(TestHierarchy *in_test_hierarchy);
        void showRequirementInfos(RequirementHierarchy *in_requirement, bool in_update_history = true);
        void showRequirementWithOriginalContentId(const char *in_original_requirement_content_id);
        void showCampaignInfos(Campaign *in_campaign, bool in_update_history = true);
        void showExecutionsStats();
        void showBugs();

        void changeWindowTitle();

        void testSaved(TestHierarchy *in_test);
        void requirementSaved(RequirementHierarchy *in_requirement);
        void writeTestsListToExportFile(QFile & in_file, QList<TestHierarchy*> in_records_list, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char);
        void writeTestToExportFile(QFile & in_file, TestHierarchy *in_test, TestContent *in_test_content, Action *in_action, AutomatedAction *in_automated_action, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char);
        void writeRequirementsListToExportFile(QFile & in_file, QList<RequirementHierarchy*> in_records_list, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char);

        // Tests
        void insertTestAtIndex(QModelIndex index, bool in_automated_test = false);

        void removeTestsList(QList<Hierarchy*> in_records_list);
        void showTestInfos(QModelIndex in_model_index);
        void selectLinkTests(QList<Hierarchy*> in_records_list);
        QModelIndexList dependantsTestsIndexesFromRequirementsList(QList<Hierarchy*> in_records_list);
        void startExportTests(QString in_filepath, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char);
        void printTests();
        void saveTestsAsHtml();


        // Exigences
        void insertRequirementAtIndex(QModelIndex index);
        void removeRequirementsList(QList<Hierarchy*> in_records_list);
        void showRequirementInfos(QModelIndex in_model_index);
        void startExportRequirements(QString in_filepath, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char);
        void printRequirements();
        void saveRequirementsAsHtml();

        // Parametres
        void parameterItemChanged(QTableWidgetItem *in_widget_item);

        // Campagnes
        void showCampaignInfos(QModelIndex in_model_index);
        void showItemProject(Record *in_record, bool in_update_project_history = true);
        void showSearchItem(QModelIndex in_model_index);
        void searchItemsSelectionChanged();

        void showSearchResults(const QList<Record*> & out_records_list);
        void setSearchRecordAtIndex(Record *in_record, int in_index);

        void updateMenus();
        void updateMenusActionsAndToolbar();

        void showCylicRedundancyAlert();

        void startImportTests(Hierarchy *in_parent);
        void importTest(Hierarchy *in_parent, GenericRecord *in_record, bool in_last_record);
        void endImportTests(Hierarchy *in_parent);

        void startImportRequirements(Hierarchy *in_parent);
        void importRequirement(Hierarchy *in_parent, GenericRecord *in_record, bool in_last_record);
        void endImportRequirements(Hierarchy *in_parent);

        void setCurrentProject(ProjectVersion* in_project_version);
};

#endif // MAINWINDOW_H

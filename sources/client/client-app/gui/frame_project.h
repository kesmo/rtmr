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

#ifndef FRAME_PROJECT_H
#define FRAME_PROJECT_H

#include <QtGui>
#include "records/projectversion.h"
#include "records/campaign.h"
#include "gui/forms/form_test.h"
#include "gui/forms/form_requirement.h"
#include "gui/forms/form_campaign.h"
#include "gui/forms/form_project_version.h"
#include "gui/forms/form_executions_reports.h"
#include "gui/forms/form_project_bugs.h"
#include "mainwindow.h"

class Frame_Project : public QWidget
{
    Q_OBJECT

private:
    QGridLayout				*_m_grid_layout;

    ProjectVersion          *_m_project_version;
    RecordsTreeModel      *_m_tests_project_tree_model;
    RecordsTreeView       *_m_tests_tree_view;

    RecordsTreeModel      *_m_requirements_project_tree_model;
    RecordsTreeView       *_m_requirements_tree_view;

    QTableWidget            *_m_campaigns_table_widget;

    QTableWidget            *_m_parameters_table_widget;
    QTableWidget            *_m_search_table_widget;

    Form_Project_Version      	*_m_form_project_version;
    Form_Test           	*_m_form_test;
    Form_Requirement      	*_m_form_requirement;
    Form_Campaign	      	*_m_form_campaign;
    Form_Executions_Reports	*_m_executions_reports_form;
    Form_Project_Bugs		*_m_bugs_form;

    QList<Record*>		_m_history;
    int				_m_history_index;

    QWidget         *_m_dock_requirements_widget;
    QWidget         *_m_dock_campaigns_widget;
    QWidget         *_m_dock_tests_widget;
    QWidget         *_m_dock_parameters_widget;
    QWidget         *_m_dock_search_widget;


    // Actions du menu projet
    QAction *insertParameterAction;
    QAction *removeParameterAction;
    QAction *showProjectPropertiesAction;
    QAction *executionsReportsAction;
    QAction *searchProjectAction;

    // Actions de tests
    QAction *insertTestAction;
#if defined(AUTOMATION_LIB) && (defined(_WINDOWS) || defined (WIN32))
    QAction *insertAutomatedTestAction;
#endif
    QAction *removeTestAction;
    QAction *insertChildTestAction;
    QAction *expandAllTestsAction;
    QAction *expandSelectedTestsAction;
    QAction *collapseAllTestsAction;
    QAction *collapseSelectedTestsAction;
    QAction *selectLinkTestsAction;
    QAction *importTestsAction;
    QAction *exportTestsAction;

    // Actions des exigences
    QAction *insertRequirementAction;
    QAction *removeRequirementAction;
    QAction *insertChildRequirementAction;
    QAction *expandAllRequirementsAction;
    QAction *expandSelectedRequirementsAction;
    QAction *collapseAllRequirementsAction;
    QAction *collapseSelectedRequirementsAction;
    QAction *selectDependantsTestsAction;
    QAction *importRequirementsAction;
    QAction *exportRequirementsAction;

    // Actions des campagnes
    QAction *insertCampaignAction;
    QAction *removeCampaignAction;

    QAction *historyPreviousAction;
    QAction *historyNextAction;

    TestHierarchy*      _m_last_imported_test;

    void createPanels();
    void createMenus(MainWindow *parent);
    void createRequirementsTreeView();
    void createTestsTreeView();

    bool hideAllForms();

    void insertRequirementAtIndex(QModelIndex index);
    void insertTestAtIndex(QModelIndex index, bool in_automated_test = false);
    void setParameterAtIndex(ProjectParameter *in_parameter, int in_index);
    void setCampaignAtIndex(Campaign *in_campaign, int in_index);
    void setSearchRecordAtIndex(Record *in_record, int in_index);

    void writeTestsListToExportFile(QFile & in_file, QList<TestHierarchy*> in_records_list, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char);
    void writeTestToExportFile(QFile & in_file, TestHierarchy *in_test, TestContent *in_test_content, Action *in_action, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char);
    void writeRequirementsListToExportFile(QFile & in_file, QList<RequirementHierarchy*> in_records_list, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char);

protected:
    virtual void closeEvent(QCloseEvent *event);

public:

    Frame_Project(ProjectVersion *in_project_version, MainWindow *parent = 0);
    ~Frame_Project();

    ProjectVersion* projectVersion(){return _m_project_version;}

    int saveProjectVersion();
    QString windowTitle();

    Record* nextRecordInHistory();
    Record* previousRecordInHistory();

    bool historyHasNextRecord();
    bool historyHasPreviousRecord();

    bool isRequirementsPanelVisible();
    bool isTestsPanelVisible();
    bool isCampaignsPanelVisible();
    bool isParametersPanelVisible();
    bool isSearchPanelVisible();

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

    // Tests
    void insertTest();
    void insertAutomatedTest();
    void removeSelectedTests();
    void removeTestsList(QList<Hierarchy*> in_records_list);
    void insertChildTest();
    void expandTestsTree();
    void expandSelectedTestsTree();
    void collapseTestsTree();
    void collapseSelectedTestsTree();
    void showTestInfos(QModelIndex in_model_index);
    void selectLinkTests();
    void selectLinkTests(QList<Hierarchy*> in_records_list);
    void selectDependantsTestsFromSelectedRequirements();
    QModelIndexList dependantsTestsIndexesFromRequirementsList(QList<Hierarchy*> in_records_list);
    void importTests();
    void exportTests();
    void startExportTests(QString in_filepath, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char);


    // Exigences
    void insertRequirement();
    void removeSelectedRequirements();
    void removeRequirementsList(QList<Hierarchy*> in_records_list);
    void insertChildRequirement();
    void expandRequirementsTree();
    void expandSelectedRequirementsTree();
    void collapseRequirementsTree();
    void collapseSelectedRequirementsTree();
    void showRequirementInfos(QModelIndex in_model_index);
    void importRequirements();
    void exportRequirements();
    void startExportRequirements(QString in_filepath, QByteArray in_field_separator, QByteArray in_record_separator, QByteArray in_field_enclosing_char);

    // Parametres
    void insertParameter();
    void removeSelectedParameter();
    void parameterItemChanged(QTableWidgetItem *in_widget_item);

    // Campagnes
    void insertCampaign();
    void insertCampaign(Campaign *in_campaign);
    void removeSelectedCampaign();
    void showCampaignInfos(QModelIndex in_model_index);
    void showItemProject(Record *in_record, bool in_update_project_history = true);
    void showSearchItem(QModelIndex in_model_index);
    void searchItemsSelectionChanged();

    void searchProject();

    void showSearchResults(const QList<Record*> & out_records_list);

    void updateMenusActionsAndToolbar();

    void toogleRequirementsPanel();
    void toogleTestsPanel();
    void toogleCampaignsPanel();
    void toogleParametersPanel();
    void toogleSearchPanel();

    void showCylicRedundancyAlert();

    void startImportTests(Hierarchy *in_parent);
    void importTest(Hierarchy *in_parent, GenericRecord *in_record, bool in_last_record);
    void endImportTests(Hierarchy *in_parent);

    void startImportRequirements(Hierarchy *in_parent);
    void importRequirement(Hierarchy *in_parent, GenericRecord *in_record, bool in_last_record);
    void endImportRequirements(Hierarchy *in_parent);

};

#endif // FRAME_PROJECT_H

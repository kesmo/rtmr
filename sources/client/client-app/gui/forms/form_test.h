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

#ifndef FORM_TEST_H
#define FORM_TEST_H

#include <QtGui>
#include "test.h"
#include "testhierarchy.h"
#include "action.h"
#include "automatedaction.h"
#include "testrequirement.h"
#include "testcontentfile.h"
#include "bug.h"
#include "gui/components/record_text_edit_container.h"
#include "gui/components/test_action_attachments_manager.h"
#include "gui/components/abstract_project_widget.h"
#include "gui/components/records_table_model.h"
#include "customfielddesc.h"
#include "customtestfield.h"
#include "gui/components/custom_fields_controls_manager.h"

#include <bugtracker.h>
#include <clientmodule.h>

#ifdef AUTOMATION_ACTIVATED
    #include "automation.h"
#endif

#define     ALL_ACTIONS_ADD_FUNCTION            0
#define     ALL_ACTIONS_REMOVE_FUNCTION         1
#define     CURRENT_ACTION_ADD_FUNCTION         2
#define     CURRENT_ACTION_MOVE_FUNCTION_UP     3
#define     CURRENT_ACTION_MOVE_FUNCTION_DOWN   4
#define     CURRENT_ACTION_REMOVE_FUNCTION      5
#define     ACTION_NONE                         6


namespace Ui {
    class Form_Test;
}

class Form_Test :
    public AbstractProjectWidget,
    public TestActionAttachmentsManager,
    public CustomFieldsControlsManager
{
        Q_OBJECT

    public:
        Form_Test(QWidget *parent = 0);
        ~Form_Test();

        void loadTest(TestHierarchy *in_test);
        void loadTestContent(TestContent *in_test_content);

    protected:
        void setActionAtIndex(Action *in_action, int in_row);
        void setRequirementForRow(TestRequirement *in_requirement, int in_row);
        void setBugForRow(Bug *in_bug, int in_row);
        void setFileForRow(TestContentFile *in_file, int in_row);
        bool saveTest();

        virtual void showEvent(QShowEvent * event);
        virtual void closeEvent(QCloseEvent *in_event);

        bool maybeClose();


    public slots:
        void save();
        void cancel();
        void updateControls(bool in_original_test_indic);
        void addAction();
        void deleteSelectedAction();
        void moveSelectedActionUp();
        void moveSelectedActionDown();
        void actionSelectionChanged();

        void actionsDrop(QList<Record*> in_list, int in_row);

        void requirementsDrop(QList<Record*> in_list, int in_row);
        void deletedRequirementAtIndex(int in_row);

        void loadPreviousTestContent();
        void loadNextTestContent();

        void filesDrop(QList<QString> in_list, int);
        void openFileAtIndex(QTableWidgetItem *in_item);
        void deletedFileAtIndex(int in_index);

        void openBugAtIndex(QTableWidgetItem *in_item);
        void deletedBugAtIndex(int in_index);

        void actionModified();

        void showOriginalTestInfosClicked();
        void setModified();

        void showRequirementAtIndex(QModelIndex in_index);

        void addActionAfterLastAction();

        void copySelectedActionsToClipboard();
        void cutSelectedActionsToClipboard();
        void pasteClipboardDataToActionsList();
        void pasteClipboardPlainTextToActionsList();

        void selectFirstColumnAction(Record *in_action);
        void selectSecondColumnAction(Record *in_action);

        void selectAction(Record *in_action, int in_column = 0);

        void changeTab(int in_tab_index);

        void selectExternalCommandForAutomation();
        void launchStartRecordSystemEvents();
        void launchStartPlaybackSystemEvents();
        void execProcessError(QProcess::ProcessError in_process_error);
        void startRecordSystemEvents();
        void stopRecordSystemEvents();
        void startPlaybackSystemEvents();
        void stopPlaybackSystemEvents();

        void stopRecordProcess();
        void stopPlaybackProcess();

        void clipboardDataChanged();

        void showAutomatedActionValidationContextMenu(const QPoint &position);
        void showAutomatedActionValidationToolTip();

    private slots:
        void updateRowsHeight();

    signals:
        void testSaved(TestHierarchy *in_test);
        void canceled();
        void showOriginalTestInfos(TestHierarchy *in_test);
        void showRequirementWithOriginalContentId(const char *in_original_requirement_content_id);

    private:
        Ui::Form_Test 		*_m_ui;
        TestHierarchy		*_m_test;
        TestContent   		*_m_test_content;
        TestContent   		*_m_previous_test_content;
        TestContent   		*_m_next_test_content;

        QList<Action*>		_m_actions;
        QList<Action*>		_m_removed_actions;

        QList<CustomTestField*>		_m_custom_tests_fields;

        RecordsTableModel<AutomatedAction>         *_m_automated_actions_model;

        QList<TestRequirement*>	_m_requirements;
        QList<TestRequirement*>	_m_removed_requirements;

        QList<TestContentFile*>	_m_removed_files;

        QList<Bug*>			_m_bugs;
        QList<Bug*>			_m_removed_bugs;

        bool						_m_modifiable;

        Bugtracker			*_m_bugtracker;

        QProcess            _m_automated_program_record_process;
        QProcess            _m_automated_program_playback_process;

        bool                _m_is_automated_program_recording;
        bool                _m_is_automated_program_playbacking;

        void writeActionDataFromTextEditAtIndex(Action *in_action, int in_index);
        void removeActionWidgetsAtIndex(int in_row);
        RecordTextEdit* textEditAt(int in_row, int in_column);

        QMap<TestModule*, QWidget*>   _m_views_modules_map;

#ifdef AUTOMATION_ACTIVATED

        log_message_list* _m_automation_playback_messages;

#endif

        void loadPluginsViews();
        void destroyPluginsViews();
        void savePluginsDatas();

        void addAction(Action *in_action);
};

#endif // FORM_TEST_H

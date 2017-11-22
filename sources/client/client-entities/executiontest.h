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

#ifndef EXECUTIONTEST_H
#define EXECUTIONTEST_H

#include "hierarchy.h"
#include "testcampaign.h"
#include "executioncampaign.h"
#include "testhierarchy.h"

#include <QColor>

class ExecutionTest;
class ExecutionAction;
class ExecutionTestParameter;

class ExecutionTest : public Hierarchy
{

private:
    ExecutionCampaign            *_m_execution_campaign;
    ExecutionTest                *_m_parent;
    TestHierarchy                *_m_project_test;

    QList<ExecutionTest*>        _m_childs;
    QList<ExecutionAction*>      _m_actions;

    QList<ExecutionTestParameter*>      _m_parameters;

    static QList < ExecutionAction* > orderedTestActions(ExecutionAction **in_test_actions, unsigned long in_actions_count);

    static QColor OK_COLOR;
    static QColor KO_COLOR;
    static QColor INCOMPLETED_COLOR;
    static QColor BY_PASSED_COLOR;

public:

    static void initDefaultColors(QColor in_ok, QColor in_ko, QColor in_incomplete, QColor in_bypass){
	    OK_COLOR = in_ok;
	    KO_COLOR = in_ko;
	    INCOMPLETED_COLOR = in_incomplete;
	    BY_PASSED_COLOR = in_bypass;
    }

    static const QColor & okColor(){return OK_COLOR;}
    static const QColor & koColor(){return KO_COLOR;}
    static const QColor & incompleteColor(){return INCOMPLETED_COLOR;}
    static const QColor & byPassedColor(){return BY_PASSED_COLOR;}

    ExecutionTest();
    ExecutionTest(ExecutionCampaign *in_execution_campaign);
    ~ExecutionTest();

    const entity_def* getEntityDef() const;

    ExecutionCampaign* executionCampaign(){return _m_execution_campaign;}
    void setExecutionCampaign(ExecutionCampaign *in_execution_campaign);

    void setCampaignTest(TestCampaign *in_campaign_test);
    bool setChilds(QList < ExecutionTest * > in_tests_list);
    QList < ExecutionTest * > childs(){return _m_childs;}

    TestHierarchy* projectTest(){return _m_project_test;}
    void setProjectTest(TestHierarchy *in_project_test);

    void loadExecutionsActions();
    int saveExecutionsActions();
    QList<ExecutionAction*> actions(){return _m_actions;}
    ExecutionAction* actionAtIndex(int in_index){return _m_actions[in_index];}

    void loadExecutionsParameters();
    int saveExecutionsParameters();
    QList<ExecutionTestParameter*> parameters(){return _m_parameters;}
    QList<ExecutionTestParameter*> inheritedParameters();
    ExecutionTestParameter* parameterAtIndex(int in_index){return _m_parameters[in_index];}
    void addExecutionParameter(ExecutionTestParameter* in_parameter){_m_parameters.append(in_parameter);}

    void updateTestResult(bool in_recursive = true);

    float executionCoverageRate();
    float executionValidatedRate();
    float executionInValidatedRate();
    float executionBypassedRate();
    float executionIncompleteRate();
    float executionRateForResults(const char* in_test_result_id, const char* in_action_result_id);
    int executionCountForResult(const char *in_result);

    QString toFragmentHtml(QString suffix);

    // Hierarchy methods
    bool insertCopyOfChildren(int position, int count, GenericRecord *in_item, int in_child_type = 0);
    bool insertChildren(int position, int count, Hierarchy *in_test = NULL, int in_child_type = 0);
    bool removeChildren(int position, int count, bool in_move_indic = true, int in_child_type = 0);

    bool isWritable(int in_child_type = 0);

    QVariant data(int column, int role = 0);

    Hierarchy* copy();
    Hierarchy* cloneForMove(bool in_recursivly = true);
    Hierarchy* parent();

    int row() const;
    Hierarchy* child(int number, int in_child_type = 0);
    int childCount(int in_child_type = 0);

    int saveRecord();

    Hierarchy* findItemWithId(const char* in_item_id, int in_child_type = 0, bool in_recursive = true);

    QList<Bug*> loadBugs();
    void synchronizeFromTestDatas(QList<TestCampaign*> in_tests_campaign_list);

    int indexForActionWithValueForKey(const char *in_value, const char *in_key);

    bool hasChangedValues() const;

};

class TestResult : public Record
{
public:
    TestResult() : Record(getEntityDef()){}
    ~TestResult(){destroy();}

    const entity_def* getEntityDef() const{return &tests_results_table_def;}

};

#endif // EXECUTIONTEST_H

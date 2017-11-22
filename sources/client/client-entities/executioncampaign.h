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

#ifndef EXECUTIONCAMPAIGN_H
#define EXECUTIONCAMPAIGN_H

#include "hierarchy.h"
#include "campaign.h"

class ExecutionTest;
class ExecutionCampaignParameter;

class ExecutionCampaign : public Hierarchy
{
private:
    Campaign                 *_m_campaign;

    QList<ExecutionTest*>    _m_tests_list;

    QList < ExecutionCampaignParameter* >		_m_parameters_list;

    QList < ExecutionTest* > orderedExecutionsTests(ExecutionCampaign *in_execution_campaign, ExecutionTest **in_executions_tests, unsigned long in_tests_count, const char *in_parent_id);

public:
    ExecutionCampaign();
    ExecutionCampaign(Campaign *in_campaign);
    ~ExecutionCampaign();

    const entity_def* getEntityDef() const;

    QList < ExecutionTest* > testsList(){return _m_tests_list;}

    static ExecutionCampaign* create(Campaign *in_campaign);

    void loadExecutionsDatas();
    void loadExecutionsTests();
    void loadExecutionsParameters();

    Campaign* campaign(){return _m_campaign;}
    void setCampaign(Campaign *in_campaign);

    void addParameter(ExecutionCampaignParameter *in_parameter);
    void removeParameter(ExecutionCampaignParameter *in_parameter);
    QList < ExecutionCampaignParameter* > parametersList(){return _m_parameters_list;}

    float executionCoverageRate();
    float executionValidatedRate();
    float executionInValidatedRate();
    float executionBypassedRate();
    int executionCountForResult(const char *in_result);

    // Hierarchy methods
    bool insertChildren(int position, int count, Hierarchy *in_test = NULL, int in_child_type = 0);
    bool insertCopyOfChildren(int position, int count, GenericRecord *in_item, int in_child_type);
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

    void synchronizeFromCampaignDatas();
    void synchronizeFromProjectParameters();

    int indexForParameterWithValueForKey(const char *in_value, const char *in_key);

    bool hasChangedValues() const;

};

#endif // EXECUTIONCAMPAIGN_H

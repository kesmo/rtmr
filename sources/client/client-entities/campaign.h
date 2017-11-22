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

#ifndef CAMPAIGN_H
#define CAMPAIGN_H

#include "projectversion.h"
#include "hierarchy.h"

class TestCampaign;
class ExecutionCampaign;

class Campaign : public Hierarchy
{
private:
    ProjectVersion   *_m_project_version;

    QList<TestCampaign*>     _m_tests_list;

    int saveTestsCampaign();
    int saveExecutionsCampaign();

    QList<TestCampaign*> loadCampaignTests();

    bool insertTestCampaignAtIndex(TestCampaign *in_test, int in_index);

public:
    Campaign();
    Campaign(ProjectVersion *in_project);
    ~Campaign();

    const entity_def* getEntityDef() const;

    ProjectVersion* projectVersion(){ return _m_project_version;}
    void setProjectVersion(ProjectVersion* in_project_version);

    QList < TestCampaign* > testsList(){return _m_tests_list;}
    void setTestsList(QList < TestCampaign* > in_test_list){_m_tests_list = in_test_list;}
    bool insertTestAtIndex(TestHierarchy *in_test, int in_index);
    bool removeTestsAtIndex(int in_index, int in_count, bool in_move_indic = true);

    void loadTests();
    static QList < TestCampaign* > orderedCampaignTests(Campaign *in_campaign, TestCampaign **in_campaign_tests, unsigned long in_tests_count);

    Campaign* duplicate();

    // Hierarchy methods
    bool insertChildren(int position, int count, Hierarchy *in_item = NULL, int in_child_type = 0);
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

    void writeXml(QXmlStreamWriter & in_xml_writer);
    bool readXml(QXmlStreamReader & in_xml_reader);
    int saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas);

    QList<Hierarchy*>* orginalsRecordsForGenericRecord(GenericRecord *in_item = NULL, int in_child_type = 0);

};

#endif // CAMPAIGN_H

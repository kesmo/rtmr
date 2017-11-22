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

#ifndef TESTCAMPAIGN_H
#define TESTCAMPAIGN_H

#include "hierarchy.h"
#include "campaign.h"
#include "testhierarchy.h"

class TestCampaign;

class TestCampaign : public Hierarchy
{
private:
    Campaign     *_m_campaign;
    TestHierarchy         *_m_project_test;

    QList<TestCampaign*>     _m_childs;
    TestCampaign *_m_parent;

public:
    TestCampaign();
    TestCampaign(Campaign *in_campaign);
    ~TestCampaign();

    const entity_def* getEntityDef() const;

    QList<TestCampaign*> childs(){return _m_childs;}

    void setCampaign(Campaign* in_campaign);
    void setProjectTest(TestHierarchy* in_project_test);
    TestHierarchy* projectTest(){return _m_project_test;}

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

    void writeXml(QXmlStreamWriter & in_xml_writer);
    bool readXml(QXmlStreamReader & in_xml_reader);
    int saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas);

};

#endif // TESTCAMPAIGN_H

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

#ifndef TESTCONTENT_H
#define TESTCONTENT_H

#include <QVariant>
#include "record.h"
#include "test.h"
#include "projectversion.h"
#include "xmlprojectdatas.h"

class Action;
class AutomatedAction;
class TestRequirement;
class TestContentFile;

class XmlProjectDatas;

class TestContent : public Record
{
private:

    ProjectVersion   *_m_project_version;

public:
    TestContent();
    TestContent(ProjectVersion *in_project_version);

    ~TestContent();

    const entity_def* getEntityDef() const;

    ProjectVersion* getProjectVersion(){ return _m_project_version;}
    void setProjectVersion(ProjectVersion* in_project_version);

    int saveRecord();
    TestContent* copy();

    static QList<Action*> loadActions(const char *in_test_content_id);
    static QList<AutomatedAction*> loadAutomatedActions(const char *in_test_content_id);

    TestContent* previousTestContent();
    TestContent* nextTestContent();

    QList<Action*> loadActions();
    QList<AutomatedAction*> loadAutomatedActions();
    QList<TestRequirement*> loadTestRequirements();
    QList<TestRequirement*> loadTestRequirementsForProjectVersion(ProjectVersion *in_project_version);
    QList<TestContentFile*> loadFiles();

    bool isAutomatedTest() const;

    void writeXml(QXmlStreamWriter & in_xml_writer);
    bool readXml(QXmlStreamReader & in_xml_reader, XmlProjectDatas & in_xml_datas);

    int saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas);
    static QList < Action* > orderedTestActions(Action **in_test_actions, unsigned long in_actions_count);

    static TestContent* loadLastTestContentForVersion(const char *in_original_test_content_id, const char *in_project_version);

    QString getPriorityLabel();

};


class TestType : public Record
{
public:
    TestType() : Record(getEntityDef()){}
    ~TestType(){destroy();}

    const entity_def* getEntityDef() const {return &tests_types_table_def;}

};


#endif // TESTCONTENT_H

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

#ifndef TEST_HIERARCHY_H
#define TEST_HIERARCHY_H

#include <QVariant>
#include "projectversion.h"
#include "hierarchy.h"
#include "test.h"
#include "bug.h"
#include "customtestfield.h"

class TestHierarchy;

class TestHierarchy : public Hierarchy

{
private:
    ProjectVersion   *_m_project_version;

    QList<TestHierarchy*>     _m_childs;
    QList<TestHierarchy*>     _m_links;

    TestHierarchy*            _m_parent;
    TestHierarchy*            _m_original;

    bool                _m_is_a_copy;

public:
    TestHierarchy();
    TestHierarchy(ProjectVersion *in_project);
    TestHierarchy(TestHierarchy* in_parent);

    ~TestHierarchy();

    const entity_def* getEntityDef() const;

    ProjectVersion* projectVersion(){ return _m_project_version;}
    void setProjectVersion(ProjectVersion* in_project_version);

    bool setChilds(QList < TestHierarchy * > in_tests_list);
    QList < TestHierarchy * > childs(){return _m_childs;}
    static TestHierarchy* findTestWithId(const QList<TestHierarchy*>& in_tests_list, const char* in_test_id, bool in_recursive = true);

    int setDataFromRequirement(RequirementHierarchy *in_requirement);
    void setDataFromTestContent(TestContent* in_test_content);

    void searchFieldWithValue(QList<Record*> *in_found_list, const char* in_field_name, const char* in_field_value, bool in_recursive, int in_comparison_value = Record::EqualTo);

        // Hierarchy methods
    bool insertCopyOfChildren(int position, int count, GenericRecord *in_item, int in_child_type = 0);
    bool insertChildren(int in_index, int count, Hierarchy *in_child = NULL, int in_child_type = 0);
    bool removeChildren(int in_index, int count, bool in_move_indic = true, int in_child_type = 0);

    bool isWritable(int in_child_type = 0);

    QVariant data(int column, int role = 0);

    Hierarchy* copy();
    Hierarchy* cloneForMove(bool in_recursivly = true);
    bool canMove(int in_child_type = 0);

    bool mayHaveCyclicRedundancy(Hierarchy *in_dest_item, bool in_check_link);
    Hierarchy* link();
    Hierarchy* parent();

    int row() const;
    Hierarchy* child(int number, int in_child_type = 0);
    int childCount(int in_child_type = 0);
    int dbChildCount(int in_child_type);

    void setAsLinkOf(TestHierarchy* in_original);
    TestHierarchy *original();
    void unsetLink();
    void unLink();

    QList<TestHierarchy*>	links(){return _m_links;}

    int saveRecord();
    int saveChildsRecords();
    bool isCopy(){return _m_is_a_copy;}
    TestHierarchy* duplicate(bool in_recursivly);

    int deleteRecord();

    const char *columnNameForPreviousItem(){return TESTS_HIERARCHY_PREVIOUS_TEST_ID;}
    const char *columnNameForParentItem(){return TESTS_HIERARCHY_PARENT_TEST_ID;}

    Hierarchy* findItemWithId(const char* in_item_id, int in_child_type = 0, bool in_recursive = true);

    Test* cloneTestRecord();
    int loadChilds();

    void writeXml(QXmlStreamWriter & in_xml_writer);
    bool readXml(QXmlStreamReader & in_xml_reader);
    int saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas);

    QList<Bug*> loadBugs();
    QList<CustomTestField*> loadCustomFields();

    bool isAutomatedTest() const;

};

#endif // TEST_HIERARCHY_H

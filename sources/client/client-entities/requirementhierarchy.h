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

#ifndef REQUIREMENT_HIERARCHY_H
#define REQUIREMENT_HIERARCHY_H

#include "hierarchy.h"
#include "projectversion.h"
#include "requirement.h"
#include "customrequirementfield.h"

class RequirementHierarchy;
class RequirementContent;

class RequirementHierarchy : public Hierarchy
{
private:
    ProjectVersion		*_m_project_version;
    RequirementHierarchy			*_m_parent;

    QList<RequirementHierarchy*>		_m_childs;

public:
    RequirementHierarchy();
    RequirementHierarchy(ProjectVersion *in_project);
    RequirementHierarchy(RequirementHierarchy *in_parent);
    ~RequirementHierarchy();

    const entity_def* getEntityDef() const;

    ProjectVersion* projectVersion(){ return _m_project_version;}
    void setProjectVersion(ProjectVersion* in_project_version);

    bool setChilds(QList < RequirementHierarchy * > in_tests_list);
    QList < RequirementHierarchy * > childs(){return _m_childs;}

    static RequirementHierarchy* findRequirementWithId(const QList<RequirementHierarchy*>& in_requirements_list, const char* in_requirement_id, bool in_recursive = true);
    static RequirementHierarchy* findRequirementWithValueForKey(const QList<RequirementHierarchy*>& in_requirements_list, const char* in_value, const char* in_key, bool in_recursive = true);

    void searchFieldWithValue(QList<Record*> *in_found_list, const char* in_field_name, const char* in_field_value, bool in_recursive, int in_comparison_value = Record::EqualTo);

    // Hierarchy methods
    bool insertChildren(int in_index, int count, Hierarchy *in_test = NULL, int in_child_type = 0);
    bool insertCopyOfChildren(int position, int count, GenericRecord *in_item, int in_child_type = 0);
    bool removeChildren(int in_index, int count, bool in_move_indic = true, int in_child_type = 0);

    bool isWritable(int in_child_type = 0);

    QVariant data(int column, int role = 0);

    Hierarchy* copy();
    Hierarchy* cloneForMove(bool in_recursivly);
    Hierarchy* parent();

    int row() const;
    Hierarchy* child(int number, int in_child_type = 0);
    int childCount(int in_child_type = 0);
    int dbChildCount(int in_child_type);

    int saveRecord();
    int deleteRecord();

    const char *columnNameForPreviousItem(){return REQUIREMENTS_HIERARCHY_PREVIOUS_REQUIREMENT_ID;}
    const char *columnNameForParentItem(){return REQUIREMENTS_HIERARCHY_PARENT_REQUIREMENT_ID;}

    Hierarchy* findItemWithId(const char* in_item_id, int in_child_type = 0, bool in_recursive = true);

    Requirement* cloneRequirementRecord();

    int loadChilds();

    void writeXml(QXmlStreamWriter & in_xml_writer);
    bool readXml(QXmlStreamReader & in_xml_reader);
    int saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas);

    QList<TestHierarchy*> dependantsTests();

    void setDataFromRequirementContent(RequirementContent *in_requirement_content);

    QList<CustomRequirementField*> loadCustomFields();

};


#endif // REQUIREMENT_HIERARCHY_H

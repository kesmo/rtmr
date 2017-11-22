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

#ifndef REQUIREMENTCONTENT_H
#define REQUIREMENTCONTENT_H

#include "record.h"
#include "requirement.h"
#include "projectversion.h"

class XmlProjectDatas;

class RequirementContent : public Record
{
private:
    ProjectVersion   *_m_project_version;

public:
    RequirementContent();
    RequirementContent(ProjectVersion *in_project_version);

    ~RequirementContent(){destroy();}

    const entity_def* getEntityDef() const;

    void setProjectVersion(ProjectVersion* in_project_version);

    RequirementContent* copy();
    int saveRecord();

    RequirementContent* previousRequirementContent();
    RequirementContent* nextRequirementContent();

    void writeXml(QXmlStreamWriter & in_xml_writer);
    bool readXml(QXmlStreamReader & in_xml_reader);
    int saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas);

    static RequirementContent* loadLastRequirementContentForVersion(const char *in_original_requirement_content_id, const char *in_project_version);

    QList<TestRequirement*> loadTestRequirementsForProjectVersion(ProjectVersion *in_project_version);

    void copyToRequirementHierarchy(RequirementHierarchy *in_requirement);

    QString getPriorityLabel();
};


class RequirementCategory : public Record
{
public:
    RequirementCategory() : Record(getEntityDef()){}
    ~RequirementCategory(){destroy();}

    const entity_def* getEntityDef() const {return &requirements_categories_table_def;}

};


class RequirementStatus : public Record
{
public:
    RequirementStatus() : Record(getEntityDef()){}
    ~RequirementStatus(){destroy();}

    const entity_def* getEntityDef() const {return &status_table_def;}

};


#endif // REQUIREMENT_H

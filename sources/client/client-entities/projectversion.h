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

#ifndef PROJECTVERSION_H
#define PROJECTVERSION_H


#include <QtCore>
#include <QList>
#include <QString>
#include "record.h"
#include "project.h"
#include "hierarchy.h"
#include "genericrecord.h"
#include "xmlprojectdatas.h"
#include "bug.h"

class TestHierarchy;
class RequirementHierarchy;

class Campaign;

class ProjectVersion: public Hierarchy
{
private:
    Project *_m_project;

    QList < TestHierarchy* > _m_tests_hierarchy;

    QList < RequirementHierarchy* > _m_requirements_hierarchy;

    QList < Campaign* > _m_campaigns_list;

    bool insertTestAtIndex(TestHierarchy *in_test, int in_index);
    bool removeTestsAtIndex(int in_index, int in_count, bool in_move_indic = true);

    bool insertRequirementAtIndex(RequirementHierarchy *in_requirement, int in_index);
    bool removeRequirementsAtIndex(int in_index, int in_count, bool in_move_indic = true);

    void loadRequirements(net_callback_fct *in_callback);
    void loadTests(net_callback_fct *in_callback);
    void loadCampaigns();

    QList < RequirementHierarchy* > loadProjectRequirements(net_callback_fct *in_callback);
    QList < TestHierarchy* > loadProjectTests(net_callback_fct *in_callback);
    QList < Campaign* > loadProjectCampaigns();

public:
    ProjectVersion();
    ProjectVersion(Project *in_project);
    ~ProjectVersion();

    const entity_def* getEntityDef() const;

    static ProjectVersion* loadVersionForProject(const char *in_version, Project *in_project);

    static QList < TestHierarchy* > orderedProjectTests(ProjectVersion *in_project_version, TestHierarchy **in_project_tests, unsigned long in_tests_count, const char *in_parent_id, net_callback_fct *in_callback = NULL);
    static QList < RequirementHierarchy* > orderedProjectRequirements(ProjectVersion *in_project_version, RequirementHierarchy **in_project_requirements, unsigned long in_requirements_count, const char *in_parent_id, net_callback_fct *in_callback = NULL);

    Project *project(){return _m_project;}
    void setProject(Project *in_project);

    void setTestsHierarchy(QList < TestHierarchy* > in_tests_hierarchy){_m_tests_hierarchy = in_tests_hierarchy;}
    QList < TestHierarchy* > testsHierarchy(){return _m_tests_hierarchy;}

    void setRequirementsHierarchy(QList < RequirementHierarchy* > in_requirements_hierarchy){_m_requirements_hierarchy = in_requirements_hierarchy;}
    QList < RequirementHierarchy* > requirementsHierarchy(){return _m_requirements_hierarchy;}

    QList < Campaign* > campaignsList(){return _m_campaigns_list;}
    void addCampaign(Campaign *in_campaign){_m_campaigns_list.append(in_campaign);}
    void removeCampaign(Campaign *in_campaign);

    void loadProjectVersionDatas(net_callback_fct *in_callback);

    // Hierarchy methods
    bool insertCopyOfChildren(int position, int count, GenericRecord *in_item, int in_child_type = 0);
    bool insertChildren(int position, int count, Hierarchy *in_child = NULL, int in_child_type = 0);
    bool removeChildren(int position, int count, bool in_move_indic = true, int in_child_type = 0);

    bool isWritable(int in_child_type = 0);

    QVariant data(int column, int role = 0);

    Hierarchy* copy();
    Hierarchy* cloneForMove(bool in_recursivly = true);
    Hierarchy* parent();

    int row() const;
    Hierarchy* child(int number, int in_child_type = 0);
    int childCount(int in_child_type = 0);
    int dbChildCount(int in_child_type);

    int saveRecord();

    Hierarchy* findItemWithId(const char* in_item_id, int in_child_type = 0, bool in_recursive = true);

    void writeXml(QXmlStreamWriter & in_xml_writer);
    bool readXml(QXmlStreamReader & in_xml_reader);
    int saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas);

    static QString formatProjectVersionNumber(const char *in_long_version_number);

    QList<Bug*> loadBugs();

protected:
    int saveTests();
    int saveRequirements();
    int saveCampaigns();

};

#endif // PROJECTVERSION_H

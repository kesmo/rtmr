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

#ifndef PROJECT_H
#define PROJECT_H


#include <QtCore>
#include "record.h"

class ProjectGrant;
class ProjectVersion;
class ProjectParameter;
class TestContent;
class RequirementContent;

class Project: public Record
{
private:
    ProjectGrant					*_m_project_grants;

    QList < ProjectParameter* >		_m_parameters_list;
    QStringList                         _m_param_names_list;

    int saveParameters();

    void loadParameters();

public:
    Project();
    ~Project();

    const entity_def* getEntityDef() const;

    ProjectGrant* projectGrants(){return _m_project_grants;}

    void loadProjectGrants();
    void loadProjectDatas();
    QList < ProjectParameter* > loadProjectParameters();
    QList < ProjectVersion* > loadProjectVersions();
    QList < TestContent* > loadProjectTetsContents(net_callback_fct *in_callback = NULL);
    QList < RequirementContent* > loadProjectRequirementsContents(net_callback_fct *in_callback = NULL);

    ProjectParameter* parameterAtIndex(int in_index){return _m_parameters_list[in_index];}
    void addParameter(ProjectParameter *in_parameter);
    void removeParameter(ProjectParameter *in_parameter);
    void updateParametersNamesList();

    QList < ProjectParameter* > parametersList(){return _m_parameters_list;}
    QStringList parametersNames();
    const char* paramValueForParamName(const char *in_param_name);

    Project* duplicate();

    int saveRecord();

    void writeXml(QXmlStreamWriter & in_xml_writer, net_callback_fct *in_callback = NULL);
    bool readXml(QXmlStreamReader & in_xml_reader);
};

#endif // PROJECT_H

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

#include "project.h"
#include "projectversion.h"
#include "requirement.h"
#include "requirementcontent.h"
#include "testcontent.h"
#include "projectparameter.h"
#include "projectgrant.h"
#include "client.h"
#include "entities.h"
#include "xmlprojectdatas.h"

#include <string.h>
#if defined (__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

/**
 Constructeur
 **/
Project::Project() :
    Record(getEntityDef())
{
    _m_project_grants = NULL;
}

/**
 Destructeur
 **/
Project::~Project()
{
    delete _m_project_grants;
    qDeleteAll(_m_parameters_list);
    destroy();
}

/**
 Renvoie l'entite reprensentee par l'objet
 **/
const entity_def* Project::getEntityDef() const
{
    return &projects_table_def;
}

void Project::loadProjectDatas()
{
    loadProjectGrants();
    loadParameters();
}

void Project::loadProjectGrants()
{
    ProjectGrant **tmp_project_grants = NULL;

    unsigned long tmp_project_grants_count = 0;

    if (_m_project_grants != NULL)
    {
        delete _m_project_grants;
        _m_project_grants = NULL;
    }

    sprintf(CLIENT_SESSION->m_last_query, "%s=%s AND %s='%s'", PROJECTS_GRANTS_TABLE_PROJECT_ID, getIdentifier(),
            PROJECTS_GRANTS_TABLE_USERNAME, CLIENT_SESSION->m_username);

    tmp_project_grants = Record::loadRecords<ProjectGrant>(CLIENT_SESSION, &projects_grants_table_def,
                                                           CLIENT_SESSION->m_last_query, NULL, &tmp_project_grants_count);
    if (tmp_project_grants != NULL)
    {
        if (tmp_project_grants_count == 1)
        {
            _m_project_grants = tmp_project_grants[0];
            Record::freeRecords<ProjectGrant>(tmp_project_grants, 0);
        }
        else
        {
            _m_project_grants = NULL;
            Record::freeRecords<ProjectGrant>(tmp_project_grants, tmp_project_grants_count);
        }
    }
}

void Project::loadParameters()
{
    _m_parameters_list = loadProjectParameters();
    updateParametersNamesList();
}

QList<ProjectParameter*> Project::loadProjectParameters()
{
    ProjectParameter **tmp_project_parameters = NULL;
    QList<ProjectParameter*> tmp_parameters_list;

    unsigned long tmp_parameters_count = 0;

    sprintf(CLIENT_SESSION->m_last_query, "%s=%s", PROJECTS_PARAMETERS_TABLE_PROJECT_ID, getIdentifier());

    tmp_project_parameters = Record::loadRecords<ProjectParameter>(CLIENT_SESSION, &projects_parameters_table_def,
                                                                   CLIENT_SESSION->m_last_query, PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME, &tmp_parameters_count);
    if (tmp_project_parameters != NULL)
    {
        for (unsigned long tmp_param_index = 0; tmp_param_index < tmp_parameters_count; tmp_param_index++)
        {
            if (tmp_project_parameters[tmp_param_index] != NULL)
            {
                tmp_project_parameters[tmp_param_index]->setProject(this);
                tmp_parameters_list.append(tmp_project_parameters[tmp_param_index]);
            }
        }
        Record::freeRecords<ProjectParameter>(tmp_project_parameters, 0);
    }

    return tmp_parameters_list;
}

QList<ProjectVersion*> Project::loadProjectVersions()
{
    ProjectVersion **tmp_project_versions = NULL;
    unsigned long tmp_project_versions_count = 0, tmp_version_index = 0;

    QList<ProjectVersion*> tmp_versions_list;

    sprintf(CLIENT_SESSION->m_last_query, "%s=%s", PROJECTS_VERSIONS_TABLE_PROJECT_ID, getIdentifier());
    tmp_project_versions = Record::loadRecords<ProjectVersion>(CLIENT_SESSION, &projects_versions_table_def,
                                                               CLIENT_SESSION->m_last_query, PROJECTS_VERSIONS_TABLE_VERSION, &tmp_project_versions_count);
    if (tmp_project_versions != NULL)
    {
        for (tmp_version_index = 0; tmp_version_index < tmp_project_versions_count; tmp_version_index++)
        {
            if (tmp_project_versions[tmp_version_index] != NULL)
            {
                tmp_project_versions[tmp_version_index]->setProject(duplicate());
                tmp_versions_list.append(tmp_project_versions[tmp_version_index]);
            }
        }
        Record::freeRecords<ProjectVersion>(tmp_project_versions, 0);
    }

    return tmp_versions_list;
}

QList<TestContent*> Project::loadProjectTetsContents(net_callback_fct *in_callback)
{
    TestContent **tmp_tests_contents = NULL;
    unsigned long tmp_tests_count = 0, tmp_test_index = 0;

    QList<TestContent*> tmp_tests_contents_list;

    sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s AND %s IN (SELECT %s FROM %s)", TESTS_CONTENTS_TABLE_PROJECT_ID,
            getIdentifier(), TESTS_CONTENTS_TABLE_TEST_CONTENT_ID, TESTS_TABLE_TEST_CONTENT_ID, TESTS_TABLE_SIG);

    tmp_tests_contents = Record::loadRecords<TestContent>(CLIENT_SESSION, &tests_contents_table_def,
                                                          CLIENT_SESSION->m_where_clause_buffer, TESTS_CONTENTS_TABLE_VERSION, &tmp_tests_count, in_callback);
    if (tmp_tests_contents != NULL)
    {
        for (tmp_test_index = 0; tmp_test_index < tmp_tests_count; tmp_test_index++)
        {
            if (tmp_tests_contents[tmp_test_index] != NULL)
            {
                tmp_tests_contents_list.append(tmp_tests_contents[tmp_test_index]);
            }
        }
        Record::freeRecords<TestContent>(tmp_tests_contents, 0);
    }

    return tmp_tests_contents_list;
}

QList<RequirementContent*> Project::loadProjectRequirementsContents(net_callback_fct *in_callback)
{
    RequirementContent **tmp_requirements_contents = NULL;
    unsigned long tmp_requirements_count = 0, tmp_requirement_index = 0;

    QList<RequirementContent*> tmp_requirements_contents_list;

    sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s AND %s IN (SELECT %s FROM %s)",
            REQUIREMENTS_CONTENTS_TABLE_PROJECT_ID, getIdentifier(),
            REQUIREMENTS_CONTENTS_TABLE_REQUIREMENT_CONTENT_ID, REQUIREMENTS_TABLE_REQUIREMENT_CONTENT_ID,
            REQUIREMENTS_TABLE_SIG);

    tmp_requirements_contents = Record::loadRecords<RequirementContent>(CLIENT_SESSION,
                                                                        &requirements_contents_table_def, CLIENT_SESSION->m_where_clause_buffer, REQUIREMENTS_CONTENTS_TABLE_VERSION,
                                                                        &tmp_requirements_count, in_callback);
    if (tmp_requirements_contents != NULL)
    {
        for (tmp_requirement_index = 0; tmp_requirement_index < tmp_requirements_count; tmp_requirement_index++)
        {
            if (tmp_requirements_contents[tmp_requirement_index] != NULL)
            {
                tmp_requirements_contents_list.append(tmp_requirements_contents[tmp_requirement_index]);
            }
        }
        Record::freeRecords<RequirementContent>(tmp_requirements_contents, 0);
    }

    return tmp_requirements_contents_list;
}

void Project::addParameter(ProjectParameter *in_parameter)
{
    _m_parameters_list.append(in_parameter);
    updateParametersNamesList();
}

void Project::removeParameter(ProjectParameter *in_parameter)
{
    for (int tmp_index = 0; tmp_index < _m_parameters_list.count(); tmp_index++)
    {
        if (in_parameter == _m_parameters_list[tmp_index])
        {
            delete _m_parameters_list.takeAt(tmp_index);
        }
    }
    updateParametersNamesList();
}

QStringList Project::parametersNames()
{
    return _m_param_names_list;
}

void Project::updateParametersNamesList()
{
    _m_param_names_list.clear();
    foreach(ProjectParameter *tmp_param, _m_parameters_list)
    {
        if (is_empty_string(PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME) == false)
            _m_param_names_list << tmp_param->getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME);
    }

    _m_param_names_list.sort();
}

const char* Project::paramValueForParamName(const char *in_param_name)
{
    foreach(ProjectParameter *tmp_param, _m_parameters_list)
    {
        if (compare_values(tmp_param->getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME), in_param_name) == 0)
            return tmp_param->getValueForKey(PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE);
    }

    return NULL;
}

int Project::saveRecord()
{
    int tmp_return = NOERR;

    tmp_return = Record::saveRecord();

    return tmp_return;
}

int Project::saveParameters()
{
    int tmp_result = NOERR;
    ProjectParameter *tmp_param = NULL;

    for (int tmp_index = 0; tmp_index < _m_parameters_list.count() && tmp_result == NOERR; tmp_index++)
    {
        tmp_param = _m_parameters_list[tmp_index];
        if (tmp_param != NULL)
            tmp_result = tmp_param->saveRecord();
    }

    return tmp_result;
}

Project* Project::duplicate()
{
    Project *tmp_copy = clone<Project> ();
    ProjectParameter *tmp_copy_parameter = NULL;

    if (_m_project_grants != NULL)
        tmp_copy->_m_project_grants = _m_project_grants->clone<ProjectGrant> ();

    foreach(ProjectParameter* tmp_parameter, _m_parameters_list)
    {
        if (tmp_parameter != NULL)
        {
            tmp_copy_parameter = tmp_parameter->clone<ProjectParameter> ();
            tmp_copy_parameter->setProject(tmp_copy);
            tmp_copy->_m_parameters_list.append(tmp_copy_parameter);
        }
    }

    return tmp_copy;
}

void Project::writeXml(QXmlStreamWriter & in_xml_writer, net_callback_fct *in_callback)
{
    QList<ProjectParameter*> tmp_parameters_list;
    QList<ProjectVersion*> tmp_versions_list;
    QList<TestContent*> tmp_tests_list;
    QList<RequirementContent*> tmp_requirements_list;
    char tmp_server_version[12];

    in_xml_writer.writeStartElement("rtmr");
    in_xml_writer.writeAttribute("client", APP_VERSION);
    in_xml_writer.writeAttribute("protocol", QString("%1.%2.%3.%4") .arg(PROTOCOL_VERSION >> 24) .arg((PROTOCOL_VERSION
                                                                                                       >> 16) & 0xFF) .arg((PROTOCOL_VERSION >> 8) & 0xFF) .arg(PROTOCOL_VERSION & 0xFF));

    if (cl_get_server_infos(CLIENT_SESSION) == NOERR)
    {
        net_get_field(NET_MESSAGE_TYPE_INDEX + 1, CLIENT_SESSION->m_response, tmp_server_version, SEPARATOR_CHAR);
        in_xml_writer.writeAttribute("server", tmp_server_version);
    }

    in_xml_writer.writeStartElement("project");
    in_xml_writer.writeAttribute("name", getValueForKey(PROJECTS_TABLE_SHORT_NAME));

    in_xml_writer.writeTextElement("description", getValueForKey(PROJECTS_TABLE_DESCRIPTION));

    // Paramètres de projet
    tmp_parameters_list = loadProjectParameters();
    if (tmp_parameters_list.count() > 0)
    {
        in_xml_writer.writeStartElement("parameters");
        foreach(ProjectParameter *tmp_parameter, tmp_parameters_list)
        {
            in_xml_writer.writeStartElement("parameter");
            in_xml_writer.writeAttribute("name", tmp_parameter->getValueForKey(
                                             PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME));
            in_xml_writer.writeAttribute("value", tmp_parameter->getValueForKey(
                                             PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE));
            in_xml_writer.writeEndElement();
        }
        in_xml_writer.writeEndElement();
        qDeleteAll(tmp_parameters_list);
    }

    // Contenus d'exigences
    tmp_requirements_list = loadProjectRequirementsContents();
    if (tmp_requirements_list.count() > 0)
    {
        in_xml_writer.writeStartElement("requirementsContents");
        foreach(RequirementContent *tmp_requirement, tmp_requirements_list)
        {
            tmp_requirement->writeXml(in_xml_writer);
        }
        in_xml_writer.writeEndElement();
        qDeleteAll(tmp_requirements_list);
    }

    // Contenus de tests
    tmp_tests_list = loadProjectTetsContents(in_callback);
    if (tmp_tests_list.count() > 0)
    {
        in_xml_writer.writeStartElement("testsContents");
        foreach(TestContent *tmp_test, tmp_tests_list)
        {
            tmp_test->writeXml(in_xml_writer);
        }
        in_xml_writer.writeEndElement();
        qDeleteAll(tmp_tests_list);
    }

    // Versions
    tmp_versions_list = loadProjectVersions();
    if (tmp_versions_list.count() > 0)
    {
        in_xml_writer.writeStartElement("versions");
        foreach(ProjectVersion *tmp_version, tmp_versions_list)
        {
            tmp_version->writeXml(in_xml_writer);
        }
        in_xml_writer.writeEndElement();
        qDeleteAll(tmp_versions_list);
    }

    // Fin de la balise project
    in_xml_writer.writeEndElement();

    // Fin de la balise client
    in_xml_writer.writeEndElement();
}

bool Project::readXml(QXmlStreamReader & in_xml_reader)
{
    const char *tmp_protocol_version = NULL;
    const char *tmp_ptr = NULL;
    char tmp_db_major_version[4];
    char tmp_db_medium_version[4];
    QString tmp_project_name;
    QString tmp_string;

    bool tmp_return_value = true;

    ProjectParameter *tmp_parameter = NULL;
    TestContent *tmp_test = NULL;
    RequirementContent *tmp_requirement = NULL;
    ProjectVersion *tmp_version = NULL;

    XmlProjectDatas tmp_xml_datas;

    ProjectGrant *tmp_project_grants = NULL;

    int tmp_save_result = NOERR;

    if (in_xml_reader.readNextStartElement())
    {
        if (in_xml_reader.name() == "rtmr")
        {
            std::string tmp_std_string =  in_xml_reader.attributes().value("protocol").toString().toStdString();
            tmp_protocol_version = tmp_std_string.c_str();
            if (is_empty_string(tmp_protocol_version) == FALSE)
            {
                tmp_ptr = tmp_protocol_version;
                tmp_ptr = net_get_field(0, tmp_ptr, tmp_db_major_version, '.');
                tmp_ptr = net_get_field(0, tmp_ptr, tmp_db_medium_version, '.');

                if (is_empty_string(tmp_db_major_version) == FALSE && is_empty_string(tmp_db_medium_version) == FALSE
                        && atoi(tmp_db_major_version) == (PROTOCOL_VERSION >> 24) && atoi(tmp_db_medium_version)
                        == ((PROTOCOL_VERSION >> 16) & 0xFF))
                {
                    if (in_xml_reader.readNextStartElement() && in_xml_reader.name() == "project")
                    {
                        tmp_project_name = in_xml_reader.attributes().value("name").toString() + " (Import du "
                                + QDateTime::currentDateTime().toString("dddd dd MMMM yyyy à hh:mm") + ")";
                        if (tmp_project_name.isEmpty() == false)
                        {
                            setValueForKey(tmp_project_name.toStdString().c_str(), PROJECTS_TABLE_SHORT_NAME);
                            tmp_save_result = saveRecord();
                            if (tmp_save_result == NOERR)
                            {
                                while (in_xml_reader.readNextStartElement())
                                {
                                    // Description
                                    if (in_xml_reader.name() == "description")
                                    {
                                        tmp_string = in_xml_reader.readElementText();
                                        setValueForKey(tmp_string.toStdString().c_str(), PROJECTS_TABLE_DESCRIPTION);
                                        saveRecord();
                                    }
                                    // Paramètres de projet
                                    else if (in_xml_reader.name() == "parameters")
                                    {
                                        while (in_xml_reader.readNextStartElement())
                                        {
                                            if (in_xml_reader.name() == "parameter")
                                            {
                                                tmp_parameter = new ProjectParameter(this);
                                                tmp_parameter->setValueForKey(
                                                            in_xml_reader.attributes().value("name").toString().toStdString().c_str(),
                                                            PROJECTS_PARAMETERS_TABLE_PARAMETER_NAME);
                                                tmp_parameter->setValueForKey(
                                                            in_xml_reader.attributes().value("value").toString().toStdString().c_str(),
                                                            PROJECTS_PARAMETERS_TABLE_PARAMETER_VALUE);
                                                tmp_parameter->saveRecord();
                                                in_xml_reader.skipCurrentElement();
                                                delete tmp_parameter;
                                            }
                                            else
                                            {
                                                LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                                                in_xml_reader.skipCurrentElement();
                                            }
                                        }
                                    }
                                    // Contenus de tests
                                    else if (in_xml_reader.name() == "testsContents")
                                    {
                                        while (in_xml_reader.readNextStartElement())
                                        {
                                            if (in_xml_reader.name() == "testContent")
                                            {
                                                tmp_test = new TestContent();
                                                tmp_test->setValueForKey(getIdentifier(),
                                                                         TESTS_CONTENTS_TABLE_PROJECT_ID);
                                                tmp_test->readXml(in_xml_reader, tmp_xml_datas);
                                                tmp_xml_datas.m_tests_contents_list.append(tmp_test);
                                            }
                                            else
                                            {
                                                LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                                                in_xml_reader.skipCurrentElement();
                                            }
                                        }
                                    }
                                    // Contenus d'exigences
                                    else if (in_xml_reader.name() == "requirementsContents")
                                    {
                                        while (in_xml_reader.readNextStartElement())
                                        {
                                            if (in_xml_reader.name() == "requirementContent")
                                            {
                                                tmp_requirement = new RequirementContent();
                                                tmp_requirement->setValueForKey(getIdentifier(),
                                                                                REQUIREMENTS_CONTENTS_TABLE_PROJECT_ID);
                                                tmp_requirement->readXml(in_xml_reader);
                                                tmp_xml_datas.m_requirements_contents_list.append(tmp_requirement);
                                            }
                                            else
                                            {
                                                LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                                                in_xml_reader.skipCurrentElement();
                                            }
                                        }
                                    }
                                    // Versions
                                    else if (in_xml_reader.name() == "versions")
                                    {
                                        while (in_xml_reader.readNextStartElement())
                                        {
                                            if (in_xml_reader.name() == "version")
                                            {
                                                tmp_version = new ProjectVersion(duplicate());
                                                tmp_version->setValueForKey(getIdentifier(),
                                                                            PROJECTS_VERSIONS_TABLE_PROJECT_ID);
                                                tmp_version->readXml(in_xml_reader);
                                                tmp_xml_datas.m_versions_list.append(tmp_version);
                                            }
                                            else
                                            {
                                                LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                                                in_xml_reader.skipCurrentElement();
                                            }
                                        }
                                    }
                                    else
                                    {
                                        LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                                        in_xml_reader.skipCurrentElement();
                                    }
                                }

                                // Traitement post-lecture
                                foreach(ProjectVersion* tmp_project_version, tmp_xml_datas.m_versions_list)
                                {
                                    tmp_project_version->saveFromXmlProjectDatas(tmp_xml_datas);
                                }

                                // Droits sur le projet
                                tmp_project_grants = new ProjectGrant();
                                tmp_project_grants->setValueForKey(getIdentifier(), PROJECTS_GRANTS_TABLE_PROJECT_ID);
                                tmp_project_grants->setValueForKey(CLIENT_SESSION->m_username,
                                                                   PROJECTS_GRANTS_TABLE_USERNAME);
                                tmp_project_grants->setValueForKey(PROJECT_GRANT_WRITE,
                                                                   PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC);
                                tmp_project_grants->setValueForKey(PROJECT_GRANT_WRITE,
                                                                   PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC);
                                tmp_project_grants->setValueForKey(PROJECT_GRANT_WRITE,
                                                                   PROJECTS_GRANTS_TABLE_MANAGE_CAMPAIGNS_INDIC);
                                tmp_project_grants->setValueForKey(PROJECT_GRANT_WRITE,
                                                                   PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC);

                                tmp_project_grants->saveRecord();
                            }
                            else
                            {
                                sprintf(CLIENT_SESSION->m_last_error_msg, "%s\n", cl_get_error_message(
                                            CLIENT_SESSION, tmp_save_result));
                                tmp_return_value = false;
                            }
                        }
                        else
                        {
                            sprintf(CLIENT_SESSION->m_last_error_msg,
                                    "Tag <b>project</b> doesn't have attribute <i>name</i>.\n");
                            tmp_return_value = false;
                        }
                    }
                    else
                    {
                        sprintf(CLIENT_SESSION->m_last_error_msg,
                                "Tag <b>project</b> must be direct child of <b>rtmr</b> tag.\n");
                        tmp_return_value = false;
                    }
                }
                else
                {
                    sprintf(CLIENT_SESSION->m_last_error_msg, "The version of the xml file isn't compliant.\n");
                    tmp_return_value = false;
                }
            }
            else
            {
                sprintf(CLIENT_SESSION->m_last_error_msg,
                        "Tag <b>rtmr</b> doesn't have attribute <i>protocol</i>.\n");
                tmp_return_value = false;
            }
        }
        else
        {
            sprintf(CLIENT_SESSION->m_last_error_msg,
                    "Tag <b>rtmr</b> must be root of xml document.\n");
            tmp_return_value = false;
        }
    }

    return tmp_return_value;
}

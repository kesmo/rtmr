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

#include "testcontent.h"
#include "testrequirement.h"
#include "action.h"
#include "automatedaction.h"
#include "testcontentfile.h"
#include "utilities.h"
#include "requirementhierarchy.h"
#include "xmlprojectdatas.h"
#include "session.h"
#include "automatedactionvalidation.h"

/**
  Constructeur
**/
TestContent::TestContent() : Record(getEntityDef())
{
    _m_project_version = NULL;
}


/**
  Constructeur
**/
TestContent::TestContent(ProjectVersion *in_project_version) : Record(getEntityDef())
{
    _m_project_version = NULL;
    setProjectVersion(in_project_version);
}


/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* TestContent::getEntityDef() const
{
    return &tests_contents_table_def;
}


/**
  Destructeur
**/
TestContent::~TestContent()
{
    destroy();
}

void TestContent::setProjectVersion(ProjectVersion* in_project_version)
{
    if (in_project_version != NULL)
    {
        _m_project_version = in_project_version;
        if (is_empty_string(getIdentifier()))
        {
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), TESTS_CONTENTS_TABLE_PROJECT_ID);
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), TESTS_CONTENTS_TABLE_VERSION);
        }
    }
}


bool TestContent::isAutomatedTest() const
{
    return (compare_values(getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATED), YES) == 0);
}


QList<Action*> TestContent::loadActions()
{
    QList<Action*>	tmp_actions_list = loadActions(getIdentifier());

    foreach(Action *tmp_action, tmp_actions_list)
    {
        tmp_action->setTestContent(this);
    }

    return tmp_actions_list;
}

QList<Action*> TestContent::loadActions(const char *in_test_content_id)
{
    Action               **tmp_actions_list = NULL;
    unsigned long           tmp_actions_count = 0;
    QList<Action*>       tmp_actions;

    if (is_empty_string(in_test_content_id) == FALSE)
    {
        sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s", ACTIONS_TABLE_TEST_CONTENT_ID, in_test_content_id);
        tmp_actions_list = Record::loadRecords<Action>(CLIENT_SESSION, &actions_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_actions_count);
        if (tmp_actions_list != NULL)
        {
            if (tmp_actions_count > 0)
                tmp_actions = orderedTestActions(tmp_actions_list, tmp_actions_count);

            Record::freeRecords<Action>(tmp_actions_list, 0);
        }
    }

    return tmp_actions;
}


QList<AutomatedAction*> TestContent::loadAutomatedActions()
{
    QList<AutomatedAction*>	tmp_actions_list = loadAutomatedActions(getIdentifier());

    foreach(AutomatedAction *tmp_action, tmp_actions_list)
    {
        tmp_action->setTestContent(this);
    }

    return tmp_actions_list;
}


QList<AutomatedAction*> TestContent::loadAutomatedActions(const char *in_test_content_id)
{
    AutomatedAction         **tmp_actions_list = NULL;
    unsigned long           tmp_actions_count = 0;
    QList<AutomatedAction*> tmp_actions;

    if (is_empty_string(in_test_content_id) == FALSE)
    {
        sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s", AUTOMATED_ACTIONS_TABLE_TEST_CONTENT_ID, in_test_content_id);
        tmp_actions_list = Record::loadRecords<AutomatedAction>(CLIENT_SESSION, &automated_actions_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_actions_count);
        if (tmp_actions_list != NULL)
        {
            if (tmp_actions_count > 0)
                tmp_actions = orderedRecords<AutomatedAction>(tmp_actions_list, tmp_actions_count, AUTOMATED_ACTIONS_TABLE_PREVIOUS_ACTION_ID);

            Record::freeRecords<AutomatedAction>(tmp_actions_list, 0);
        }
    }

    return tmp_actions;
}




QList<TestContentFile*> TestContent::loadFiles()
{
    TestContentFile      **tmp_files_list = NULL;
    unsigned long           tmp_files_count = 0;
    TestContentFile		*tmp_file = NULL;
    QList<TestContentFile*>       tmp_files;

    if (is_empty_string(getIdentifier()) == FALSE)
    {
        sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s", TESTS_CONTENTS_FILES_TABLE_TEST_CONTENT_ID, getIdentifier());
        tmp_files_list = Record::loadRecords<TestContentFile>(CLIENT_SESSION, &tests_contents_files_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_files_count);
        if (tmp_files_list != NULL)
        {
            if (tmp_files_count > 0)
            {
                for (unsigned long tmp_files_index = 0; tmp_files_index < tmp_files_count; tmp_files_index++)
                {
                    tmp_file = tmp_files_list[tmp_files_index];
                    if (tmp_file != NULL)
                    {
                        tmp_file->setTestContent(this);
                        tmp_files.append(tmp_file);
                    }
                }
            }
            Record::freeRecords<TestContentFile>(tmp_files_list, 0);
        }
    }

    return tmp_files;
}



/**
  Ordonner la liste des actions
**/
QList < Action* > TestContent::orderedTestActions(Action **in_test_actions, unsigned long in_actions_count)
{
    unsigned long         tmp_actions_index = 0;
    Action             *tmp_action = NULL;
    QList < Action* >  tmp_actions_list;

    const char                  *tmp_previous_id = NULL;


    if (in_test_actions != NULL && in_actions_count > 0)
    {
        do
        {
            for (tmp_actions_index = 0; tmp_actions_index < in_actions_count; tmp_actions_index++)
            {
                tmp_action = in_test_actions[tmp_actions_index];
                if (tmp_action != NULL
                && compare_values(tmp_previous_id, tmp_action->getValueForKey(ACTIONS_TABLE_PREVIOUS_ACTION_ID)) == 0)
                {
                    tmp_actions_list.append(tmp_action);
                    tmp_previous_id = tmp_action->getIdentifier();

                    break;
                }
            }
        }
        while (is_empty_string(tmp_previous_id) == FALSE && !(tmp_actions_index >= in_actions_count));

        for (tmp_actions_index = 0; tmp_actions_index < in_actions_count; tmp_actions_index++)
        {
            tmp_action = in_test_actions[tmp_actions_index];
            if (tmp_action != NULL && tmp_actions_list.indexOf(tmp_action) < 0)
            {
                tmp_actions_list.append(tmp_action);
            }
        }

    }

    return tmp_actions_list;
}


QList<TestRequirement*> TestContent::loadTestRequirements()
{
    TestRequirement      **tmp_requirements_list = NULL;
    unsigned long           tmp_requirements_count = 0;

    QList<TestRequirement*>       tmp_requirements;

    sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s",TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID, getValueForKey(TESTS_CONTENTS_TABLE_TEST_CONTENT_ID));

    tmp_requirements_list = Record::loadRecords<TestRequirement>(CLIENT_SESSION, &tests_requirements_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_requirements_count);
    if (tmp_requirements_list != NULL)
    {
        for (unsigned long tmp_requirements_index = 0; tmp_requirements_index < tmp_requirements_count; tmp_requirements_index++)
        {
            tmp_requirements.append(tmp_requirements_list[tmp_requirements_index]);
        }
        Record::freeRecords(tmp_requirements_list, 0);
    }

    return tmp_requirements;
}

QList<TestRequirement*> TestContent::loadTestRequirementsForProjectVersion(ProjectVersion *in_project_version)
{
    QList<TestRequirement*>       	tmp_requirements;
    RequirementHierarchy			*tmp_requirement_hierachy = NULL;

    tmp_requirements = loadTestRequirements();
    if (in_project_version != NULL)
    {
        foreach (TestRequirement *tmp_test_requirement, tmp_requirements)
        {
            tmp_requirement_hierachy = RequirementHierarchy::findRequirementWithValueForKey(in_project_version->requirementsHierarchy(), tmp_test_requirement->getValueForKey(TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID), REQUIREMENTS_HIERARCHY_ORIGINAL_REQUIREMENT_CONTENT_ID);
            if (tmp_requirement_hierachy != NULL)
            {
                tmp_test_requirement->setRequirementShortName(tmp_requirement_hierachy->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME));
                tmp_test_requirement->setRequirementCategory(tmp_requirement_hierachy->getValueForKey(REQUIREMENTS_HIERARCHY_CATEGORY_ID));
                tmp_test_requirement->setRequirementVersion(tmp_requirement_hierachy->getValueForKey(REQUIREMENTS_HIERARCHY_CONTENT_VERSION));
            }
            else
            {
                tmp_test_requirement->setRequirementShortName("inconnu");
                tmp_test_requirement->setRequirementCategory("inconnue");
                tmp_test_requirement->setRequirementVersion("inconnue");
            }
        }
    }

    return tmp_requirements;
}


int TestContent::saveRecord()
{
    int tmp_return = NOERR;

    if (is_empty_string(getIdentifier()))
    {
        if (_m_project_version != NULL)
        {
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), TESTS_CONTENTS_TABLE_PROJECT_ID);
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), TESTS_CONTENTS_TABLE_VERSION);
        }

        tmp_return = Record::saveRecord();
    }
    else
        tmp_return = Record::saveRecord();

    return tmp_return;
}


TestContent* TestContent::copy()
{
    TestContent          *tmp_copy = new TestContent(_m_project_version);

    tmp_copy->setValueForKey(getValueForKey(TESTS_CONTENTS_TABLE_SHORT_NAME),TESTS_CONTENTS_TABLE_SHORT_NAME);
    tmp_copy->setValueForKey(getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION),TESTS_CONTENTS_TABLE_DESCRIPTION);
    tmp_copy->setValueForKey(getValueForKey(TESTS_CONTENTS_TABLE_PRIORITY_LEVEL),TESTS_CONTENTS_TABLE_PRIORITY_LEVEL);
    tmp_copy->setValueForKey(getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID),TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID);

    return tmp_copy;
}


TestContent* TestContent::previousTestContent()
{
    TestContent			*tmp_previous_test = NULL;
    TestContent			**tmp_project_tests = NULL;
    unsigned long         	tmp_tests_count = 0;

    sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s AND %s<'%s' ORDER BY %s DESC LIMIT 1",
    TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID,
    getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID),
    TESTS_CONTENTS_TABLE_VERSION,
    getValueForKey(TESTS_CONTENTS_TABLE_VERSION),
    TESTS_CONTENTS_TABLE_VERSION);

    tmp_project_tests = Record::loadRecords<TestContent>(CLIENT_SESSION, &tests_contents_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_tests_count);
    if (tmp_project_tests != NULL)
    {
        if (tmp_tests_count == 1)
        {
            tmp_previous_test = tmp_project_tests[0];
            Record::freeRecords<TestContent>(tmp_project_tests, 0);
        }
        else
        {
            Record::freeRecords<TestContent>(tmp_project_tests, tmp_tests_count);
        }
    }

    return tmp_previous_test;
}


TestContent* TestContent::nextTestContent()
{
    TestContent			*tmp_next_test = NULL;
    TestContent			**tmp_project_tests = NULL;
    unsigned long         	tmp_tests_count = 0;

    sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s AND %s>'%s' ORDER BY %s ASC LIMIT 1",
    TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID,
    getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID),
    TESTS_CONTENTS_TABLE_VERSION,
    getValueForKey(TESTS_CONTENTS_TABLE_VERSION),
    TESTS_CONTENTS_TABLE_VERSION);

    tmp_project_tests = Record::loadRecords<TestContent>(CLIENT_SESSION, &tests_contents_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_tests_count);
    if (tmp_project_tests != NULL)
    {
        if (tmp_tests_count == 1)
        {
            tmp_next_test = tmp_project_tests[0];
            Record::freeRecords<TestContent>(tmp_project_tests, 0);
        }
        else
        {
            Record::freeRecords<TestContent>(tmp_project_tests, tmp_tests_count);
        }
    }

    return tmp_next_test;
}


TestContent* TestContent::loadLastTestContentForVersion(const char *in_original_test_content_id, const char *in_project_version)
{
    TestContent			*tmp_test_content = NULL;
    TestContent			**tmp_project_tests = NULL;
    unsigned long         	tmp_tests_count = 0;

    sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s AND %s<='%s' ORDER BY %s DESC LIMIT 1",
    TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID,
    in_original_test_content_id,
    TESTS_CONTENTS_TABLE_VERSION,
    in_project_version,
    TESTS_CONTENTS_TABLE_VERSION);

    tmp_project_tests = Record::loadRecords<TestContent>(CLIENT_SESSION, &tests_contents_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_tests_count);
    if (tmp_project_tests != NULL)
    {
        if (tmp_tests_count == 1)
        {
            tmp_test_content = tmp_project_tests[0];
            Record::freeRecords<TestContent>(tmp_project_tests, 0);
        }
        else
        {
            Record::freeRecords<TestContent>(tmp_project_tests, tmp_tests_count);
        }
    }

    return tmp_test_content;
}



QString TestContent::getPriorityLabel()
{
    switch(QString(getValueForKey(TESTS_CONTENTS_TABLE_PRIORITY_LEVEL)).toInt())
    {
	case 1:
            return TR_CUSTOM_MESSAGE("Nulle");
            break;
	case 2:
            return TR_CUSTOM_MESSAGE("Faible");
            break;
	case 3:
            return TR_CUSTOM_MESSAGE("Elevée");
            break;
	case 4:
            return TR_CUSTOM_MESSAGE("Moyenne");
            break;
	case 5:
            return TR_CUSTOM_MESSAGE("Critique");
            break;
    }

    return QString();}

void TestContent::writeXml(QXmlStreamWriter & in_xml_writer)
{
    QList<Action*>		tmp_actions_list;
    QList<AutomatedAction*>		tmp_automated_actions_list;
    QList<TestRequirement*>	tmp_requirements_list;

    in_xml_writer.writeStartElement("testContent");

    in_xml_writer.writeAttribute("id", getIdentifier());
    in_xml_writer.writeAttribute("originalTestContentId", getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID));
    in_xml_writer.writeAttribute("version", getValueForKey(TESTS_CONTENTS_TABLE_VERSION));
    in_xml_writer.writeAttribute("name", getValueForKey(TESTS_CONTENTS_TABLE_SHORT_NAME));
    in_xml_writer.writeAttribute("type", getValueForKey(TESTS_CONTENTS_TABLE_TYPE));
    in_xml_writer.writeAttribute("category", getValueForKey(TESTS_CONTENTS_TABLE_CATEGORY_ID));
    in_xml_writer.writeAttribute("limit_test_case", getValueForKey(TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE));
    in_xml_writer.writeAttribute("priority", getValueForKey(TESTS_CONTENTS_TABLE_PRIORITY_LEVEL));
    in_xml_writer.writeAttribute("status", getValueForKey(TESTS_CONTENTS_TABLE_STATUS));
    in_xml_writer.writeAttribute("automated", getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATED));
    in_xml_writer.writeTextElement("automation_command", getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND));
    in_xml_writer.writeTextElement("automation_command_parameters", getValueForKey(TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND_PARAMETERS));
    in_xml_writer.writeTextElement("description", getValueForKey(TESTS_CONTENTS_TABLE_DESCRIPTION));

    if (!isAutomatedTest())
    {
        tmp_actions_list = loadActions();
        if (tmp_actions_list.count() > 0)
        {
            in_xml_writer.writeStartElement("actions");
            foreach(Action *tmp_action, tmp_actions_list)
            {
                in_xml_writer.writeStartElement("action");
                in_xml_writer.writeAttribute("name", tmp_action->getValueForKey(ACTIONS_TABLE_SHORT_NAME));
                in_xml_writer.writeAttribute("linkOriginalTestContentId", tmp_action->getValueForKey(ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID));
                in_xml_writer.writeTextElement("description", tmp_action->getValueForKey(ACTIONS_TABLE_DESCRIPTION));
                in_xml_writer.writeTextElement("result", tmp_action->getValueForKey(ACTIONS_TABLE_WAIT_RESULT));
                in_xml_writer.writeEndElement();
            }
            in_xml_writer.writeEndElement();
            qDeleteAll(tmp_actions_list);
        }
    }
    else
    {
        tmp_automated_actions_list = loadAutomatedActions();
        if (tmp_automated_actions_list.count() > 0)
        {
            in_xml_writer.writeStartElement("automated_actions");
            foreach(AutomatedAction *tmp_action, tmp_automated_actions_list)
            {
                in_xml_writer.writeStartElement("automated_action");
                in_xml_writer.writeTextElement("window_id", tmp_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_WINDOW_ID));
                in_xml_writer.writeTextElement("message_type", tmp_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE));
                in_xml_writer.writeTextElement("message_data", tmp_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA));
                in_xml_writer.writeTextElement("message_time_delay", tmp_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY));

                QList<AutomatedActionValidation*> tmp_automated_action_validations_list = tmp_action->loadChilds();
                if (tmp_automated_action_validations_list.count() > 0)
                {
                    in_xml_writer.writeStartElement("validations");
                    foreach(AutomatedActionValidation *tmp_validation, tmp_automated_action_validations_list)
                    {
                        in_xml_writer.writeStartElement("validation");
                        in_xml_writer.writeAttribute("module_name", tmp_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_NAME));
                        in_xml_writer.writeAttribute("module_version", tmp_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_VERSION));
                        in_xml_writer.writeAttribute("module_function_name", tmp_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME));
                        in_xml_writer.writeAttribute("module_function_parameters", tmp_validation->getValueForKey(AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_PARAMETERS));
                        in_xml_writer.writeEndElement();
                    }
                    in_xml_writer.writeEndElement();
                    qDeleteAll(tmp_automated_action_validations_list);
                }

                in_xml_writer.writeEndElement();
            }
            in_xml_writer.writeEndElement();
            qDeleteAll(tmp_automated_actions_list);
        }
    }

    tmp_requirements_list = loadTestRequirements();
    if (tmp_requirements_list.count() > 0)
    {
        in_xml_writer.writeStartElement("testsRequirements");
        foreach(TestRequirement *tmp_test_requirement, tmp_requirements_list)
        {
            tmp_test_requirement->writeXml(in_xml_writer);
        }
        in_xml_writer.writeEndElement();
        qDeleteAll(tmp_requirements_list);
    }

    in_xml_writer.writeEndElement();
}


bool TestContent::readXml(QXmlStreamReader & in_xml_reader, XmlProjectDatas & in_xml_datas)
{
    QString		tmp_id = in_xml_reader.attributes().value("id").toString();
    QString		tmp_original_test_content_id = in_xml_reader.attributes().value("originalTestContentId").toString();
    QString		tmp_version = in_xml_reader.attributes().value("version").toString();
    QString		tmp_name = in_xml_reader.attributes().value("name").toString();
    QString		tmp_type = in_xml_reader.attributes().value("type").toString();
    QString		tmp_category = in_xml_reader.attributes().value("category").toString();
    QString		tmp_limit = in_xml_reader.attributes().value("limit_test_case").toString();
    QString		tmp_priority = in_xml_reader.attributes().value("priority").toString();
    QString		tmp_status = in_xml_reader.attributes().value("status").toString();
    QString		tmp_automated = in_xml_reader.attributes().value("automated").toString();

    QString		tmp_text;

    QList<Action*>	tmp_actions_list;
    QList<AutomatedAction*>	tmp_automated_actions_list;
    QList<TestRequirement*>	tmp_tests_requirements_list;
    Action		*tmp_action = NULL;
    AutomatedAction		*tmp_automated_action = NULL;
    AutomatedActionValidation		*tmp_automated_action_validation = NULL;
    TestRequirement	*tmp_test_requirement = NULL;

    setValueForKey(tmp_id.toStdString().c_str(), TESTS_CONTENTS_TABLE_TEST_CONTENT_ID);
    setValueForKey(tmp_original_test_content_id.toStdString().c_str(), TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID);
    setValueForKey(tmp_version.toStdString().c_str(), TESTS_CONTENTS_TABLE_VERSION);
    setValueForKey(tmp_name.toStdString().c_str(), TESTS_CONTENTS_TABLE_SHORT_NAME);
    setValueForKey(tmp_type.toStdString().c_str(), TESTS_CONTENTS_TABLE_TYPE);
    setValueForKey(tmp_category.toStdString().c_str(), TESTS_CONTENTS_TABLE_CATEGORY_ID);
    setValueForKey(tmp_limit.toStdString().c_str(), TESTS_CONTENTS_TABLE_LIMIT_TEST_CASE);
    setValueForKey(tmp_priority.toStdString().c_str(), TESTS_CONTENTS_TABLE_PRIORITY_LEVEL);
    setValueForKey(tmp_status.toStdString().c_str(), TESTS_CONTENTS_TABLE_STATUS);
    setValueForKey(tmp_automated.toStdString().c_str(), TESTS_CONTENTS_TABLE_AUTOMATED);

    while (in_xml_reader.readNextStartElement())
    {
        // Description
        if (in_xml_reader.name() == "description")
        {
            tmp_text = in_xml_reader.readElementText();
            setValueForKey(tmp_text.toStdString().c_str(), TESTS_CONTENTS_TABLE_DESCRIPTION);
        }
        else if (in_xml_reader.name() == "automation_command")
        {
            tmp_text = in_xml_reader.readElementText();
            setValueForKey(tmp_text.toStdString().c_str(), TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND);
        }
        else if (in_xml_reader.name() == "automation_command_parameters")
        {
            tmp_text = in_xml_reader.readElementText();
            setValueForKey(tmp_text.toStdString().c_str(), TESTS_CONTENTS_TABLE_AUTOMATION_COMMAND_PARAMETERS);
        }
        else if (in_xml_reader.name() == "actions")
        {
            while (in_xml_reader.readNextStartElement())
            {
                if (in_xml_reader.name() == "action")
                {
                    tmp_action = new Action();
                    tmp_action->setValueForKey(in_xml_reader.attributes().value("name").toString().toStdString().c_str(), ACTIONS_TABLE_SHORT_NAME);
                    tmp_action->setValueForKey(in_xml_reader.attributes().value("linkOriginalTestContentId").toString().toStdString().c_str(), ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID);

                    while (in_xml_reader.readNextStartElement())
                    {
                        if (in_xml_reader.name() == "description")
                        {
                            tmp_text = in_xml_reader.readElementText();
                            tmp_action->setValueForKey(tmp_text.toStdString().c_str(), ACTIONS_TABLE_DESCRIPTION);
                        }
                        else if (in_xml_reader.name() == "result")
                        {
                            tmp_text = in_xml_reader.readElementText();
                            tmp_action->setValueForKey(tmp_text.toStdString().c_str(), ACTIONS_TABLE_WAIT_RESULT);
                        }
                        else
                        {
                            LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                            in_xml_reader.skipCurrentElement();
                        }
                    }
                    tmp_actions_list.append(tmp_action);
                }
                else
                {
                    LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                    in_xml_reader.skipCurrentElement();
                }
            }
        }
        else if (in_xml_reader.name() == "automated_actions")
        {
            while (in_xml_reader.readNextStartElement())
            {
                if (in_xml_reader.name() == "automated_action")
                {
                    tmp_automated_action = new AutomatedAction();
                    tmp_automated_action->setValueForKey(in_xml_reader.attributes().value("id").toString().toStdString().c_str(), AUTOMATED_ACTIONS_TABLE_ACTION_ID);

                    while (in_xml_reader.readNextStartElement())
                    {
                        if (in_xml_reader.name() == "window_id")
                        {
                            tmp_text = in_xml_reader.readElementText();
                            tmp_automated_action->setValueForKey(tmp_text.toStdString().c_str(), AUTOMATED_ACTIONS_TABLE_WINDOW_ID);
                        }
                        else if (in_xml_reader.name() == "message_type")
                        {
                            tmp_text = in_xml_reader.readElementText();
                            tmp_automated_action->setValueForKey(tmp_text.toStdString().c_str(), AUTOMATED_ACTIONS_TABLE_MESSAGE_TYPE);
                        }
                        else if (in_xml_reader.name() == "message_data")
                        {
                            tmp_text = in_xml_reader.readElementText();
                            tmp_automated_action->setValueForKey(tmp_text.toStdString().c_str(), AUTOMATED_ACTIONS_TABLE_MESSAGE_DATA);
                        }
                        else if (in_xml_reader.name() == "message_time_delay")
                        {
                            tmp_text = in_xml_reader.readElementText();
                            tmp_automated_action->setValueForKey(tmp_text.toStdString().c_str(), AUTOMATED_ACTIONS_TABLE_MESSAGE_TIME_DELAY);
                        }
                        else if (in_xml_reader.name() == "validations")
                        {
                            while (in_xml_reader.readNextStartElement())
                            {
                                if (in_xml_reader.name() == "validation")
                                {
                                    tmp_automated_action_validation = new AutomatedActionValidation(tmp_automated_action);
                                    tmp_automated_action_validation->setValueForKey(in_xml_reader.attributes().value("module_name").toString().toStdString().c_str(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_NAME);
                                    tmp_automated_action_validation->setValueForKey(in_xml_reader.attributes().value("module_version").toString().toStdString().c_str(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_VERSION);
                                    tmp_automated_action_validation->setValueForKey(in_xml_reader.attributes().value("module_function_name").toString().toStdString().c_str(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_NAME);
                                    tmp_automated_action_validation->setValueForKey(in_xml_reader.attributes().value("module_function_parameters").toString().toStdString().c_str(), AUTOMATED_ACTIONS_VALIDATIONS_TABLE_VALIDATION_MODULE_FUNCTION_PARAMETERS);
                                    tmp_automated_action->addChild(tmp_automated_action_validation);
                                }
                                else
                                {
                                    LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                                }
                                in_xml_reader.skipCurrentElement();
                            }
                        }
                        else
                        {
                            LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                            in_xml_reader.skipCurrentElement();
                        }
                    }
                    tmp_automated_actions_list.append(tmp_automated_action);
                }
                else
                {
                    LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
                    in_xml_reader.skipCurrentElement();
                }
            }
        }

        else if (in_xml_reader.name() == "testsRequirements")
        {
            while (in_xml_reader.readNextStartElement())
            {
                if (in_xml_reader.name() == "testRequirement")
                {
                    tmp_test_requirement = new TestRequirement();
                    tmp_test_requirement->readXml(in_xml_reader);
                    tmp_tests_requirements_list.append(tmp_test_requirement);
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

    in_xml_datas.m_actions_dict.insert(getIdentifier(), tmp_actions_list);
    in_xml_datas.m_automated_actions_dict.insert(getIdentifier(), tmp_automated_actions_list);
    in_xml_datas.m_tests_requirements_dict.insert(getIdentifier(), tmp_tests_requirements_list);

    return true;
}


int TestContent::saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas)
{
    int		tmp_result = NOERR;
    QString		tmp_test_content_id = QString(getIdentifier());
    QString		tmp_original_test_content_id = QString(getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID));
    QList<Action*>	tmp_actions_list;
    QList<AutomatedAction*>	tmp_automated_actions_list;
    QList<TestRequirement*>	tmp_tests_requirements_list;
    QString		tmp_id_str;

    const char	*tmp_previous_action_id = NULL;

    if (is_empty_string(getOriginalValueForKey(TESTS_CONTENTS_TABLE_TEST_CONTENT_ID)) == TRUE)
    {
        tmp_id_str= in_xml_datas.getNewTestContentOriginalIdentifier(this, getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID));
        setValueForKey(NULL, TESTS_CONTENTS_TABLE_TEST_CONTENT_ID);
        if (tmp_id_str.isEmpty())
            setValueForKey(NULL, TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID);
        else
            setValueForKey(tmp_id_str.toStdString().c_str(), TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID);

        tmp_result = saveRecord();
        if (tmp_result == NOERR)
        {
            in_xml_datas.m_tests_contents_dict.insert(tmp_test_content_id, this);

            if (tmp_id_str.isEmpty() && is_empty_string(getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID)) == FALSE)
            {
                in_xml_datas.m_originals_tests_contents_dict.insert(tmp_original_test_content_id, getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID));
            }

            if (in_xml_datas.m_actions_dict.contains(tmp_test_content_id))
            {
                tmp_actions_list = in_xml_datas.m_actions_dict.value(tmp_test_content_id);
                tmp_previous_action_id = NULL;

                foreach(Action *tmp_action, tmp_actions_list)
                {
                    tmp_id_str = in_xml_datas.getNewTestContentOriginalIdentifier(NULL, tmp_action->getValueForKey(ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID));
                    if (tmp_id_str.isEmpty())
                        tmp_action->setValueForKey(NULL, ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID);
                    else
                        tmp_action->setValueForKey(tmp_id_str.toStdString().c_str(), ACTIONS_TABLE_LINK_ORIGINAL_TEST_CONTENT_ID);

                    tmp_action->setValueForKey(tmp_previous_action_id, ACTIONS_TABLE_PREVIOUS_ACTION_ID);
                    tmp_action->setValueForKey(getIdentifier(), ACTIONS_TABLE_TEST_CONTENT_ID);
                    if (tmp_action->saveRecord() == NOERR)
                        tmp_previous_action_id = tmp_action->getIdentifier();
                    else
                      LOG_ERROR(Session::instance()->getClientSession(), "%s\n", Session::instance()->getClientSession()->m_last_error_msg);
                }
            }

            if (in_xml_datas.m_automated_actions_dict.contains(tmp_test_content_id))
            {
                tmp_automated_actions_list = in_xml_datas.m_automated_actions_dict.value(tmp_test_content_id);
                tmp_previous_action_id = NULL;

                foreach(AutomatedAction *tmp_automated_action, tmp_automated_actions_list)
                {
                    tmp_id_str = in_xml_datas.getNewTestContentOriginalIdentifier(NULL, tmp_automated_action->getValueForKey(AUTOMATED_ACTIONS_TABLE_TEST_CONTENT_ID));
                    tmp_automated_action->setValueForKey(getIdentifier(), AUTOMATED_ACTIONS_TABLE_TEST_CONTENT_ID);

                    tmp_automated_action->setValueForKey(tmp_previous_action_id, AUTOMATED_ACTIONS_TABLE_PREVIOUS_ACTION_ID);

                    if (tmp_automated_action->saveRecord() == NOERR)
                        tmp_previous_action_id = tmp_automated_action->getIdentifier();
                    else
                      LOG_ERROR(Session::instance()->getClientSession(), "%s\n", Session::instance()->getClientSession()->m_last_error_msg);
                }
            }


            if (in_xml_datas.m_tests_requirements_dict.contains(tmp_test_content_id))
            {
                tmp_tests_requirements_list = in_xml_datas.m_tests_requirements_dict.value(tmp_test_content_id);

                foreach(TestRequirement *tmp_test_requirement, tmp_tests_requirements_list)
                {
                    tmp_test_requirement->saveFromXmlProjectDatas(in_xml_datas);
                }
            }
        }
    }

    return tmp_result;
}

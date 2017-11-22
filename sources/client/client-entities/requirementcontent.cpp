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

#include "requirementcontent.h"
#include "entities.h"
#include "utilities.h"
#include "xmlprojectdatas.h"

#include "testrequirement.h"
#include "session.h"

/**
  Constructeur
**/
RequirementContent::RequirementContent() : Record(getEntityDef())
{
    _m_project_version = NULL;
}


/**
  Constructeur
**/
RequirementContent::RequirementContent(ProjectVersion *in_project_version) : Record(getEntityDef())
{
    _m_project_version = NULL;
    setProjectVersion(in_project_version);
}


/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* RequirementContent::getEntityDef() const
{
    return &requirements_contents_table_def;
}


void RequirementContent::setProjectVersion(ProjectVersion* in_project_version)
{
    if (in_project_version != NULL)
    {
        _m_project_version = in_project_version;
        if (is_empty_string(getIdentifier()))
        {
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), REQUIREMENTS_CONTENTS_TABLE_PROJECT_ID);
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), REQUIREMENTS_CONTENTS_TABLE_VERSION);
        }
    }
}


RequirementContent* RequirementContent::copy()
{
    RequirementContent *tmp_copy = new RequirementContent(_m_project_version);

    tmp_copy->setValueForKey(getValueForKey(REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME),REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME);
    tmp_copy->setValueForKey(getValueForKey(REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION),REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION);
    tmp_copy->setValueForKey(getValueForKey(REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID),REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID);
    tmp_copy->setValueForKey(getValueForKey(REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL),REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL);
    tmp_copy->setValueForKey(getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID),REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);

    return tmp_copy;
}


int RequirementContent::saveRecord()
{
    int tmp_return = NOERR;

    if (is_empty_string(getIdentifier()))
    {
        if (_m_project_version != NULL)
        {
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), REQUIREMENTS_CONTENTS_TABLE_PROJECT_ID);
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), REQUIREMENTS_CONTENTS_TABLE_VERSION);
        }

        tmp_return = Record::saveRecord();
    }
    else
        tmp_return = Record::saveRecord();

    return tmp_return;
}


RequirementContent* RequirementContent::previousRequirementContent()
{
    RequirementContent	*tmp_previous_requirement = NULL;
    RequirementContent	**tmp_project_requirements = NULL;
    unsigned long         	tmp_requirements_count = 0;

    sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s AND %s<'%s' ORDER BY %s DESC LIMIT 1",
    	REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID,
        getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID),
    	REQUIREMENTS_CONTENTS_TABLE_VERSION,
        getValueForKey(REQUIREMENTS_CONTENTS_TABLE_VERSION),
	    REQUIREMENTS_CONTENTS_TABLE_VERSION);

	tmp_project_requirements = Record::loadRecords<RequirementContent>(CLIENT_SESSION, &requirements_contents_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_requirements_count);
	if (tmp_project_requirements != NULL)
	{
		if (tmp_requirements_count == 1)
		{
			tmp_previous_requirement = tmp_project_requirements[0];
			Record::freeRecords<RequirementContent>(tmp_project_requirements, 0);
		}
		else
		{
			Record::freeRecords<RequirementContent>(tmp_project_requirements, tmp_requirements_count);
		}
	}

    return tmp_previous_requirement;
}


RequirementContent* RequirementContent::nextRequirementContent()
{
    RequirementContent	*tmp_next_requirement = NULL;
    RequirementContent	**tmp_project_requirements = NULL;
    unsigned long         	tmp_requirements_count = 0;

    sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s AND %s>'%s' ORDER BY %s ASC LIMIT 1",
    	REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID,
        getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID),
    	REQUIREMENTS_CONTENTS_TABLE_VERSION,
        getValueForKey(REQUIREMENTS_CONTENTS_TABLE_VERSION),
	    REQUIREMENTS_CONTENTS_TABLE_VERSION);

	tmp_project_requirements = Record::loadRecords<RequirementContent>(CLIENT_SESSION, &requirements_contents_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_requirements_count);
	if (tmp_project_requirements != NULL)
	{
		if (tmp_requirements_count == 1)
		{
			tmp_next_requirement = tmp_project_requirements[0];
			Record::freeRecords<RequirementContent>(tmp_project_requirements, 0);
		}
		else
		{
			Record::freeRecords<RequirementContent>(tmp_project_requirements, tmp_requirements_count);
		}
	}

    return tmp_next_requirement;
}


RequirementContent* RequirementContent::loadLastRequirementContentForVersion(const char *in_original_requirement_content_id, const char *in_project_version)
{
    RequirementContent	*tmp_requirement = NULL;
    RequirementContent	**tmp_project_requirements = NULL;
    unsigned long         	tmp_requirements_count = 0;

    sprintf(CLIENT_SESSION->m_where_clause_buffer, "%s=%s AND %s<='%s' ORDER BY %s DESC LIMIT 1",
    	REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID,
    	in_original_requirement_content_id,
    	REQUIREMENTS_CONTENTS_TABLE_VERSION,
    	in_project_version,
	    REQUIREMENTS_CONTENTS_TABLE_VERSION);

	tmp_project_requirements = Record::loadRecords<RequirementContent>(CLIENT_SESSION, &requirements_contents_table_def, CLIENT_SESSION->m_where_clause_buffer, NULL, &tmp_requirements_count);
	if (tmp_project_requirements != NULL)
	{
		if (tmp_requirements_count == 1)
		{
			tmp_requirement = tmp_project_requirements[0];
			Record::freeRecords<RequirementContent>(tmp_project_requirements, 0);
		}
		else
		{
			Record::freeRecords<RequirementContent>(tmp_project_requirements, tmp_requirements_count);
		}
	}

    return tmp_requirement;
}


QList<TestRequirement*> RequirementContent::loadTestRequirementsForProjectVersion(ProjectVersion *in_project_version)
{
    QList<TestRequirement*>       	tmp_requirements;
    char				***tmp_results = NULL;
    unsigned long			tmp_rows_count = 0, tmp_columns_count = 0, tmp_row_index = 0;
    TestRequirement			*tmp_test_requirement = NULL;

    if (in_project_version != NULL)
    {
	sprintf(CLIENT_SESSION->m_last_query,
		"select " \
		"%s.%s,%s.%s,%s.%s,%s.%s,%s.%s " \
		"from " \
		"%s, %s, %s " \
		"where " \
		"%s.%s=%s.%s and " \
		"%s.%s=%s.%s and " \
		"%s.%s=%s and " \
		"%s.%s='%s';",
		TESTS_REQUIREMENTS_TABLE_SIG, TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID, TESTS_REQUIREMENTS_TABLE_SIG, TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID,
		TESTS_CONTENTS_TABLE_SIG, TESTS_CONTENTS_TABLE_SHORT_NAME, TESTS_CONTENTS_TABLE_SIG, TESTS_CONTENTS_TABLE_CATEGORY_ID, TESTS_CONTENTS_TABLE_SIG, TESTS_CONTENTS_TABLE_VERSION,
		TESTS_REQUIREMENTS_TABLE_SIG, TESTS_CONTENTS_TABLE_SIG, TESTS_TABLE_SIG,
		TESTS_REQUIREMENTS_TABLE_SIG, TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID, TESTS_CONTENTS_TABLE_SIG, TESTS_CONTENTS_TABLE_TEST_CONTENT_ID,
		TESTS_REQUIREMENTS_TABLE_SIG, TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID, TESTS_TABLE_SIG, TESTS_TABLE_TEST_CONTENT_ID,
        TESTS_REQUIREMENTS_TABLE_SIG, TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID, getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID),
		TESTS_TABLE_SIG, TESTS_TABLE_VERSION,
        in_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION)
		);

	tmp_results = cl_run_sql(CLIENT_SESSION, CLIENT_SESSION->m_last_query, &tmp_rows_count, &tmp_columns_count);
	if (tmp_results != NULL)
	{
	    if (tmp_rows_count > 0 && tmp_columns_count > 0)
	    {
		for (tmp_row_index = 0; tmp_row_index < tmp_rows_count; tmp_row_index++)
		{
		    tmp_test_requirement = new TestRequirement();
            tmp_test_requirement->setValueForKey(tmp_results[tmp_row_index][0], TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID);
            tmp_test_requirement->setValueForKey(tmp_results[tmp_row_index][1], TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);
		    tmp_test_requirement->setTestShortName(tmp_results[tmp_row_index][2]);
		    tmp_test_requirement->setTestCategory(tmp_results[tmp_row_index][3]);
		    tmp_test_requirement->setTestVersion(tmp_results[tmp_row_index][4]);
                    tmp_test_requirement->cloneColumns();

		    tmp_requirements.append(tmp_test_requirement);
		}
	    }

	    cl_free_rows_columns_array(&tmp_results, tmp_rows_count, tmp_columns_count);
	}

    }

    return tmp_requirements;
}


QString RequirementContent::getPriorityLabel()
{
    switch(QString(getValueForKey(REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL)).toInt())
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

    return QString();
}


void RequirementContent::writeXml(QXmlStreamWriter & in_xml_writer)
{
    in_xml_writer.writeStartElement("requirementContent");

    in_xml_writer.writeAttribute("id", getIdentifier());
    in_xml_writer.writeAttribute("originalRequirementId", getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID));
    in_xml_writer.writeAttribute("name", getValueForKey(REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME));
    in_xml_writer.writeAttribute("version", getValueForKey(REQUIREMENTS_CONTENTS_TABLE_VERSION));
    in_xml_writer.writeAttribute("category", getValueForKey(REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID));
    in_xml_writer.writeAttribute("priority", getValueForKey(REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL));
    in_xml_writer.writeTextElement("description", getValueForKey(REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION));

    in_xml_writer.writeEndElement();
}


bool RequirementContent::readXml(QXmlStreamReader & in_xml_reader)
{
    QString		tmp_id = in_xml_reader.attributes().value("id").toString();
    QString		tmp_original_requirement_id = in_xml_reader.attributes().value("originalRequirementId").toString();
    QString		tmp_name = in_xml_reader.attributes().value("name").toString();
    QString		tmp_version = in_xml_reader.attributes().value("version").toString();
    QString		tmp_category = in_xml_reader.attributes().value("category").toString();
    QString		tmp_priority = in_xml_reader.attributes().value("priority").toString();

    QString		tmp_text;

    setValueForKey(tmp_id.toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_REQUIREMENT_CONTENT_ID);
    setValueForKey(tmp_original_requirement_id.toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);
    setValueForKey(tmp_name.toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME);
    setValueForKey(tmp_version.toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_VERSION);
    setValueForKey(tmp_category.toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID);
    setValueForKey(tmp_priority.toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL);

    while (in_xml_reader.readNextStartElement())
    {
	// Description
	if (in_xml_reader.name() == "description")
	{
	    tmp_text = in_xml_reader.readElementText();
        setValueForKey(tmp_text.toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION);
	}
	else
	{
        LOG_ERROR(CLIENT_SESSION, "Unknow tag (%s) line %lli.\n", in_xml_reader.name().toString().toStdString().c_str(), in_xml_reader.lineNumber());
	    in_xml_reader.skipCurrentElement();
	}
    }

    return true;
}


int RequirementContent::saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas)
{
    int		tmp_result = NOERR;
    QString	tmp_requirement_content_id = QString(getIdentifier());
    QString	tmp_original_requirement_content_id = QString(getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID));

    if (is_empty_string(getOriginalValueForKey(REQUIREMENTS_CONTENTS_TABLE_REQUIREMENT_CONTENT_ID)) == TRUE)
    {
        QString tmp_std_string = in_xml_datas.getNewRequirementContentOriginalIdentifier(getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID));
        if (tmp_std_string.isEmpty())
            setValueForKey(NULL, REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);
        else
            setValueForKey(tmp_std_string.toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);

        setValueForKey(NULL, REQUIREMENTS_CONTENTS_TABLE_REQUIREMENT_CONTENT_ID);
		tmp_result = saveRecord();
		if (tmp_result == NOERR)
		{
			in_xml_datas.m_requirements_contents_dict.insert(tmp_requirement_content_id, this);
            if (tmp_std_string.isEmpty() && is_empty_string(getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID)) == FALSE)
			{
                in_xml_datas.m_originals_requirements_contents_dict.insert(tmp_original_requirement_content_id, getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID));
			}
		}
    }

    return tmp_result;
}

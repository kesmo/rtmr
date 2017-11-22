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

#include "testrequirement.h"
#include "entities.h"
#include "utilities.h"


/**
  Constructeur
**/
TestRequirement::TestRequirement() : Record(getEntityDef())
{
    _m_test_short_name = NULL;
    _m_test_category = NULL;
    _m_test_version = NULL;
    _m_requirement_short_name = NULL;
    _m_requirement_category = NULL;
    _m_requirement_version = NULL;
}



TestRequirement::~TestRequirement()
{
    delete _m_test_short_name;
    delete _m_test_category;
    delete _m_test_version;
    delete _m_requirement_short_name;
    delete _m_requirement_category;
    delete _m_requirement_version;
    destroy();
}


/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* TestRequirement::getEntityDef() const
{
    return &tests_requirements_table_def;
}

void TestRequirement::setTestShortName(const char *in_test_short_name)
{
    if (_m_test_short_name != NULL)
    {
	delete _m_test_short_name;
	_m_test_short_name = NULL;
    }

    if (in_test_short_name != NULL)
    {
	_m_test_short_name = (char*)malloc(strlen(in_test_short_name) + 1);
	strcpy(_m_test_short_name, in_test_short_name);
    }
}

void TestRequirement::setTestCategory(const char *in_test_category)
{
    if (_m_test_category != NULL)
    {
	delete _m_test_category;
	_m_test_category = NULL;
    }

    if (in_test_category != NULL)
    {
	_m_test_category = (char*)malloc(strlen(in_test_category) + 1);
	strcpy(_m_test_category, in_test_category);
    }
}

void TestRequirement::setTestVersion(const char *in_test_version)
{
    if (_m_test_version != NULL)
    {
	delete _m_test_version;
	_m_test_version = NULL;
    }

    if (in_test_version != NULL)
    {
	_m_test_version = (char*)malloc(strlen(in_test_version) + 1);
	strcpy(_m_test_version, in_test_version);
    }
}


void TestRequirement::setRequirementShortName(const char *in_requirement_short_name)
{
    if (_m_requirement_short_name != NULL)
    {
	delete _m_requirement_short_name;
	_m_requirement_short_name = NULL;
    }

    if (in_requirement_short_name != NULL)
    {
	_m_requirement_short_name = (char*)malloc(strlen(in_requirement_short_name) + 1);
	strcpy(_m_requirement_short_name, in_requirement_short_name);
    }
}

void TestRequirement::setRequirementCategory(const char *in_requirement_category)
{
    if (_m_requirement_category != NULL)
    {
	delete _m_requirement_category;
	_m_requirement_category = NULL;
    }

    if (in_requirement_category != NULL)
    {
	_m_requirement_category = (char*)malloc(strlen(in_requirement_category) + 1);
	strcpy(_m_requirement_category, in_requirement_category);
    }
}

void TestRequirement::setRequirementVersion(const char *in_requirement_version)
{
    if (_m_requirement_version != NULL)
    {
	delete _m_requirement_version;
	_m_requirement_version = NULL;
    }

    if (in_requirement_version != NULL)
    {
	_m_requirement_version = (char*)malloc(strlen(in_requirement_version) + 1);
	strcpy(_m_requirement_version, in_requirement_version);
    }
}


void TestRequirement::writeXml(QXmlStreamWriter & in_xml_writer)
{
    in_xml_writer.writeStartElement("testRequirement");
    in_xml_writer.writeAttribute("testContentId", getValueForKey(TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID));
    in_xml_writer.writeAttribute("originalRequirementContentId", getValueForKey(TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID));
    in_xml_writer.writeEndElement();
}


bool TestRequirement::readXml(QXmlStreamReader & in_xml_reader)
{
    setValueForKey(in_xml_reader.attributes().value("testContentId").toString().toStdString().c_str(), TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID);
    setValueForKey(in_xml_reader.attributes().value("originalRequirementContentId").toString().toStdString().c_str(), TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);

    in_xml_reader.skipCurrentElement();

    return true;
}


int TestRequirement::saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas)
{
	int			tmp_result = NOERR;
    const char	*tmp_test_content_id = getValueForKey(TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID);
    const char	*tmp_original_requirement_content_id = getValueForKey(TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);

    QString tmp_std_string = in_xml_datas.getNewRequirementContentOriginalIdentifier(tmp_original_requirement_content_id);
    setValueForKey(in_xml_datas.getNewTestContentIdentifier(tmp_test_content_id), TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID);
    if (tmp_std_string.isEmpty())
        setValueForKey(NULL, TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);
    else
        setValueForKey(tmp_std_string.toStdString().c_str(), TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);

    tmp_result = saveRecord();
    if (tmp_result != NOERR)
    {
		LOG_ERROR(CLIENT_SESSION, "IMPORT TestRequirement ====> %s\n", cl_get_error_message(CLIENT_SESSION, tmp_result));
    }

    return tmp_result;
}

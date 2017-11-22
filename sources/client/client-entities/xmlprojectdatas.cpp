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

#include "xmlprojectdatas.h"
#include "testcontent.h"
#include "requirementcontent.h"
#include "testhierarchy.h"
#include "requirementhierarchy.h"
#include "action.h"
#include "testrequirement.h"
#include "automatedaction.h"
#include "automatedactionvalidation.h"

XmlProjectDatas::XmlProjectDatas()
{
}


XmlProjectDatas::~XmlProjectDatas()
{
    QList< QList <Action*> >		tmp_all_actions = m_actions_dict.values();
    QList< QList <TestRequirement*> >	tmp_all_tests_requirements = m_tests_requirements_dict.values();
    QList< QList <AutomatedAction*> >		tmp_all_automated_actions = m_automated_actions_dict.values();

    qDeleteAll(m_tests_contents_list);
    qDeleteAll(m_requirements_contents_list);
    qDeleteAll(m_versions_list);

    foreach(QList <Action*> tmp_actions_list, tmp_all_actions)
	qDeleteAll(tmp_actions_list);

    foreach(QList <TestRequirement*> tmp_tests_requirements_list, tmp_all_tests_requirements)
	qDeleteAll(tmp_tests_requirements_list);

    foreach(QList <AutomatedAction*> tmp_automated_actions_list, tmp_all_automated_actions)
    qDeleteAll(tmp_automated_actions_list);

}


const char* XmlProjectDatas::getNewRequirementContentIdentifier(const char *in_identifier)
{
    RequirementContent	    *tmp_requirement_content = NULL;
    QString		    tmp_identifier;

    if (is_empty_string(in_identifier) == FALSE)
    {
	tmp_identifier = QString(in_identifier);
	if (m_requirements_contents_dict.contains(tmp_identifier))
	{
	    tmp_requirement_content = m_requirements_contents_dict.value(tmp_identifier);
	    if (tmp_requirement_content != NULL)
		return tmp_requirement_content->getIdentifier();
	}
	else
	{
	    foreach(RequirementContent *tmp_requirement_content, m_requirements_contents_list)
	    {
		if (compare_values(tmp_requirement_content->getIdentifier(), in_identifier) == 0)
		{
		    if (tmp_requirement_content->saveFromXmlProjectDatas(*this) == NOERR)
			return tmp_requirement_content->getIdentifier();
		}
	    }
	}
    }

    return NULL;
}


QString XmlProjectDatas::getNewRequirementContentOriginalIdentifier(const char *in_original_identifier)
{
    QString		    tmp_identifier;

    if (is_empty_string(in_original_identifier) == FALSE)
    {
	tmp_identifier = QString(in_original_identifier);
	if (m_originals_requirements_contents_dict.contains(tmp_identifier))
	{
        return m_originals_requirements_contents_dict.value(tmp_identifier);
	}
    }

    return QString();
}


const char* XmlProjectDatas::getNewTestContentIdentifier(const char *in_identifier)
{
    TestContent	    *tmp_test_content = NULL;
    QString		    tmp_identifier;

    if (is_empty_string(in_identifier) == FALSE)
    {
	tmp_identifier = QString(in_identifier);
	if (m_tests_contents_dict.contains(tmp_identifier))
	{
	    tmp_test_content = m_tests_contents_dict.value(tmp_identifier);
	    if (tmp_test_content != NULL)
		return tmp_test_content->getIdentifier();
	}
	else
	{
	    foreach(TestContent *tmp_test_content, m_tests_contents_list)
	    {
		if (compare_values(tmp_test_content->getIdentifier(), in_identifier) == 0)
		{
		    if (tmp_test_content->saveFromXmlProjectDatas(*this) == NOERR)
			return tmp_test_content->getIdentifier();
		}
	    }
	}
    }

    return NULL;
}


QString XmlProjectDatas::getNewTestContentOriginalIdentifier(TestContent *in_test_content, const char *in_original_identifier)
{
    QString		    tmp_original_identifier;

    if (is_empty_string(in_original_identifier) == FALSE)
    {
	tmp_original_identifier = QString(in_original_identifier);
	if (m_originals_tests_contents_dict.contains(tmp_original_identifier))
	{
	    return  m_originals_tests_contents_dict.value(tmp_original_identifier);
	}
	else if (in_test_content == NULL)
	{
	    foreach(TestContent *tmp_test_content, m_tests_contents_list)
	    {
        if (compare_values(tmp_test_content->getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID), in_original_identifier) == 0)
		{
		    if (tmp_test_content->saveFromXmlProjectDatas(*this) == NOERR)
            return tmp_test_content->getValueForKey(TESTS_CONTENTS_TABLE_ORIGINAL_TEST_CONTENT_ID);
		}
	    }
	}
    }

    return QString();
}


const char* XmlProjectDatas::getNewTestHierarchyIdentifier(ProjectVersion* in_project_version, const char *in_identifier)
{
    TestHierarchy	    *tmp_test = NULL;
    QString		    tmp_identifier;

    if (is_empty_string(in_identifier) == FALSE)
    {
	tmp_identifier = QString(in_identifier);
	if (m_tests_dict.contains(tmp_identifier))
	{
	    tmp_test = m_tests_dict.value(tmp_identifier);
	    if (tmp_test != NULL)
		return tmp_test->getIdentifier();
	}
	else
	{
	    tmp_test = (TestHierarchy*)in_project_version->findItemWithId(in_identifier, TESTS_HIERARCHY_SIG_ID);
	    if (tmp_test != NULL)
	    {
		if (tmp_test->saveFromXmlProjectDatas(*this) == NOERR)
		    return tmp_test->getIdentifier();
	    }
	}
    }

    return NULL;
}


const char* XmlProjectDatas::getNewRequirementHierarchyIdentifier(ProjectVersion* in_project_version, const char *in_identifier)
{
    RequirementHierarchy	    *tmp_requirement = NULL;
    QString		    tmp_identifier;

    if (is_empty_string(in_identifier) == FALSE)
    {
	tmp_identifier = QString(in_identifier);
	if (m_requirements_dict.contains(tmp_identifier))
	{
	    tmp_requirement = m_requirements_dict.value(tmp_identifier);
	    if (tmp_requirement != NULL)
		return tmp_requirement->getIdentifier();
	}
	else
	{
	    tmp_requirement = (RequirementHierarchy*)in_project_version->findItemWithId(in_identifier, REQUIREMENTS_HIERARCHY_SIG_ID);
	    if (tmp_requirement != NULL)
	    {
		if (tmp_requirement->saveFromXmlProjectDatas(*this) == NOERR)
		    return tmp_requirement->getIdentifier();
	    }
	}
    }

    return NULL;
}

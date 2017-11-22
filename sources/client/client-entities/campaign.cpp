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

#include "projectgrant.h"
#include "campaign.h"
#include "testcampaign.h"
#include "executioncampaign.h"
#include "utilities.h"
#include "requirementhierarchy.h"

Campaign::Campaign() : Hierarchy(getEntityDef())
{
    _m_project_version = NULL;
}


Campaign::Campaign(ProjectVersion *in_project) : Hierarchy(getEntityDef())
{
    setProjectVersion(in_project);
}

/**
  Destructeur
**/
Campaign::~Campaign()
{
    qDeleteAll(_m_tests_list);
    destroy();
}


/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* Campaign::getEntityDef() const
{
    return &campaigns_table_def;
}

void Campaign::setProjectVersion(ProjectVersion* in_project_version)
{
    if (in_project_version != NULL)
    {
        _m_project_version = in_project_version;
    	if (is_empty_string(getIdentifier()))
    	{
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), CAMPAIGNS_TABLE_PROJECT_ID);
            setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), CAMPAIGNS_TABLE_PROJECT_VERSION);
    	}
    }
}


Campaign* Campaign::duplicate()
{
    TestCampaign *tmp_test_cloned = NULL;
    Campaign     *tmp_copy = clone<Campaign>();

    tmp_copy->setProjectVersion(_m_project_version);

    if (_m_tests_list.count() > 0)
    {
        foreach (TestCampaign *tmp_test, _m_tests_list)
        {
            tmp_test_cloned = tmp_test->clone<TestCampaign>();
            tmp_test_cloned->setCampaign(tmp_copy);
            tmp_test_cloned->setProjectTest(tmp_test->projectTest());
            tmp_copy->_m_tests_list.append(tmp_test_cloned);
        }
    }

    return tmp_copy;
}

void Campaign::loadTests()
{
    _m_tests_list = loadCampaignTests();
}

QList<TestCampaign*> Campaign::loadCampaignTests()
{
    TestCampaign		**tmp_campaign_tests = NULL;
    QList<TestCampaign*>	tmp_tests_list;

    unsigned long               tmp_tests_count = 0;
    char                        tmp_where_clause[128];

    sprintf(tmp_where_clause, "%s=%s", TESTS_CAMPAIGNS_TABLE_CAMPAIGN_ID, getIdentifier());

    tmp_campaign_tests = Record::loadRecords<TestCampaign>(CLIENT_SESSION, &tests_campaigns_table_def, tmp_where_clause, NULL, &tmp_tests_count);
    if (tmp_campaign_tests != NULL)
    {
        if (tmp_tests_count > 0)
	    tmp_tests_list = Campaign::orderedCampaignTests(this, tmp_campaign_tests, tmp_tests_count);

        Record::freeRecords<TestCampaign>(tmp_campaign_tests, 0);
    }

    return tmp_tests_list;
}


QList < TestCampaign* > Campaign::orderedCampaignTests(Campaign *in_campaign, TestCampaign **in_campaign_tests, unsigned long in_tests_count)
{
    TestCampaign               *tmp_campaign_test = NULL;
    QList < TestCampaign* >    tmp_tests_list;

    const char                  *tmp_previous_id = NULL;

    unsigned long         tmp_tests_index = 0;

    if (in_campaign != NULL && in_tests_count > 0)
    {
        do
        {
            for (tmp_tests_index = 0; tmp_tests_index < in_tests_count; tmp_tests_index++)
            {
                tmp_campaign_test = in_campaign_tests[tmp_tests_index];
                if (tmp_campaign_test != NULL
                    && compare_values(tmp_previous_id, tmp_campaign_test->getValueForKey(TESTS_CAMPAIGNS_TABLE_PREVIOUS_TEST_CAMPAIGN_ID)) == 0)
                {
		    // Associer à la campagne
                    tmp_campaign_test->setCampaign(in_campaign);

                    tmp_tests_list.append(tmp_campaign_test);
                    tmp_previous_id = tmp_campaign_test->getIdentifier();

                    // Affecter le contenu du test
            tmp_campaign_test->setProjectTest(TestHierarchy::findTestWithId(in_campaign->projectVersion()->testsHierarchy(), tmp_campaign_test->getValueForKey(TESTS_CAMPAIGNS_TABLE_TEST_ID)));

                    break;
                }
            }
        }
        while (is_empty_string(tmp_previous_id) == FALSE && !(tmp_tests_index >= in_tests_count));
    }

    return tmp_tests_list;
}


bool Campaign::insertTestCampaignAtIndex(TestCampaign *in_test, int in_index)
{
    int     tmp_save_result = NOERR;

    if (in_test != NULL && _m_tests_list.indexOf(in_test) < 0)
    {
        if (in_index > 0)
            in_test->setValueForKey(_m_tests_list[in_index - 1]->getIdentifier(), TESTS_CAMPAIGNS_TABLE_PREVIOUS_TEST_CAMPAIGN_ID);
        else
            in_test->setValueForKey(NULL, TESTS_CAMPAIGNS_TABLE_PREVIOUS_TEST_CAMPAIGN_ID);

        tmp_save_result = in_test->saveRecord();
        if (tmp_save_result == NOERR)
        {
            _m_tests_list.insert(in_index, in_test);

            if (_m_tests_list.isEmpty() == false && in_index + 1 < _m_tests_list.count())
            {
                _m_tests_list[in_index + 1]->setValueForKey(in_test->getIdentifier(), TESTS_CAMPAIGNS_TABLE_PREVIOUS_TEST_CAMPAIGN_ID);
                tmp_save_result = _m_tests_list[in_index + 1]->saveRecord();
            }

            return (tmp_save_result == NOERR);
        }
    }

    return false;
}


bool Campaign::insertTestAtIndex(TestHierarchy *in_test, int in_index)
{
    TestCampaign *tmp_campaign_test = NULL;

    if (in_test != NULL)
    {
        tmp_campaign_test = new TestCampaign(this);
        tmp_campaign_test->setProjectTest(in_test);

        return insertTestCampaignAtIndex(tmp_campaign_test, in_index);
    }

    return false;
}


bool Campaign::removeTestsAtIndex(int in_index, int in_count, bool in_move_indic)
{
    TestCampaign *tmp_test_campaign = NULL;
    int		    tmp_save_result = NOERR;

    if (in_index + in_count < _m_tests_list.size())
    {
	if (in_index > 0)
        _m_tests_list[in_index + in_count]->setValueForKey(_m_tests_list[in_index - 1]->getIdentifier(), TESTS_CAMPAIGNS_TABLE_PREVIOUS_TEST_CAMPAIGN_ID);
	else
        _m_tests_list[in_index + in_count]->setValueForKey(NULL, TESTS_CAMPAIGNS_TABLE_PREVIOUS_TEST_CAMPAIGN_ID);

	tmp_save_result = _m_tests_list[in_index + in_count]->saveRecord();
    }

    if (tmp_save_result == NOERR)
    {
	for (int tmp_index = 0; tmp_index < in_count; tmp_index++)
	{
	    tmp_test_campaign = _m_tests_list.takeAt(in_index);
	    if (in_move_indic == false)
	    {
		tmp_test_campaign->deleteRecord();
	    }
	}
    }

    return (tmp_save_result == NOERR);
}

QList<Hierarchy*>* Campaign::orginalsRecordsForGenericRecord(GenericRecord *in_item, int in_child_type)
{
    QList<Hierarchy*>*	tmp_list = new QList<Hierarchy*>();
    Hierarchy			*tmp_src_item = NULL;
    RequirementHierarchy    *tmp_requirement = NULL;

	if (in_item != NULL)
	{
	    if (in_child_type == TESTS_CAMPAIGNS_TABLE_SIG_ID)
	    {
	    	tmp_src_item = _m_project_version->findItemWithId(in_item->getIdentifier(), in_item->getEntityDefSignatureId());
	    	if (tmp_src_item != NULL)
	    	{
                if (tmp_src_item->getEntityDefSignatureId() == REQUIREMENTS_HIERARCHY_SIG_ID)
                {
                    tmp_requirement = (RequirementHierarchy*)tmp_src_item;
                    foreach(TestHierarchy *tmp_test, tmp_requirement->dependantsTests())
                    {
			tmp_list->append(tmp_test);
                    }
                }
                else
                {
			tmp_list->append(tmp_src_item);
                }
	    	}
	    }
	}

	return tmp_list;
}

bool Campaign::insertChildren(int position, int /* count */, Hierarchy *in_item, int in_child_type)
{
    switch (in_child_type)
    {
	case TESTS_CAMPAIGNS_TABLE_SIG_ID:
                if (in_item != NULL)
        	{
                    if (in_item->getEntityDefSignatureId() == TESTS_HIERARCHY_SIG_ID)
                        return insertTestAtIndex((TestHierarchy*)in_item, position);
                    else if (in_item->getEntityDefSignatureId() == TESTS_CAMPAIGNS_TABLE_SIG_ID)
                        return insertTestCampaignAtIndex((TestCampaign*)in_item, position);
                }
            break;

        default:
            break;
    }

    return false;
}


bool Campaign::insertCopyOfChildren(int position, int count, GenericRecord *in_item, int in_child_type)
{
    Hierarchy	*tmp_src_item = NULL;

    if (in_child_type == TESTS_CAMPAIGNS_TABLE_SIG_ID)
    {
	tmp_src_item = _m_project_version->findItemWithId(in_item->getIdentifier(), in_item->getEntityDefSignatureId());
	if (tmp_src_item != NULL)
	    return insertChildren(position, count, tmp_src_item, in_child_type);
    }

    return false;
}


bool Campaign::removeChildren(int position, int count, bool in_move_indic, int in_child_type)
{
    switch (in_child_type)
    {
	case TESTS_CAMPAIGNS_TABLE_SIG_ID:
	    return removeTestsAtIndex(position, count, in_move_indic);
            break;

        default:
            break;
    }

    return false;
}

bool Campaign::isWritable(int /* in_child_type */)
{
	if (_m_project_version == NULL || _m_project_version->project() == NULL || _m_project_version->project()->projectGrants() == NULL)
		return false;

    return (compare_values(_m_project_version->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_CAMPAIGNS_INDIC), PROJECT_GRANT_WRITE) == 0) && (_m_record_status == RECORD_STATUS_MODIFIABLE || _m_record_status == RECORD_STATUS_OWN_LOCK);
}

QVariant Campaign::data(int /* column */, int /* role */)
{
    return QVariant();
}

Hierarchy* Campaign::copy()
{
    return NULL;
}

Hierarchy* Campaign::cloneForMove(bool /* in_recursivly */)
{
    return NULL;
}


Hierarchy* Campaign::parent()
{
    return NULL;
}

int Campaign::row() const
{
    return 0;
}


Hierarchy* Campaign::child(int number, int in_child_type)
{
    switch (in_child_type)
    {
	case TESTS_CAMPAIGNS_TABLE_SIG_ID:
            return _m_tests_list[number];
            break;

        default:
            break;
    }

    return NULL;
}


int Campaign::childCount(int in_child_type)
{
    switch (in_child_type)
    {
	case TESTS_CAMPAIGNS_TABLE_SIG_ID:
            return _m_tests_list.count();
            break;

        default:
            break;
    }

    return 0;
}

int Campaign::saveRecord()
{
    int     tmp_result = NOERR;

    if (is_empty_string(getIdentifier()))
    {
        setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID), CAMPAIGNS_TABLE_PROJECT_ID);
        setValueForKey(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), CAMPAIGNS_TABLE_PROJECT_VERSION);
    	tmp_result = Record::insertRecord();
    }
    else
    {
    	tmp_result = Record::saveRecord();
    }

    return tmp_result;
}


int Campaign::saveTestsCampaign()
{
    int             tmp_result = NOERR;

    tmp_result = saveHierarchicalList(_m_tests_list, TESTS_CAMPAIGNS_TABLE_PREVIOUS_TEST_CAMPAIGN_ID);

    return tmp_result;
}


Hierarchy* Campaign::findItemWithId(const char* in_item_id , int /* in_child_type */, bool in_recursive)
{
    TestCampaign *tmp_found_test = NULL;

    foreach(TestCampaign *tmp_test, _m_tests_list)
    {
        if (compare_values(tmp_test->getIdentifier(), in_item_id) == 0)
            return tmp_test;

        if (in_recursive)
        {
            tmp_found_test = (TestCampaign*)tmp_test->findItemWithId(in_item_id);
            if (tmp_found_test != NULL)
                return tmp_found_test;
        }
    }

    return NULL;
}


void Campaign::writeXml(QXmlStreamWriter & in_xml_writer)
{
    QList<TestCampaign*>	tmp_tests_list;

    in_xml_writer.writeStartElement("campaign");
    in_xml_writer.writeAttribute("name", getValueForKey(CAMPAIGNS_TABLE_SHORT_NAME));
    in_xml_writer.writeTextElement("description", getValueForKey(CAMPAIGNS_TABLE_DESCRIPTION));

    tmp_tests_list = loadCampaignTests();
    if (tmp_tests_list.count() > 0)
    {
	in_xml_writer.writeStartElement("campaignTests");
	foreach(TestCampaign *tmp_test, tmp_tests_list)
	{
	    tmp_test->writeXml(in_xml_writer);
	}
	in_xml_writer.writeEndElement();
	qDeleteAll(tmp_tests_list);
    }

    in_xml_writer.writeEndElement();
}


bool Campaign::readXml(QXmlStreamReader & in_xml_reader)
{
    TestCampaign		*tmp_test = NULL;

    QString			tmp_name = in_xml_reader.attributes().value("name").toString();
    QString			tmp_text;

    setValueForKey(tmp_name.toStdString().c_str(), CAMPAIGNS_TABLE_SHORT_NAME);

    while (in_xml_reader.readNextStartElement())
    {
	// Description
	if (in_xml_reader.name() == "description")
	{
	    tmp_text = in_xml_reader.readElementText();
        setValueForKey(tmp_text.toStdString().c_str(), CAMPAIGNS_TABLE_DESCRIPTION);
	}
	else if (in_xml_reader.name() == "campaignTests")
	{
	    while (in_xml_reader.readNextStartElement())
	    {
		if (in_xml_reader.name() == "campaignTest")
		{
		    tmp_test = new TestCampaign(this);
		    tmp_test->readXml(in_xml_reader);
		    _m_tests_list.append(tmp_test);
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

    return true;
}


int Campaign::saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas)
{
    int		tmp_result = NOERR;
    const char	*tmp_previous_test_campaign_id = NULL;

    if (is_empty_string(getOriginalValueForKey(CAMPAIGNS_TABLE_CAMPAIGN_ID)) == TRUE)
    {
	tmp_result = saveRecord();
	if (tmp_result == NOERR)
	{
	    foreach(TestCampaign *tmp_test, _m_tests_list)
	    {
        tmp_test->setValueForKey(tmp_previous_test_campaign_id, TESTS_CAMPAIGNS_TABLE_PREVIOUS_TEST_CAMPAIGN_ID);
		if (tmp_test->saveFromXmlProjectDatas(in_xml_datas) == NOERR)
		    tmp_previous_test_campaign_id = tmp_test->getIdentifier();
	    }
	}
    }

    return tmp_result;
}

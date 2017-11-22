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

#include "testcampaign.h"
#include "utilities.h"

TestCampaign::TestCampaign() : Hierarchy(getEntityDef())
{
    _m_campaign = NULL;
    _m_parent = NULL;
    _m_project_test = NULL;
}


TestCampaign::TestCampaign(Campaign *in_campaign) : Hierarchy(getEntityDef())
{
    _m_campaign = NULL;
    _m_parent = NULL;
    _m_project_test = NULL;
    setCampaign(in_campaign);
}


/**
  Destructeur
**/
TestCampaign::~TestCampaign()
{
    qDeleteAll(_m_childs);
    destroy();
}



/**
  Renvoie l'entite reprensentee par l'objet
**/
const entity_def* TestCampaign::getEntityDef() const
{
    return &tests_campaigns_table_def;
}



void TestCampaign::setCampaign(Campaign* in_campaign)
{
    _m_campaign = in_campaign;
}


void TestCampaign::setProjectTest(TestHierarchy *in_project_test)
{
    TestCampaign *tmp_campaign_test = NULL;

    _m_project_test = in_project_test;
    if (_m_project_test != NULL)
    {
    setValueForKey(_m_project_test->getIdentifier(), TESTS_CAMPAIGNS_TABLE_TEST_ID);

	foreach(TestHierarchy *tmp_child_test, _m_project_test->childs())
        {
            tmp_campaign_test = new TestCampaign(_m_campaign);
            tmp_campaign_test->_m_parent = this;
            tmp_campaign_test->setProjectTest(tmp_child_test);
            _m_childs.append(tmp_campaign_test);
        }
    }
}



bool TestCampaign::insertCopyOfChildren(int /* in_index */, int /* count */, GenericRecord* /* in_item */, int /* in_child_type */)
{
    return false;
}


bool TestCampaign::insertChildren(int /* position */, int /* count */, Hierarchy* /* in_test */, int /* in_child_type */)
{
    return false;
}


bool TestCampaign::removeChildren(int /* position */, int /* count */, bool /* in_move_indic */, int /* in_child_type */)
{
    return false;
}

bool TestCampaign::isWritable(int /* in_child_type */)
{
    return false;
}

QVariant TestCampaign::data(int column, int role)
{
    if (_m_project_test != NULL)
        return _m_project_test->data(column, role);

    return QVariant();
}

Hierarchy* TestCampaign::copy()
{
    return NULL;
}

Hierarchy* TestCampaign::cloneForMove(bool in_recursivly)
{
    TestCampaign*    tmp_clone = clone<TestCampaign>();
    TestCampaign*    tmp_child_test = NULL;

    tmp_clone->_m_record_status = _m_record_status;
    tmp_clone->_m_campaign = _m_campaign;
    tmp_clone->_m_project_test = _m_project_test;

    if (in_recursivly)
    {
        foreach (TestCampaign *tmp_test, _m_childs)
        {
            tmp_child_test = (TestCampaign*)tmp_test->cloneForMove(in_recursivly);
            tmp_child_test->_m_parent = tmp_clone;
            tmp_clone->_m_childs.append(tmp_child_test);
        }
    }

    return tmp_clone;
}


Hierarchy* TestCampaign::parent()
{
	if (_m_parent != NULL)
		return _m_parent;
	else
		return _m_campaign;
}

int TestCampaign::row() const
{
     if (_m_parent)
         return _m_parent->_m_childs.indexOf(const_cast<TestCampaign*>(this));
     else
        return _m_campaign->testsList().indexOf(const_cast<TestCampaign*>(this));
}


Hierarchy* TestCampaign::child(int number, int /* in_child_type */)
{
    return _m_childs[number];
}


int TestCampaign::childCount(int /* in_child_type */)
{
    return _m_childs.count();
}

int TestCampaign::saveRecord()
{
    int tmp_return = NOERR;

	if (_m_project_test != NULL)
	{
		if (is_empty_string(_m_project_test->getIdentifier()))
		{
	tmp_return = _m_project_test->saveRecord();
			if (tmp_return == NOERR)
                setValueForKey(_m_project_test->getIdentifier(), TESTS_CAMPAIGNS_TABLE_TEST_ID);
			else
				return tmp_return;
		}
		else
            setValueForKey(_m_project_test->getIdentifier(), TESTS_CAMPAIGNS_TABLE_TEST_ID);
	}
	else
        setValueForKey(NULL, TESTS_CAMPAIGNS_TABLE_TEST_ID);

	if (tmp_return == NOERR)
	{
        setValueForKey(_m_campaign->getIdentifier(), TESTS_CAMPAIGNS_TABLE_CAMPAIGN_ID);
		tmp_return =  Record::saveRecord();
	}

    return tmp_return;
}


Hierarchy* TestCampaign::findItemWithId(const char* in_item_id, int in_child_type, bool in_recursive)
{
    TestCampaign	*tmp_found_test = NULL;

    if (in_child_type == TESTS_CAMPAIGNS_TABLE_SIG_ID)
    {
	foreach(TestCampaign *tmp_test, _m_childs)
	{
	    if (compare_values(tmp_test->getIdentifier(), in_item_id) == 0)
		return tmp_test;

	    if (in_recursive)
	    {
		tmp_found_test = (TestCampaign*)findItemWithId(in_item_id, in_child_type, in_recursive);
		if (tmp_found_test != NULL)
		    return tmp_found_test;
	    }
	}
    }

    return NULL;
}


void TestCampaign::writeXml(QXmlStreamWriter & in_xml_writer)
{
    in_xml_writer.writeStartElement("campaignTest");
    in_xml_writer.writeAttribute("testId", getValueForKey(TESTS_CAMPAIGNS_TABLE_TEST_ID));
    in_xml_writer.writeEndElement();
}


bool TestCampaign::readXml(QXmlStreamReader & in_xml_reader)
{
    QString			tmp_test_id = in_xml_reader.attributes().value("testId").toString();

    setValueForKey(tmp_test_id.toStdString().c_str(), TESTS_CAMPAIGNS_TABLE_TEST_ID);
    in_xml_reader.skipCurrentElement();

    return true;
}


int TestCampaign::saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas)
{
    int			tmp_result = NOERR;
    const char		*tmp_test_id = getValueForKey(TESTS_CAMPAIGNS_TABLE_TEST_ID);

    if (is_empty_string(getOriginalValueForKey(TESTS_CAMPAIGNS_TABLE_TEST_CAMPAIGN_ID)) == TRUE)
    {
	if (_m_campaign != NULL)
	{
        setValueForKey(_m_campaign->getIdentifier(), TESTS_CAMPAIGNS_TABLE_CAMPAIGN_ID);
        setValueForKey(in_xml_datas.getNewTestHierarchyIdentifier(_m_campaign->projectVersion(), tmp_test_id), TESTS_CAMPAIGNS_TABLE_TEST_ID);
	}

	tmp_result = insertRecord();
    }

    return tmp_result;
}


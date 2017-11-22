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

#ifndef TESTREQUIREMENT_H
#define TESTREQUIREMENT_H

#include "record.h"
#include "testcontent.h"
#include "requirementcontent.h"
#include "xmlprojectdatas.h"


class TestRequirement : public Record
{
private:
    char        *_m_test_short_name;
    char        *_m_test_category;
    char        *_m_test_version;

    char        *_m_requirement_short_name;
    char        *_m_requirement_category;
    char        *_m_requirement_version;

public:
    TestRequirement();
    ~TestRequirement();

    const entity_def* getEntityDef() const;

    void setTestShortName(const char *in_test_short_name);
    void setTestCategory(const char *in_test_category);
    void setTestVersion(const char *in_test_version);

    const char *testShortName(){return _m_test_short_name;}
    const char *testCategory(){return _m_test_category;}
    const char *testVersion(){return _m_test_version;}

    void setRequirementShortName(const char *in_requirement_short_name);
    void setRequirementCategory(const char *in_requirement_category);
    void setRequirementVersion(const char *in_requirement_version);

    const char *requirementShortName(){return _m_requirement_short_name;}
    const char *requirementCategory(){return _m_requirement_category;}
    const char *requirementVersion(){return _m_requirement_version;}

    void writeXml(QXmlStreamWriter & in_xml_writer);
    bool readXml(QXmlStreamReader & in_xml_reader);
    int saveFromXmlProjectDatas(XmlProjectDatas & in_xml_datas);

};

#endif // TESTREQUIREMENT_H

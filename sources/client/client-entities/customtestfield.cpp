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

#include "customtestfield.h"

CustomTestField::CustomTestField() :
  Entity<CustomTestField>(),
  CustomField(),
  _m_test_content(NULL)
{
}

CustomTestField::~CustomTestField()
{
  delete _m_test_content;
}

const entity_def* CustomTestField::getEntityDefinition()
{
  return &custom_test_fields_table_def;
}


int CustomTestField::saveRecord()
{
  const CustomFieldDesc* fieldDesc = getFieldDesc();
  if (fieldDesc != NULL){
      setValueForKey(fieldDesc->getIdentifier(), CUSTOM_TEST_FIELDS_TABLE_CUSTOM_FIELD_DESC_ID);
    }

  return Entity<CustomTestField>::saveRecord();
}


void CustomTestField::setTestContent(TestContent *in_test_content)
{
  if (in_test_content)
    {
      delete _m_test_content;
      _m_test_content = in_test_content->clone<TestContent>(false);
      setValueForKey(_m_test_content->getIdentifier(), CUSTOM_TEST_FIELDS_TABLE_TEST_CONTENT_ID);
    }
}


CustomTestField* CustomTestField::copy(TestContent *in_test_content)
{
  CustomTestField* new_field = new CustomTestField();

  new_field->setTestContent(in_test_content);
  new_field->setFieldDesc(getFieldDesc());
  new_field->setValueForKey(getValueForKey(CUSTOM_TEST_FIELDS_TABLE_FIELD_VALUE), CUSTOM_TEST_FIELDS_TABLE_FIELD_VALUE);

  return new_field;
}

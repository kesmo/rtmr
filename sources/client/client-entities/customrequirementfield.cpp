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

#include "customrequirementfield.h"

CustomRequirementField::CustomRequirementField() :
  Entity<CustomRequirementField>(),
  CustomField(),
  _m_requirement_content(NULL)
{
}

CustomRequirementField::~CustomRequirementField()
{
  delete _m_requirement_content;
}

const entity_def* CustomRequirementField::getEntityDefinition()
{
  return &custom_requirement_fields_table_def;
}


int CustomRequirementField::saveRecord()
{
  const CustomFieldDesc* fieldDesc = getFieldDesc();
  if (fieldDesc != NULL){
      setValueForKey(fieldDesc->getIdentifier(), CUSTOM_REQUIREMENT_FIELDS_TABLE_CUSTOM_FIELD_DESC_ID);
    }

  return Entity<CustomRequirementField>::saveRecord();
}


void CustomRequirementField::setRequirementContent(RequirementContent *in_requirement_content)
{
  if (in_requirement_content)
    {
      delete _m_requirement_content;
      _m_requirement_content = in_requirement_content->clone<RequirementContent>(false);
      setValueForKey(_m_requirement_content->getIdentifier(), CUSTOM_REQUIREMENT_FIELDS_TABLE_REQUIREMENT_CONTENT_ID);
    }
}


CustomRequirementField* CustomRequirementField::copy(RequirementContent *in_requirement_content)
{
  CustomRequirementField* new_field = new CustomRequirementField();

  new_field->setRequirementContent(in_requirement_content);
  new_field->setFieldDesc(getFieldDesc());
  new_field->setValueForKey(getValueForKey(CUSTOM_REQUIREMENT_FIELDS_TABLE_FIELD_VALUE), CUSTOM_REQUIREMENT_FIELDS_TABLE_FIELD_VALUE);

  return new_field;
}

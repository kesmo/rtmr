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

#ifndef CustomFieldsControlsManager_H
#define CustomFieldsControlsManager_H

#include "customfield.h"
#include "customfielddesc.h"

#include <QTabWidget>
#include <QToolBox>

class CustomFieldsControlsManager
{
    public:
        CustomFieldsControlsManager();

        void loadCustomFieldsView(QToolBox* toolBox, const QList<CustomFieldDesc *> &customFieldsDesc);
        void popCustomFieldValue(const CustomFieldDesc* customFieldDesc, Record* in_custom_field_record, const char* in_value_key, bool enable_control);
        int pushEnteredCustomFieldValue(const CustomFieldDesc* customFieldDesc, Record* in_custom_test_field, const char* in_value_key);

    private:
        QTabWidget*         _m_custom_fields_tab_widget;
        QMap<const CustomFieldDesc*, QWidget*>      _m_custom_fields_controls_map;

        void addCustomFields(CustomFieldDesc* customFieldDesc, int tabIndex);
        void addCustomFieldsTab(QString tabName);
};

#endif // CustomFieldsControlsManager_H

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

#ifndef FORM_REQUIREMENT_H
#define FORM_REQUIREMENT_H

#include <QtGui/QDialog>
#include "requirementhierarchy.h"

#include "gui/components/record_text_edit_container.h"
#include "gui/components/abstract_project_widget.h"

#include "testrequirement.h"
#include "imaging/Chart.h"

#include "gui/components/custom_fields_controls_manager.h"

#include <clientmodule.h>

namespace Ui
{
    class Form_Requirement;
}

class Form_Requirement :
    public AbstractProjectWidget,
    public CustomFieldsControlsManager
{
    Q_OBJECT
    Q_DISABLE_COPY(Form_Requirement)
public:
    explicit Form_Requirement(QWidget *parent = 0);
    virtual ~Form_Requirement();

    void loadRequirement(RequirementHierarchy *in_requirement);
    void loadRequirementContent(RequirementContent *in_requirement_content);

signals:
    void requirementSaved(RequirementHierarchy *in_requirement);
    void canceled();
    void showTestWithContentId(const char *in_test_content_id);

public slots:
	void loadPreviousRequirementContent();
	void loadNextRequirementContent();
	void setModified();
    void save();
    void cancel();

    void testsDrop(QList<Record*> in_list, int in_row);
    void deletedTestAtIndex(int in_row);
    void showTestAtIndex(QModelIndex in_index);

protected:
    bool saveRequirement();
    bool maybeClose();
    void setTestForRow(TestRequirement *in_test, int in_row);

    virtual void closeEvent(QCloseEvent *in_event);

private:
    RequirementHierarchy          *_m_requirement;
    RequirementContent   *_m_requirement_content;
    RequirementContent   *_m_previous_requirement_content;
    RequirementContent   *_m_next_requirement_content;

    QList<TestRequirement*>	_m_tests;
    QList<TestRequirement*>	_m_removed_tests;

    bool		_m_modifiable;

    Ui::Form_Requirement *_m_ui;

    Chart					*_m_chart;
    QList<Trinome*>         _m_chart_data;

    void initGraph(RequirementContent *in_requirement_content);

    QMap<RequirementModule*, QWidget*>   _m_views_modules_map;

    QList<CustomRequirementField*>		_m_custom_requirements_fields;

    void loadPluginsViews();
    void destroyPluginsViews();
    void savePluginsDatas();

};

#endif // FORM_REQUIREMENT_H

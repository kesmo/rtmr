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

#include "form_search_project.h"
#include "ui_Form_Search_Project.h"
#include "requirementcontent.h"
#include "session.h"

Form_Search_Project::Form_Search_Project(ProjectVersion *in_project_version, QWidget *parent) :
    QDialog(parent),
    _m_ui(new Ui::Form_Search_Project)
{
    setAttribute(Qt::WA_DeleteOnClose);

    _m_project_version = in_project_version;

    _m_ui->setupUi(this);
    _m_ui->search_text->setFocus();

    foreach(RequirementCategory *tmp_requiremetn_category, Session::instance()->requirementsCategories())
    {
    _m_ui->test_category->addItem(TR_CUSTOM_MESSAGE(tmp_requiremetn_category->getValueForKey(REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_LABEL)), tmp_requiremetn_category->getValueForKey(REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_ID));
    }

    _m_ui->comparison->addItem(tr("Egal à "), TestHierarchy::EqualTo);
    _m_ui->comparison->addItem(tr("Inférieur à "), TestHierarchy::LowerThan);
    _m_ui->comparison->addItem(tr("Supérieur à "), TestHierarchy::UpperThan);
    _m_ui->comparison->addItem(tr("Inférieur ou égal à "), TestHierarchy::LowerOrEqualTo);
    _m_ui->comparison->addItem(tr("Supérieur ou égal à "), TestHierarchy::UpperOrEqualTo);

    connect(_m_ui->text_search_indic, SIGNAL(clicked()), this, SLOT(updateControls()));
    connect(_m_ui->tests_from_current_project_version, SIGNAL(clicked()), this, SLOT(updateControls()));
    connect(_m_ui->requirements_from_current_project_version, SIGNAL(clicked()), this, SLOT(updateControls()));
    connect(_m_ui->not_covered_requirements_search_indic, SIGNAL(clicked()), this, SLOT(updateControls()));

    connect(_m_ui->category_indic, SIGNAL(clicked()), this, SLOT(updateControls()));
    connect(_m_ui->priority_indic, SIGNAL(clicked()), this, SLOT(updateControls()));

    connect(_m_ui->search_button, SIGNAL(clicked()), this, SLOT(accept()));
    connect(_m_ui->cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
}


Form_Search_Project::~Form_Search_Project()
{
    delete _m_ui;
}


void Form_Search_Project::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        _m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}


void Form_Search_Project::accept()
{
	if (_m_ui->text_search_indic->isChecked())
	    launchTextualSearch();
	else if (_m_ui->tests_from_current_project_version->isChecked())
	    launchTestsFromCurrentProjectVersionSearch();
	else if (_m_ui->requirements_from_current_project_version->isChecked())
	    launchRequirementsFromCurrentProjectVersionSearch();
        else if (_m_ui->not_covered_requirements_search_indic->isChecked())
	    launchNotCoveredRequirementsSearch();

	QDialog::accept();
}


void Form_Search_Project::updateControls()
{
	bool	tmp_text_search_indic = _m_ui->text_search_indic->isChecked();

	_m_ui->search_text->setReadOnly(!tmp_text_search_indic);
	_m_ui->tests_search_indic->setEnabled(tmp_text_search_indic);
	_m_ui->tests_cases_search_indic->setEnabled(tmp_text_search_indic);
	_m_ui->requirements_search_indic->setEnabled(tmp_text_search_indic);
	_m_ui->case_sensitive_indic->setEnabled(tmp_text_search_indic);
	_m_ui->label->setEnabled(tmp_text_search_indic);

	_m_ui->category_indic->setEnabled(tmp_text_search_indic);
	_m_ui->test_category->setEnabled(tmp_text_search_indic && _m_ui->category_indic->isChecked());

	_m_ui->priority_indic->setEnabled(tmp_text_search_indic);
	_m_ui->comparison->setEnabled(tmp_text_search_indic && _m_ui->priority_indic->isChecked());
	_m_ui->test_priority_level->setEnabled(tmp_text_search_indic && _m_ui->priority_indic->isChecked());

}


void Form_Search_Project::launchTextualSearch()
{
    TestHierarchy		*tmp_test = NULL;
    RequirementHierarchy	*tmp_requirement = NULL;

    char			***tmp_tests_ids = NULL;
    char			***tmp_tests_cases_ids = NULL;
    char			***tmp_requirements_id = NULL;

    net_session			*tmp_session = Session::instance()->getClientSession();

    unsigned long		tmp_rows_count = 0;
    unsigned long		tmp_columns_count = 0;

    unsigned long		tmp_row_index = 0;

    QList<Record*>		tmp_records_list;

    int		tmp_comparison = 0;
    int		tmp_comparison_value = _m_ui->comparison->itemData(_m_ui->comparison->currentIndex()).toInt();

    bool			tmp_add_record = false;


    if (_m_ui->tests_search_indic->isChecked())
    {
	sprintf(tmp_session->m_last_query, "SELECT %s FROM %s WHERE %s=%s AND %s='%s' AND %s IN (SELECT %s FROM %s WHERE %s %s '%s' OR description_plain_text %s '%s');",
		TESTS_TABLE_TEST_ID,
		TESTS_TABLE_SIG,
		TESTS_TABLE_PROJECT_ID,
        _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID),
		TESTS_TABLE_VERSION,
        _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION),
		TESTS_TABLE_TEST_CONTENT_ID,
		TESTS_CONTENTS_TABLE_TEST_CONTENT_ID,
		TESTS_CONTENTS_TABLE_SIG,
		TESTS_CONTENTS_TABLE_SHORT_NAME,
		(_m_ui->case_sensitive_indic->isChecked() ? "~" : "~*"),
		_m_ui->search_text->text().toStdString().c_str(),
		(_m_ui->case_sensitive_indic->isChecked() ? "~" : "~*"),
		_m_ui->search_text->text().toStdString().c_str());
	tmp_tests_ids = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
	if (tmp_tests_ids != NULL)
	{
	    for (tmp_row_index = 0; tmp_row_index < tmp_rows_count; tmp_row_index++)
	    {
		tmp_test = TestHierarchy::findTestWithId(_m_project_version->testsHierarchy(), tmp_tests_ids[tmp_row_index][0]);
		if (tmp_test != NULL)
		{
		    tmp_add_record = true;
		    if (_m_ui->category_indic->isChecked())
		    {
			QVariant tmp_test_category = _m_ui->test_category->itemData(_m_ui->test_category->currentIndex());
            if (compare_values(tmp_test_category.toString().toStdString().c_str(), tmp_test->getValueForKey(TESTS_HIERARCHY_CATEGORY_ID)) != 0)
			    tmp_add_record = false;
		    }

		    if (_m_ui->priority_indic->isChecked())
		    {
            tmp_comparison = compare_values(tmp_test->getValueForKey(TESTS_HIERARCHY_PRIORITY_LEVEL), QString::number(_m_ui->test_priority_level->value()).toStdString().c_str());
			if (!((tmp_comparison == 0 && (tmp_comparison_value == Record::EqualTo || tmp_comparison_value == Record::LowerOrEqualTo || tmp_comparison_value == Record::UpperOrEqualTo))
			    || (tmp_comparison < 0 && (tmp_comparison_value == Record::LowerThan || tmp_comparison_value == Record::LowerOrEqualTo))
			    || (tmp_comparison > 0 && (tmp_comparison_value == Record::UpperThan || tmp_comparison_value == Record::UpperOrEqualTo))))
			{
				tmp_add_record = false;
			}

		    }

		    if (tmp_add_record)
			tmp_records_list.append(tmp_test);
		}
	    }
	    cl_free_rows_columns_array(&tmp_tests_ids, tmp_rows_count, tmp_columns_count);

	}
    }

    if (_m_ui->tests_cases_search_indic->isChecked())
    {
	sprintf(tmp_session->m_last_query, "SELECT %s FROM %s WHERE %s=%s AND %s='%s' AND %s IN (SELECT %s FROM %s WHERE description_plain_text %s '%s' OR wait_result_plain_text %s '%s');",
		TESTS_TABLE_TEST_ID,
		TESTS_TABLE_SIG,
		TESTS_TABLE_PROJECT_ID,
        _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID),
		TESTS_TABLE_VERSION,
        _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION),
		TESTS_TABLE_TEST_CONTENT_ID,
		ACTIONS_TABLE_TEST_CONTENT_ID,
		ACTIONS_TABLE_SIG,
		(_m_ui->case_sensitive_indic->isChecked() ? "~" : "~*"),
		_m_ui->search_text->text().toStdString().c_str(),
		(_m_ui->case_sensitive_indic->isChecked() ? "~" : "~*"),
		_m_ui->search_text->text().toStdString().c_str());
	tmp_tests_cases_ids = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
	if (tmp_tests_cases_ids != NULL)
	{
	    for (tmp_row_index = 0; tmp_row_index < tmp_rows_count; tmp_row_index++)
	    {
		tmp_test = TestHierarchy::findTestWithId(_m_project_version->testsHierarchy(), tmp_tests_cases_ids[tmp_row_index][0]);
		if (tmp_test != NULL)
		{
		    if (tmp_records_list.indexOf(tmp_test) < 0)
		    {
			tmp_add_record = true;
			if (_m_ui->category_indic->isChecked())
			{
			    QVariant tmp_test_category = _m_ui->test_category->itemData(_m_ui->test_category->currentIndex());
                if (compare_values(tmp_test_category.toString().toStdString().c_str(), tmp_test->getValueForKey(TESTS_HIERARCHY_CATEGORY_ID)) != 0)
				tmp_add_record = false;
			}

			if (_m_ui->priority_indic->isChecked())
			{
                tmp_comparison = compare_values(tmp_test->getValueForKey(TESTS_HIERARCHY_PRIORITY_LEVEL), QString::number(_m_ui->test_priority_level->value()).toStdString().c_str());
			    if (!((tmp_comparison == 0 && (tmp_comparison_value == Record::EqualTo || tmp_comparison_value == Record::LowerOrEqualTo || tmp_comparison_value == Record::UpperOrEqualTo))
				|| (tmp_comparison < 0 && (tmp_comparison_value == Record::LowerThan || tmp_comparison_value == Record::LowerOrEqualTo))
				|| (tmp_comparison > 0 && (tmp_comparison_value == Record::UpperThan || tmp_comparison_value == Record::UpperOrEqualTo))))
				{
				tmp_add_record = false;
			    }

			}

			if (tmp_add_record)
			    tmp_records_list.append(tmp_test);
		    }
		}
	    }
	    cl_free_rows_columns_array(&tmp_tests_cases_ids, tmp_rows_count, tmp_columns_count);
	}
    }

    if (_m_ui->requirements_search_indic->isChecked())
    {
	sprintf(tmp_session->m_last_query, "SELECT %s FROM %s WHERE %s=%s AND %s='%s' AND %s IN (SELECT %s FROM %s WHERE %s %s '%s' OR description_plain_text %s '%s');",
		REQUIREMENTS_TABLE_REQUIREMENT_ID,
		REQUIREMENTS_TABLE_SIG,
		REQUIREMENTS_TABLE_PROJECT_ID,
        _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID),
		REQUIREMENTS_TABLE_VERSION,
        _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION),
		REQUIREMENTS_TABLE_REQUIREMENT_CONTENT_ID,
		REQUIREMENTS_CONTENTS_TABLE_REQUIREMENT_CONTENT_ID,
		REQUIREMENTS_CONTENTS_TABLE_SIG,
		REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME,
		(_m_ui->case_sensitive_indic->isChecked() ? "~" : "~*"),
		_m_ui->search_text->text().toStdString().c_str(),
		(_m_ui->case_sensitive_indic->isChecked() ? "~" : "~*"),
		_m_ui->search_text->text().toStdString().c_str());
	tmp_requirements_id = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
	if (tmp_requirements_id != NULL)
	{
	    for (tmp_row_index = 0; tmp_row_index < tmp_rows_count; tmp_row_index++)
	    {
		tmp_requirement = RequirementHierarchy::findRequirementWithId(_m_project_version->requirementsHierarchy(), tmp_requirements_id[tmp_row_index][0]);
		if (tmp_requirement != NULL)
		{
		    tmp_add_record = true;
		    if (_m_ui->category_indic->isChecked())
		    {
			QVariant tmp_requirement_category = _m_ui->test_category->itemData(_m_ui->test_category->currentIndex());
            if (compare_values(tmp_requirement_category.toString().toStdString().c_str(), tmp_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_CATEGORY_ID)) != 0)
			    tmp_add_record = false;
		    }

		    if (_m_ui->priority_indic->isChecked())
		    {
            tmp_comparison = compare_values(tmp_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_PRIORITY_LEVEL), QString::number(_m_ui->test_priority_level->value()).toStdString().c_str());
			if (!((tmp_comparison == 0 && (tmp_comparison_value == Record::EqualTo || tmp_comparison_value == Record::LowerOrEqualTo || tmp_comparison_value == Record::UpperOrEqualTo))
			    || (tmp_comparison < 0 && (tmp_comparison_value == Record::LowerThan || tmp_comparison_value == Record::LowerOrEqualTo))
			    || (tmp_comparison > 0 && (tmp_comparison_value == Record::UpperThan || tmp_comparison_value == Record::UpperOrEqualTo))))
			{
			    tmp_add_record = false;
			}

		    }

		    if (tmp_add_record)
			tmp_records_list.append(tmp_requirement);
		}
	    }
	    cl_free_rows_columns_array(&tmp_requirements_id, tmp_rows_count, tmp_columns_count);
	}
    }

    emit foundRecords(tmp_records_list);
}


void Form_Search_Project::launchTestsFromCurrentProjectVersionSearch()
{
    QList<Record*>		tmp_records_list;

    foreach(TestHierarchy *tmp_test, _m_project_version->testsHierarchy())
    {
    tmp_test->searchFieldWithValue(&tmp_records_list, TESTS_HIERARCHY_CONTENT_VERSION, _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), true);
    }

    emit foundRecords(tmp_records_list);

}



void Form_Search_Project::launchRequirementsFromCurrentProjectVersionSearch()
{
    QList<Record*>		tmp_records_list;

    foreach(RequirementHierarchy *tmp_requirement, _m_project_version->requirementsHierarchy())
    {
    tmp_requirement->searchFieldWithValue(&tmp_records_list, REQUIREMENTS_HIERARCHY_CONTENT_VERSION, _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION), true);
    }

    emit foundRecords(tmp_records_list);
}


void Form_Search_Project::launchNotCoveredRequirementsSearch()
{
    net_session		*tmp_session = Session::instance()->getClientSession();
    char                *tmp_query = tmp_session->m_last_query;
    char                ***tmp_query_results = NULL;
    unsigned long       tmp_rows_count = 0;
    unsigned long       tmp_columns_count = 0;
    RequirementHierarchy      *tmp_requirement = NULL;

    QList<Record*>		tmp_records_list;

    tmp_query += sprintf(tmp_query, "SELECT %s FROM %s WHERE %s=%s AND %s='%s' AND %s NOT IN ("\
			 "SELECT %s FROM %s WHERE %s IN ("\
			 "SELECT %s FROM %s WHERE %s IN ("\
			 "SELECT %s FROM %s WHERE %s='%s')));"
			 ,REQUIREMENTS_TABLE_REQUIREMENT_ID
			 ,REQUIREMENTS_TABLE_SIG
			 ,REQUIREMENTS_TABLE_PROJECT_ID
             ,_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_PROJECT_ID)
			 ,REQUIREMENTS_TABLE_VERSION
             ,_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION)
			 ,REQUIREMENTS_TABLE_REQUIREMENT_CONTENT_ID
			 ,REQUIREMENTS_CONTENTS_TABLE_REQUIREMENT_CONTENT_ID
			 ,REQUIREMENTS_CONTENTS_TABLE_SIG
			 ,REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID
			 ,TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID
			 ,TESTS_REQUIREMENTS_TABLE_SIG
			 ,TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID
			 ,TESTS_TABLE_TEST_CONTENT_ID
			 ,TESTS_TABLE_SIG
			 ,TESTS_TABLE_VERSION
             ,_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION)
			 );

    tmp_query_results = cl_run_sql(tmp_session, tmp_session->m_last_query, &tmp_rows_count, &tmp_columns_count);
    if (tmp_query_results != NULL)
    {
	for(unsigned long tmp_index = 0; tmp_index < tmp_rows_count; tmp_index++)
	{
	    tmp_requirement = RequirementHierarchy::findRequirementWithId(_m_project_version->requirementsHierarchy(), tmp_query_results[tmp_index][0]);
	    if (tmp_requirement != NULL)
	    {
		tmp_records_list.append(tmp_requirement);
	    }
	}
	cl_free_rows_columns_array(&tmp_query_results, tmp_rows_count, tmp_columns_count);
    }

    emit foundRecords(tmp_records_list);

}



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

#include "form_campaign_wizard.h"
#include "ui_Form_Campaign_Wizard.h"
#include "test.h"
#include "testhierarchy.h"
#include "session.h"

Form_Campaign_Wizard::Form_Campaign_Wizard(ProjectVersion *in_project, QWidget *parent) :
	QWizard(parent),
	_m_ui(new Ui::Form_Campaign_Wizard)
{
    setAttribute(Qt::WA_DeleteOnClose);

    _m_project = in_project;

    _m_ui->setupUi(this);

    // Initialisation des pages
    _m_ui->project_short_name_label->setText(in_project->project()->getValueForKey(PROJECTS_TABLE_SHORT_NAME));
    _m_ui->project_version_label->setText(ProjectVersion::formatProjectVersionNumber(in_project->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION)));

    foreach(RequirementCategory *tmp_requiremetn_category, Session::instance()->requirementsCategories())
    {
    _m_ui->test_category->addItem(TR_CUSTOM_MESSAGE(tmp_requiremetn_category->getValueForKey(REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_LABEL)), tmp_requiremetn_category->getValueForKey(REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_ID));
    }

    _m_ui->comparison->addItem(tr("Egal à "), TestHierarchy::EqualTo);
    _m_ui->comparison->addItem(tr("Inférieur à "), TestHierarchy::LowerThan);
    _m_ui->comparison->addItem(tr("Supérieur à "), TestHierarchy::UpperThan);
    _m_ui->comparison->addItem(tr("Inférieur ou égal à "), TestHierarchy::LowerOrEqualTo);
    _m_ui->comparison->addItem(tr("Supérieur ou égal à "), TestHierarchy::UpperOrEqualTo);

    connect(_m_ui->campaign_short_name, SIGNAL(textChanged(QString)), this, SLOT(campaignShortNameChanged()));

    connect(_m_ui->select_none_test_indic, SIGNAL(clicked()), this, SLOT(updateControls()));
    connect(_m_ui->select_specifics_tests_indic, SIGNAL(clicked()), this, SLOT(updateControls()));
    connect(_m_ui->select_all_tests_indic, SIGNAL(clicked()), this, SLOT(updateControls()));

    connect(_m_ui->category_indic, SIGNAL(clicked()), this, SLOT(updateControls()));
    connect(_m_ui->priority_indic, SIGNAL(clicked()), this, SLOT(updateControls()));
}

Form_Campaign_Wizard::~Form_Campaign_Wizard()
{
    delete _m_ui;
}

void Form_Campaign_Wizard::changeEvent(QEvent *e)
{
    QWizard::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        _m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void Form_Campaign_Wizard::campaignShortNameChanged()
{
    if (_m_ui->campaign_short_name->text().isEmpty())
    {
    }
}


void Form_Campaign_Wizard::accept()
{
    Campaign 		*tmp_new_campaign = NULL;
    int					tmp_tests_index = 0;
    QList<Record*>	*tmp_tests_category_list = NULL;
    QList<Record*>	*tmp_tests_priority_list = NULL;
    TestHierarchy				*tmp_test = NULL;
    QVariant				tmp_test_category;
    int			tmp_save_result = NOERR;
    QList<Hierarchy*>	tmp_tests_list;
    QList<Hierarchy*>	tmp_final_tests_list;

    if (_m_ui->campaign_short_name->text().isEmpty())
    {
        return;
    }

    tmp_new_campaign = new Campaign(_m_project);
    tmp_new_campaign->setValueForKey(_m_ui->campaign_short_name->text().toStdString().c_str(), CAMPAIGNS_TABLE_SHORT_NAME);
    tmp_new_campaign->setValueForKey(_m_ui->campaign_description->toHtml().toStdString().c_str(), CAMPAIGNS_TABLE_DESCRIPTION);
    tmp_new_campaign->saveRecord();
    if (tmp_save_result == NOERR)
    {
	// Campagne vide
	if (_m_ui->select_none_test_indic->isChecked())
	{

	}
	// Campagne de tests spécifiques
	else if (_m_ui->select_specifics_tests_indic->isChecked())
	{
	    tmp_test_category = _m_ui->test_category->itemData(_m_ui->test_category->currentIndex());

	    // Recherche par catégories
	    if (_m_ui->category_indic->isChecked() && tmp_test_category.isValid())
	    {
		tmp_tests_category_list = new QList<Record*>();
		foreach(tmp_test, _m_project->testsHierarchy())
		{
		    tmp_test->searchFieldWithValue(tmp_tests_category_list, TESTS_HIERARCHY_CATEGORY_ID, tmp_test_category.toString().toStdString().c_str(), true);
		}

		// Recherche par catégories et par niveau de priorité
		if (_m_ui->priority_indic->isChecked())
		{
		    tmp_tests_priority_list = new QList<Record*>();
		    for(tmp_tests_index = 0; tmp_tests_index < tmp_tests_category_list->count(); tmp_tests_index++)
		    {
			((TestHierarchy*)tmp_tests_category_list->at(tmp_tests_index))->searchFieldWithValue(tmp_tests_priority_list, TESTS_HIERARCHY_PRIORITY_LEVEL, QString::number(_m_ui->test_priority_level->value()).toStdString().c_str(), false, _m_ui->comparison->itemData(_m_ui->comparison->currentIndex()).toInt());
		    }

		    for(tmp_tests_index = 0; tmp_tests_index < tmp_tests_priority_list->count(); tmp_tests_index++)
		    {
            if (is_empty_string(tmp_tests_priority_list->at(tmp_tests_index)->getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_ID)))
			    tmp_tests_list.append((TestHierarchy*)tmp_tests_priority_list->at(tmp_tests_index));
		    }
		}
		// Recherche par catégories seulement
		else
		{
		    for(tmp_tests_index = 0; tmp_tests_index < tmp_tests_category_list->count(); tmp_tests_index++)
		    {
            if (is_empty_string(tmp_tests_category_list->at(tmp_tests_index)->getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_ID)))
			    tmp_tests_list.append((TestHierarchy*)tmp_tests_category_list->at(tmp_tests_index));
		    }
		}
	    }
	    // Recherche par niveau de priorité
	    else if (_m_ui->priority_indic->isChecked())
	    {
		tmp_tests_priority_list = new QList<Record*>();
		foreach(tmp_test, _m_project->testsHierarchy())
		{
		    tmp_test->searchFieldWithValue(tmp_tests_priority_list, TESTS_HIERARCHY_PRIORITY_LEVEL, QString::number(_m_ui->test_priority_level->value()).toStdString().c_str(), true, _m_ui->comparison->itemData(_m_ui->comparison->currentIndex()).toInt());
		}

		for(tmp_tests_index = 0; tmp_tests_index < tmp_tests_priority_list->count(); tmp_tests_index++)
		{
            if (is_empty_string(tmp_tests_priority_list->at(tmp_tests_index)->getValueForKey(TESTS_HIERARCHY_ORIGINAL_TEST_ID)))
			tmp_tests_list.append((TestHierarchy*)tmp_tests_priority_list->at(tmp_tests_index));
		}
	    }

	    tmp_tests_index = 0;
	    tmp_final_tests_list = Hierarchy::parentRecordsFromRecordsList(tmp_tests_list);
	    foreach(Hierarchy* tmp_tests_list, tmp_final_tests_list)
	    {
		tmp_new_campaign->insertTestAtIndex((TestHierarchy*)tmp_tests_list, tmp_tests_index);
		tmp_tests_index++;
	    }

	}
	// Campagne complète (copier tous les tests du projet)
	else if (_m_ui->select_all_tests_indic->isChecked())
	{
	    foreach(TestHierarchy *tmp_test, _m_project->testsHierarchy())
	    {
		tmp_new_campaign->insertTestAtIndex(tmp_test, tmp_tests_index);
		tmp_tests_index++;
	    }
	}
	else
	{
	    return;
	}
    }

    QWizard::accept();
    emit campaignCreated(tmp_new_campaign);
}


void Form_Campaign_Wizard::updateControls()
{
    bool	tmp_specifics_tests_indic = (_m_ui->select_specifics_tests_indic->isChecked());

    _m_ui->category_indic->setEnabled(tmp_specifics_tests_indic);
    _m_ui->test_category->setEnabled(_m_ui->category_indic->isChecked());

    _m_ui->priority_indic->setEnabled(tmp_specifics_tests_indic);
    _m_ui->comparison->setEnabled(_m_ui->priority_indic->isChecked());
    _m_ui->test_priority_level->setEnabled(_m_ui->priority_indic->isChecked());
}

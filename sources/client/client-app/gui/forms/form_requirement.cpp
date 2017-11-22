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

#include "session.h"
#include "utilities.h"
#include "form_requirement.h"
#include "ui_Form_Requirement.h"
#include "testrequirement.h"

#include <QtGui>

/**
  Constructeur
**/
Form_Requirement::Form_Requirement(QWidget *parent)
    : AbstractProjectWidget(parent),
    CustomFieldsControlsManager(),
    _m_ui(new Ui::Form_Requirement),
    _m_chart(NULL)
{
    vector<QColor>              tmp_colors;

    setAttribute(Qt::WA_DeleteOnClose);

    _m_requirement = NULL;
    _m_requirement_content = NULL;
    _m_previous_requirement_content = NULL;
    _m_next_requirement_content = NULL;
    _m_modifiable = false;

    _m_ui->setupUi(this);

    _m_ui->requirement_description->addTextToolBar(RecordTextEditToolBar::Small);

    foreach(RequirementCategory *tmp_requiremetn_category, Session::instance()->requirementsCategories())
    {
        _m_ui->requirement_category->addItem(TR_CUSTOM_MESSAGE(tmp_requiremetn_category->getValueForKey(REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_LABEL)), tmp_requiremetn_category->getValueForKey(REQUIREMENTS_CATEGORIES_TABLE_CATEGORY_ID));
    }

    connect(_m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(save()));
    connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));

    connect(_m_ui->requirement_name, SIGNAL(textChanged(const QString &)), this, SLOT(setModified()));
    connect(_m_ui->requirement_description->textEditor(), SIGNAL(textChanged()), this, SLOT(setModified()));
    connect(_m_ui->requirement_category, SIGNAL(currentIndexChanged(int)), this, SLOT(setModified()));
    connect(_m_ui->requirement_priority_level, SIGNAL(valueChanged(int)), this, SLOT(setModified()));

    connect(_m_ui->tests_list, SIGNAL(recordslistDrop(QList<Record*>, int)), this, SLOT(testsDrop(QList<Record*>, int)));
    connect(_m_ui->tests_list, SIGNAL(rowRemoved(int)), this, SLOT(deletedTestAtIndex(int)));
    connect(_m_ui->tests_list, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showTestAtIndex(QModelIndex)));


    connect(_m_ui->previous_version_button, SIGNAL(clicked()), this, SLOT(loadPreviousRequirementContent()));
    connect(_m_ui->next_version_button, SIGNAL(clicked()), this, SLOT(loadNextRequirementContent()));

    _m_chart = new Chart(800, 600);
    _m_chart->setRenderingForQuality();
    _m_chart->m_stack_histogram_indic = true;
    _m_chart->m_show_legend = true;

    tmp_colors.push_back(ExecutionTest::incompleteColor());
    tmp_colors.push_back(ExecutionTest::koColor());
    tmp_colors.push_back(ExecutionTest::okColor());
    tmp_colors.push_back(ExecutionTest::byPassedColor());

    _m_chart->setGraphColors(tmp_colors);
    _m_chart->setLegendPosition(Chart::LEGEND_LEFT);

    _m_ui->requirement_id->setVisible(Session::instance()->getClientSession()->m_debug);

    loadPluginsViews();
    loadCustomFieldsView(_m_ui->toolBox, Session::instance()->customRequirementsFieldsDesc());
}


/**
  Destructeur
**/
Form_Requirement::~Form_Requirement()
{
    destroyPluginsViews();

    delete _m_chart;
    delete _m_requirement_content;
    delete _m_previous_requirement_content;
    delete _m_next_requirement_content;
    delete _m_ui;

    qDeleteAll(_m_tests);
    qDeleteAll(_m_removed_tests);
    qDeleteAll(_m_chart_data);

    qDeleteAll(_m_custom_requirements_fields);

}


void Form_Requirement::setModified()
{
    _m_modified = true;
}


void Form_Requirement::initGraph(RequirementContent *in_requirement_content)
{
    char                ***tmp_results = NULL;
    unsigned long       tmp_rows_count = 0;
    unsigned long       tmp_columns_count = 0;

    QDateTime           tmp_date_time;

    const char          *tmp_tests_results[] = {EXECUTION_TEST_VALIDATED, EXECUTION_TEST_INVALIDATED, EXECUTION_TEST_BYPASSED, EXECUTION_TEST_INCOMPLETED};

    char                *tmp_ptr = Session::instance()->getClientSession()->m_last_query;

    qDeleteAll(_m_chart_data);
    _m_chart_data.clear();

    if (Session::instance()->testsResults().count() == 4)
    {
        for (int tmp_index = 0; tmp_index < 4; tmp_index++)
        {
            tmp_ptr += sprintf(tmp_ptr, "select count(resultat), '%s', executions_campaigns_table.execution_date, campaigns_table.short_name "\
            "from campaigns_table, executions_campaigns_table LEFT OUTER JOIN "\
            "("\
            "select executions_campaigns_table.execution_campaign_id "\
            "from requirements_contents_table, tests_requirements_table, tests_table, "\
            "executions_campaigns_table, campaigns_table, executions_tests_table "\
            "where  "\
            "requirements_contents_table.requirement_content_id=%s and "\
            "requirements_contents_table.original_requirement_content_id=tests_requirements_table.original_requirement_content_id and "\
            "tests_requirements_table.test_content_id=tests_table.test_content_id and "\
            "tests_table.version>='%s' and "\
            "executions_campaigns_table.campaign_id=campaigns_table.campaign_id and "\
            "campaigns_table.version>='%s' and " \
            "executions_tests_table.execution_campaign_id=executions_campaigns_table.execution_campaign_id and "\
            "tests_table.test_id=executions_tests_table.test_id and "\
            "executions_tests_table.result_id='%s' "\
            ") as resultat "\
            "ON (executions_campaigns_table.execution_campaign_id=resultat.execution_campaign_id) "\
            "where "\
            "executions_campaigns_table.campaign_id=campaigns_table.campaign_id "\
            "group by executions_campaigns_table.execution_date, campaigns_table.short_name",
            TR_CUSTOM_MESSAGE(Session::instance()->testsResults()[tmp_index]->getValueForKey(TESTS_RESULTS_TABLE_DESCRIPTION)).toStdString().c_str(),
            in_requirement_content->getIdentifier(),
            in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_VERSION),
            in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_VERSION),
            tmp_tests_results[tmp_index]);

            if (tmp_index < 3)
                tmp_ptr += sprintf(tmp_ptr, "\nUNION ALL\n");
            else
                tmp_ptr += sprintf(tmp_ptr, ";");
        }

        tmp_results = cl_run_sql(Session::instance()->getClientSession(), Session::instance()->getClientSession()->m_last_query, &tmp_rows_count, &tmp_columns_count);
        if (tmp_results != NULL)
        {
            if (tmp_columns_count == 4)
            {
                for (unsigned long tmp_index = 0; tmp_index < tmp_rows_count; tmp_index++)
                {
                    tmp_date_time = QDateTime::fromString(QString(tmp_results[tmp_index][2]).left(16), "yyyy-MM-dd hh:mm");
                    _m_chart_data.append(new Trinome(new String(QString(tmp_results[tmp_index][3]) + " " + QString(tmp_date_time.toString("yyyy-MM-dd hh:mm"))),
                    new Double(atol(tmp_results[tmp_index][0])),
                    new String(tmp_results[tmp_index][1])));
                }
            }

            cl_free_rows_columns_array(&tmp_results, tmp_rows_count, tmp_columns_count);
        }
    }

    _m_chart->m_auto_borders = true;
    _m_chart->setData(TrinomeArray(_m_chart_data));
    _m_chart->setBorderSize(-1, -1, -1, -1);
    _m_chart->m_auto_borders = false;
    _m_chart->setBorderSize(-1, 10, 10, 10);
    _m_chart->clear();
    _m_chart->drawGraph(Chart::Camembert);

    _m_ui->pixmap->setPixmap(*_m_chart->getImage());
    _m_ui->pixmap->update();
}


void Form_Requirement::loadRequirement(RequirementHierarchy *in_requirement)
{
    QStringList             tmp_tests_headers;

    _m_ui->requirement_name->setFocus();

    _m_ui->lock_widget->setVisible(false);

    tmp_tests_headers << tr("Description") << tr("Catégorie") << tr("Version");
    _m_ui->tests_list->setHorizontalHeaderLabels(tmp_tests_headers);
    _m_ui->tests_list->horizontalHeader()->setResizeMode(QHeaderView::Interactive);

    if (in_requirement != NULL)
    {
        _m_requirement = in_requirement;

        setWindowTitle(tr("Exigence : %1").arg(_m_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)));

        _m_ui->requirement_id->setText(QString(_m_requirement->getIdentifier()) + "(" + QString(_m_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID)) + ")");

        if (_m_requirement_content != NULL)
            delete _m_requirement_content;

        _m_requirement_content = new RequirementContent(_m_requirement->projectVersion());
        if (_m_requirement_content->loadRecord(_m_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_REQUIREMENT_CONTENT_ID)) == NOERR)
        {
            loadRequirementContent(_m_requirement_content);
        }
    }
    else
        cancel();
}


void Form_Requirement::loadPreviousRequirementContent()
{
    RequirementContent	*tmp_requirement_content = _m_previous_requirement_content;

    if (maybeClose())
    {
        if (_m_requirement_content != NULL
        && _m_previous_requirement_content != NULL
        && compare_values(_m_requirement_content->getIdentifier(), _m_previous_requirement_content->getIdentifier()) == 0)
            tmp_requirement_content = _m_requirement_content;

        loadRequirementContent(tmp_requirement_content);
    }
}


void Form_Requirement::loadNextRequirementContent()
{
    RequirementContent	*tmp_requirement_content = _m_next_requirement_content;

    if (maybeClose())
    {
        if (_m_requirement_content != NULL
        && _m_next_requirement_content != NULL
        && compare_values(_m_requirement_content->getIdentifier(), _m_next_requirement_content->getIdentifier()) == 0)
            tmp_requirement_content = _m_requirement_content;

        loadRequirementContent(tmp_requirement_content);
    }
}


void Form_Requirement::loadRequirementContent(RequirementContent *in_requirement_content)
{

    TestRequirement     *tmp_test_requirement = NULL;
    int                 tmp_tests_index = 0;

    _m_modifiable = false;

    _m_ui->tests_list->setRowCount(0);

    if (_m_requirement_content == in_requirement_content)
    {
        if (_m_requirement->isWritable())
        {
            _m_requirement_content->lockRecord(true);

            if (_m_requirement_content->lockRecordStatus() == RECORD_STATUS_LOCKED)
            {
                _m_ui->lock_widget->setVisible(true);
                net_get_field(NET_MESSAGE_TYPE_INDEX+1, Session::instance()->getClientSession()->m_response, Session::instance()->getClientSession()->m_column_buffer, SEPARATOR_CHAR);
                _m_ui->label_lock_by->setText(tr("Verrouillée par ") + QString(Session::instance()->getClientSession()->m_column_buffer));
            }
            else
                _m_modifiable = true;
        }
    }

    initGraph(in_requirement_content);

    _m_ui->previous_version_button->setVisible(false);
    _m_ui->next_version_button->setVisible(false);

    _m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(_m_modifiable);

    _m_ui->tests_list->setRowCount(0);

    qDeleteAll(_m_tests);
    _m_tests.clear();
    qDeleteAll(_m_removed_tests);
    _m_removed_tests.clear();

    qDeleteAll(_m_custom_requirements_fields);
    _m_custom_requirements_fields.clear();

    if (in_requirement_content != NULL)
    {
        _m_previous_requirement_content = in_requirement_content->previousRequirementContent();
        _m_next_requirement_content = in_requirement_content->nextRequirementContent();

        _m_ui->previous_version_button->setVisible(_m_previous_requirement_content != NULL);
        _m_ui->next_version_button->setVisible(_m_next_requirement_content != NULL);

        _m_ui->title->setText(tr("Exigence : %1").arg(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME)));
        _m_ui->requirement_name->setText(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME));
        _m_ui->requirement_name->setEnabled(_m_modifiable);
        _m_ui->requirement_description->textEditor()->setText(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION));
        _m_ui->requirement_description->textEditor()->setReadOnly(!_m_modifiable);
        if (_m_modifiable)
            _m_ui->requirement_description->toolBar()->show();
        else
            _m_ui->requirement_description->toolBar()->hide();

        _m_ui->version_label->setText(ProjectVersion::formatProjectVersionNumber(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_VERSION)));
        _m_ui->requirement_priority_level->setEnabled(_m_modifiable);
        _m_ui->requirement_priority_level->setValue(QString(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL)).toInt());

        _m_ui->requirement_category->setEnabled(_m_modifiable);
        if (is_empty_string(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID)) == FALSE)
        {
            _m_ui->requirement_category->setCurrentIndex(_m_ui->requirement_category->findData(in_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID)));
        }
        else
            _m_ui->requirement_category->setCurrentIndex(0);

        // Charger les tests associes
        _m_tests = in_requirement_content->loadTestRequirementsForProjectVersion(_m_requirement->projectVersion());
        if (_m_tests.count() > 0)
        {
            _m_ui->tests_list->setRowCount(_m_tests.count());
            for (tmp_tests_index = 0; tmp_tests_index < _m_tests.count(); tmp_tests_index++)
            {
                tmp_test_requirement = _m_tests[tmp_tests_index];
                setTestForRow(tmp_test_requirement, tmp_tests_index);
            }
            _m_ui->tests_list->resizeColumnsToContents();
        }
        _m_ui->tests_list->setDisabled(!_m_modifiable);

        // Charger les champs personnalises
        _m_custom_requirements_fields = _m_requirement->loadCustomFields();
        QList<CustomFieldDesc*> customFieldsDesc = Session::instance()->customRequirementsFieldsDesc();

        foreach(CustomFieldDesc* customFieldDesc, customFieldsDesc)
        {
            bool foundRequirementField = false;
            foreach(CustomRequirementField* customRequirementField, _m_custom_requirements_fields)
            {
                if (compare_values(customRequirementField->getValueForKey(CUSTOM_REQUIREMENT_FIELDS_TABLE_CUSTOM_FIELD_DESC_ID),
                    customFieldDesc->getIdentifier()) == 0)
                {
                    customRequirementField->setFieldDesc(customFieldDesc);
                    customRequirementField->setRequirementContent(_m_requirement_content);
                    popCustomFieldValue(customFieldDesc, customRequirementField, CUSTOM_REQUIREMENT_FIELDS_TABLE_FIELD_VALUE, _m_modifiable);
                    foundRequirementField = true;
                    break;
                }
            }

            if (!foundRequirementField)
            {
                CustomRequirementField* customRequirementField = new CustomRequirementField();
                customRequirementField->setFieldDesc(customFieldDesc);
                customRequirementField->setRequirementContent(_m_requirement_content);
                customRequirementField->setValueForKey(customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE), CUSTOM_REQUIREMENT_FIELDS_TABLE_FIELD_VALUE);
                popCustomFieldValue(customFieldDesc, customRequirementField, CUSTOM_REQUIREMENT_FIELDS_TABLE_FIELD_VALUE, _m_modifiable);
                _m_custom_requirements_fields.append(customRequirementField);
            }
        }


    }

    _m_modified = false;
}


void Form_Requirement::save()
{
    if (saveRequirement())
    {
        savePluginsDatas();
        _m_modified = false;
        emit requirementSaved(_m_requirement);
    }
}


/**
  Validation par le bouton OK
**/
bool Form_Requirement::saveRequirement()
{
    int                 tmp_compare_versions_result = 0;
    QMessageBox         *tmp_msg_box;
    QPushButton         *tmp_update_button;
    QPushButton         *tmp_conserv_button;
    QPushButton         *tmp_cancel_button;

    RequirementContent* tmp_old_content = NULL;
    bool                tmp_upgrade = false;
    char                    tmp_priority_level[32];

    int                 tmp_save_result = NOERR;

    TestRequirement	*tmp_test_requirement = NULL;

    QVariant		tmp_requirement_category;

    CustomRequirementField		*tmp_custom_requirement_field = NULL;

    if (_m_ui->requirement_name->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Donnée obligatoire non saisi"), tr("Le nom de l'exigence est nécessaire."));
        return false;
    }

    // Mettre a jour le contenu de l'exigence
    tmp_compare_versions_result = compare_values(_m_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_VERSION), _m_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_VERSION));
    if (tmp_compare_versions_result < 0)
    {
        // La version du contenu de l'exigence est anterieure a la version de l'exigence
        tmp_msg_box = new QMessageBox(QMessageBox::Question, tr("Confirmation..."),
        tr("La version du contenu de l'exigence (%1) est antérieure à  la version courante (%2).\nVoulez-vous mettre à niveau la version du contenu vers la version courante ?").arg(ProjectVersion::formatProjectVersionNumber(_m_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_VERSION))).arg(ProjectVersion::formatProjectVersionNumber(_m_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_VERSION))));
        tmp_update_button = tmp_msg_box->addButton(tr("Mettre à niveau"), QMessageBox::YesRole);
        tmp_conserv_button = tmp_msg_box->addButton(tr("Conserver la version"), QMessageBox::NoRole);
        tmp_cancel_button = tmp_msg_box->addButton(tr("Annuler"), QMessageBox::RejectRole);

        tmp_msg_box->exec();
        if (tmp_msg_box->clickedButton() == tmp_update_button)
        {
            delete tmp_msg_box;
                tmp_upgrade = true;
                tmp_old_content = _m_requirement_content;
                _m_requirement_content = tmp_old_content->copy();
                delete tmp_old_content;
        }
        else if(tmp_msg_box->clickedButton() == tmp_cancel_button)
	{
	    delete tmp_msg_box;
	    return false;
	}
    }

    _m_requirement_content->setValueForKey(_m_ui->requirement_name->text().toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_SHORT_NAME);
    _m_requirement_content->setValueForKey(_m_ui->requirement_description->textEditor()->toHtml().toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_DESCRIPTION);
    sprintf(tmp_priority_level, "%i", _m_ui->requirement_priority_level->value());
    _m_requirement_content->setValueForKey(tmp_priority_level, REQUIREMENTS_CONTENTS_TABLE_PRIORITY_LEVEL);

    tmp_requirement_category =_m_ui->requirement_category->itemData(_m_ui->requirement_category->currentIndex());
    if (tmp_requirement_category.isValid())
        _m_requirement_content->setValueForKey(tmp_requirement_category.toString().toStdString().c_str(), REQUIREMENTS_CONTENTS_TABLE_CATEGORY_ID);

    tmp_save_result = _m_requirement_content->saveRecord();
    if (tmp_save_result == NOERR)
    {
        _m_requirement->setDataFromRequirementContent(_m_requirement_content);
        tmp_save_result = _m_requirement->saveRecord();
    }

    // Mettre a jour les tests a supprimer
    for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_removed_tests.count(); tmp_index++)
    {
        tmp_test_requirement = _m_removed_tests[tmp_index];
        if (tmp_test_requirement != NULL)
        {
            tmp_save_result = tmp_test_requirement->deleteRecord();
        }
    }

    // Mettre a jour les tests
    for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_tests.count(); tmp_index++)
    {
        tmp_test_requirement = _m_tests[tmp_index];
        if (tmp_test_requirement != NULL)
        {
            tmp_save_result = tmp_test_requirement->saveRecord();
        }
    }

    // Mettre a jour les champs personnalises
    for (int tmp_index = 0; tmp_save_result == NOERR && tmp_index < _m_custom_requirements_fields.count(); tmp_index++)
    {
        tmp_custom_requirement_field = _m_custom_requirements_fields[tmp_index];
        if (tmp_custom_requirement_field != NULL)
        {
            if (tmp_upgrade)
                tmp_custom_requirement_field = tmp_custom_requirement_field->copy(_m_requirement_content);

            tmp_save_result = pushEnteredCustomFieldValue(tmp_custom_requirement_field->getFieldDesc(), tmp_custom_requirement_field, CUSTOM_REQUIREMENT_FIELDS_TABLE_FIELD_VALUE);
            if (tmp_save_result == EMPTY_OBJECT)
            {
                QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), tr("Le champ personnalisé <b>%1</b> de l'onglet <b>%2</b> est obligatoire.")
                    .arg(tmp_custom_requirement_field->getFieldDesc()->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_LABEL))
                    .arg(tmp_custom_requirement_field->getFieldDesc()->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME)));
                return false;
            }
            else if (tmp_save_result == NOERR)
                tmp_save_result = tmp_custom_requirement_field->saveRecord();

        }
    }

    if (tmp_save_result != NOERR)
    {
        QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_save_result));
        return false;
    }

    qDeleteAll(_m_removed_tests);
    _m_removed_tests.clear();

    _m_ui->version_label->setText(ProjectVersion::formatProjectVersionNumber(_m_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_VERSION)));

    return true;
}


void Form_Requirement::cancel()
{
    QWidget::close();
}


void Form_Requirement::closeEvent(QCloseEvent *in_event)
{
    if (maybeClose())
        in_event->accept();
    else
        in_event->ignore();
}


bool Form_Requirement::maybeClose()
{
    int		tmp_confirm_choice = 0;
    bool    tmp_return = true;

    if (_m_requirement_content != NULL)
    {
        if (_m_modified)
        {
            tmp_confirm_choice = QMessageBox::question(
            this,
            tr("Confirmation..."),
            tr("L'exigence a été modifiée. Voulez-vous enregistrer les modifications ?"),
            QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
            QMessageBox::Cancel);

            if (tmp_confirm_choice == QMessageBox::Yes)
                tmp_return = saveRequirement();
            else if (tmp_confirm_choice == QMessageBox::Cancel)
                tmp_return = false;
        }

        if (tmp_return)
            _m_requirement_content->unlockRecord();
    }

    return tmp_return;
}



void Form_Requirement::testsDrop(QList<Record*> in_list, int /* in_row */)
{
    TestRequirement  		*tmp_test_requirement = NULL;
    TestRequirement  		*tmp_current_test_requirement = NULL;
    QTableWidgetItem    	*tmp_item = NULL;
    bool                	tmp_item_exists = false;

    if (_m_modifiable)
    {
        foreach(Record *tmp_record, in_list)
        {
            if (tmp_record->getEntityDefSignatureId() == TESTS_HIERARCHY_SIG_ID)
            {
                if (tmp_record != NULL)
                {
                    // Verifier le projet
                    if (compare_values(tmp_record->getValueForKey(TESTS_HIERARCHY_PROJECT_ID), _m_requirement->getValueForKey(REQUIREMENTS_HIERARCHY_PROJECT_ID)) == 0)
                    {
                        tmp_item_exists = false;
                        for (int tmp_index = 0; tmp_item_exists == false && tmp_index < _m_ui->tests_list->rowCount(); tmp_index++)
                        {
                            tmp_item = _m_ui->tests_list->item(tmp_index, 0);
                            tmp_current_test_requirement = (TestRequirement*)tmp_item->data(Qt::UserRole).value<void*>();
                            tmp_item_exists = (tmp_current_test_requirement != NULL
                            && compare_values(
                            tmp_record->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID),
                            tmp_current_test_requirement->getValueForKey(TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID)) == 0);
                        }

                        if (tmp_item_exists == false)
                        {
                            setModified();
                            tmp_test_requirement = new TestRequirement();
                            tmp_test_requirement->setTestShortName(tmp_record->getValueForKey(TESTS_HIERARCHY_SHORT_NAME));
                            tmp_test_requirement->setTestCategory(tmp_record->getValueForKey(TESTS_HIERARCHY_CATEGORY_ID));
                            tmp_test_requirement->setTestVersion(tmp_record->getValueForKey(TESTS_HIERARCHY_CONTENT_VERSION));
                            tmp_test_requirement->setValueForKey(tmp_record->getValueForKey(TESTS_HIERARCHY_TEST_CONTENT_ID), TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID);
                            tmp_test_requirement->setValueForKey(_m_requirement_content->getValueForKey(REQUIREMENTS_CONTENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID), TESTS_REQUIREMENTS_TABLE_ORIGINAL_REQUIREMENT_CONTENT_ID);
                            _m_tests.insert(_m_tests.count(), tmp_test_requirement);
                            _m_ui->tests_list->insertRow(_m_ui->tests_list->rowCount());
                            setTestForRow(tmp_test_requirement, _m_ui->tests_list->rowCount() - 1);
                        }
                    }
                }
            }
        }
    }
}


void Form_Requirement::deletedTestAtIndex(int in_row)
{
    setModified();
    _m_removed_tests.append(_m_tests.takeAt(in_row));
}

void Form_Requirement::setTestForRow(TestRequirement *in_test, int in_row)
{
    QTableWidgetItem    *tmp_first_column_item = NULL;
    QTableWidgetItem    *tmp_second_column_item = NULL;
    QTableWidgetItem    *tmp_third_column_item = NULL;

    if (in_test != NULL)
    {
        // Premiere colonne
        tmp_first_column_item = new QTableWidgetItem;
        tmp_first_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_test));
        tmp_first_column_item->setText(in_test->testShortName());
        tmp_first_column_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        _m_ui->tests_list->setItem(in_row, 0, tmp_first_column_item);

        // Deuxieme colonne
        tmp_second_column_item = new QTableWidgetItem;
        tmp_second_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_test));
        tmp_second_column_item->setText(_m_ui->requirement_category->itemText(_m_ui->requirement_category->findData(in_test->testCategory())));

        tmp_second_column_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        _m_ui->tests_list->setItem(in_row, 1, tmp_second_column_item);

        // TroisiÃ¨me colonne
        tmp_third_column_item = new QTableWidgetItem;
        tmp_third_column_item->setData(Qt::UserRole, QVariant::fromValue<void*>((void*)in_test));
        tmp_third_column_item->setText(ProjectVersion::formatProjectVersionNumber(in_test->testVersion()));
        tmp_third_column_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
        _m_ui->tests_list->setItem(in_row, 2, tmp_third_column_item);

    }
}


void Form_Requirement::showTestAtIndex(QModelIndex in_index)
{
    TestRequirement      		*tmp_test_requirement = NULL;
    QTableWidgetItem            *tmp_column_item = NULL;

    if (in_index.isValid())
    {
        tmp_column_item = _m_ui->tests_list->item(in_index.row(), 0);
        if (tmp_column_item != NULL)
        {
            tmp_test_requirement = (TestRequirement*)tmp_column_item->data(Qt::UserRole).value<void*>();
            if (tmp_test_requirement != NULL)
            {
                emit showTestWithContentId(tmp_test_requirement->getValueForKey(TESTS_REQUIREMENTS_TABLE_TEST_CONTENT_ID));
            }
        }
    }
}


void Form_Requirement::loadPluginsViews()
{
    QMap < QString, ClientModule*>	tmp_modules_map = Session::instance()->externalsModules().value(ClientModule::RequirementPlugin);

    RequirementModule   *tmp_requirement_module = NULL;

    foreach(ClientModule *tmp_module, tmp_modules_map)
    {
        tmp_requirement_module = static_cast<RequirementModule*>(tmp_module);

        tmp_requirement_module->loadRequirementModuleDatas(_m_requirement);

        QWidget	    *tmp_module_view = tmp_requirement_module->createView(this);
        if (tmp_module_view != NULL)
        {
            _m_views_modules_map[tmp_requirement_module] = tmp_module_view;
            _m_ui->tabWidget->addTab(tmp_module_view, tmp_module->getModuleName());
        }
    }
}


void Form_Requirement::savePluginsDatas()
{
    QMap<RequirementModule*, QWidget*>::iterator tmp_module_iterator;

    for(tmp_module_iterator = _m_views_modules_map.begin(); tmp_module_iterator != _m_views_modules_map.end(); tmp_module_iterator++)
    {
        tmp_module_iterator.key()->saveRequirementModuleDatas();
    }
}


void Form_Requirement::destroyPluginsViews()
{
    QMap<RequirementModule*, QWidget*>::iterator tmp_module_iterator;

    for(tmp_module_iterator = _m_views_modules_map.begin(); tmp_module_iterator != _m_views_modules_map.end(); tmp_module_iterator++)
    {
        tmp_module_iterator.key()->destroyView(tmp_module_iterator.value());
    }
}

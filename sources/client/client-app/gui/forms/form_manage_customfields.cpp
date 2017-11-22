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

#include "form_manage_customfields.h"
#include "form_customfielddesc.h"

#include "session.h"

#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QScrollArea>
#include <QSortFilterProxyModel>

#include "ui_Form_Manage_CustomFields.h"

Form_Manage_CustomFields::Form_Manage_CustomFields(const char *fieldEntity, QWidget *parent) :
    QDialog(parent),
    _m_ui(new Ui::Form_Manage_CustomFields),
    _m_field_entity(fieldEntity)
{
  setAttribute(Qt::WA_DeleteOnClose);

    _m_ui->setupUi(this);

    QList<CustomFieldDesc*> fieldsDesc;
    if (compare_values(_m_field_entity, CUSTOM_FIELDS_TEST) == 0)
    {
        fieldsDesc = Session::instance()->customTestsFieldsDesc();
        setWindowTitle(tr("Gestion de champs personnalisés de tests"));
    }
    else
    {
        fieldsDesc = Session::instance()->customRequirementsFieldsDesc();
        setWindowTitle(tr("Gestion de champs personnalisés d'exigences"));
    }

    QList< QPair<QString, QString> > in_headers;
    in_headers << QPair<QString, QString>(tr("Libellé"), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_LABEL);
    in_headers << QPair<QString, QString>(tr("Défaut"), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_DEFAULT_VALUE);
    in_headers << QPair<QString, QString>(tr("Obligatoire"), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_MANDATORY);
    in_headers << QPair<QString, QString>(tr("Onglet"), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME);

    _m_custom_fields_model = new RecordsTableModel<CustomFieldDesc>(in_headers, fieldsDesc);

    // Create tabs
    foreach (CustomFieldDesc* customField, fieldsDesc)
    {
        bool tabNameExists = false;
        for(int tmp_index = 0; tmp_index < _m_ui->tabWidget->count(); ++tmp_index)
        {
            if (compare_values(_m_ui->tabWidget->tabText(tmp_index).toStdString().c_str(), customField->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME)) == 0){
                tabNameExists = true;
                break;
            }
        }

        if (!tabNameExists)
            addTab(customField->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME));
    }

    connect(_m_ui->pushButtonAddTab, SIGNAL(clicked()), this, SLOT(addTab()));
    connect(_m_ui->pushButtonRemoveTab, SIGNAL(clicked()), this, SLOT(removeSelectedTab()));
}

Form_Manage_CustomFields::~Form_Manage_CustomFields()
{
    delete _m_ui;
    delete _m_custom_fields_model;

    // reload custom fields
    Session::instance()->loadCustomFields();

}

void Form_Manage_CustomFields::addTab()
{
    bool tabNameExists = true;
    int index = _m_ui->tabWidget->count();
    QString tabName(tr("Onglet %1").arg(index));

    do {
        tabNameExists = false;
        for(int tmp_index = 0; tmp_index < _m_ui->tabWidget->count(); ++tmp_index)
        {
            if (_m_ui->tabWidget->tabText(tmp_index) == tabName){
                tabNameExists = true;
                ++index;
                tabName = tr("Onglet %1").arg(index);
                break;
            }
        }
    }
    while(tabNameExists);

    addTab(tabName);
}

void Form_Manage_CustomFields::addTab(QString tabName)
{
    QWidget *tab = new QWidget(_m_ui->tabWidget);
    QGridLayout *gridLayout_4 = new QGridLayout(tab);

    //--------------------------------
    // Layout pour renommer l'onglet
    QHBoxLayout *horizontalLayout = new QHBoxLayout();
    QLabel *label = new QLabel(tab);
    QLineEdit *lineEdit = new QLineEdit(tab);
    QPushButton *pushButtonRename = new QPushButton(tab);

    label->setText(tr("Nom de l'onglet :"));
    lineEdit->setText(tabName);
    pushButtonRename->setText(tr("Renommer"));


    connect(pushButtonRename, SIGNAL(clicked()), this, SLOT(renameSelectedTab()));

    horizontalLayout->addWidget(label);
    horizontalLayout->addWidget(lineEdit);
    horizontalLayout->addWidget(pushButtonRename);

    gridLayout_4->addLayout(horizontalLayout, 0, 0, 1, 1);
    //--------------------------------

    //--------------------------------
    // Ajout de la widget contenant les champs personnalisés
    QTableView *tableView = new QTableView(tab);

    tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableView->verticalHeader()->setVisible(false);
    tableView->verticalHeader()->setDefaultSectionSize(18);
    tableView->horizontalHeader()->setVisible(true);
    tableView->horizontalHeader()->setMinimumSectionSize(50);
    tableView->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    tableView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);

    // add a second filter to keep only custom fields with corresponding tab name
    QSortFilterProxyModel *proxyModel = new QSortFilterProxyModel(tableView);
    proxyModel->setSourceModel(_m_custom_fields_model);
    proxyModel->setFilterFixedString(tabName);
    proxyModel->setFilterKeyColumn(3);

    tableView->setColumnHidden(3, true);

    tableView->setModel(proxyModel);
    connect(tableView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(editField()));

    _m_custom_fields_model_proxy[tableView] = proxyModel;

    gridLayout_4->addWidget(tableView, 1, 0, 1, 1);

    _m_rename_tab_button_map[pushButtonRename] = QPair<QLineEdit*, QTableView*>(lineEdit, tableView);
    //--------------------------------

    //--------------------------------
    // Layout pour ajouter un champ
    QHBoxLayout *horizontalLayout2 = new QHBoxLayout();
    QPushButton *pushButtonAddField = new QPushButton(tab);
    QPushButton *pushButtonDelField = new QPushButton(tab);

    pushButtonAddField->setText(tr("Ajouter un champ"));
    pushButtonDelField->setText(tr("Supprimer le champ sélectionné"));

    _m_add_field_buttons_map[pushButtonAddField] = tableView;
    _m_remove_field_buttons_map[pushButtonDelField] = tableView;

    connect(pushButtonAddField, SIGNAL(clicked()), this, SLOT(addField()));
    connect(pushButtonDelField, SIGNAL(clicked()), this, SLOT(removeField()));

    horizontalLayout2->addWidget(pushButtonAddField);
    horizontalLayout2->addWidget(pushButtonDelField);

    gridLayout_4->addLayout(horizontalLayout2, 2, 0, 1, 1);
    //--------------------------------


    _m_ui->tabWidget->addTab(tab, tabName);
}


void Form_Manage_CustomFields::removeSelectedTab()
{
    if (_m_ui->tabWidget->currentIndex() >= 0)
    {
        QString tmp_tab_name = _m_ui->tabWidget->tabText(_m_ui->tabWidget->currentIndex());
        if (QMessageBox::question(this, tr("Confirmation..."), tr("Etes-vous sûr(e) de vouloir supprimer l'onglet <b>%1</b> ?").arg(tmp_tab_name),QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes)
        {
            QList<CustomFieldDesc*> list = _m_custom_fields_model->getRecordsList();
            foreach (CustomFieldDesc *customFieldDesc, list)
            {
                if (compare_values(_m_field_entity, customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_ENTITY)) == 0
                    && compare_values(tmp_tab_name.toStdString().c_str(), customFieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME)) == 0)
                {
                    _m_custom_fields_model->removeRecord(customFieldDesc);
                }
            }

            _m_ui->tabWidget->removeTab(_m_ui->tabWidget->currentIndex());
        }
    }
}


void Form_Manage_CustomFields::addField()
{
    QPushButton *button = dynamic_cast<QPushButton*>(sender());
    Form_CustomFieldDesc *tmp_form_custom_field = NULL;

    if (button)
    {
        CustomFieldDesc *customField = new CustomFieldDesc();
        customField->setValueForKey(tr("Nouveau champ").toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_LABEL);
        customField->setValueForKey(_m_field_entity, CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_ENTITY);
        customField->setValueForKey(CUSTOM_FIELDS_PLAIN_TEXT, CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TYPE);
        customField->setValueForKey(_m_ui->tabWidget->tabText(_m_ui->tabWidget->currentIndex()).toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME);

        tmp_form_custom_field = new Form_CustomFieldDesc(customField);

        connect(tmp_form_custom_field, SIGNAL(saved(CustomFieldDesc*)), this, SLOT(acceptAddedField(CustomFieldDesc*)));
        connect(tmp_form_custom_field, SIGNAL(unsaved(CustomFieldDesc*)), this, SLOT(rejectAddedField(CustomFieldDesc*)));

        tmp_form_custom_field->show();
    }
}

void Form_Manage_CustomFields::removeField()
{
    QPushButton *button = dynamic_cast<QPushButton*>(sender());
    QSortFilterProxyModel* proxy = NULL;

    if (button)
    {
        QTableView* tableView = _m_remove_field_buttons_map[button];

        proxy = _m_custom_fields_model_proxy[tableView];
        QModelIndexList selection = tableView->selectionModel()->selectedRows();
        foreach(QModelIndex index, selection)
        {
            QModelIndex sourceIndex = proxy->mapToSource(index);
            if (sourceIndex.isValid())
            {
                CustomFieldDesc* customField = _m_custom_fields_model->recordAtIndex(sourceIndex);
                if (customField)
                {
                    if (QMessageBox::question(
                        this,
                        tr("Confirmation..."),
                        tr("Etes-vous sûr(e) de vouloir supprimer le champ <b>%1</b> ?").arg(customField->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_LABEL))
                        ,QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes)
                    _m_custom_fields_model->removeRow(sourceIndex.row());
                }
            }
        }
    }
}


void Form_Manage_CustomFields::editField()
{
    QTableView *tableView = dynamic_cast<QTableView*>(sender());
    if (tableView)
    {
        QSortFilterProxyModel* proxy = _m_custom_fields_model_proxy[tableView];

        QModelIndex index = tableView->selectionModel()->currentIndex();
        if (index.isValid())
        {
            QModelIndex sourceIndex = proxy->mapToSource(index);
            if (sourceIndex.isValid())
            {
                CustomFieldDesc* customField = _m_custom_fields_model->recordAtIndex(sourceIndex);
                if (customField != NULL)
                {
                    Form_CustomFieldDesc* tmp_form_custom_field = new Form_CustomFieldDesc(customField);

                    tmp_form_custom_field->show();
                }
            }
        }
    }
}


void Form_Manage_CustomFields::acceptAddedField(CustomFieldDesc* customField)
{
    _m_custom_fields_model->addRecord(customField);
}


void Form_Manage_CustomFields::rejectAddedField(CustomFieldDesc* customField)
{
    delete customField;
}


void Form_Manage_CustomFields::renameSelectedTab()
{
    QPushButton *button = dynamic_cast<QPushButton*>(sender());
    QSortFilterProxyModel* proxy = NULL;

    if (button)
    {
        QLineEdit* lineEdit = _m_rename_tab_button_map[button].first;
        QTableView* tableView = _m_rename_tab_button_map[button].second;

        for (int tabIndex = 0; tabIndex < _m_ui->tabWidget->count(); ++tabIndex)
        {
            // Il existe déjà un onglet portant le nom saisi
            if (_m_ui->tabWidget->currentIndex() != tabIndex && _m_ui->tabWidget->tabText(tabIndex) == lineEdit->text())
            {
                QMessageBox::warning(this, tr("Onglet déjà existant"), tr("Il existe déjà un onglet nommé [%1].").arg(lineEdit->text()));
                return;
            }
        }


        // Mettre à jour les champs personnalisés associés
        foreach (CustomFieldDesc* fieldDesc, _m_custom_fields_model->getRecordsList())
        {
            if (compare_values(_m_field_entity, fieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_ENTITY)) == 0
                && compare_values(_m_ui->tabWidget->tabText(_m_ui->tabWidget->currentIndex()).toStdString().c_str(), fieldDesc->getValueForKey(CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME)) == 0)
            {
                fieldDesc->setValueForKey(lineEdit->text().toStdString().c_str(), CUSTOM_FIELDS_DESC_TABLE_CUSTOM_FIELD_DESC_TAB_NAME);
            }
        }

        _m_ui->tabWidget->setTabText(_m_ui->tabWidget->currentIndex(), lineEdit->text());

        proxy = _m_custom_fields_model_proxy[tableView];
        proxy->setFilterFixedString(lineEdit->text());
    }
}



void Form_Manage_CustomFields::accept()
{
    if (_m_custom_fields_model->submit())
    {
        QDialog::accept();
    }
    else
    {
        QMessageBox::critical(this, tr("Erreur d'enregistrement"), Session::instance()->getLastErrorMessage());
    }
}

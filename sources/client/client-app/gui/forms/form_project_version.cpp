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

#include "form_project_version.h"
#include "ui_Form_Project_Version.h"

#include "client.h"
#include "constants.h"
#include "session.h"

#include <QtGui>

#define QLINEEDIT_WRITABLE "QLineEdit {background: #FFFFFF;border: 1px solid #4080b2;}"
#define QLINEEDIT_UNWRITABLE "QLineEdit {background: #FFFFFF;dfe0e8: color: gray;}"


/**
  Constructeur
**/
Form_Project_Version::Form_Project_Version(ProjectVersion* in_project_record, QWidget *parent) : AbstractProjectWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    _m_project_record = in_project_record;
    _m_modifiable = false;
    _m_modified = false;
    _m_bugtracker = NULL;

    _m_ui = new  Ui_Form_Project_Version();

    _m_ui->setupUi(this);
    _m_ui->title->setText(QString(_m_project_record->project()->getValueForKey(PROJECTS_TABLE_SHORT_NAME))
                          + QString(" - VERSION ")
                          + ProjectVersion::formatProjectVersionNumber(_m_project_record->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION)));
    _m_ui->project_description->addTextToolBar(RecordTextEditToolBar::Small);
    _m_ui->project_version_info->addTextToolBar(RecordTextEditToolBar::Small);
    _m_ui->project_name->setFocus();

    foreach(ClientModule *tmp_bugtracker_module, Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin))
        _m_ui->bug_tracker_type->addItem(tmp_bugtracker_module->getModuleName());

    setWindowTitle(_m_project_record->project()->getValueForKey(PROJECTS_TABLE_SHORT_NAME));
    _m_ui->project_name->setText(_m_project_record->project()->getValueForKey(PROJECTS_TABLE_SHORT_NAME));
    _m_ui->project_version->setText(ProjectVersion::formatProjectVersionNumber(_m_project_record->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION)));

    initLayout();

    connect(_m_ui->project_name, SIGNAL(textChanged(const QString &)), this, SLOT(setModified()));
    connect(_m_ui->project_description->textEditor(), SIGNAL(textChanged()), this, SLOT(setModified()));
    connect(_m_ui->project_version_info->textEditor(), SIGNAL(textChanged()), this, SLOT(setModified()));
    connect(_m_ui->bug_tracker_type, SIGNAL(currentIndexChanged(int)), this, SLOT(setModified()));
    connect(_m_ui->bug_tracker_host, SIGNAL(textChanged(const QString &)), this, SLOT(setModified()));
    connect(_m_ui->bug_tracker_url, SIGNAL(textChanged(const QString &)), this, SLOT(setModified()));
    connect(_m_ui->bug_tracker_project_id, SIGNAL(textChanged(const QString &)), this, SLOT(setModified()));
    connect(_m_ui->bug_tracker_project_version, SIGNAL(textChanged(const QString &)), this, SLOT(setModified()));

    connect(_m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(save()));
    connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(cancel()));

    connect(_m_ui->bugtracker_test_button, SIGNAL(clicked()), this, SLOT(checkBugtrackerConnexion()));

    updateControls();

    loadPluginsViews();
}



/**
  Destruction
**/
Form_Project_Version::~Form_Project_Version()
{
    destroyPluginsViews();

    if (_m_bugtracker != NULL)
    {
        ClientModule *tmp_bugtracker_module = Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).value(_m_project_record->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE));
        if (tmp_bugtracker_module != NULL)
        {
            static_cast<BugtrackerModule*>(tmp_bugtracker_module)->destroyBugtracker(_m_bugtracker);
        }
    }

    delete _m_ui;
}


/**
  Validation par le bouton OK
**/
bool Form_Project_Version::saveProject()
{
    int	    tmp_save_result = NOERR;

    _m_project_record->project()->setValueForKey(_m_ui->project_name->text().toStdString().c_str(), PROJECTS_TABLE_SHORT_NAME);
    _m_project_record->project()->setValueForKey(_m_ui->project_description->textEditor()->toHtml().toStdString().c_str(), PROJECTS_TABLE_DESCRIPTION);
    tmp_save_result = _m_project_record->project()->saveRecord();
    if (tmp_save_result == NOERR)
    {
        _m_project_record->setValueForKey(_m_ui->project_version_info->textEditor()->toHtml().toStdString().c_str(), PROJECTS_VERSIONS_TABLE_DESCRIPTION);

        if (_m_ui->bug_tracker_type->currentIndex() >= 0)
            _m_project_record->setValueForKey(_m_ui->bug_tracker_type->currentText().toStdString().c_str(), PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE);

        _m_project_record->setValueForKey(_m_ui->bug_tracker_host->text().toStdString().c_str(), PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST);
        _m_project_record->setValueForKey(_m_ui->bug_tracker_url->text().toStdString().c_str(), PROJECTS_VERSIONS_TABLE_BUG_TRACKER_URL);
        _m_project_record->setValueForKey(_m_ui->bug_tracker_project_id->text().toStdString().c_str(), PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_ID);
        _m_project_record->setValueForKey(_m_ui->bug_tracker_project_version->text().toStdString().c_str(), PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_VERSION);

        tmp_save_result = _m_project_record->saveRecord();
        if (tmp_save_result == NOERR)
        {
            _m_modified = false;
            updateControls();
            return true;
        }
    }

    QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_save_result));
    return false;
}



void Form_Project_Version::setModified()
{
    _m_modified = true;
}


void Form_Project_Version::initLayout()
{
    _m_ui->project_name->setText(_m_project_record->project()->getValueForKey(PROJECTS_TABLE_SHORT_NAME));
    _m_ui->project_description->textEditor()->setHtml(_m_project_record->project()->getValueForKey(PROJECTS_TABLE_DESCRIPTION));
    _m_ui->project_version_info->textEditor()->setHtml(_m_project_record->getValueForKey(PROJECTS_VERSIONS_TABLE_DESCRIPTION));

    int tmp_bt_index = _m_ui->bug_tracker_type->findText(_m_project_record->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE));
    if (tmp_bt_index >= 0)
    {
        _m_ui->bug_tracker_type->setCurrentIndex(tmp_bt_index);
    }

    _m_ui->bug_tracker_host->setText(_m_project_record->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST));
    _m_ui->bug_tracker_url->setText(_m_project_record->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_URL));
    _m_ui->bug_tracker_project_id->setText(_m_project_record->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_ID));
    _m_ui->bug_tracker_project_version->setText(_m_project_record->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_VERSION));

    if (compare_values(_m_project_record->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_TESTS_INDIC), PROJECT_GRANT_WRITE) != 0
            || compare_values(_m_project_record->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_REQUIREMENTS_INDIC), PROJECT_GRANT_WRITE) != 0
            || compare_values(_m_project_record->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_CAMPAIGNS_INDIC), PROJECT_GRANT_WRITE) != 0
            || compare_values(_m_project_record->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC), PROJECT_GRANT_WRITE) != 0)
    {
        _m_modifiable = false;
    }
    else
    {
        _m_project_record->lockRecord(true);
        if (_m_project_record->lockRecordStatus() == RECORD_STATUS_LOCKED)
        {
            _m_ui->lock_widget->setVisible(true);
            net_get_field(NET_MESSAGE_TYPE_INDEX+1, Session::instance()->getClientSession()->m_response, Session::instance()->getClientSession()->m_column_buffer, SEPARATOR_CHAR);
            _m_ui->label_lock_by->setText(tr("Verrouillé par ") + QString(Session::instance()->getClientSession()->m_column_buffer));
            _m_modifiable = false;
        }
        else
            _m_modifiable = true;
    }

}


void Form_Project_Version::updateControls()
{
    _m_ui->lock_widget->setVisible(_m_project_record->lockRecordStatus() == RECORD_STATUS_LOCKED);
    _m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(_m_modifiable);

    _m_ui->project_name->setReadOnly(!_m_modifiable);
    _m_ui->project_description->textEditor()->setReadOnly(!_m_modifiable);
    _m_ui->project_version_info->textEditor()->setReadOnly(!_m_modifiable);

    _m_ui->bug_tracker_type->setEnabled(_m_modifiable);
    _m_ui->bug_tracker_host->setReadOnly(!_m_modifiable);
    _m_ui->bug_tracker_url->setReadOnly(!_m_modifiable);
    _m_ui->bug_tracker_project_id->setReadOnly(!_m_modifiable);
    _m_ui->bug_tracker_project_version->setReadOnly(!_m_modifiable);

    _m_ui->bugtracker_test_button->setEnabled(_m_modifiable);

    if (_m_modifiable == false)
    {
        _m_ui->project_name->setStyleSheet(QLINEEDIT_UNWRITABLE);
        _m_ui->project_description->toolBar()->hide();
        _m_ui->project_version_info->toolBar()->hide();

        _m_ui->project_description->textEditor()->setStyleSheet(QTEXTEDIT_UNWRITABLE);
        _m_ui->project_version_info->textEditor()->setStyleSheet(QTEXTEDIT_UNWRITABLE);

        _m_ui->bug_tracker_host->setStyleSheet(QLINEEDIT_UNWRITABLE);
        _m_ui->bug_tracker_url->setStyleSheet(QLINEEDIT_UNWRITABLE);
        _m_ui->bug_tracker_project_id->setStyleSheet(QLINEEDIT_UNWRITABLE);
        _m_ui->bug_tracker_project_version->setStyleSheet(QLINEEDIT_UNWRITABLE);
    }
    else
    {
        _m_ui->project_name->setStyleSheet(QLINEEDIT_WRITABLE);
        _m_ui->project_description->toolBar()->show();
        _m_ui->project_version_info->toolBar()->show();

        _m_ui->project_description->textEditor()->setStyleSheet(QTEXTEDIT_WRITABLE);
        _m_ui->project_version_info->textEditor()->setStyleSheet(QTEXTEDIT_WRITABLE);

        _m_ui->bug_tracker_host->setStyleSheet(QLINEEDIT_WRITABLE);
        _m_ui->bug_tracker_url->setStyleSheet(QLINEEDIT_WRITABLE);
        _m_ui->bug_tracker_project_id->setStyleSheet(QLINEEDIT_WRITABLE);
        _m_ui->bug_tracker_project_version->setStyleSheet(QLINEEDIT_WRITABLE);
    }
}


void Form_Project_Version::closeEvent(QCloseEvent *in_event)
{
    if (maybeClose())
        in_event->accept();
    else
        in_event->ignore();
}


bool Form_Project_Version::maybeClose()
{
    int		tmp_confirm_choice = 0;
    bool    tmp_return = true;

    if (_m_project_record != NULL)
    {
        if (_m_modified)
        {
            tmp_confirm_choice = QMessageBox::question(
                        this,
                        tr("Confirmation..."),
                        tr("Le projet a été modifié. Voulez-vous enregistrer les modifications ?"),
                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                        QMessageBox::Cancel);

            if (tmp_confirm_choice == QMessageBox::Yes)
                tmp_return = saveProject();
            else if (tmp_confirm_choice == QMessageBox::Cancel)
                tmp_return = false;
        }

        if (tmp_return)
            _m_project_record->unlockRecord();
    }

    return tmp_return;
}


void Form_Project_Version::save()
{
    if (saveProject())
    {
        savePluginsDatas();

        emit projectSaved();
    }
}



void Form_Project_Version::checkBugtrackerConnexion()
{
    QString tmp_base_url = _m_ui->bug_tracker_host->text();
    QString tmp_url = _m_ui->bug_tracker_url->text();

    if (tmp_base_url.isEmpty() && tmp_url.isEmpty())
    {
        QMessageBox::critical(this, tr("Paramètre manquant"), tr("L'URL de base ou l'URL du Webservice est indispensable pour tester la connexion."));
    }
    else
    {
        if (_m_ui->bug_tracker_type->currentIndex() >= 0)
        {
            ClientModule *tmp_bugtracker_module = Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).value(_m_ui->bug_tracker_type->currentText());
            if (tmp_bugtracker_module != NULL)
            {
                if (_m_bugtracker != NULL)
                    static_cast<BugtrackerModule*>(tmp_bugtracker_module)->destroyBugtracker(_m_bugtracker);

                _m_bugtracker = static_cast<BugtrackerModule*>(tmp_bugtracker_module)->createBugtracker();

                if (_m_bugtracker != NULL)
                {
                    if (!tmp_base_url.isEmpty())
                        _m_bugtracker->setBaseUrl(tmp_base_url);

                    if (!tmp_url.isEmpty())
                        _m_bugtracker->setWebserviceUrl(tmp_url);

                    connect(_m_bugtracker, SIGNAL(version(QString)), this, SLOT(getBugtrackerVersion(QString)));
                    connect(_m_bugtracker, SIGNAL(error(QString)), this, SLOT(getBugtrackerError(QString)));

                    _m_bugtracker->getVersion();
                }
            }
        }
    }
}


void Form_Project_Version::getBugtrackerVersion(QString version)
{
    QMessageBox::information(this, _m_ui->bug_tracker_url->text(), tr("La version du bugtracker est : %1").arg("<b>" + version + "</b>") + "<p><span style=\"color: green\"><b>" + tr("La connexion est correcte.") + "</b></span></p>");
}


void Form_Project_Version::getBugtrackerError(QString errorMsg)
{
    QMessageBox::critical(this, _m_ui->bug_tracker_url->text(),  errorMsg+"<p><span style=\"color: red\"><b>"+tr("La requête a échouée.")+"</b></span><p>");
}



void Form_Project_Version::loadPluginsViews()
{
    QMap < QString, ClientModule*>	tmp_modules_map = Session::instance()->externalsModules().value(ClientModule::ProjectPlugin);

    ProjectModule   *tmp_project_module = NULL;

    foreach(ClientModule *tmp_module, tmp_modules_map)
    {
        tmp_project_module = static_cast<ProjectModule*>(tmp_module);

        tmp_project_module->loadProjectModuleDatas(_m_project_record);

        QWidget	    *tmp_module_view = tmp_project_module->createView(this);
        if (tmp_module_view != NULL)
        {
            _m_views_modules_map[tmp_project_module] = tmp_module_view;
            _m_ui->tool_box->addItem(tmp_module_view, tmp_module->getModuleName());
        }
    }
}


void Form_Project_Version::savePluginsDatas()
{
    QMap<ProjectModule*, QWidget*>::iterator tmp_module_iterator;

    for(tmp_module_iterator = _m_views_modules_map.begin(); tmp_module_iterator != _m_views_modules_map.end(); tmp_module_iterator++)
    {
        tmp_module_iterator.key()->saveProjectModuleDatas();
    }
}


void Form_Project_Version::destroyPluginsViews()
{
    QMap<ProjectModule*, QWidget*>::iterator tmp_module_iterator;

    for(tmp_module_iterator = _m_views_modules_map.begin(); tmp_module_iterator != _m_views_modules_map.end(); tmp_module_iterator++)
    {
        tmp_module_iterator.key()->destroyView(tmp_module_iterator.value());
    }
}

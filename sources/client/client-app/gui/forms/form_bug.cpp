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

#include "form_bug.h"
#include "session.h"
#include "gui/components/record_text_edit.h"
#include "form_execution_campaign.h"

#include "bug.h"
#include "executiontestparameter.h"

#include <QMessageBox>


Form_Bug::Form_Bug(ProjectVersion *in_project_version, ExecutionTest *in_execution_test, ExecutionAction *in_execution_action, Bug *in_bug, QWidget *parent, Qt::WindowFlags in_windows_flags) 
    : AbstractProjectWidget(parent, in_windows_flags)
{
    QString tmp_base_url;
    QString tmp_url;

    _m_bugtracker = NULL;
    _m_bugtracker_connection_active = false;

    setAttribute(Qt::WA_DeleteOnClose);

    _m_ui = new  Ui_Form_Bug;

    _m_ui->setupUi(this);

    _m_project_version = in_project_version;
    _m_execution_test = in_execution_test;
    _m_execution_action = in_execution_action;
    _m_bug = in_bug;

    _m_new_bug = (_m_bug == NULL);
    if (_m_new_bug)
	_m_bug = new Bug();

    if (_m_project_version == NULL)
    {
	if (_m_execution_test != NULL && _m_execution_test->projectTest() != NULL)
	{
	    _m_project_version = _m_execution_test->projectTest()->projectVersion();
	}
    }


    if (_m_project_version != NULL)
    {
    tmp_base_url = _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST);
    tmp_url = _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_URL);

	if (tmp_base_url.isEmpty() && tmp_url.isEmpty())
	{
	    _m_ui->priorite->addItem("P1");
	    _m_ui->priorite->addItem("P2");
	    _m_ui->priorite->addItem("P3");
	    _m_ui->priorite->addItem("P4");
	    _m_ui->priorite->addItem("P5");

	    _m_ui->gravite->addItem(TR_CUSTOM_MESSAGE("Bloquant"));
	    _m_ui->gravite->addItem(TR_CUSTOM_MESSAGE("Critique"));
	    _m_ui->gravite->addItem(TR_CUSTOM_MESSAGE("Majeur"));
	    _m_ui->gravite->addItem(TR_CUSTOM_MESSAGE("Normal"));
	    _m_ui->gravite->addItem(TR_CUSTOM_MESSAGE("Mineur"));
	    _m_ui->gravite->addItem(TR_CUSTOM_MESSAGE("Banal"));
	    _m_ui->gravite->addItem(TR_CUSTOM_MESSAGE("Evolution"));

	    _m_ui->plateforme->addItem(TR_CUSTOM_MESSAGE("Toutes"));
	    _m_ui->plateforme->addItem(TR_CUSTOM_MESSAGE("PC"));
	    _m_ui->plateforme->addItem(TR_CUSTOM_MESSAGE("Macintosh"));
	    _m_ui->plateforme->addItem(TR_CUSTOM_MESSAGE("Autre"));

	    _m_ui->systeme->addItem(TR_CUSTOM_MESSAGE("Tous"));
	    _m_ui->systeme->addItem(TR_CUSTOM_MESSAGE("Windows"));
	    _m_ui->systeme->addItem(TR_CUSTOM_MESSAGE("Mac OS"));
	    _m_ui->systeme->addItem(TR_CUSTOM_MESSAGE("Linux"));
	    _m_ui->systeme->addItem(TR_CUSTOM_MESSAGE("UNIX"));
	    _m_ui->systeme->addItem(TR_CUSTOM_MESSAGE("Autre"));
	}
	else
	{
	    if (!tmp_base_url.isEmpty())
	    	_m_ui->bugtracker_url->setText("<a href=\"" + tmp_base_url + "\">" + tmp_base_url + "</a>");
	    else if (!tmp_url.isEmpty())
	    	_m_ui->bugtracker_url->setText("<a href=\"" + tmp_url + "\">" + tmp_url + "</a>");

        ClientModule *tmp_bugtracker_module = Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).value(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE));
        if (tmp_bugtracker_module != NULL)
        {
        	_m_bugtracker = static_cast<BugtrackerModule*>(tmp_bugtracker_module)->createBugtracker();

			if (_m_bugtracker != NULL)
			{
				if (!tmp_base_url.isEmpty())
				_m_bugtracker->setBaseUrl(tmp_base_url);

				if (!tmp_url.isEmpty())
				_m_bugtracker->setWebserviceUrl(tmp_url);

                _m_ui->username->setText(Session::instance()->getBugtrackersCredentials()[tmp_bugtracker_module->getModuleName()].first);
                _m_ui->password->setText(Session::instance()->getBugtrackersCredentials()[tmp_bugtracker_module->getModuleName()].second);

				connect(_m_ui->connect_button, SIGNAL(clicked()), this, SLOT(loginBugtracker()));
				connect(_m_bugtracker, SIGNAL(error(QString)), this, SLOT(bugtrackerError(QString)));
			}
        }
	}

        _m_ui->bug_status->addItem(TR_CUSTOM_MESSAGE("Ouvert"), BUG_STATUS_OPENED);
        _m_ui->bug_status->addItem(TR_CUSTOM_MESSAGE("Fermé"), BUG_STATUS_CLOSED);

	connect(_m_ui->buttonBox->button(QDialogButtonBox::Save), SIGNAL(clicked()), this, SLOT(save()));
	connect(_m_ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(cancel()));

	connect(_m_ui->execution_button, SIGNAL(clicked()), this, SLOT(openExecutionCampaign()));

	updateControls();
    }
}

Form_Bug::~Form_Bug()
{
    if (_m_bugtracker != NULL)
    {
    ClientModule *tmp_bugtracker_module = Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).value(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_TYPE));
	if (tmp_bugtracker_module != NULL)
	{
		static_cast<BugtrackerModule*>(tmp_bugtracker_module)->destroyBugtracker(_m_bugtracker);
	}
    }

    if(_m_new_bug)
	delete _m_bug;

    delete _m_ui;
}


void Form_Bug::save()
{
    bool	tmp_has_bugtracker_id = is_empty_string(_m_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID)) == FALSE;

    if (_m_ui->resume->text().isEmpty() || _m_ui->description->toPlainText().isEmpty())
    {
	QMessageBox::critical(this, tr("Données non saisies"), tr("Le résumé de l'anomalie ainsi que sa description sont nécessaires."));
    }
    else
    {
	if (_m_bugtracker == NULL)
	{
	    saveBug(QString());
	}
	else
	{
            if (tmp_has_bugtracker_id)
            {
                int tmp_save_result = _m_bug->saveRecord();
                if (tmp_save_result != NOERR)
                {
                    QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_save_result));
                }
                else
                {
                    emit bugSaved();
                    close();
                }
            }
            else
                addBugtrackerIssue();
	}
    }
}


void Form_Bug::loginBugtracker()
{
    _m_bugtracker->setCredential(_m_ui->username->text(), _m_ui->password->text());
    connect(_m_bugtracker, SIGNAL(loginSignal()), this, SLOT(bugtrackerLogin()));

    _m_bugtracker->login();

    if (_m_bugtracker_connection_active)
    {
    }
    else
    {
    }
}

void Form_Bug::logoutBugtracker()
{
    connect(_m_bugtracker, SIGNAL(logoutSignal()), this, SLOT(bugtrackerLogout()));

    _m_bugtracker->logout();
}


void Form_Bug::loadBugtrackerProjectsInformations(QList<QString> in_projects_ids)
{
    connect(_m_bugtracker, SIGNAL(projectsInformations(QList<QPair<QString,QString> >)), this, SLOT(bugtrackerProjectsInformations(QList<QPair<QString,QString> >)));

    _m_bugtracker->getProjectsInformations(in_projects_ids);
}


void Form_Bug::loadBugtrackerPriorities()
{
    connect(_m_bugtracker, SIGNAL(priorities(QMap<QString, QString>)), this, SLOT(bugtrackerPriorities(QMap<QString, QString>)));

    _m_bugtracker->getPriorities();
}


void Form_Bug::loadBugtrackerSeverities()
{
    connect(_m_bugtracker, SIGNAL(severities(QMap<QString, QString>)), this, SLOT(bugtrackerSeverities(QMap<QString, QString>)));

    _m_bugtracker->getSeverities();
}


void Form_Bug::loadBugtrackerReproducibilities()
{
    connect(_m_bugtracker, SIGNAL(reproducibilities(QMap<QString, QString>)), this, SLOT(bugtrackerReproducibilities(QMap<QString, QString>)));

    _m_bugtracker->getReproducibilities();
}


void Form_Bug::loadBugtrackerProjectComponents()
{
    connect(_m_bugtracker, SIGNAL(projectComponents(QList<QString>)), this, SLOT(bugtrackerProjectComponents(QList<QString>)));

    _m_bugtracker->getProjectComponents(_m_project_id);
}


void Form_Bug::loadBugtrackerProjectVersions()
{
    connect(_m_bugtracker, SIGNAL(projectVersions(QList<QString>)), this, SLOT(bugtrackerProjectVersions(QList<QString>)));

    _m_bugtracker->getProjectVersions(_m_project_id);
}


void Form_Bug::loadBugtrackerPlatforms()
{
    connect(_m_bugtracker, SIGNAL(platforms(QList<QString>)), this, SLOT(bugtrackerPlatforms(QList<QString>)));

    _m_bugtracker->getPlatforms();
}


void Form_Bug::loadBugtrackerOsTypes()
{
    connect(_m_bugtracker, SIGNAL(operatingSystems(QList<QString>)), this, SLOT(bugtrackerOperatingSystems(QList<QString>)));

    _m_bugtracker->getOperatingSystems();
}


void Form_Bug::loadBugtrackerAvailableProjectsIds()
{
    connect(_m_bugtracker, SIGNAL(availableProjectsIds(QList<QString>)), this, SLOT(bugtrackerAvailableProjectsIds(QList<QString>)));

    _m_bugtracker->getAvailableProjectsIds();
}


void Form_Bug::addBugtrackerIssue()
{
    connect(_m_bugtracker, SIGNAL(bugCreated(QString)), this, SLOT(bugtrackerBugCreated(QString)));

    _m_bugtracker->addBug(_m_project_id,
                          _m_project_name,
			  _m_ui->composant->itemText(_m_ui->composant->currentIndex()),
			  _m_ui->version->itemText(_m_ui->version->currentIndex()),
              _m_execution_test->executionCampaign()->getValueForKey(EXECUTIONS_CAMPAIGNS_TABLE_REVISION),
			  _m_ui->plateforme->itemText(_m_ui->plateforme->currentIndex()),
			  _m_ui->systeme->itemText(_m_ui->systeme->currentIndex()),
			  _m_ui->resume->text(),
			  _m_ui->description->toPlainText(),
			  _m_ui->priorite->itemData(_m_ui->priorite->currentIndex()),
			  _m_ui->gravite->itemData(_m_ui->gravite->currentIndex()),
			  _m_ui->reproductibilite->itemData(_m_ui->reproductibilite->currentIndex())
			  );
}


/* Traitement des réponses du bugtracker */
void Form_Bug::bugtrackerLogin()
{
    bool	tmp_has_bugtracker_id = is_empty_string(_m_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID)) == FALSE;

    disconnect(_m_bugtracker, SIGNAL(loginSignal()), this, SLOT(bugtrackerLogin()));

    _m_bugtracker_connection_active = true;
    if (tmp_has_bugtracker_id)
    {
	connect(_m_bugtracker, SIGNAL(bugsInformations(QMap< QString, QMap<Bugtracker::BugProperty, QString> >)), this, SLOT(updateBugFromBugtracker(QMap< QString, QMap<Bugtracker::BugProperty, QString> >)));
        _m_bugtracker->getBug(_m_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID));
    }
    else
        loadBugtrackerPriorities();
}


void Form_Bug::bugtrackerLogout()
{
    disconnect(_m_bugtracker, SIGNAL(logoutSignal()), this, SLOT(bugtrackerLogout()));

    _m_bugtracker_connection_active = false;
}



void Form_Bug::bugtrackerPriorities(QMap<QString, QString> priorities)
{
    disconnect(_m_bugtracker, SIGNAL(priorities(QMap<QString, QString>)), this, SLOT(bugtrackerPriorities(QMap<QString, QString>)));

    _m_ui->priorite->clear();
    foreach(QString priority_id, priorities.keys())
    {
	_m_ui->priorite->addItem(priorities.value(priority_id), priority_id);
    }

    loadBugtrackerSeverities();
}


void Form_Bug::bugtrackerSeverities(QMap<QString, QString> severities)
{
    disconnect(_m_bugtracker, SIGNAL(severities(QMap<QString, QString>)), this, SLOT(bugtrackerSeverities(QMap<QString, QString>)));

    _m_ui->gravite->clear();
    foreach(QString severity_id, severities.keys())
    {
	_m_ui->gravite->addItem(severities.value(severity_id), severity_id);
    }

    loadBugtrackerReproducibilities();
}


void Form_Bug::bugtrackerReproducibilities(QMap<QString, QString> reproducibilities)
{
    disconnect(_m_bugtracker, SIGNAL(reproducibilities(QMap<QString, QString>)), this, SLOT(bugtrackerReproducibilities(QMap<QString, QString>)));

    _m_ui->reproductibilite->clear();
    foreach(QString reproducibility_id, reproducibilities.keys())
    {
	_m_ui->reproductibilite->addItem(reproducibilities.value(reproducibility_id), reproducibility_id);
    }

    loadBugtrackerPlatforms();
}



void Form_Bug::bugtrackerPlatforms(QList<QString> platforms)
{
    disconnect(_m_bugtracker, SIGNAL(platforms(QList<QString>)), this, SLOT(bugtrackerPlatforms(QList<QString>)));

    _m_ui->plateforme->clear();
    foreach(QString platform, platforms)
    {
	_m_ui->plateforme->addItem(platform);
    }

    loadBugtrackerOsTypes();
}


void Form_Bug::bugtrackerOperatingSystems(QList<QString> operatingSystems)
{
    disconnect(_m_bugtracker, SIGNAL(operatingSystems(QList<QString>)), this, SLOT(bugtrackerOperatingSystems(QList<QString>)));

    _m_ui->systeme->clear();
    foreach(QString operatingSystem, operatingSystems)
    {
	_m_ui->systeme->addItem(operatingSystem);
    }

   loadBugtrackerAvailableProjectsIds();
}


void Form_Bug::bugtrackerAvailableProjectsIds(QList<QString> in_projects_ids)
{
    disconnect(_m_bugtracker, SIGNAL(availableProjectsIds(QList<QString>)), this, SLOT(bugtrackerAvailableProjectsIds(QList<QString>)));

    loadBugtrackerProjectsInformations(in_projects_ids);
}



void Form_Bug::bugtrackerProjectsInformations(QList< QPair<QString, QString> > in_projects_infos)
{
    bool                    tmp_found_project = false;
    QPair<QString,QString>  tmp_project_infos;

    disconnect(_m_bugtracker, SIGNAL(projectsInformations(QList<QPair<QString,QString> >)), this, SLOT(bugtrackerProjectsInformations(QList<QPair<QString,QString> >)));

    // Vérifier l'existance du projet courant
    // Recherche le projet courant par son identifiant paramétré au niveau des propriétés de la version de projet
    if (is_empty_string(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_ID)) == FALSE)
    {
        foreach (tmp_project_infos, in_projects_infos)
        {
            if (compare_values(tmp_project_infos.first.toStdString().c_str(), _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_ID)) == 0)
            {
                _m_project_id = _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_ID);
                _m_project_name = tmp_project_infos.second;
                tmp_found_project = true;
                break;
            }
        }
    }
    else
    {
        // Recherche le produit courant par son nom de projet
        foreach (tmp_project_infos, in_projects_infos)
        {
            if (compare_values(tmp_project_infos.second.toStdString().c_str(), _m_project_version->project()->getValueForKey(PROJECTS_TABLE_SHORT_NAME)) == 0)
            {
                _m_project_id = tmp_project_infos.first;
                _m_project_name = _m_project_version->project()->getValueForKey(PROJECTS_TABLE_SHORT_NAME);
                tmp_found_project = true;
                break;
            }
        }
    }

    if (!tmp_found_project)
    {
        QMessageBox::critical(this, tr("Projet introuvable"), tr("Le projet %1 n'a pas été trouvé dans le bugtracker.").arg(_m_project_version->project()->getValueForKey(PROJECTS_TABLE_SHORT_NAME)));
        _m_bugtracker->disconnect();
    }
    else
        loadBugtrackerProjectComponents();
}


void Form_Bug::bugtrackerProjectComponents(QList<QString> componants)
{
    disconnect(_m_bugtracker, SIGNAL(projectComponents(QList<QString>)), this, SLOT(bugtrackerProjectComponents(QList<QString>)));

    _m_ui->composant->clear();
    foreach(QString componant, componants)
    {
	_m_ui->composant->addItem(componant);
    }

    loadBugtrackerProjectVersions();
}


void Form_Bug::bugtrackerProjectVersions(QList<QString> versions)
{
    bool    tmp_current_version_found = false;
    QString tmp_bugtracker_project_version = _m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_PROJECT_VERSION);
    QString tmp_project_version = ProjectVersion::formatProjectVersionNumber(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_VERSION));

    disconnect(_m_bugtracker, SIGNAL(projectVersions(QList<QString>)), this, SLOT(bugtrackerProjectVersions(QList<QString>)));

    _m_ui->version->clear();
    foreach(QString version, versions)
    {
	_m_ui->version->addItem(version);
    }


    // Recherche de la version correspondante paramétrée au niveau des propriétés de la version de projet
    if (!tmp_bugtracker_project_version.isEmpty())
    {
        int     tmp_index = 0;
        foreach(QString tmp_version, versions)
        {
            if (compare_values(tmp_bugtracker_project_version.toStdString().c_str(), tmp_version.toStdString().c_str()) == 0)
            {
                tmp_current_version_found = true;
                _m_ui->version->setCurrentIndex(tmp_index);
            }
            tmp_index++;
        }
    }

    // Recherche de la version correspondante à la version du projet
    if (!tmp_current_version_found)
    {
        int     tmp_index = 0;
        foreach(QString tmp_version, versions)
        {
            if (compare_values(tmp_project_version.toStdString().c_str(), tmp_version.toStdString().c_str()) == 0)
            {
                tmp_current_version_found = true;
                _m_ui->version->setCurrentIndex(tmp_index);
            }
            tmp_index++;
        }
    }

    if (!tmp_current_version_found)
    {
        if (tmp_bugtracker_project_version.isEmpty())
            QMessageBox::warning(this, tr("Version non trouvée"), tr("La version courante du projet (%1) n'existe pas dans le bugtracker cible.").arg(tmp_project_version));
        else
            QMessageBox::warning(this, tr("Version non trouvée"), tr("La version du projet prédéfinie (%1) n'existe pas dans le bugtracker cible.").arg(tmp_bugtracker_project_version));
    }

    updateControls();
}



void Form_Bug::bugtrackerBugCreated(QString in_bug_id)
{
    disconnect(_m_bugtracker, SIGNAL(bugCreated(QString)), this, SLOT(bugtrackerBugCreated(QString)));

    saveBug(in_bug_id);
    //logoutBugtracker();
}



void Form_Bug::bugtrackerError(QString errorMsg)
{
    QMessageBox::critical(this, tr("Erreur"), errorMsg+"<p><span style=\"color: red\"><b>"+tr("La requête a échouée.")+"</b></span></p>");
}


void Form_Bug::saveBug(QString in_bugtracker_bug_id)
{
    int				tmp_save_result = NOERR;

    _m_bug->setValueForKey(CLIENT_MACRO_NOW, BUGS_TABLE_CREATION_DATE);
    _m_bug->setValueForKey(_m_ui->resume->text().toStdString().c_str(), BUGS_TABLE_SHORT_NAME);
    _m_bug->setValueForKey(_m_ui->description->toPlainText().toStdString().c_str(), BUGS_TABLE_DESCRIPTION);
    _m_bug->setValueForKey(_m_ui->priorite->itemText(_m_ui->priorite->currentIndex()).toStdString().c_str(), BUGS_TABLE_PRIORITY);
    _m_bug->setValueForKey(_m_ui->gravite->itemText(_m_ui->gravite->currentIndex()).toStdString().c_str(), BUGS_TABLE_SEVERITY);
    _m_bug->setValueForKey(_m_ui->reproductibilite->itemText(_m_ui->reproductibilite->currentIndex()).toStdString().c_str(), BUGS_TABLE_REPRODUCIBILITY);
    _m_bug->setValueForKey(_m_ui->plateforme->itemText(_m_ui->plateforme->currentIndex()).toStdString().c_str(), BUGS_TABLE_PLATFORM);
    _m_bug->setValueForKey(_m_ui->systeme->itemText(_m_ui->systeme->currentIndex()).toStdString().c_str(), BUGS_TABLE_SYSTEM);
    _m_bug->setValueForKey(_m_ui->bug_status->itemData(_m_ui->bug_status->currentIndex()).toString().toStdString().c_str(), BUGS_TABLE_STATUS);

    if (_m_execution_test != NULL)
    _m_bug->setValueForKey(_m_execution_test->getIdentifier(), BUGS_TABLE_EXECUTION_TEST_ID);

    if (_m_execution_action != NULL)
    _m_bug->setValueForKey(_m_execution_action->getIdentifier(), BUGS_TABLE_EXECUTION_ACTION_ID);

    if (!in_bugtracker_bug_id.isEmpty())
    _m_bug->setValueForKey(in_bugtracker_bug_id.toStdString().c_str(), BUGS_TABLE_BUGTRACKER_BUG_ID);

    tmp_save_result = _m_bug->saveRecord();
    if (tmp_save_result != NOERR)
    {
    QMessageBox::critical(this, tr("Erreur lors de l'enregistrement"), Session::instance()->getErrorMessage(tmp_save_result));
    }

    emit bugSaved();
    close();
}


void Form_Bug::updateControls()
{
    bool	tmp_has_bugtracker = (_m_bugtracker != NULL);
    bool	tmp_has_bugtracker_id = is_empty_string(_m_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID)) == FALSE;

    int		tmp_index = 0;

    QString	tmp_url;
    QString	tmp_external_link;

    ExecutionTest        *tmp_parent_execution_test = NULL;

    QString				tmp_bug_description = tr("Scénario : ");

    bool	tmp_write_access = _m_project_version != NULL
				   && _m_project_version->project() != NULL
				   && _m_project_version->project()->projectGrants() != NULL
                   && compare_values(_m_project_version->project()->projectGrants()->getValueForKey(PROJECTS_GRANTS_TABLE_MANAGE_EXECUTIONS_INDIC), PROJECT_GRANT_WRITE) == 0;

    bool	tmp_read_write = tmp_write_access && !tmp_has_bugtracker_id && (!tmp_has_bugtracker || _m_bugtracker_connection_active);

    _m_ui->resume->setText(_m_bug->getValueForKey(BUGS_TABLE_SHORT_NAME));

    if (is_empty_string(_m_bug->getIdentifier()) && _m_execution_test != NULL)
    {
		tmp_parent_execution_test = _m_execution_test;
		while (tmp_parent_execution_test->parent() != NULL && tmp_parent_execution_test->parent() != _m_execution_test->executionCampaign())
		{
			tmp_parent_execution_test = (ExecutionTest*)tmp_parent_execution_test->parent();
		}

		generateBugDescriptionUntilAction(tmp_parent_execution_test, tmp_bug_description, "", _m_execution_test, _m_execution_action);
    	_m_ui->description->setPlainText(tmp_bug_description);
    }
    else
        _m_ui->description->setPlainText(_m_bug->getValueForKey(BUGS_TABLE_DESCRIPTION));

    if (tmp_has_bugtracker && is_empty_string(_m_project_version->getValueForKey(PROJECTS_VERSIONS_TABLE_BUG_TRACKER_HOST)) == FALSE)
    {
    tmp_url = _m_bugtracker->urlForBugWithId(_m_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID));
	if (tmp_url.isEmpty())
        tmp_external_link = QString(_m_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID));
	else
	    tmp_external_link = "<a href=\"" + tmp_url + "\">" + tmp_url + "</a>";
    }
    else
    tmp_external_link = QString(_m_bug->getValueForKey(BUGS_TABLE_BUGTRACKER_BUG_ID));

    _m_ui->url_externe->setText(tmp_external_link);

    // Priorité
    if (is_empty_string(_m_bug->getValueForKey(BUGS_TABLE_PRIORITY)) == FALSE)
    {
    tmp_index = _m_ui->priorite->findText(_m_bug->getValueForKey(BUGS_TABLE_PRIORITY));
	if (tmp_index < 0)
	{
        _m_ui->priorite->addItem(_m_bug->getValueForKey(BUGS_TABLE_PRIORITY));
	    _m_ui->priorite->setCurrentIndex(_m_ui->priorite->count() - 1);
	}
	else
	    _m_ui->priorite->setCurrentIndex(tmp_index);
    }

    // Gravité
    if (is_empty_string(_m_bug->getValueForKey(BUGS_TABLE_SEVERITY)) == FALSE)
    {
    tmp_index = _m_ui->gravite->findText(_m_bug->getValueForKey(BUGS_TABLE_SEVERITY));
	if (tmp_index < 0)
	{
        _m_ui->gravite->addItem(_m_bug->getValueForKey(BUGS_TABLE_SEVERITY));
        _m_ui->gravite->setCurrentIndex(_m_ui->gravite->count() - 1);
	}
	else
	    _m_ui->gravite->setCurrentIndex(tmp_index);
    }

    // Reproductibilité
    if (is_empty_string(_m_bug->getValueForKey(BUGS_TABLE_REPRODUCIBILITY)) == FALSE)
    {
        tmp_index = _m_ui->reproductibilite->findText(_m_bug->getValueForKey(BUGS_TABLE_REPRODUCIBILITY));
        if (tmp_index < 0)
        {
            _m_ui->reproductibilite->addItem(_m_bug->getValueForKey(BUGS_TABLE_REPRODUCIBILITY));
            _m_ui->reproductibilite->setCurrentIndex(_m_ui->reproductibilite->count() - 1);
        }
        else
            _m_ui->reproductibilite->setCurrentIndex(tmp_index);
    }

    // Plateforme
    if (is_empty_string(_m_bug->getValueForKey(BUGS_TABLE_PLATFORM)) == FALSE)
    {
    tmp_index = _m_ui->plateforme->findText(_m_bug->getValueForKey(BUGS_TABLE_PLATFORM));
	if (tmp_index < 0)
	{
        _m_ui->plateforme->addItem(_m_bug->getValueForKey(BUGS_TABLE_PLATFORM));
        _m_ui->plateforme->setCurrentIndex(_m_ui->plateforme->count() - 1);
	}
	else
	    _m_ui->plateforme->setCurrentIndex(tmp_index);
    }

    // OS
    if (is_empty_string(_m_bug->getValueForKey(BUGS_TABLE_SYSTEM)) == FALSE)
    {
    tmp_index = _m_ui->systeme->findText(_m_bug->getValueForKey(BUGS_TABLE_SYSTEM));
	if (tmp_index < 0)
	{
        _m_ui->systeme->addItem(_m_bug->getValueForKey(BUGS_TABLE_SYSTEM));
        _m_ui->systeme->setCurrentIndex(_m_ui->systeme->count() - 1);
	}
	else
	    _m_ui->systeme->setCurrentIndex(tmp_index);
    }

    // Status
    if (is_empty_string(_m_bug->getValueForKey(BUGS_TABLE_STATUS)) == FALSE)
    {
    tmp_index = _m_ui->bug_status->findData(_m_bug->getValueForKey(BUGS_TABLE_STATUS));
	if (tmp_index < 0)
	{
        _m_ui->bug_status->addItem(_m_bug->getValueForKey(BUGS_TABLE_STATUS));
	    _m_ui->bug_status->setCurrentIndex(_m_ui->bug_status->count() - 1);
	}
	else
	    _m_ui->bug_status->setCurrentIndex(tmp_index);
    }
    else
        _m_ui->status_widget->setVisible(false);


    _m_ui->url_externe_widget->setVisible(tmp_has_bugtracker_id);
    _m_ui->bugtracker_group_box->setVisible(!_m_bugtracker_connection_active && tmp_has_bugtracker);
    _m_ui->bugtracker_component_frame->setVisible(tmp_has_bugtracker && !tmp_has_bugtracker_id);

    _m_ui->connect_button->setEnabled(!_m_bugtracker_connection_active || tmp_has_bugtracker_id);
    _m_ui->username->setEnabled(!_m_bugtracker_connection_active);
    _m_ui->password->setEnabled(!_m_bugtracker_connection_active);

    _m_ui->composant->setEnabled(_m_bugtracker_connection_active);
    _m_ui->version->setEnabled(_m_bugtracker_connection_active);

    _m_ui->gravite->setEnabled(tmp_read_write);
    _m_ui->priorite->setEnabled(tmp_read_write);
    _m_ui->reproductibilite->setEnabled(tmp_read_write);
    _m_ui->plateforme->setEnabled(tmp_read_write);
    _m_ui->systeme->setEnabled(tmp_read_write);
    _m_ui->resume->setEnabled(tmp_read_write);

    _m_ui->description->setReadOnly(!tmp_read_write);
    if (tmp_read_write)
	_m_ui->description->setStyleSheet(QPLAINTEXTEDIT_WRITABLE);
    else
	_m_ui->description->setStyleSheet(QPLAINTEXTEDIT_UNWRITABLE);

    _m_ui->bug_status->setEnabled(tmp_read_write);

    _m_ui->buttonBox->button(QDialogButtonBox::Save)->setEnabled(tmp_write_access && (!tmp_has_bugtracker || _m_bugtracker_connection_active));

    _m_ui->execution_button->setVisible(_m_execution_test == NULL);

    _m_ui->read_warning_widget->setVisible(_m_bugtracker_connection_active && tmp_has_bugtracker_id);
}


void Form_Bug::updateBugFromBugtracker(QMap< QString, QMap<Bugtracker::BugProperty, QString> > in_bugs_list)
{
    QMap< QString, QMap<Bugtracker::BugProperty, QString> >::iterator tmp_bug_iterator;

    for(tmp_bug_iterator = in_bugs_list.begin(); tmp_bug_iterator != in_bugs_list.end(); tmp_bug_iterator++)
    {
        // Mettre à jour le bug
	QMap<Bugtracker::BugProperty, QString> tmp_bug_infos = tmp_bug_iterator.value();

        if (!tmp_bug_infos[Bugtracker::Priority].isEmpty())
            _m_bug->setValueForKey(tmp_bug_infos[Bugtracker::Priority].toStdString().c_str(), BUGS_TABLE_PRIORITY);

        if (!tmp_bug_infos[Bugtracker::Severity].isEmpty())
            _m_bug->setValueForKey(tmp_bug_infos[Bugtracker::Severity].toStdString().c_str(), BUGS_TABLE_SEVERITY);

        if (!tmp_bug_infos[Bugtracker::Summary].isEmpty())
            _m_bug->setValueForKey(tmp_bug_infos[Bugtracker::Summary].toStdString().c_str(), BUGS_TABLE_SHORT_NAME);

        if (!tmp_bug_infos[Bugtracker::Status].isEmpty())
            _m_bug->setValueForKey(tmp_bug_infos[Bugtracker::Status].toStdString().c_str(), BUGS_TABLE_STATUS);
    }

    updateControls();
}


void Form_Bug::openExecutionCampaign()
{
    Form_Execution_Campaign	*tmp_dialog = NULL;
    ExecutionCampaign		*tmp_execution_campaign = NULL;
    Campaign			*tmp_campaign = NULL;
    ExecutionCampaign		**tmp_executions = NULL;

    unsigned long		tmp_execution_count = 0;

    sprintf(Session::instance()->getClientSession()->m_where_clause_buffer, "%s IN (SELECT %s FROM %s WHERE %s=%s)",
	    EXECUTIONS_CAMPAIGNS_TABLE_EXECUTION_CAMPAIGN_ID,
	    EXECUTIONS_TESTS_TABLE_EXECUTION_CAMPAIGN_ID,
	    EXECUTIONS_TESTS_TABLE_SIG,
	    EXECUTIONS_TESTS_TABLE_EXECUTION_TEST_ID,
        _m_bug->getValueForKey(BUGS_TABLE_EXECUTION_TEST_ID));

    tmp_executions = Record::loadRecords<ExecutionCampaign>(Session::instance()->getClientSession(), &executions_campaigns_table_def, Session::instance()->getClientSession()->m_where_clause_buffer, NULL, &tmp_execution_count);
    if (tmp_executions != NULL)
    {
	if (tmp_execution_count == 1)
	{
	    tmp_execution_campaign = tmp_executions[0];
	    tmp_executions = Record::freeRecords<ExecutionCampaign>(tmp_executions, 0);
	}
	else
	    tmp_executions = Record::freeRecords<ExecutionCampaign>(tmp_executions, tmp_execution_count);

    }

    if (tmp_execution_campaign != NULL)
    {
	tmp_campaign = new Campaign(_m_project_version);
    if (tmp_campaign->loadRecord(tmp_execution_campaign->getValueForKey(EXECUTIONS_CAMPAIGNS_TABLE_CAMPAIGN_ID)) == NOERR)
	{
	    tmp_campaign->loadTests();
	    tmp_execution_campaign->setCampaign(tmp_campaign);
	    tmp_execution_campaign->loadExecutionsDatas();
	    tmp_dialog = new Form_Execution_Campaign(tmp_execution_campaign, parentWidget());
	    tmp_dialog->showMaximized();
        tmp_dialog->showTestInfosForTestWithId(_m_bug->getValueForKey(BUGS_TABLE_EXECUTION_TEST_ID));
	}
    }
}


bool Form_Bug::generateBugDescriptionUntilAction(ExecutionTest *in_root_test, QString & description, QString in_prefix, ExecutionTest *in_test, ExecutionAction *in_action)
{
    if (in_root_test != NULL)
    {
	description.append(in_prefix);
	if (in_root_test->projectTest() != NULL)
	{
        description.append(in_root_test->projectTest()->getValueForKey(TESTS_HIERARCHY_SHORT_NAME));
	}
	description.append("\n");

	if (in_test == in_root_test)
	{
		int	tmp_action_index = 1;

		foreach(ExecutionAction *tmp_action, in_root_test->actions())
		{
			QTextDocument	tmp_doc;

			tmp_doc.setHtml(RecordTextEdit::toHtmlWithParametersValues<ExecutionTestParameter>(
			    in_test->inheritedParameters(),
                tmp_action->action()->getValueForKey(ACTIONS_TABLE_DESCRIPTION)));
			description.append(tr("Action %1 => ").arg(tmp_action_index));
			description.append(tmp_doc.toPlainText());
			description.append("\n");

			tmp_doc.setHtml(RecordTextEdit::toHtmlWithParametersValues<ExecutionTestParameter>(
			    in_test->inheritedParameters(),
                tmp_action->action()->getValueForKey(ACTIONS_TABLE_WAIT_RESULT)));
			description.append(tr("Résultat attendu => "));
			description.append(tmp_doc.toPlainText());
			description.append("\n");

			if (in_action != NULL && tmp_action == in_action)
			{
                tmp_doc.setHtml(tmp_action->getValueForKey(EXECUTIONS_ACTIONS_TABLE_COMMENTS));
				description.append(tr("Problème rencontré => "));
				description.append(tmp_doc.toPlainText());
				description.append("\n");

				return true;
			}

			tmp_action_index++;
		}

		return true;
	}

	int	tmp_test_index = 1;

	foreach(ExecutionTest *tmp_child_test, in_root_test->childs())
	{
		if (generateBugDescriptionUntilAction(tmp_child_test, description, in_prefix + QString::number(tmp_test_index) + ".", in_test, in_action))
			return true;

		tmp_test_index++;
	}
    }

    return false;
}

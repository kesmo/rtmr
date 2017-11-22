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

#ifndef FORM_CAMPAIGN_WIZARD_H
#define FORM_CAMPAIGN_WIZARD_H

#include <QtGui/QWizard>
#include "projectversion.h"
#include "campaign.h"
#include "gui/components/record_text_edit_container.h"


namespace Ui {
    class Form_Campaign_Wizard;
}

class Form_Campaign_Wizard : public QWizard {
    Q_OBJECT
public:
    Form_Campaign_Wizard(ProjectVersion *in_project, QWidget *parent = 0);
    ~Form_Campaign_Wizard();

signals:
    void campaignCreated(Campaign *in_new_campaign);

public slots:
    void accept();
    void campaignShortNameChanged();
    void updateControls();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::Form_Campaign_Wizard    *_m_ui;

    ProjectVersion           *_m_project;
};

#endif // FORM_CAMPAIGN_WIZARD_H

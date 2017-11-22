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

#ifndef FORM_CAMPAIGN_H
#define FORM_CAMPAIGN_H

#include <QtGui/QTreeView>
#include "campaign.h"
#include "testcampaign.h"
#include "gui/components/records_tree_model.h"
#include "gui/components/records_tree_view.h"
#include "gui/components/abstract_project_widget.h"

#include <clientmodule.h>

namespace Ui {
    class Form_Campaign;
}

class Form_Campaign : public AbstractProjectWidget {
    Q_OBJECT
    Q_DISABLE_COPY(Form_Campaign)
public:
    explicit Form_Campaign(QWidget *parent = 0);
    virtual ~Form_Campaign();

    void loadCampaign(Campaign *in_campaign);

signals:
    void campaignSaved();
    void canceled();

public slots:
    void save();
    void cancel();
    void deleteTestsCampaignList(QList<Hierarchy*> in_records_list);
    void execute();
    void open(QModelIndex in_index);
    void reloadCampaign();
    void deleteSelectedExecutionCampaign();
    void executeSelectedExecutionCampaign();
    void manageExecutionsSelection();

    void setModified();

protected:
    virtual void changeEvent(QEvent *e);
    void setExecutionCampaignForRow(ExecutionCampaign *in_execution_campaign, int in_row);

    bool saveCampaign();

    virtual void closeEvent(QCloseEvent *in_event);

private:
    Ui::Form_Campaign       *_m_ui;

    RecordsTreeView       *_m_tests_tree_view;
    RecordsTreeModel      *_m_tests_tree_model;

    Campaign             *_m_campaign;
    Campaign             *_m_original_campaign;

    QList < ExecutionCampaign* >  _m_executions_list;

    int saveExecutionsCampaign();
    void loadCampaignExecutions();

    QMap<CampaignModule*, QWidget*>   _m_views_modules_map;

    void loadPluginsViews();
    void destroyPluginsViews();
    void savePluginsDatas();

};

#endif // FORM_CAMPAIGN_H

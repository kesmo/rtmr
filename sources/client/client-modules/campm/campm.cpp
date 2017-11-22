#include "campm.h"

#include <QGridLayout>
#include <QLabel>

Campm::Campm() : CampaignModule()
{
}


QString Campm::getModuleName()
{
    return "Campm";
}


QString Campm::getModuleVersion()
{
    return "1.0";
}


QString Campm::getModuleInformationsUrl()
{
    return "http://rtmr.net";
}


QWidget* Campm::createView(QWidget *parent)
{
    QWidget *tmp_view_module = new QWidget(parent);
    QGridLayout	*tmp_grid_layout = new QGridLayout(tmp_view_module);

    tmp_grid_layout->addWidget(new QLabel("Hello !"), 0, 0, Qt::AlignRight);
    tmp_grid_layout->addWidget(new QLabel("World !"), 0, 1, Qt::AlignLeft);
    tmp_grid_layout->addWidget(new QLabel(_m_campaign->valueForKey(CAMPAIGNS_TABLE_SHORT_NAME)), 1, 0, Qt::AlignRight);
    tmp_grid_layout->addWidget(new QLabel("User => "+QString(_m_session->m_username)), 1, 1, Qt::AlignLeft);

    LOG_TRACE(_m_session, "Module view creation\n");

    return tmp_view_module;
}


void Campm::loadCampaignModuleDatas(Campaign *in_campaign)
{
    _m_campaign = in_campaign;
}

void Campm::saveCampaignModuleDatas()
{

}


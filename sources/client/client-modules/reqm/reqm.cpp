#include "reqm.h"

#include <QGridLayout>
#include <QLabel>

Reqm::Reqm() : RequirementModule()
{
}


QString Reqm::getModuleName()
{
    return "Reqm";
}


QString Reqm::getModuleVersion()
{
    return "1.0";
}


QString Reqm::getModuleInformationsUrl()
{
    return "http://rtmr.net";
}


QWidget* Reqm::createView(QWidget *parent)
{
    QWidget *tmp_view_module = new QWidget(parent);
    QGridLayout	*tmp_grid_layout = new QGridLayout(tmp_view_module);

    tmp_grid_layout->addWidget(new QLabel("Hello !"), 0, 0, Qt::AlignRight);
    tmp_grid_layout->addWidget(new QLabel("World !"), 0, 1, Qt::AlignLeft);
    tmp_grid_layout->addWidget(new QLabel(_m_requirement->valueForKey(REQUIREMENTS_HIERARCHY_SHORT_NAME)), 1, 0, Qt::AlignRight);
    tmp_grid_layout->addWidget(new QLabel("User => "+QString(_m_session->m_username)), 1, 1, Qt::AlignLeft);

    LOG_TRACE(_m_session, "Module view creation\n");

    return tmp_view_module;
}


void Reqm::loadRequirementModuleDatas(RequirementHierarchy *in_requirement)
{
    _m_requirement = in_requirement;
}

void Reqm::saveRequirementModuleDatas()
{

}


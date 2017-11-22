#include "prom.h"

#include <QGridLayout>
#include <QLabel>

Prom::Prom() : ProjectModule()
{
}


QString Prom::getModuleName()
{
    return "Prom";
}


QString Prom::getModuleVersion()
{
    return "1.0";
}


QString Prom::getModuleInformationsUrl()
{
    return "http://rtmr.net";
}


QWidget* Prom::createView(QWidget *parent)
{
    QWidget *tmp_view_module = new QWidget(parent);
    QGridLayout	*tmp_grid_layout = new QGridLayout(tmp_view_module);

    tmp_grid_layout->addWidget(new QLabel("Hello !"), 0, 0, Qt::AlignRight);
    tmp_grid_layout->addWidget(new QLabel("World !"), 0, 1, Qt::AlignLeft);
    tmp_grid_layout->addWidget(new QLabel(_m_project_version->valueForKey(PROJECTS_VERSIONS_TABLE_VERSION)), 1, 0, Qt::AlignRight);
    tmp_grid_layout->addWidget(new QLabel("User => "+QString(_m_session->m_username)), 1, 1, Qt::AlignLeft);

    LOG_TRACE(_m_session, "Module view creation\n");

    return tmp_view_module;
}


void Prom::loadProjectModuleDatas(ProjectVersion * in_project_version)
{
    _m_project_version = in_project_version;
}

void Prom::saveProjectModuleDatas()
{

}


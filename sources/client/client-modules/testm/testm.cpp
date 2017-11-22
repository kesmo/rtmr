#include "testm.h"

#include <QGridLayout>
#include <QLabel>

Testm::Testm() : TestModule()
{
}


QString Testm::getModuleName()
{
    return "Testm";
}


QString Testm::getModuleVersion()
{
    return "1.0";
}


QString Testm::getModuleInformationsUrl()
{
    return "http://rtmr.net";
}


QWidget* Testm::createView(QWidget *parent)
{
    QWidget *tmp_view_module = new QWidget(parent);
    QGridLayout	*tmp_grid_layout = new QGridLayout(tmp_view_module);

    tmp_grid_layout->addWidget(new QLabel("Hello !"), 0, 0, Qt::AlignRight);
    tmp_grid_layout->addWidget(new QLabel("World !"), 0, 1, Qt::AlignLeft);
    tmp_grid_layout->addWidget(new QLabel(_m_test->valueForKey(TESTS_HIERARCHY_SHORT_NAME)), 1, 0, Qt::AlignRight);
    tmp_grid_layout->addWidget(new QLabel("User => "+QString(_m_session->m_username)), 1, 1, Qt::AlignLeft);

    LOG_TRACE(_m_session, "Module view creation\n");

    return tmp_view_module;
}


void Testm::loadTestModuleDatas(TestHierarchy *in_test)
{
    _m_test = in_test;
}

void Testm::saveTestModuleDatas()
{

}


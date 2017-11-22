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

#ifndef CLIENTMODULE_H
#define CLIENTMODULE_H

#if defined(CLIENT_MODULE_LIBRARY)
#  define CLIENT_MODULE_EXPORT Q_DECL_EXPORT
#else
#  define CLIENT_MODULE_EXPORT Q_DECL_IMPORT
#endif

#include <QWidget>

#include <QString>
#include <entities.h>
#include <netcommon.h>
#include <bugtracker.h>

class Project;
class ProjectVersion;
class RequirementHierarchy;
class TestHierarchy;
class Campaign;
class AutomatedActionValidation;

/*!
The function create_module_instance_fct is call by the client to create module instance.<br>
Plug-ins developpers must write such a function to allow client application create module instance at startup.
*/
#define MODULE_INIT_INSTANCE_ENTRY_NAME         "create_module_instance"


/*!
\brief The function destroy_module_instance_fct is call by the client to destroy module instance.<br>
Plug-ins developpers must write such a function to allow client application destroy module instance at stop.
*/
#define MODULE_DESTROY_INSTANCE_ENTRY_NAME      "destroy_module_instance"

/**
\brief Main class for all plug-ins.<br>
Plug-ins are loaded by the client application at startup.
Depending on the type of platform, plug-ins are look up like this :
- each \a .cmo file in \a modules folder on Windows
- each \a .dylib file in \a Contents/MacOS/modules folder from rtmr.app package on MacOS
- each \a .cmo file in \a /usr/share/rtmr/modules folder on Linux platform

When a plug-in library matches the search pattern into the modules folder,<br>
the application tries to load it by invoking the library symbol (function) called \a create_module_instance.<br>
When the application quits, it tries to unload the plug-in by invoking the library symbol (function) called \a destroy_module_instance.<br>
Developpers must implements such functions into their rtmr module library.<br>
Below a sample code used by the Bugzilla module :
- \a create_module_instance function :
\code
    CLIENT_MODULE_EXPORT ClientModule* create_module_instance()
    {
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
        QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));

        return BugzillaModule::getModuleInstance();
    }
\endcode
- \a destroy_module_instance function :
\code
    CLIENT_MODULE_EXPORT void destroy_module_instance(ClientModule *in_module_instance)
    {
        BugzillaModule::destroyModuleInstance(in_module_instance);
    }
\endcode

Above CLIENT_MODULE_EXPORT is a preprocessor directive used to export symbols from the library according to the platform.
*/
class ClientModule
{
public:
    /**
    \brief This is the list of available plug-ins.
    */
    enum ModuleType {
        BugtrackerPlugin,
        ProjectPlugin,
        RequirementPlugin,
        TestPlugin,
        CampaignPlugin,
        AutomationPlugin
    };

    /**
    Retrieve the description of the plug-in type.<br>
    See ClientModule::ModuleType.
    */
    static QString getTypeDescriptionForModule(ClientModule *in_module)
    {
	switch (in_module->getModuleType())
	{
	case BugtrackerPlugin:
	    return "Gestionnaire d'anomalies";
	    break;
	case ProjectPlugin:
	    return "Plugin de projets";
	    break;
	case RequirementPlugin:
	    return "Plugin d'exigences";
	    break;
	case TestPlugin:
	    return "Plugin de tests";
	    break;
	case CampaignPlugin:
	    return "Plugin de campagnes";
	    break;
	case AutomationPlugin:
        return "Plugin d'automatisation";
	    break;
	default:
	    return "Type de module inconnu";
	    break;
	}
    }

    /**
    Retrieve the plug-in name.<br>
    Developper must avoid module name with empty characters.
    */
    virtual QString getModuleName() = 0;
    
    /**
    Retrieve the plug-in version.
    */
    virtual QString getModuleVersion() = 0;
    
    /**
    Retrieve the url from where to find informations about the plug-in.
    */
    virtual QString getModuleInformationsUrl() = 0;
    
    /**
    Retrieve the plug-ins type.<br>
    See ClientModule::ModuleType.
    */
    virtual ModuleType getModuleType() = 0;

    /**
    Initialize the plug-in at application startup with the network session.
    */
    virtual void initModuleFromSession(const net_session *session)
    {
	_m_session = session;
    }

    virtual ~ClientModule(){}

protected:
    /**
    This is the network session allowing access to the gateway server.
    */
    const net_session *_m_session;

};

/**
\brief Base class for plug-ins using graphic user interface.
*/
class ViewModule : public ClientModule
{
public:
    ViewModule(){}

    /**
    destroyView function is call by the application for plug-ins with GUI.
    */
    virtual void destroyView(QWidget* in_view_module)
    {
	delete in_view_module;
    }

    /**
    createView function is call by the application for plug-ins with GUI.<br>
    Developpers must implement this function to create their own plug-ins interface.<br>
    i.e :
    \code
    QWidget* Testm::createView(QWidget *parent)
    {
	QWidget *tmp_view_module = new QWidget(parent);
	QGridLayout	*tmp_grid_layout = new QGridLayout(tmp_view_module);

	tmp_grid_layout->addWidget(new QLabel("Hello !"), 0, 0, Qt::AlignRight);
	tmp_grid_layout->addWidget(new QLabel("World !"), 0, 1, Qt::AlignLeft);
	tmp_grid_layout->addWidget(new QLabel(_m_test->getValueForKey(TESTS_HIERARCHY_SHORT_NAME)), 1, 0, Qt::AlignRight);
	tmp_grid_layout->addWidget(new QLabel("User => "+QString(_m_session->m_username)), 1, 1, Qt::AlignLeft);

	LOG_TRACE(_m_session, "Module view creation\n");

	return tmp_view_module;
    }
    \endcode
    */
    virtual QWidget* createView(QWidget *parent) = 0;

};


/**
\brief Base class for projects plug-ins.
*/
class ProjectModule : public ViewModule
{
public:
    ProjectModule() : ViewModule(){}

    /**
        Return ClientModule::ProjectPlugin.
		*/
    ClientModule::ModuleType getModuleType()
    {
        return ProjectPlugin;
    }

    /**
	loadProjectModuleDatas is call by the client application when the screen of project version properties is opened.
	*/
    virtual void loadProjectModuleDatas(ProjectVersion *in_project_version) = 0;

    /**
	saveProjectModuleDatas is call by the client application when the screen of project version properties is saved.
	*/
    virtual void saveProjectModuleDatas() = 0;
};


/**
\brief Base class for requirements plug-ins
*/
class RequirementModule : public ViewModule
{
public:
    RequirementModule() : ViewModule(){}

    ClientModule::ModuleType getModuleType()
    {
        return RequirementPlugin;
    }

    /**
	loadRequirementModuleDatas is call by the client application when the screen of a requirement is opened.
	*/
    virtual void loadRequirementModuleDatas(RequirementHierarchy *in_requirement) = 0;

    /**
	saveRequirementModuleDatas is call by the client application when the screen of a requirement is saved.
	*/
    virtual void saveRequirementModuleDatas() = 0;
};


/**
\brief Base class for tests plug-ins
*/
class TestModule : public ViewModule
{
public:
    TestModule() : ViewModule(){}

    /**
        Return ClientModule::TestPlugin.
		*/
    ClientModule::ModuleType getModuleType()
    {
        return TestPlugin;
    }

    /**
	loadTestModuleDatas is call by the client application when the screen of a scenario/test case is opened.
	*/
    virtual void loadTestModuleDatas(TestHierarchy *in_test) = 0;

    /**
	saveTestModuleDatas is call by the client application when the screen of a scenario/test case is saved.
	*/
    virtual void saveTestModuleDatas() = 0;
};


/**
\brief Base class for campaign plug-ins
*/
class CampaignModule : public ViewModule
{
public:
    CampaignModule() : ViewModule(){}

    /**
        Return ClientModule::CampaignPlugin.
		*/
    ClientModule::ModuleType getModuleType()
    {
        return CampaignPlugin;
    }

    /**
	loadCampaignModuleDatas is call by the client application when the screen of a campaign is opened.
	*/
    virtual void loadCampaignModuleDatas(Campaign *in_campaign) = 0;

    /**
	saveCampaignModuleDatas is call by the client application when the screen of a campaign is saved.
	*/
    virtual void saveCampaignModuleDatas() = 0;
};


/**
\brief Base class for bugtrackers plug-ins.
*/
class BugtrackerModule : public ClientModule
{
public:

    /**
        Return ClientModule::BugtrackerPlugin.
		*/
    ClientModule::ModuleType getModuleType()
    {
        return BugtrackerPlugin;
    }

    /**
		createBugtracker is call by the client application into severals contexts :
		- from the screen showing the list of bugs of a project version
		- when adding a bug from execution campaign
		- when consulting bug details from bugs tab into test screen
		
		It returns a pointer to an object of type Bugtracker.
		*/
    virtual Bugtracker* createBugtracker() = 0;

    /**
		destroyBugtracker is call by the client into severals contexts like those discribe in BugtrackerModule::createBugtracker function.<br>
		It frees a pointer to an object of type Bugtracker which has been allocated with a previous call to BugtrackerModule::createBugtracker.
		*/
    static void destroyBugtracker(Bugtracker *in_bugtracker)
    {
	delete in_bugtracker;
    }
};


/**
\brief Base class for automation plug-ins.
*/
class AutomationModule;

/**
\brief Function definition for automation callback validation.
*/
typedef int (automation_callback)(WId in_window_id, bool in_must_be_active_window, AutomatedActionValidation* in_automated_action, char* out_error_msg, void* in_ptr, ...);

/**
\brief Automation functions module class.
*/

class AutomationCallbackFunction
{
public:
    AutomationCallbackFunction(const QString& name, const QString& description, automation_callback* callback, AutomationModule* in_automation_module):
        _m_name(name),
        _m_description(description),
        _m_callback(callback),
      _m_automation_module(in_automation_module){}

    const QString getName() const {return _m_name;}
    const QString getDescription() const {return _m_description;}
    AutomationModule* getModule() const {return _m_automation_module;}

    automation_callback* getEntryPoint() const {return _m_callback;}

private:
    QString _m_name;
    QString _m_description;
    automation_callback* _m_callback;
    AutomationModule* _m_automation_module;
};

/**
\brief Base class for automation plug-ins.
*/
class AutomationModule : public ClientModule
{
public:

    /**
        Return ClientModule::AutomationPlugin.
        */
    ClientModule::ModuleType getModuleType()
    {
        return AutomationPlugin;
    }

    virtual QMap<QString, AutomationCallbackFunction*> getFunctionsMap() = 0;
};

/*! \fn typedef ClientModule* (*create_module_instance_fct)()
*/
typedef ClientModule* (*create_module_instance_fct)();

/*! \fn typedef void (*destroy_module_instance_fct)(ClientModule*)
*/
typedef void (*destroy_module_instance_fct)(ClientModule*);


#endif // CLIENTMODULE_H

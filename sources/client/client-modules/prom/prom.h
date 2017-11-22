#ifndef PROM_H
#define PROM_H

#include <clientmodule.h>

#ifdef  __cplusplus
extern "C" {
#endif


    class Prom : public ProjectModule
    {
    public:
	Prom();

	static ClientModule* getModuleInstance()
	{
	    return new Prom;
	}

	static void destroyModuleInstance(ClientModule *in_module_instance)
	{
	    delete  in_module_instance;
	}

	virtual QString getModuleName();
	virtual QString getModuleVersion();
	virtual QString getModuleInformationsUrl();

	virtual QWidget* createView(QWidget *parent);

	virtual void loadProjectModuleDatas(ProjectVersion *in_project_version);
	virtual void saveProjectModuleDatas();

    private:
	ProjectVersion * _m_project_version;
    };

    CLIENT_MODULE_EXPORT ClientModule* create_module_instance()
    {
	return Prom::getModuleInstance();
    }

    CLIENT_MODULE_EXPORT void destroy_module_instance(ClientModule *in_module_instance)
    {
	Prom::destroyModuleInstance(in_module_instance);
    }

#ifdef  __cplusplus
}
#endif

#endif // PROM_H

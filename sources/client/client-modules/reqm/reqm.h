#ifndef PROM_H
#define PROM_H

#include <clientmodule.h>

#ifdef  __cplusplus
extern "C" {
#endif


    class Reqm : public RequirementModule
    {
    public:
	Reqm();

	static ClientModule* getModuleInstance()
	{
	    return new Reqm;
	}

	static void destroyModuleInstance(ClientModule *in_module_instance)
	{
	    delete  in_module_instance;
	}

	virtual QString getModuleName();
	virtual QString getModuleVersion();
	virtual QString getModuleInformationsUrl();

	virtual QWidget* createView(QWidget *parent);

	virtual void loadRequirementModuleDatas(RequirementHierarchy *in_requirement);
	virtual void saveRequirementModuleDatas();

    private:
	RequirementHierarchy * _m_requirement;
    };

    CLIENT_MODULE_EXPORT ClientModule* create_module_instance()
    {
	return Reqm::getModuleInstance();
    }

    CLIENT_MODULE_EXPORT void destroy_module_instance(ClientModule *in_module_instance)
    {
	Reqm::destroyModuleInstance(in_module_instance);
    }

#ifdef  __cplusplus
}
#endif

#endif // PROM_H

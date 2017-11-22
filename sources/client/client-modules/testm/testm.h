#ifndef PROM_H
#define PROM_H

#include <clientmodule.h>

#ifdef  __cplusplus
extern "C" {
#endif


    class Testm : public TestModule
    {
    public:
	Testm();

	static ClientModule* getModuleInstance()
	{
	    return new Testm;
	}

	static void destroyModuleInstance(ClientModule *in_module_instance)
	{
	    delete  in_module_instance;
	}

	virtual QString getModuleName();
	virtual QString getModuleVersion();
	virtual QString getModuleInformationsUrl();

	virtual QWidget* createView(QWidget *parent);

	virtual void loadTestModuleDatas(TestHierarchy *in_test);
	virtual void saveTestModuleDatas();

    private:
	TestHierarchy * _m_test;
    };

    CLIENT_MODULE_EXPORT ClientModule* create_module_instance()
    {
	return Testm::getModuleInstance();
    }

    CLIENT_MODULE_EXPORT void destroy_module_instance(ClientModule *in_module_instance)
    {
	Testm::destroyModuleInstance(in_module_instance);
    }

#ifdef  __cplusplus
}
#endif

#endif // PROM_H

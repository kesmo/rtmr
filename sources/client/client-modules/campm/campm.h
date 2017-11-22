#ifndef PROM_H
#define PROM_H

#include <clientmodule.h>

#ifdef  __cplusplus
extern "C" {
#endif


    class Campm : public CampaignModule
    {
    public:
	Campm();

	static ClientModule* getModuleInstance()
	{
	    return new Campm;
	}

	static void destroyModuleInstance(ClientModule *in_module_instance)
	{
	    delete  in_module_instance;
	}

	virtual QString getModuleName();
	virtual QString getModuleVersion();
	virtual QString getModuleInformationsUrl();

	virtual QWidget* createView(QWidget *parent);

	virtual void loadCampaignModuleDatas(Campaign *in_campaign);
	virtual void saveCampaignModuleDatas();

    private:
	Campaign * _m_campaign;
    };

    CLIENT_MODULE_EXPORT ClientModule* create_module_instance()
    {
	return Campm::getModuleInstance();
    }

    CLIENT_MODULE_EXPORT void destroy_module_instance(ClientModule *in_module_instance)
    {
	Campm::destroyModuleInstance(in_module_instance);
    }

#ifdef  __cplusplus
}
#endif

#endif // PROM_H

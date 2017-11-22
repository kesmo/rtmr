#include <QtGui>
#include <QNetworkProxy>

#include "form_options.h"
#include "session.h"

#include "../client-launcher/process_utils.h"

Form_Options::Form_Options(QWidget *parent) :
	QDialog(parent)
{
	_m_ui = new Ui_Form_Options;

	_m_ui->setupUi(this);
	_m_ui->modules_widget->setAttribute(Qt::WA_TranslucentBackground);
	_m_ui->no_modules_widget->setAttribute(Qt::WA_TranslucentBackground);

	connect(_m_ui->clear_prefs_button, SIGNAL(clicked()), this, SLOT(clearPreferences()));

	connect(_m_ui->bugtracker_type, SIGNAL(currentIndexChanged(int)), this, SLOT(updateBugtrackerView()));

	connect(_m_ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(_m_ui->buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	connect(_m_ui->proxy_none, SIGNAL(clicked()), this, SLOT(selectedProxyTypeChanged()));
	connect(_m_ui->proxy_socks5, SIGNAL(clicked()), this, SLOT(selectedProxyTypeChanged()));
	connect(_m_ui->proxy_http, SIGNAL(clicked()), this, SLOT(selectedProxyTypeChanged()));
	connect(_m_ui->proxy_http_only, SIGNAL(clicked()), this, SLOT(selectedProxyTypeChanged()));
	connect(_m_ui->proxy_ftp_only, SIGNAL(clicked()), this, SLOT(selectedProxyTypeChanged()));

	connect(_m_ui->check_new_versions_button, SIGNAL(clicked()), this, SLOT(checkNewVersions()));

	/* Paramètres de couleur */
        connect(_m_ui->test_execution_ok_button, SIGNAL(clicked()), this, SLOT(selectColorForExecutionTestOk()));
        connect(_m_ui->test_execution_ko_button, SIGNAL(clicked()), this, SLOT(selectColorForExecutionTestKo()));
        connect(_m_ui->test_execution_incompleted_button, SIGNAL(clicked()), this, SLOT(selectColorForExecutionTestIncompleted()));
        connect(_m_ui->test_execution_by_passed_button, SIGNAL(clicked()), this, SLOT(selectColorForExecutionTestBypassed()));

	initPrefsTab();
	readSettings();
}

Form_Options::~Form_Options()
{
	delete _m_ui;
}

void Form_Options::readSettings()
{
	QSettings tmp_settings("rtmr", "options");
	int tmp_proxy_type = QNetworkProxy::NoProxy;
	int tmp_proxy_capabilities = 0;
	bool tmp_proxy_use_system_config = tmp_settings.value("proxy_use_system_config", QVariant(false)).toBool();

	_m_ui->check_new_versions->setChecked(tmp_settings.value("versions_check_new", QVariant(false)).toBool());
	_m_ui->versions_url->setText(tmp_settings.value("versions_url", "http://rtmr.net").toString());
	_m_ui->proxy->setText(tmp_settings.value("proxy_address", "").toString());
	_m_ui->port->setText(tmp_settings.value("proxy_port", "").toString());
	_m_ui->login->setText(tmp_settings.value("proxy_login", "").toString());
	_m_ui->password->setText(tmp_settings.value("proxy_password", "").toString());

	if (!tmp_proxy_use_system_config)
	{
		tmp_proxy_type = tmp_settings.value("proxy_type", QVariant(QNetworkProxy::NoProxy)).toInt();
		switch (tmp_proxy_type)
		{
		case QNetworkProxy::Socks5Proxy:
			_m_ui->proxy_socks5->setChecked(true);
			break;

		case QNetworkProxy::HttpProxy:
			_m_ui->proxy_http->setChecked(true);
			break;

		case QNetworkProxy::HttpCachingProxy:
			_m_ui->proxy_http_only->setChecked(true);
			break;

		case QNetworkProxy::FtpCachingProxy:
			_m_ui->proxy_ftp_only->setChecked(true);
			break;

		default:
			_m_ui->proxy_none->setChecked(true);
			break;
		}
	}

	tmp_proxy_capabilities = tmp_settings.value("proxy_capabilities", QVariant(0)).toInt();
	_m_ui->proxy_tunneling->setChecked((tmp_proxy_capabilities & QNetworkProxy::TunnelingCapability)
			== QNetworkProxy::TunnelingCapability);
	_m_ui->proxy_listening->setChecked((tmp_proxy_capabilities & QNetworkProxy::ListeningCapability)
			== QNetworkProxy::ListeningCapability);
	_m_ui->proxy_udptunneling->setChecked((tmp_proxy_capabilities & QNetworkProxy::UdpTunnelingCapability)
			== QNetworkProxy::UdpTunnelingCapability);
	_m_ui->proxy_caching->setChecked((tmp_proxy_capabilities & QNetworkProxy::CachingCapability)
			== QNetworkProxy::CachingCapability);
	_m_ui->proxy_lookup->setChecked((tmp_proxy_capabilities & QNetworkProxy::HostNameLookupCapability)
			== QNetworkProxy::HostNameLookupCapability);

        /* Couleurs */
        _m_test_execution_ok_color = ExecutionTest::okColor();
        _m_test_execution_ko_color = ExecutionTest::koColor();
        _m_test_execution_incompleted_color = ExecutionTest::incompleteColor();
        _m_test_execution_by_passed_color = ExecutionTest::byPassedColor();

        setButtonColor(_m_test_execution_ok_color, _m_ui->test_execution_ok_button);
        setButtonColor(_m_test_execution_ko_color, _m_ui->test_execution_ko_button);
        setButtonColor(_m_test_execution_incompleted_color, _m_ui->test_execution_incompleted_button);
        setButtonColor(_m_test_execution_by_passed_color, _m_ui->test_execution_by_passed_button);

        /* Bugtrackers */
    QList<ClientModule*>	tmp_externals_modules = Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).values();

	_m_ui->modules_widget->setVisible(!tmp_externals_modules.isEmpty());
	_m_ui->no_modules_widget->setVisible(tmp_externals_modules.isEmpty());

	foreach(ClientModule *tmp_module, tmp_externals_modules)
	{
	    QString	tmp_module_name = tmp_module->getModuleName();

	    _m_ui->bugtracker_type->addItem(tmp_module_name);
        _m_bugtrackers_credentials[tmp_module_name] = Session::instance()->getBugtrackersCredentials()[tmp_module_name];
	}
	_m_ui->bugtracker_type->setCurrentIndex(0);
	_m_current_bugtracker_name = "";

	updateBugtrackerView();

    /* Onglet serveur */
    _m_ui->server_ping_interval->setValue(tmp_settings.value("server_keepalive_interval", 10).toInt());
    _m_ui->server_recv_timeout->setValue(tmp_settings.value("server_recv_timeout", 30).toInt());
    _m_ui->server_send_timeout->setValue(tmp_settings.value("server_send_timeout", 30).toInt());


    }

void Form_Options::accept()
{
	QSettings tmp_settings("rtmr", "options");
	QNetworkProxy tmp_proxy;
	QNetworkProxy::ProxyType tmp_proxy_type = QNetworkProxy::NoProxy;
	QFlags<QNetworkProxy::Capability> tmp_proxy_capabilities = 0;

	tmp_settings.setValue("versions_check_new", _m_ui->check_new_versions->isChecked());

	tmp_settings.setValue("versions_url", _m_ui->versions_url->text());

	tmp_settings.setValue("proxy_use_system_config", _m_ui->proxy_system_config->isChecked());

	tmp_settings.setValue("proxy_address", _m_ui->proxy->text());
	tmp_settings.setValue("proxy_port", _m_ui->port->text());
	tmp_settings.setValue("proxy_login", _m_ui->login->text());
	tmp_settings.setValue("proxy_password", _m_ui->password->text());

	if (_m_ui->proxy_system_config->isChecked())
	{
		QNetworkProxyFactory::setUseSystemConfiguration(true);
	}
	else
	{
		QNetworkProxyFactory::setUseSystemConfiguration(false);
		if (_m_ui->proxy->text().isEmpty())
		{
			tmp_proxy_type = QNetworkProxy::NoProxy;
		}
		else
		{
			if (_m_ui->proxy_none->isChecked())
				tmp_proxy_type = QNetworkProxy::NoProxy;
			else if (_m_ui->proxy_socks5->isChecked())
				tmp_proxy_type = QNetworkProxy::Socks5Proxy;
			else if (_m_ui->proxy_http->isChecked())
				tmp_proxy_type = QNetworkProxy::HttpProxy;
			else if (_m_ui->proxy_http_only->isChecked())
				tmp_proxy_type = QNetworkProxy::HttpCachingProxy;
			else if (_m_ui->proxy_ftp_only->isChecked())
				tmp_proxy_type = QNetworkProxy::FtpCachingProxy;

			if (_m_ui->proxy_tunneling->isChecked())
				tmp_proxy_capabilities = QNetworkProxy::TunnelingCapability;

			if (_m_ui->proxy_listening->isChecked())
				tmp_proxy_capabilities = tmp_proxy_capabilities | QNetworkProxy::ListeningCapability;

			if (_m_ui->proxy_udptunneling->isChecked())
				tmp_proxy_capabilities = tmp_proxy_capabilities | QNetworkProxy::UdpTunnelingCapability;

			if (_m_ui->proxy_caching->isChecked())
				tmp_proxy_capabilities = tmp_proxy_capabilities | QNetworkProxy::CachingCapability;

			if (_m_ui->proxy_lookup->isChecked())
				tmp_proxy_capabilities = tmp_proxy_capabilities | QNetworkProxy::HostNameLookupCapability;

			tmp_proxy.setCapabilities(tmp_proxy_capabilities);
			tmp_proxy.setHostName(_m_ui->proxy->text());
			tmp_proxy.setPort(_m_ui->port->text().toInt());
			if (!_m_ui->login->text().isEmpty())
			{
				tmp_proxy.setUser(_m_ui->login->text());
				tmp_proxy.setPassword(_m_ui->password->text());
			}
		}

		tmp_settings.setValue("proxy_type", QVariant(tmp_proxy_type));
		tmp_settings.setValue("proxy_capabilities", QVariant(tmp_proxy_capabilities));

		tmp_proxy.setType(tmp_proxy_type);
		QNetworkProxy::setApplicationProxy(tmp_proxy);
	}

        /* Couleurs */
	tmp_settings.setValue("test_execution_ok_color", _m_test_execution_ok_color);
	tmp_settings.setValue("test_execution_ko_color", _m_test_execution_ko_color);
	tmp_settings.setValue("test_execution_incompleted_color", _m_test_execution_incompleted_color);
	tmp_settings.setValue("test_execution_by_passed_color", _m_test_execution_by_passed_color);

	ExecutionTest::initDefaultColors(
			_m_test_execution_ok_color,
			_m_test_execution_ko_color,
			_m_test_execution_incompleted_color,
			_m_test_execution_by_passed_color);

        /* Bugtrackers */
        updateBugtrackerView();

    foreach(ClientModule *tmp_module, Session::instance()->externalsModules().value(ClientModule::BugtrackerPlugin).values())
	{
	    QString	tmp_module_name = tmp_module->getModuleName();
	    QString	tmp_setting_prefix = tmp_module_name.toLower().trimmed();

        Session::instance()->setBugtrackerCredential(tmp_module_name, _m_bugtrackers_credentials[tmp_module_name]);

	    tmp_settings.setValue(tmp_setting_prefix + "_login", _m_bugtrackers_credentials[tmp_module_name].first);
	    tmp_settings.setValue(tmp_setting_prefix + "_password", encrypt_str(_m_bugtrackers_credentials[tmp_module_name].second.toStdString().c_str()));
	}

    /* Onglet serveur */
    tmp_settings.setValue("server_keepalive_interval", _m_ui->server_ping_interval->value());
    tmp_settings.setValue("server_recv_timeout", _m_ui->server_recv_timeout->value());
    tmp_settings.setValue("server_send_timeout", _m_ui->server_send_timeout->value());

	QDialog::accept();
}

void Form_Options::selectedProxyTypeChanged()
{
	QNetworkProxy tmp_proxy;
	QNetworkProxy::ProxyType tmp_proxy_type = QNetworkProxy::NoProxy;
	QFlags<QNetworkProxy::Capability> tmp_proxy_capabilities = 0;

	if (_m_ui->proxy_none->isChecked())
		tmp_proxy_type = QNetworkProxy::NoProxy;
	else if (_m_ui->proxy_socks5->isChecked())
		tmp_proxy_type = QNetworkProxy::Socks5Proxy;
	else if (_m_ui->proxy_http->isChecked())
		tmp_proxy_type = QNetworkProxy::HttpProxy;
	else if (_m_ui->proxy_http_only->isChecked())
		tmp_proxy_type = QNetworkProxy::HttpCachingProxy;
	else if (_m_ui->proxy_ftp_only->isChecked())
		tmp_proxy_type = QNetworkProxy::FtpCachingProxy;

	tmp_proxy.setType(tmp_proxy_type);

	tmp_proxy_capabilities = tmp_proxy.capabilities();
	_m_ui->proxy_tunneling->setChecked((tmp_proxy_capabilities & QNetworkProxy::TunnelingCapability)
			== QNetworkProxy::TunnelingCapability);
	_m_ui->proxy_listening->setChecked((tmp_proxy_capabilities & QNetworkProxy::ListeningCapability)
			== QNetworkProxy::ListeningCapability);
	_m_ui->proxy_udptunneling->setChecked((tmp_proxy_capabilities & QNetworkProxy::UdpTunnelingCapability)
			== QNetworkProxy::UdpTunnelingCapability);
	_m_ui->proxy_caching->setChecked((tmp_proxy_capabilities & QNetworkProxy::CachingCapability)
			== QNetworkProxy::CachingCapability);
	_m_ui->proxy_lookup->setChecked((tmp_proxy_capabilities & QNetworkProxy::HostNameLookupCapability)
			== QNetworkProxy::HostNameLookupCapability);
}

void Form_Options::checkNewVersions()
{
    QProcess tmp_client_launcher;
    QString tmp_app_path = QApplication::applicationDirPath() + "/";

#if defined(__WINDOWS) ||  defined(WIN32)
    tmp_client_launcher.startDetached("\""+tmp_app_path+"rtmr.exe\" -check -url "+_m_ui->versions_url->text());
#else
#ifdef __APPLE__
    tmp_client_launcher.startDetached(tmp_app_path+"rtmr -check -url "+_m_ui->versions_url->text());
    QTimer::singleShot(1000, this, SLOT(bringClientLauncherToFront()));
#else
#if defined __linux__
    tmp_client_launcher.startDetached(tmp_app_path+"rtmr -check -url "+_m_ui->versions_url->text());
#endif // __linux__
#endif // __APPLE__
#endif //__WINDOWS

}


void Form_Options::bringClientLauncherToFront()
{
    ProcessUtils::isRunning("rtmr", true);
}

void Form_Options::selectColorForExecutionTestOk()
{
    pickColor(_m_test_execution_ok_color, _m_ui->test_execution_ok_button);
}


void Form_Options::selectColorForExecutionTestKo()
{
    pickColor(_m_test_execution_ko_color, _m_ui->test_execution_ko_button);
}


void Form_Options::selectColorForExecutionTestIncompleted()
{
    pickColor(_m_test_execution_incompleted_color, _m_ui->test_execution_incompleted_button);
}


void Form_Options::selectColorForExecutionTestBypassed()
{
    pickColor(_m_test_execution_by_passed_color, _m_ui->test_execution_by_passed_button);
}


void Form_Options::pickColor(QColor & inColor, QPushButton *inButton)
{
    QColor tmpColor = QColorDialog::getColor(inColor, this);

    if (tmpColor.isValid()) {
        inColor = tmpColor;
        setButtonColor(inColor, inButton);
    }
}


void Form_Options::setButtonColor(const QColor & inColor, QPushButton *inButton)
{
    QPixmap tmpPix = QPixmap(24, 24);

    tmpPix.fill(inColor);
    inButton->setIcon(QIcon(tmpPix));
}



void Form_Options::updateBugtrackerView()
{
    QString	    tmp_bugtracker_name = _m_ui->bugtracker_type->currentText();

    if (!_m_current_bugtracker_name.isEmpty())
    {
    	_m_bugtrackers_credentials[_m_current_bugtracker_name] = QPair<QString, QString>(_m_ui->bugtracker_login->text(), _m_ui->bugtracker_password->text());
    }

    if (!tmp_bugtracker_name.isEmpty())
    {
		_m_ui->bugtracker_groupbox->setTitle(tmp_bugtracker_name);

		_m_ui->bugtracker_login->setText(_m_bugtrackers_credentials[tmp_bugtracker_name].first);
		_m_ui->bugtracker_password->setText(_m_bugtrackers_credentials[tmp_bugtracker_name].second);

		_m_current_bugtracker_name = tmp_bugtracker_name;
    }
}


void Form_Options::initPrefsTab()
{
    QSettings tmp_settings("rtmr");
    QStringList tmp_prefs_list = tmp_settings.allKeys();

    QSettings tmp_options_settings("rtmr","options");
    QStringList tmp_options_prefs_list = tmp_options_settings.allKeys();
    QStringList	tmp_headers;

    QTableWidgetItem        *tmp_widget_item = NULL;

    int		    tmp_index = 0;

    tmp_headers << tr("Nom") << tr("Valeur") << tr("Groupe");

    _m_ui->user_prefs_list->clear();
    _m_ui->user_prefs_list->setRowCount(tmp_prefs_list.count()+tmp_options_prefs_list.count());
    _m_ui->user_prefs_list->setColumnCount(3);

    _m_ui->user_prefs_list->setHorizontalHeaderLabels(tmp_headers);

    _m_ui->user_prefs_list->setEditTriggers(QAbstractItemView::NoEditTriggers);

    foreach(QString tmp_pref, tmp_prefs_list)
    {
	tmp_widget_item = new QTableWidgetItem;
	tmp_widget_item->setText(tmp_pref);
	_m_ui->user_prefs_list->setItem(tmp_index, 0, tmp_widget_item);

	tmp_widget_item = new QTableWidgetItem;
	tmp_widget_item->setText(tmp_settings.value(tmp_pref, "").toString());
	_m_ui->user_prefs_list->setItem(tmp_index, 1, tmp_widget_item);

	tmp_widget_item = new QTableWidgetItem;
	tmp_widget_item->setText("");
	_m_ui->user_prefs_list->setItem(tmp_index, 2, tmp_widget_item);

	tmp_index++;
    }


    foreach(QString tmp_pref, tmp_options_prefs_list)
    {
	tmp_widget_item = new QTableWidgetItem;
	tmp_widget_item->setText(tmp_pref);
	_m_ui->user_prefs_list->setItem(tmp_index, 0, tmp_widget_item);

	tmp_widget_item = new QTableWidgetItem;
	tmp_widget_item->setText(tmp_options_settings.value(tmp_pref, "").toString());
	_m_ui->user_prefs_list->setItem(tmp_index, 1, tmp_widget_item);

	tmp_widget_item = new QTableWidgetItem;
        tmp_widget_item->setText(tr("options"));
	_m_ui->user_prefs_list->setItem(tmp_index, 2, tmp_widget_item);

	tmp_index++;
    }

    _m_ui->user_prefs_list->resizeColumnsToContents();
}



void Form_Options::clearPreferences()
{
    QSettings tmp_settings("rtmr");
    QSettings tmp_options_settings("rtmr","options");

    if (QMessageBox::question(this, tr("Confirmation..."), tr("Etes-vous sûr(e) de vouloir supprimer les préférences utilisateur ?"),QMessageBox::Yes | QMessageBox::No, QMessageBox::No ) == QMessageBox::Yes)
    {
	tmp_settings.clear();
	tmp_options_settings.clear();
    }

    initPrefsTab();
}

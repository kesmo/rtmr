#ifndef FORM_OPTIONS_H
#define FORM_OPTIONS_H

#include <QtGui/QDialog>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QAuthenticator>

#include "ui_Form_Options.h"

QT_FORWARD_DECLARE_CLASS(Form_Options);

class Form_Options : public QDialog
{
    Q_OBJECT

public:
    Form_Options(QWidget *parent = 0);
    ~Form_Options();

public Q_SLOTS:
    void accept();
    void selectedProxyTypeChanged();

    void checkNewVersions();

    void selectColorForExecutionTestOk();
    void selectColorForExecutionTestKo();
    void selectColorForExecutionTestIncompleted();
    void selectColorForExecutionTestBypassed();
    void pickColor(QColor & inColor, QPushButton *inButton);

    void updateBugtrackerView();

    void clearPreferences();

private:
    Ui_Form_Options *_m_ui;

    QColor                  _m_test_execution_ok_color;
    QColor                  _m_test_execution_ko_color;
    QColor                  _m_test_execution_incompleted_color;
    QColor                  _m_test_execution_by_passed_color;

    QMap<QString, QPair<QString, QString> >	_m_bugtrackers_credentials;

    QString	    _m_current_bugtracker_name;

    void setButtonColor(const QColor & inColor, QPushButton *inButton);

    void readSettings();
    void initPrefsTab();

private slots:
    void bringClientLauncherToFront();

};

#endif // FORM_OPTIONS_H

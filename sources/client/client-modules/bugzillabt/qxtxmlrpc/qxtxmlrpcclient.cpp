/*!
  @file
  QxtXmlRpc, a Qt4 library for XML-RPC network communication

  (C) 2006 Music Pictures, Ltd. (www.musicpictures.com)
  @author Tobias G. Pfeiffer (tgpfeiffer@web.de)
  @date 2007/02/05
  @version 0.3

  See http://qtnode.net/wiki/QxtXmlRpc or the README file for license terms.
*/

#include <QtCore>
#include <QNetworkProxy>

#include "qxtxmlrpcclient.h"
#include "qxtxmlrpcrequestcomposer.h"
#include "qxtxmlrpcresponseparser.h"

QxtXmlRpcClient::QxtXmlRpcClient(QUrl in_url)
{
    url = in_url;

    connect(&netMgr, SIGNAL(finished(QNetworkReply *)), SLOT(handleResponse(QNetworkReply *)));
    connect(&netMgr, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));
}


QxtXmlRpcClient::~QxtXmlRpcClient() {
}


QNetworkAccessManager *QxtXmlRpcClient::networkAccessManager()
{
    return &netMgr;
}


void QxtXmlRpcClient::setHost(const QString &host, bool useSecureHTTP, int port)
{
    url.setHost(host);
    url.setScheme(useSecureHTTP ? QLatin1String("https") : QLatin1String("http"));
    if (port)
	url.setPort(port);
    else
	url.setPort(useSecureHTTP ? 443 : 80);
}

QNetworkReply* QxtXmlRpcClient::call(QString procName, QList<QVariant> params) {
    QxtXmlRpcRequestComposer composer(procName);
    QVariant var;
    foreach(var, params) {
        composer.appendArgument( var );
    }

    QByteArray xml = composer.getXmlRequest().toUtf8();
    QNetworkRequest request(url);

    request.setRawHeader(QByteArray("Host"),  QString(url.host() + ((url.port() != -1) ? (QString(":").number(url.port())) : "" )).toUtf8());

    QPair<QString, QString> tmp_pair;
    foreach(tmp_pair, requestHeaderValues)
    {
	request.setRawHeader(tmp_pair.first.toUtf8(), tmp_pair.second.toUtf8().constData());
    }

    request.setRawHeader(QByteArray("User-Agent"), QByteArray("QxtXmlRpc"));
    request.setRawHeader(QByteArray("Content-Type"), QByteArray("text/xml"));

    return netMgr.post(request, xml);
}

void QxtXmlRpcClient::handleResponse(QNetworkReply *response) {
    if (response->error() != QNetworkReply::NoError)  {
	emit networkError(response->error(), response->errorString());
    }
    else{
        QString sdata = QString::fromUtf8( response->readAll() );
        try {
            QxtXmlRpcResponseParser rp(sdata);
            emit(finished( rp.returnValue() ));
        } catch (QxtXmlRpcFault f) {
            emit(fault(f.faultCode(), f.faultString(), sdata));
        } catch (QxtXmlRpcParseError e) {
            emit(parseError(e.errorMsg(), sdata));
        }
    }

    response->deleteLater();
}


void QxtXmlRpcClient::handleResponseHeader(const QHttpResponseHeader &in_response_header)
{
    responseHeaderValues = in_response_header.values();
}


QList<QPair<QString,QString> > QxtXmlRpcClient::responseHeaderValuesForKey(QString in_key)
{
    QList<QPair<QString,QString> >	    tmp_values;
    QPair<QString,QString>		    tmp_pair;

    foreach(tmp_pair, responseHeaderValues)
    {
	if (tmp_pair.first.startsWith(in_key))
	    tmp_values.append(tmp_pair);
    }

    return tmp_values;
}


void QxtXmlRpcClient::addRequestHeaderValueForKey(QPair<QString,QString> in_key_value)
{
    requestHeaderValues.append(in_key_value);
}


void QxtXmlRpcClient::removeAllRequestHeaderValues()
{
    requestHeaderValues.clear();
}


void QxtXmlRpcClient::authenticationRequired(QNetworkReply * /* in_response */,QAuthenticator *in_auth)
{
    if(url.userName().length() > 0) {
    	in_auth->setUser(url.userName());
	in_auth->setPassword(url.password());
    }
}

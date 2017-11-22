/*!
  @file
  QxtXmlRpc, a Qt4 library for XML-RPC network communication

  (C) 2006 Music Pictures, Ltd. (www.musicpictures.com)
  @author Tobias G. Pfeiffer (tgpfeiffer@web.de)
  @date 2007/02/05
  @version 0.3

  See http://qtnode.net/wiki/QxtXmlRpc or the README file for license terms.
*/

#ifndef QXTXMLRPCCLIENT_H
#define QXTXMLRPCCLIENT_H

#include <QtCore>
#include <QtNetwork>

//! main class; use this for composing your calls
/*! This class is the one to be used for doing XML-RPC calls. Call the
    constructor with the URL of an XML-RPC server. Then, for each
    individual call, create a QList<QVariant> that contains your function
    parameters for the remote procedure and execute the call() method.
    This will return an integer uniquely identifying this call; this will be
    the first parameter of all of the signals that will be emitted when the
    server's response arrives. Important: One (and only one) of the four
    signals *will* be emitted for each call. */
class QxtXmlRpcClient : public QObject {
  Q_OBJECT
  private:
      QUrl				url;
      QNetworkAccessManager		netMgr;
      //QMap<int, QIODevice*>		requests;
      QList<QPair<QString, QString> >	responseHeaderValues;
      QList<QPair<QString, QString> >	requestHeaderValues;

  public:
    //! constructor taking the server url (keep the trailing slash!!!)
    QxtXmlRpcClient(QUrl in_url);

    QList< QPair <QString,QString> > responseHeaderValuesForKey(QString in_key);

    void addRequestHeaderValueForKey(QPair<QString,QString> in_key_value);
    void removeAllRequestHeaderValues();

    QUrl xmlRpxUrl(){return url;};

    QNetworkAccessManager *networkAccessManager();

    void setHost(const QString &host, bool useSecureHTTP = false, int port = 0);

    //! destructor
    ~QxtXmlRpcClient();
  private slots:
    void handleResponse(QNetworkReply *);
    void handleResponseHeader(const QHttpResponseHeader &in_response_header);
    void authenticationRequired(QNetworkReply *in_response,QAuthenticator *in_auth);

  public slots:
    //! calls the given RPC procedure
    QNetworkReply* call(QString procName, QList<QVariant> params);
  signals:
    //! is emitted when RPC call has finished successfully
    void finished(QVariant retValue);
    //! is emitted on error during call
    void networkError(QNetworkReply::NetworkError error, QString errorMsg);
    //! is emitted on XML-RPC fault
    void fault(int faultCode, QString faultString, QString response);
    //! is emitted on XML parsing error
    void parseError(QString errorMsg, QString response);


};

#endif

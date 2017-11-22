/*!
  @file
  QxtXmlRpc, a Qt4 library for XML-RPC network communication

  (C) 2006 Music Pictures, Ltd. (www.musicpictures.com)
  @author Tobias G. Pfeiffer (tgpfeiffer@web.de)
  @date 2007/02/05
  @version 0.3

  See http://qtnode.net/wiki/QxtXmlRpc or the README file for license terms.
*/

#ifndef QxtXmlRpcRESPONSEPARSER_H
#define QxtXmlRpcRESPONSEPARSER_H

#include <QtCore>
#include <QtXml>

//! takes in a XML-RPC response string and makes the results available
class QxtXmlRpcResponseParser {
    private:
        QString xmldata;
        QDomDocument doc;
        QVariant retVal;
        //! this converts a XML subtree to the corresponding structure of QVariants
        void parseSubtree(QDomNode node, QList<QVariant>* parent);
    public:
        //! constructor; takes in the obtained XML-RPC data
        QxtXmlRpcResponseParser(const QString& responseData);
        //! returns the i-th argument
        QVariant returnValue() { return retVal; }
};


//! Exception that is thrown on XML parsing error
class QxtXmlRpcParseError : public std::exception {
    private:
        QString fmsg;
    public:
        //! Constructor
        QxtXmlRpcParseError(const QString& msg) : fmsg(msg) {}
        //! returns whatever message was given at instantiation time
        virtual const char* what() const throw() {
            return QString("XML parse Error: %1").arg(fmsg).toUtf8().constData();
        }
        //! returns whatever message was given at instantiation time as a QString
        QString errorMsg() const { return fmsg; }
        //! Destructor
        ~QxtXmlRpcParseError() throw() {}
};

//! Exception that is thrown on XML-RPC fault
class QxtXmlRpcFault : public std::exception {
    private:
        int fcode;
        QString fmsg;
    public:
        //! Constructor
        QxtXmlRpcFault(int code, const QString& msg) : fcode(code), fmsg(msg) {}
        //! returns the fault code
        int faultCode() const { return fcode; }
        //! returns the fault message
        QString faultString() const { return fmsg; }
        //! returns whatever message was given at instantiation time
        virtual const char* what() const throw() {
            return QString("XML-RPC Fault: %1 (%2)").arg(fcode).arg(fmsg).toUtf8().constData();
        }
        //! Destructor
        ~QxtXmlRpcFault() throw() {}
};

#endif

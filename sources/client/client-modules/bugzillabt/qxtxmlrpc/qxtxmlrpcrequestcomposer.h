/*!
  @file
  QxtXmlRpc, a Qt4 library for XML-RPC network communication

  (C) 2006 Music Pictures, Ltd. (www.musicpictures.com)
  @author Tobias G. Pfeiffer (tgpfeiffer@web.de)
  @date 2007/02/05
  @version 0.3

  See http://qtnode.net/wiki/QxtXmlRpc or the README file for license terms.
*/

#ifndef QxtXmlRpcREQUESTCOMPOSER_H
#define QxtXmlRpcREQUESTCOMPOSER_H

#include <QtCore>
#include <QtXml>

//! composes a string conforming to XML-RPC spec that represents a function call
class QxtXmlRpcRequestComposer {
    private:
        QString procName;
        QList<QVariant> args;
        QDomDocument doc;
        //! recursive function to construct XML data out of QVariants
        void appendDom(const QVariant& var, QDomNode* parent);
    public:
        //! constructor; takes in the name of the procedure to be called
        QxtXmlRpcRequestComposer(const QString& procedureName) 
            : procName(procedureName) {}
        //! makes the i-th argument a variable of the given type
        void appendArgument(const QVariant& arg);
        //! composes the XML-RPC string
        QString getXmlRequest();
};

#endif

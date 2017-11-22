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
#include <QtXml>
#include <QtGlobal>

#include "qxtxmlrpcresponseparser.h"
#include "qxtxmlrpcrequestcomposer.h"
#include "qxtxmlrpcclient.h"

//! prints out a representation of the given QVariant (to be removed)
void print(const QVariant&, QTextStream&);

/*!
    Constructor of the QxtXmlRpcResponseParser class. Takes in an XML-RPC methodResponse string confirming to the specification defined at http://www.xmlrpc.com/spec and converts the contained data to a list of QVariants that can later be fetched with argument().
*/
QxtXmlRpcResponseParser::QxtXmlRpcResponseParser(const QString& responseData)
    : xmldata(responseData) {
    QString* errorMsg = new QString();
    bool parseSuccess = doc.setContent(xmldata, false, errorMsg);
    if (!parseSuccess) {
        // throw error: syntax error in xml data
        throw QxtXmlRpcParseError(*errorMsg);
    }
    QDomNode parseNode;
    QDomElement root = doc.documentElement();
    if (root.tagName() != "methodResponse") {
        // throw error: root element of an XML-RPC response must be <methodResponse>
        qDebug("document element has bad name");
        return;
    }
    QDomElement paramsElem = root.firstChildElement();
    QDomNodeList children;
    if (paramsElem.tagName() == "params") {
        // structure is /params/param/value
        QDomNode currentParam = paramsElem.firstChildElement("param");
        QDomElement currentValue = currentParam.firstChildElement();
        if (currentValue.tagName() != "value") {
            // throw error: param must contain exactly one <value> element
            qDebug("error, structure is %s/%s", currentParam.nodeName().toStdString().c_str(), currentValue.tagName().toStdString().c_str());
            return;
        }
        parseNode = currentValue;
    } else if (paramsElem.tagName() == "fault") {
        // structure is /fault/value
        children = paramsElem.childNodes();
        // push only values onto the stack
        for (int i=children.size()-1; i>=0; i--) {
            QDomNode current = children.at(i);
            if (current.isElement()) {
                parseNode = current;
                break;
            }
        }
    } else {
        // throw error: no other elements than <params> or <fault> accepted here
        qDebug("params element has bad name");
        return;
    }
    QList<QVariant> args;
    parseSubtree(parseNode, &args);
    retVal = args.takeFirst();
    /*QTextStream qout(stdout);
    qout << "====================================\n";*/
    if (paramsElem.tagName() == "fault") {
        QMap<QString, QVariant> errMap = retVal.toMap();
        int code = errMap["faultCode"].toInt();
        QString msg = errMap["faultString"].toString();
        throw QxtXmlRpcFault(code, msg);
    }
    /*print(retVal, qout);
    qout << "\n";
    qout.flush();*/
}

/*!
    This function takes in a QVariant and prints an appropriate description of it to the given QTextStream. It handles QLists and QMaps correctly.
*/
void print(const QVariant& item, QTextStream& qout) {
    if (item.canConvert( QVariant::String )) {
        qout << item.toString();
    } else if (item.canConvert( QVariant::List )) {
        qout << "[";
        QVariant item2;
        foreach(item2, item.toList()) {
            print(item2, qout);
            qout << ", ";
        }
        qout << "]";
    } else if (item.canConvert( QVariant::Map )) {
        qout << "{";
        QList<QString> keys = item.toMap().keys();
        QString key;
        foreach(key, keys) {
            qout << key << " -> ";
            print((item.toMap())[key], qout);
            qout << ", ";
        }
        qout << "}";
    }
}

/*!
    This function takes in a QDomNode that represents a <value> node in the XML-RPC response, recognizes what data type is stored within this node and appends this to the local list of arguments. It handles arrays and structs correctly and nests them within QVariants.
*/
void QxtXmlRpcResponseParser::parseSubtree( QDomNode node, QList<QVariant>* parent ) {
    // value *must* contain one of <i4>, <int>, <boolean>, <string>, <double>,
    // <dateTime.iso8601>, <base64>, <struct>, <array> or simple text (i.e. string)
    QDomElement child = node.firstChildElement();
    QString val = child.text();
    if (node.parentNode().parentNode().nodeName() == "struct") {
    }
    if (child.tagName() == "i4" || child.tagName() == "int") {
        parent->append( QVariant(val.toInt()));
    } else if (child.tagName() == "boolean") {
        if (val == "0") {  parent->append(QVariant(false)); }
        else            {  parent->append(QVariant(true)); }
    } else if (child.tagName() == "string") {
        parent->append(QVariant(val));
    } else if (child.tagName() == "double") {
        parent->append(QVariant(val.toDouble( )));
    } else if (child.tagName() == "dateTime.iso8601") {
        parent->append(QVariant(QDateTime::fromString( val, "yyyyMMddTHH:mm:ss")));
    } else if (child.tagName() == "base64") {
        parent->append(QVariant(QByteArray::fromBase64(val.toAscii())));
    } else if (child.isNull()) {
        if (!node.toElement().isNull()) {
            parent->append(QVariant(node.toElement().text()));
        } else {
            // throw error
            qDebug("i could not find valid data!");
        }
    } else if (child.tagName() == "array") {
        QList<QVariant> array;
        QDomElement grandchild = child.firstChildElement();
        if (grandchild.tagName() != "data") {
            // throw error: only <data> tag is allowed here
            qDebug("bad structure");
            return;
        }
        QDomNodeList grandchildren = grandchild.childNodes();
        for (int i=0; i<grandchildren.size(); i++) {
            QDomNode cur = grandchildren.at(i);
            if (cur.isElement()) {
                parseSubtree(cur, &array );
            }
        }
        parent->append(QVariant(array));
    } else if (child.tagName() == "struct") {
        QList<QString> keys;
        QList<QVariant> values;
        QDomNodeList children = child.childNodes();
        for (int i=0; i<children.size(); i++) {
            QDomNode cur = children.at(i); // cur should be a <member> tag
            if (cur.isElement() && cur.nodeName() == "member") {
                QDomElement nameChild = cur.firstChildElement("name");
                QDomElement valueChild = cur.firstChildElement("value");
                if (!nameChild.isNull() && !valueChild.isNull()) {
                    keys.append(nameChild.text());
                    parseSubtree(valueChild, &values );
                } else {
                    // throw error: violated <struct> structure
                    qDebug( "bad <struct> structure");
                }
            }
        }
        QMap<QString, QVariant> structs;
        for (int i=0; i<keys.size(); i++) {
            structs[keys.at(i)] = values.at(i);
        }
        parent->append(QVariant(structs));
    }
}

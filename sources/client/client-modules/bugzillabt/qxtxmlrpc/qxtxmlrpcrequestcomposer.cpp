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

#include "qxtxmlrpcrequestcomposer.h"

/*!
  This appends a given QVariant to the list of objects that are to be serialized to XML-RPC later on.
  \param arg The object to be appended to the parameter list. May be int, bool, QString, double, QDateTime, QByteArray, QList<QVariant> or QMap<QString, QVariant>.
*/
void QxtXmlRpcRequestComposer::appendArgument(const QVariant& arg) {
    args.append(arg);
}


QString QxtXmlRpcRequestComposer::getXmlRequest() {
    doc.setContent(QString("<?xml version=\"1.0\" encoding=\"utf-8\"?>"));
    QDomElement root = doc.createElement("methodCall");
    QDomElement methodName = doc.createElement("methodName");
    QDomText methodNameNode = doc.createTextNode(procName);
    QDomElement params = doc.createElement("params");
    doc.appendChild(root);
    root.appendChild(methodName);
    methodName.appendChild(methodNameNode);
    root.appendChild(params);
    QVariant val;
    foreach(val, args) {
        QDomElement param = doc.createElement("param");
        params.appendChild(param);
        appendDom( val, &param );
    }
    return doc.toString();
}

void QxtXmlRpcRequestComposer::appendDom(const QVariant& var, QDomNode* parent) {
    QDomElement valNode = doc.createElement("value");
    parent->appendChild(valNode);
    QDomElement node;
    if (var.type() == QVariant::Int) {
        node = doc.createElement("int");
        QDomText contentNode = doc.createTextNode(var.toString());
        node.appendChild(contentNode);
    } else if (var.type() == QVariant::Bool) {
        node = doc.createElement("boolean");
        QDomText contentNode = doc.createTextNode(QString::number(var.toInt()));
        node.appendChild(contentNode);
    } else if (var.type() == QVariant::String) {
        node = doc.createElement("string");
        QDomText contentNode = doc.createTextNode(var.toString());
        node.appendChild(contentNode);
    } else if (var.type() == QVariant::Double) {
        node = doc.createElement("double");
        QDomText contentNode = doc.createTextNode(var.toString());
        node.appendChild(contentNode);
    } else if (var.type() == QVariant::DateTime) {
        node = doc.createElement("dateTime.iso8601");
        QDomText contentNode = doc.createTextNode(var.toDateTime().toString("yyyyMMddTHH:mm:ss"));
        node.appendChild(contentNode);
    } else if (var.type() == QVariant::ByteArray) {
        node = doc.createElement("base64");
        QDomText contentNode = doc.createTextNode(var.toByteArray().toBase64());
        node.appendChild(contentNode);
    } else if (var.type() == QVariant::List) {
        node = doc.createElement("array");
        QDomElement contentNode = doc.createElement("data");
        QList<QVariant> list = var.toList();
        for (int i=0; i<list.size(); i++) {
            appendDom(list.at(i), &contentNode);
        }
        node.appendChild(contentNode);
    } else if (var.type() == QVariant::Map) {
        node = doc.createElement("struct");
        QMap<QString, QVariant> map = var.toMap();
        QString key;
        foreach(key, map.keys()) {
            QDomElement memberNode = doc.createElement("member");
            QDomElement memberNameNode = doc.createElement("name");
            QDomText memberNameText = doc.createTextNode(key);
            memberNameNode.appendChild(memberNameText);
            memberNode.appendChild(memberNameNode);
            appendDom(map[key], &memberNode);
            node.appendChild(memberNode);
        }
    }
    valNode.appendChild(node);
}

/*****************************************************************************
Copyright (C) 2012 Emmanuel Jorge ejorge@free.fr

This file is part of R.T.M.R.

R.T.M.R is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

R.T.M.R is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with R.T.M.R.  If not, see <http://www.gnu.org/licenses/>.
*****************************************************************************/


#include <QApplication>
#include <QTextList>
#include <QColor>
#include <QColorDialog>
#include <QClipboard>
#include <QUrl>
#include <QStringListModel>
#include <QTextCursor>
#include <QKeyEvent>
#include <QScrollBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextDocumentFragment>
#include <QDomDocument>
#include <QDomNodeList>
#include <QDomNode>
#include "record_text_edit.h"


RecordTextEdit::RecordTextEdit(QWidget * parent) : QTextBrowser(parent)
{
    _m_completer = NULL;
    _m_record = NULL;

    setReadOnly(false);
    setUndoRedoEnabled(true);
    setOpenExternalLinks(true);

    document()->setIndentWidth(20.0);
    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardDataChanged()));
}


RecordTextEdit::~RecordTextEdit()
{
    delete _m_completer;
}


void RecordTextEdit::setRecord(Record *in_record)
{
    _m_record = in_record;
}


void RecordTextEdit::focusInEvent(QFocusEvent *in_event)
{
    if (_m_completer != NULL)
        _m_completer->setWidget(this);

    if (m_text_edit_tool_bar != NULL)
        m_text_edit_tool_bar->connectToEditor(this);

    if (_m_record != NULL && in_event->reason() == Qt::MouseFocusReason)
    {
        if ((QApplication::mouseButtons() & Qt::LeftButton))
            emit focused(_m_record);
        else
            return;
    }

    QTextEdit::focusInEvent(in_event);

}

void RecordTextEdit::textBold()
{
    QTextCharFormat fmt;
    if (m_text_edit_tool_bar != NULL && m_text_edit_tool_bar->boldAction != NULL)
        fmt.setFontWeight(m_text_edit_tool_bar->boldAction->isChecked() ? QFont::Bold : QFont::Normal);

    mergeFormatOnWordOrSelection(fmt);
}

void RecordTextEdit::textUnderline()
{
    QTextCharFormat fmt;
    if (m_text_edit_tool_bar != NULL && m_text_edit_tool_bar->underlineAction != NULL)
        fmt.setFontUnderline(m_text_edit_tool_bar->underlineAction->isChecked());

    mergeFormatOnWordOrSelection(fmt);
}


void RecordTextEdit::addLink()
{
    QTextCharFormat fmt;
    bool tmp_url_ok = true;
    if (m_text_edit_tool_bar != NULL && m_text_edit_tool_bar->underlineAction != NULL)
    {
	if (textCursor().charFormat().isAnchor())
	{
	    fmt.setAnchor(false);
	    fmt.setFontUnderline(false);
	    fmt.setForeground(QColor(Qt::black));
	    setFormatOnSelection(fmt);
	}
	else
	{
	    bool ok;
	    QString tmp_link_label = textCursor().selectedText();
	    if (!tmp_link_label.isEmpty())
	    {
		QString tmp_link_url = QInputDialog::getText(this, tr("Ajout d'un lien"),
						  tr("Veuillez saisir l'adresse du lien :"), QLineEdit::Normal,
						  tmp_link_label, &ok);
		if (ok && !tmp_link_url.isEmpty())
		{
		    if(!tmp_link_url.contains("://"))
		    {
			if (!tmp_link_url.contains(QRegExp("${*}", Qt::CaseInsensitive, QRegExp::Wildcard)))
			    tmp_url_ok = false;
		    }

		    if (tmp_url_ok)
		    {
			fmt.setAnchor(true);
			fmt.setAnchorHref(tmp_link_url);
			fmt.setFontUnderline(true);
			fmt.setForeground(QColor(Qt::blue));
			fmt.setToolTip(tmp_link_url);
			setFormatOnSelection(fmt);
		    }
		    else
		    {
			QMessageBox::information(this, tr("Ajout d'un lien"), tr("L'adresse saisie ne semble pas correcte.<br>" \
										 "P.S : Vous pouvez utiliser un paramètre de projet (ex : <i>${application.adresse}</i>)" ));
		    }
		}
	    }
	    else
	    {
		QMessageBox::information(this, tr("Ajout d'un lien"), tr("Veuillez sélectionner le texte qui sera le label du lien hypertexte."));
	    }
	}
    }

    setFocus();
}


void RecordTextEdit::textItalic()
{
    QTextCharFormat fmt;
    if (m_text_edit_tool_bar != NULL && m_text_edit_tool_bar->italicAction != NULL)
        fmt.setFontItalic(m_text_edit_tool_bar->italicAction->isChecked());

    mergeFormatOnWordOrSelection(fmt);
}

void RecordTextEdit::textStrikeOut()
{
    QTextCharFormat fmt;
    if (m_text_edit_tool_bar != NULL && m_text_edit_tool_bar->strikeAction != NULL)
        fmt.setFontStrikeOut(m_text_edit_tool_bar->strikeAction->isChecked());

    mergeFormatOnWordOrSelection(fmt);
}


void RecordTextEdit::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void RecordTextEdit::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void RecordTextEdit::indentLeft()
{
    indentBy(-1);
}

void RecordTextEdit::indentRight()
{
    indentBy(1);
}

void RecordTextEdit::indentBy(int indent)
{
    QTextCursor             tmp_cursor = textCursor();
    QTextBlockFormat        tmp_block_format = tmp_cursor.blockFormat();
    QTextListFormat         tmp_list_format;
    QTextListFormat::Style  tmp_style = QTextListFormat::ListDisc;
    int                     tmp_indent = tmp_block_format.indent() + indent;

    switch (tmp_indent)
    {
    default:
    case 1:
	tmp_style = QTextListFormat::ListDisc;
	break;
    case 2:
	tmp_style = QTextListFormat::ListCircle;
	break;
    case 3:
	tmp_style = QTextListFormat::ListSquare;
	break;
    case 4:
	tmp_style = QTextListFormat::ListDecimal;
	break;
    case 5:
	tmp_style = QTextListFormat::ListLowerAlpha;
	break;
    case 6:
	tmp_style = QTextListFormat::ListUpperAlpha;
	break;
    }


    tmp_cursor.beginEditBlock();

    tmp_block_format.setIndent(tmp_indent);

    if (tmp_cursor.currentList())
    {
        tmp_list_format = tmp_cursor.currentList()->format();
    }

    tmp_cursor.setBlockFormat(tmp_block_format);
    tmp_list_format.setStyle(tmp_style);
    tmp_cursor.createList(tmp_list_format);

    tmp_cursor.endEditBlock();
    setFocus();
}



void RecordTextEdit::setTextColor()
{
    QColor col = QColorDialog::getColor(textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    if (m_text_edit_tool_bar != NULL)
    	m_text_edit_tool_bar->colorChanged(col);
}

void RecordTextEdit::textAlignLeft()
{
    setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    setFocus();
}


void RecordTextEdit::textAlignRight()
{
    setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    setFocus();
}

void RecordTextEdit::textAlignCenter()
{
    setAlignment(Qt::AlignHCenter);
    setFocus();
}


void RecordTextEdit::textJustify()
{
    setAlignment(Qt::AlignJustify);
    setFocus();
}


void RecordTextEdit::clipboardDataChanged()
{
    if (m_text_edit_tool_bar != NULL && m_text_edit_tool_bar->pasteAction != NULL)
	m_text_edit_tool_bar->pasteAction->setEnabled(!QApplication::clipboard()->text().isEmpty());
}


void RecordTextEdit::setFormatOnSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();
    if (cursor.hasSelection())
	cursor.select(QTextCursor::WordUnderCursor);
    cursor.setCharFormat(format);
    setCurrentCharFormat(format);
    setFocus();
}


void RecordTextEdit::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    mergeCurrentCharFormat(format);
    setFocus();
}


bool RecordTextEdit::canInsertFromMimeData( const QMimeData *source ) const
{
    if (source->hasImage())
	return true;
    else
	return QTextEdit::canInsertFromMimeData(source);
}


void RecordTextEdit::insertFromMimeData( const QMimeData *source )
{
    if (source->hasImage())
    {
	QVariant tmp_image_data = source->imageData();
	QImage image = qvariant_cast<QImage>(tmp_image_data);
	QTextCursor cursor = this->textCursor();
	QTextDocument *document = this->document();
    QString imageName = QString("resource_%1_%2").arg(image.format()).arg(_m_resources.count());
	_m_resources[imageName] = image;
	document->addResource(QTextDocument::ImageResource, QUrl(imageName), tmp_image_data);

	QTextImageFormat tmp_image_format;
	tmp_image_format.setName(imageName);
	tmp_image_format.setWidth(48);
	tmp_image_format.setHeight(48 * image.height() / image.width());
	cursor.insertImage(imageName);
    }
    else
    {
	QTextEdit::insertFromMimeData(source);
    }
}



QMap<QString, QImage> RecordTextEdit::images()
{
    return _m_resources;
}



void RecordTextEdit::setCompletionFromList(QStringList in_completion_list)
{
    QCompleter              *tmp_completer = NULL;

    if (in_completion_list.isEmpty() == false)
    {
        tmp_completer = new QCompleter(this);
        tmp_completer->setModel(new QStringListModel(in_completion_list, tmp_completer));
        setCompleter(tmp_completer);
    }
}


void RecordTextEdit::setCompleter(QCompleter *completer)
{
    if (_m_completer != NULL)
        disconnect(_m_completer, 0, this, 0);

    _m_completer = completer;

    if (_m_completer == NULL)
        return;

    _m_completer->setWidget(this);
    _m_completer->setCompletionMode(QCompleter::PopupCompletion);
    _m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    connect(_m_completer, SIGNAL(activated(const QString&)), this, SLOT(insertCompletion(const QString&)));
}

QCompleter *RecordTextEdit::completer() const
{
    return _m_completer;
}

void RecordTextEdit::insertCompletion(const QString& completion)
{
    if (_m_completer->widget() != this)
        return;

    QTextCursor tc = textCursor();

    tc.movePosition(QTextCursor::StartOfWord);
    QTextCharFormat tmp_format = tc.charFormat();

    tc.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);

    tc.insertHtml("${<span style=\"background-color:#CCCCCC;\">" + completion + "</span>}");

    tc.movePosition(QTextCursor::EndOfWord);
    tc.setCharFormat(tmp_format);

    setTextCursor(tc);
}

QString RecordTextEdit::textUnderCursor() const
{
    QTextCursor tc = textCursor();
    tc.select(QTextCursor::WordUnderCursor);
    return tc.selectedText();
}


void RecordTextEdit::keyPressEvent(QKeyEvent *in_event)
{
    if (_m_completer && _m_completer->popup()->isVisible())
    {
	// The following keys are forwarded by the completer to the widget
        switch (in_event->key())
        {
	case Qt::Key_Enter:
	case Qt::Key_Return:
	case Qt::Key_Escape:
	case Qt::Key_Tab:
	case Qt::Key_Backtab:
	    in_event->ignore();
	    return; // let the completer do default behavior
	default:
	    break;
        }
    }

    bool isShortcut = ((in_event->modifiers() & Qt::ControlModifier) && in_event->key() == Qt::Key_Space); // CTRL+Space
    if (!_m_completer || !isShortcut || isReadOnly()) // dont process the shortcut when we have a completer
	QTextEdit::keyPressEvent(in_event);

    const bool ctrlOrShift = in_event->modifiers() & (Qt::ControlModifier | Qt::ShiftModifier);
    if (!_m_completer || (ctrlOrShift && in_event->text().isEmpty()))
	return;

    static QString eow("~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-="); // end of word
    bool hasModifier = (in_event->modifiers() != Qt::NoModifier) && !ctrlOrShift;
    QString completionPrefix = textUnderCursor();


    if (!isShortcut && (hasModifier || in_event->text().isEmpty()|| completionPrefix.length() < 1
			|| eow.contains(in_event->text().right(1))))
    {
	_m_completer->popup()->hide();
	return;
    }

    if (completionPrefix != _m_completer->completionPrefix())
    {
	_m_completer->setCompletionPrefix(completionPrefix);
	_m_completer->popup()->setCurrentIndex(_m_completer->completionModel()->index(0, 0));
    }
    QRect cr = cursorRect();
    cr.setWidth(_m_completer->popup()->sizeHintForColumn(0) + _m_completer->popup()->verticalScrollBar()->sizeHint().width());
    _m_completer->complete(cr); // popup it up!
}



QList<QString> RecordTextEdit::parametersList(const QString &text)
{
    int				tmp_start_index = 0, tmp_end_index = 0;
    QString			tmp_text = text, tmp_parameter;
    QList<QString>          tmp_params_list;

    while (tmp_start_index >= 0 && tmp_end_index >= 0)
    {
	tmp_start_index = tmp_text.indexOf("${", tmp_end_index);
	if (tmp_start_index >= 0)
	{
	    tmp_end_index = tmp_text.indexOf("}", tmp_start_index);
	    if (tmp_end_index >= 0)
	    {
		tmp_parameter = removeHtmlTags(tmp_text.mid(tmp_start_index + 2, (tmp_end_index - tmp_start_index - 2)));
		if (!tmp_parameter.isEmpty())
		    tmp_params_list.append(tmp_parameter);
	    }
	}
    }

    return tmp_params_list;
}


QString RecordTextEdit::removeHtmlTags(const QString &text)
{
    int				tmp_start_index = 0, tmp_end_index = 0;
    QString			tmp_text = text;

    while (tmp_start_index >= 0 && tmp_end_index >= 0)
    {
	tmp_start_index = tmp_text.indexOf("<", tmp_end_index);
	if (tmp_start_index >= 0)
	{
	    tmp_end_index = tmp_text.indexOf(">", tmp_start_index);
	    if (tmp_end_index >= 0)
	    {
		tmp_text.remove(tmp_start_index, tmp_end_index - tmp_start_index + 1);
		tmp_end_index = 0;
	    }
	    tmp_start_index = 0;
	}
    }

    return tmp_text;

}

QString RecordTextEdit::toHtmld() const
{
    QDomDocument tmp_doc;
    tmp_doc.setContent(QTextBrowser::toHtml());
    QDomNodeList tmp_links_list = tmp_doc.elementsByTagName("a");

    for(int tmp_node_index = 0; tmp_node_index < tmp_links_list.count(); tmp_node_index++)
    {
	QDomElement tmp_link = tmp_links_list.item(tmp_node_index).toElement();
	if (!tmp_link.isNull())
	{
	    qDebug() << "texte = " << tmp_link.text() << "\n";
	    QDomText tmp_text = tmp_doc.createTextNode(tmp_link.text().trimmed());
	    QString tmp_href = tmp_link.attribute("href");
	    QDomNode old;

	    QDomElement tmp_child = tmp_link.firstChildElement();
	    while (!tmp_child.isNull()) {
		qDebug() << "suppression de " << tmp_child.tagName() << "\n";
		old = tmp_link.removeChild(tmp_child);
		tmp_child = tmp_child.nextSiblingElement();
	    }

	    tmp_link.appendChild(tmp_text);
	    tmp_link.setAttribute("href", tmp_href);
	}
    }

    return tmp_doc.toString(0).remove('\n');
}


void RecordTextEdit::setReadOnly(bool readonly)
{
    QTextEdit::setReadOnly(readonly);

    if (readonly)
	setStyleSheet(QTEXTEDIT_UNWRITABLE);
    else
	setStyleSheet(QTEXTEDIT_WRITABLE);

}

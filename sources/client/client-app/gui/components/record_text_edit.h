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


#ifndef RECORD_TEXT_EDIT_H_
#define RECORD_TEXT_EDIT_H_

#include <QWidget>
#include <QTextBrowser>
#include <QMimeData>
#include <QCompleter>
#include "record_text_edit_container.h"
#include "executioncampaign.h"
#include "parameter.h"

#define QTEXTEDIT_WRITABLE "QTextEdit {background: #FFFFFF;border: 1px solid #4080b2; color: black;}"
#define QTEXTEDIT_UNWRITABLE "QTextEdit {background: #FFFFFF;dfe0e8: color: gray;}"

#define QPLAINTEXTEDIT_WRITABLE "QPlainTextEdit {background: #FFFFFF;border: 1px solid #4080b2; color: black;}"
#define QPLAINTEXTEDIT_UNWRITABLE "QPlainTextEdit {background: #FFFFFF;dfe0e8: color: gray;}"

class RecordTextEdit : public QTextBrowser, public RecordTextEditContainer
{
    Q_OBJECT

public:
    RecordTextEdit(QWidget * parent = 0);
    ~RecordTextEdit();

    void setReadOnly(bool ro);

    QString toHtmld() const;

    void setFormatOnSelection(const QTextCharFormat &format);
    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void setCompletionFromList(QStringList in_completion_list);

    template <class T>
    static QString toHtmlWithParametersValues(QList<T*> in_parameters_list, const QString &text)
    {
	    int				tmp_start_index = 0, tmp_end_index = 0;
	    QString			tmp_text = text, tmp_parameter;
	    const char		*tmp_param_value = NULL;

	    while (tmp_start_index >= 0 && tmp_end_index >= 0)
	    {
		    tmp_start_index = tmp_text.indexOf("${", tmp_end_index);
		    if (tmp_start_index >= 0)
		    {
			    tmp_end_index = tmp_text.indexOf("}", tmp_start_index);
			    if (tmp_end_index >= 0)
			    {
				    tmp_parameter = removeHtmlTags(tmp_text.mid(tmp_start_index + 2, (tmp_end_index - tmp_start_index - 2)));
				    tmp_param_value = Parameter::paramValueForParamName<T>(in_parameters_list, tmp_parameter.toStdString());
				    if (tmp_param_value != NULL)
				    {
					    tmp_text.replace(tmp_start_index, tmp_end_index - tmp_start_index + 1, tmp_param_value);
					    tmp_end_index = 0;
					    tmp_start_index = 0;
				    }
			    }
		    }
	    }

	    return tmp_text;
    }

    static QString removeHtmlTags(const QString &text);
    static QList<QString> parametersList(const QString &text);

    void setRecord(Record *in_record);

    QMap<QString, QImage> images();

public slots:
	void setCompleter(QCompleter *c);
	QCompleter *completer() const;

    void textBold();
    void textUnderline();
    void addLink();
    void textItalic();
    void textStrikeOut();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void indentLeft();
    void indentRight();
    void setTextColor();
    void textAlignLeft();
    void textAlignRight();
    void textAlignCenter();
    void textJustify();

    void clipboardDataChanged();

protected:
    virtual void keyPressEvent(QKeyEvent *in_event);
    virtual void focusInEvent(QFocusEvent *in_event);
	virtual bool canInsertFromMimeData(const QMimeData * source) const;
	virtual void insertFromMimeData(const QMimeData * source);

	void indentBy(int indent);

signals:
    void focused(Record *in_record);

private slots:
    void insertCompletion(const QString &completion);

private:
    QString textUnderCursor() const;
    QCompleter *_m_completer;
    Record	*_m_record;

    QMap<QString, QImage > _m_resources;
};

#endif /* RECORD_TEXT_EDIT_H_ */

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


#ifndef RECORD_TEXT_EDIT_TOOLBAR_H_
#define RECORD_TEXT_EDIT_TOOLBAR_H_

#include <QToolBar>
#include <QApplication>
#include <QAction>
#include <QActionGroup>
#include <QWidgetAction>

#include <QComboBox>
#include <QFontComboBox>
#include <QTextCharFormat>

class RecordTextEdit;

class RecordTextEditToolBar: public QToolBar
{

Q_OBJECT

public:
        enum ToolAction {
            CutCopyPaste = 0x01,
            RedoUndo = 0x02,
            Align = 0x04,
            Indent = 0x08,
            Font = 0x10,
            Mask = 0xFF
        };

	enum ToolSize {
		Small = 16,
		Medium = 22,
		Large = 32
	};

	RecordTextEditToolBar(QWidget *parent = 0);
	RecordTextEditToolBar(RecordTextEditToolBar::ToolSize in_size, QWidget *parent = 0);

	void connectToEditor(RecordTextEdit *in_text_editor);
	void disconnectFromEditor();
	void disconnectFromEditor(RecordTextEdit *in_text_editor);

public:

	void fontChanged(const QFont &in_font);
	void colorChanged(const QColor &c);

	QAction *cutAction;
	QAction *copyAction;
	QAction *pasteAction;
	QAction *redoAction;
	QAction *undoAction;

        QActionGroup    *alignementAction;
	QAction *indentLeftAction;
	QAction *indentRightAction;
	QAction *centerAction;
	QAction *justifyAction;

	QAction *alignLeftAction;
	QAction *alignRightAction;

	QAction *boldAction;
	QAction *italicAction;
	QAction *strikeAction;
	QAction *underlineAction;
	QAction *addLinkAction;
	QAction *colorAction;

	QFontComboBox *fontsList;
	QComboBox *fontsSizesList;

public slots :
	void setCursorPositionChanged();
	void setCurrentCharFormatChanged(const QTextCharFormat &format);

private:

        RecordTextEdit    *_m_text_editor;
        int                 _m_toolbar_actions;

	void initLayout(RecordTextEditToolBar::ToolSize in_size, QWidget *parent);
};

#endif /* RECORD_TEXT_EDIT_TOOLBAR_H_ */

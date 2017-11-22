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
#include <QLayout>
#include <QTextBlock>
#include <QTextDocumentFragment>
#include "record_text_edit.h"
#include "record_text_edit_toolbar.h"

RecordTextEditToolBar::RecordTextEditToolBar(QWidget *parent) : QToolBar(parent)
{
	initLayout(Small, parent);
}

RecordTextEditToolBar::RecordTextEditToolBar(RecordTextEditToolBar::ToolSize in_size, QWidget *parent) : QToolBar(parent)
{
        initLayout(in_size, parent);
}


void RecordTextEditToolBar::initLayout(RecordTextEditToolBar::ToolSize in_size, QWidget *parent)
{
	QString			tmp_root_images_location;
	QWidgetAction	*tmp_font_widget_action = NULL;
	QWidgetAction	*tmp_font_size_widget_action = NULL;

	_m_text_editor = NULL;
    //_m_toolbar_actions = CutCopyPaste | RedoUndo | Align | Indent | Font;
    _m_toolbar_actions = Align | Indent | Font;

	switch (in_size)
	{
		case Small:
                        setIconSize(QSize(Small,Small));
			tmp_root_images_location = QString(":/images/text-editor/16x16/");
			break;
		case Medium:
                        setIconSize(QSize(Medium,Medium));
                        tmp_root_images_location = QString(":/images/text-editor/22x22/");
			break;
		case Large:
		default:
                        setIconSize(QSize(Large,Large));
                        tmp_root_images_location = QString(":/images/text-editor/32x32/");
			break;
	}

        cutAction = new QAction(QIcon(tmp_root_images_location + "editcut.png"), tr("Couper"), parent);
	cutAction->setEnabled(false);
        cutAction->setVisible(_m_toolbar_actions & CutCopyPaste);

        copyAction = new QAction(QIcon(tmp_root_images_location + "editcopy.png"), tr("Copier"), parent);
	copyAction->setEnabled(false);
        copyAction->setVisible(_m_toolbar_actions & CutCopyPaste);

        pasteAction = new QAction(QIcon(tmp_root_images_location + "editpaste.png"), tr("Coller"), parent);
	pasteAction->setEnabled(false);
        pasteAction->setVisible(_m_toolbar_actions & CutCopyPaste);

        redoAction = new QAction(QIcon(tmp_root_images_location + "edit-redo.png"), tr("Refaire"), parent);
	redoAction->setEnabled(false);
        redoAction->setVisible(_m_toolbar_actions & RedoUndo);

        undoAction = new QAction(QIcon(tmp_root_images_location + "edit-undo.png"), tr("Annuler"), parent);
	undoAction->setEnabled(false);
        undoAction->setVisible(_m_toolbar_actions & RedoUndo);

        indentLeftAction = new QAction(QIcon(tmp_root_images_location + "format-indent-less.png"), tr("Indenter à gauche"), parent);
	indentLeftAction->setEnabled(false);

        indentRightAction = new QAction(QIcon(tmp_root_images_location + "format-indent-more.png"), tr("Indenter à droite"), parent);
	indentRightAction->setEnabled(false);

        alignementAction = new QActionGroup(this);
        centerAction = new QAction(QIcon(tmp_root_images_location + "format-justify-center.png"), tr("Centrer"), parent);
	centerAction->setEnabled(false);
	centerAction->setCheckable(true);

        justifyAction = new QAction(QIcon(tmp_root_images_location + "format-justify-fill.png"), tr("Justifier"), parent);
	justifyAction->setEnabled(false);
	justifyAction->setCheckable(true);

        alignLeftAction = new QAction(QIcon(tmp_root_images_location + "format-justify-left.png"), tr("Aligner à gauche"), parent);
	alignLeftAction->setEnabled(false);
	alignLeftAction->setCheckable(true);

        alignRightAction = new QAction(QIcon(tmp_root_images_location + "format-justify-right.png"), tr("Aligner à droite"), parent);
	alignRightAction->setEnabled(false);
	alignRightAction->setCheckable(true);

        boldAction = new QAction(QIcon(tmp_root_images_location + "format-text-bold.png"), tr("Gras"), parent);
	boldAction->setEnabled(false);
	boldAction->setCheckable(true);

        italicAction = new QAction(QIcon(tmp_root_images_location + "format-text-italic.png"), tr("Italique"), parent);
	italicAction->setEnabled(false);
	italicAction->setCheckable(true);

        strikeAction = new QAction(QIcon(tmp_root_images_location + "format-text-strikethrough.png"), tr("Barré"), parent);
	strikeAction->setEnabled(false);
	strikeAction->setCheckable(true);

        underlineAction = new QAction(QIcon(tmp_root_images_location + "format-text-underline.png"), tr("Souligné"), parent);
	underlineAction->setEnabled(false);
	underlineAction->setCheckable(true);

	addLinkAction = new QAction(QIcon(tmp_root_images_location + "www.png"), tr("Ajouter un lien"), parent);
	addLinkAction->setEnabled(false);
	addLinkAction->setCheckable(true);

	tmp_font_widget_action = new QWidgetAction(parent);
	fontsList = new QFontComboBox(parent);
	tmp_font_widget_action->setDefaultWidget(fontsList);
	fontsList->setEnabled(false);
	fontsList->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	tmp_font_size_widget_action = new QWidgetAction(parent);
	fontsSizesList = new QComboBox(parent);
	tmp_font_size_widget_action->setDefaultWidget(fontsSizesList);
	fontsSizesList->setEnabled(false);
	fontsSizesList->setMinimumWidth(60);

    QFontDatabase	tmp_fonts_database;
    foreach(int size, tmp_fonts_database.standardSizes())
		fontsSizesList->addItem(QString::number(size));
    fontsSizesList->setCurrentIndex(fontsSizesList->findText(QString::number(QApplication::font().pointSize())));
    fontsSizesList->setSizeAdjustPolicy(QComboBox::AdjustToContents);

	QPixmap pix(16, 16);
	pix.fill(Qt::black);
	colorAction = new QAction(pix, tr("&Couleur..."), parent);
	colorAction->setEnabled(false);

	addAction( cutAction);
	addAction( copyAction);
	addAction( pasteAction);

        addSeparator()->setVisible(_m_toolbar_actions & CutCopyPaste);

	addAction( redoAction);
	addAction( undoAction);

        addSeparator()->setVisible(_m_toolbar_actions & RedoUndo);

	addAction( indentLeftAction);
	addAction( indentRightAction);

        addAction( centerAction);
        addAction( justifyAction);
        addAction( alignLeftAction);
        addAction( alignRightAction);

        addSeparator();

	addAction( boldAction);
	addAction( italicAction);
	addAction( strikeAction);
	addAction( underlineAction);
	addAction( addLinkAction);

	addSeparator();

	addAction( tmp_font_widget_action);
	addAction( tmp_font_size_widget_action);

	addAction( colorAction);

}


void RecordTextEditToolBar::connectToEditor(RecordTextEdit *in_text_editor)
{
    bool	tmp_text_editor_enabled = false;

        if (in_text_editor != NULL && in_text_editor != _m_text_editor)
	{
		disconnectFromEditor();

                _m_text_editor = in_text_editor;

                if (_m_text_editor->document() != NULL)
                {
		    tmp_text_editor_enabled = _m_text_editor->isEnabled() && !_m_text_editor->isReadOnly();

                    _m_text_editor->setTextEditToolBar(this);

                    connect(_m_text_editor->document(), SIGNAL(undoAvailable(bool)), undoAction, SLOT(setEnabled(bool)));
                    connect(_m_text_editor->document(), SIGNAL(redoAvailable(bool)), redoAction, SLOT(setEnabled(bool)));

		    undoAction->setEnabled(tmp_text_editor_enabled && _m_text_editor->document()->isUndoAvailable());
		    redoAction->setEnabled(tmp_text_editor_enabled && _m_text_editor->document()->isRedoAvailable());

                    connect(undoAction, SIGNAL(triggered()), _m_text_editor, SLOT(undo()));
                    connect(redoAction, SIGNAL(triggered()), _m_text_editor, SLOT(redo()));

                    cutAction->setEnabled(false);
                    copyAction->setEnabled(false);

                    connect(cutAction, SIGNAL(triggered()), _m_text_editor, SLOT(cut()));
                    connect(copyAction, SIGNAL(triggered()), _m_text_editor, SLOT(copy()));
                    connect(pasteAction, SIGNAL(triggered()), _m_text_editor, SLOT(paste()));

                    connect(_m_text_editor, SIGNAL(copyAvailable(bool)), cutAction, SLOT(setEnabled(bool)));
                    connect(_m_text_editor, SIGNAL(copyAvailable(bool)), copyAction, SLOT(setEnabled(bool)));

		    indentLeftAction->setEnabled(tmp_text_editor_enabled);
                    connect(indentLeftAction, SIGNAL(triggered()), _m_text_editor, SLOT(indentLeft()));

		    indentRightAction->setEnabled(tmp_text_editor_enabled);
                    connect(indentRightAction, SIGNAL(triggered()), _m_text_editor, SLOT(indentRight()));

		    centerAction->setEnabled(tmp_text_editor_enabled);
                    connect(centerAction, SIGNAL(triggered()), _m_text_editor, SLOT(textAlignCenter()));

		    justifyAction->setEnabled(tmp_text_editor_enabled);
                    connect(justifyAction, SIGNAL(triggered()), _m_text_editor, SLOT(textJustify()));

		    alignLeftAction->setEnabled(tmp_text_editor_enabled);
                    connect(alignLeftAction, SIGNAL(triggered()), _m_text_editor, SLOT(textAlignLeft()));

		    alignRightAction->setEnabled(tmp_text_editor_enabled);
                    connect(alignRightAction, SIGNAL(triggered()), _m_text_editor, SLOT(textAlignRight()));

		    boldAction->setEnabled(tmp_text_editor_enabled);
                    connect(boldAction, SIGNAL(triggered()), _m_text_editor, SLOT(textBold()));

		    italicAction->setEnabled(tmp_text_editor_enabled);
                    connect(italicAction, SIGNAL(triggered()), _m_text_editor, SLOT(textItalic()));

		    strikeAction->setEnabled(tmp_text_editor_enabled);
                    connect(strikeAction, SIGNAL(triggered()), _m_text_editor, SLOT(textStrikeOut()));

		    underlineAction->setEnabled(tmp_text_editor_enabled);
                    connect(underlineAction, SIGNAL(triggered()), _m_text_editor, SLOT(textUnderline()));

		    addLinkAction->setEnabled(tmp_text_editor_enabled);
		    connect(addLinkAction, SIGNAL(triggered()), _m_text_editor, SLOT(addLink()));

		    colorAction->setEnabled(tmp_text_editor_enabled);
                    connect(colorAction, SIGNAL(triggered()), _m_text_editor, SLOT(setTextColor()));

		    fontsList->setEnabled(tmp_text_editor_enabled);
                    connect(fontsList, SIGNAL(activated(const QString &)), _m_text_editor, SLOT(textFamily(const QString &)));

		    fontsSizesList->setEnabled(tmp_text_editor_enabled);
                    connect(fontsSizesList, SIGNAL(activated(const QString &)), _m_text_editor, SLOT(textSize(const QString &)));

                    connect(_m_text_editor, SIGNAL(cursorPositionChanged()),this, SLOT(setCursorPositionChanged()));
                    connect(_m_text_editor, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)), this, SLOT(setCurrentCharFormatChanged(const QTextCharFormat &)));

                    fontChanged(_m_text_editor->font());
                    colorChanged(_m_text_editor->textColor());
                    setCursorPositionChanged();
                }
	}
}


void RecordTextEditToolBar::disconnectFromEditor(RecordTextEdit *in_text_editor)
{
	if (_m_text_editor == in_text_editor)
		disconnectFromEditor();
}

void RecordTextEditToolBar::disconnectFromEditor()
{
        if (_m_text_editor != NULL && _m_text_editor->document() != NULL)
	{
		disconnect(_m_text_editor->document(), SIGNAL(modificationChanged(bool)), _m_text_editor, SLOT(setWindowModified(bool)));
		disconnect(_m_text_editor->document(), SIGNAL(undoAvailable(bool)), undoAction, SLOT(setEnabled(bool)));
		disconnect(_m_text_editor->document(), SIGNAL(redoAvailable(bool)), redoAction, SLOT(setEnabled(bool)));

		undoAction->setEnabled(false);
		redoAction->setEnabled(false);

		disconnect(undoAction, SIGNAL(triggered()), _m_text_editor, SLOT(undo()));
		disconnect(redoAction, SIGNAL(triggered()), _m_text_editor, SLOT(redo()));

		cutAction->setEnabled(false);
		copyAction->setEnabled(false);

		disconnect(cutAction, SIGNAL(triggered()), _m_text_editor, SLOT(cut()));
		disconnect(copyAction, SIGNAL(triggered()), _m_text_editor, SLOT(copy()));
		disconnect(pasteAction, SIGNAL(triggered()), _m_text_editor, SLOT(paste()));

		disconnect(_m_text_editor, SIGNAL(copyAvailable(bool)), cutAction, SLOT(setEnabled(bool)));
		disconnect(_m_text_editor, SIGNAL(copyAvailable(bool)), copyAction, SLOT(setEnabled(bool)));

		indentLeftAction->setEnabled(false);
                disconnect(indentLeftAction, SIGNAL(triggered()), _m_text_editor, SLOT(indentLeft()));
                indentRightAction->setEnabled(false);
                disconnect(indentRightAction, SIGNAL(triggered()), _m_text_editor, SLOT(indentRight()));

		centerAction->setEnabled(false);
		disconnect(centerAction, SIGNAL(triggered()), _m_text_editor, SLOT(textAlignCenter()));

		justifyAction->setEnabled(false);
		disconnect(justifyAction, SIGNAL(triggered()), _m_text_editor, SLOT(textJustify()));

		alignLeftAction->setEnabled(false);
		disconnect(alignLeftAction, SIGNAL(triggered()), _m_text_editor, SLOT(textAlignLeft()));

		alignRightAction->setEnabled(false);
		disconnect(alignRightAction, SIGNAL(triggered()), _m_text_editor, SLOT(textAlignRight()));

		boldAction->setEnabled(false);
		disconnect(boldAction, SIGNAL(triggered()), _m_text_editor, SLOT(textBold()));

		italicAction->setEnabled(false);
		disconnect(italicAction, SIGNAL(triggered()), _m_text_editor, SLOT(textItalic()));

		strikeAction->setEnabled(false);
		disconnect(strikeAction, SIGNAL(triggered()), _m_text_editor, SLOT(textStrikeOut()));

		underlineAction->setEnabled(false);
		disconnect(underlineAction, SIGNAL(triggered()), _m_text_editor, SLOT(textUnderline()));

		addLinkAction->setEnabled(false);
		disconnect(addLinkAction, SIGNAL(triggered()), _m_text_editor, SLOT(addLink()));

		colorAction->setEnabled(false);
		disconnect(colorAction, SIGNAL(triggered()), _m_text_editor, SLOT(setTextColor()));

		fontsList->setEnabled(false);
		disconnect(fontsList, SIGNAL(activated(const QString &)), _m_text_editor, SLOT(textFamily(const QString &)));

		fontsSizesList->setEnabled(false);
		disconnect(fontsSizesList, SIGNAL(activated(const QString &)), _m_text_editor, SLOT(textSize(const QString &)));

		disconnect(_m_text_editor, SIGNAL(currentCharFormatChanged(const QTextCharFormat &)), this, SLOT(setCurrentCharFormatChanged(const QTextCharFormat &)));
		disconnect(_m_text_editor, SIGNAL(cursorPositionChanged()), this, SLOT(setCursorPositionChanged()));

		_m_text_editor = NULL;
	}

}


void RecordTextEditToolBar::setCursorPositionChanged()
{
	if (_m_text_editor != NULL)
	{
		Qt::Alignment tmp_alignment = _m_text_editor->alignment();

		if (alignLeftAction != NULL)
			alignLeftAction->setChecked(tmp_alignment & Qt::AlignLeft);

		if (centerAction != NULL)
			centerAction->setChecked(tmp_alignment & Qt::AlignHCenter);

		if (alignRightAction != NULL)
			alignRightAction->setChecked(tmp_alignment & Qt::AlignRight);

		if (justifyAction != NULL)
			justifyAction->setChecked(tmp_alignment & Qt::AlignJustify);

		if(addLinkAction != NULL)
		    addLinkAction->setChecked(_m_text_editor->currentCharFormat().isAnchor());
	}
}


void RecordTextEditToolBar::setCurrentCharFormatChanged(const QTextCharFormat &format)
{
	if (format.isValid())
	{
		fontChanged(format.font());
		colorChanged(format.foreground().color());
	}
}



void RecordTextEditToolBar::fontChanged(const QFont &in_font)
{
	if (fontsList != NULL)
		fontsList->setCurrentIndex(fontsList->findText(QFontInfo(in_font).family()));

	if (fontsSizesList != NULL)
		fontsSizesList->setCurrentIndex(fontsSizesList->findText(QString::number(in_font.pointSize())));

	if (boldAction != NULL)
		boldAction->setChecked(in_font.bold());

	if (italicAction != NULL)
		italicAction->setChecked(in_font.italic());

	if (underlineAction != NULL)
		underlineAction->setChecked(in_font.underline());

	if (strikeAction != NULL)
		strikeAction->setChecked(in_font.strikeOut());
}


void RecordTextEditToolBar::colorChanged(const QColor &c)
{
	if (colorAction != NULL)
	{
		QPixmap pix(16, 16);
		pix.fill(c);
		colorAction->setIcon(pix);
	}
}


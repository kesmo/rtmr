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


#include <QVBoxLayout>
#include "record_text_edit_widget.h"

RecordTextEditWidget::RecordTextEditWidget(QWidget *parent) : QWidget(parent)
{
	_m_vertical_layout = new QVBoxLayout(this);

	_m_text_edit_tool_bar = NULL;
	_m_text_edit = new RecordTextEdit(this);

	setLayout(_m_vertical_layout);
	_m_vertical_layout->setMargin(0);
	_m_vertical_layout->setSpacing(0);

	_m_vertical_layout->addWidget(_m_text_edit);
}



void RecordTextEditWidget::addTextToolBar(RecordTextEditToolBar::ToolSize in_size)
{
	if (_m_text_edit_tool_bar == NULL)
	{
		_m_text_edit_tool_bar = new RecordTextEditToolBar(in_size, this);
		_m_text_edit_tool_bar->connectToEditor(_m_text_edit);

		_m_vertical_layout->insertWidget(0, _m_text_edit_tool_bar);
	}
}

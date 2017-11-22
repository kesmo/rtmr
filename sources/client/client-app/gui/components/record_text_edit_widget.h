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

#ifndef RECORD_TEXT_EDIT_WIDGET_H_
#define RECORD_TEXT_EDIT_WIDGET_H_

#include <QVBoxLayout>
#include <QWidget>
#include "record_text_edit.h"
#include "record_text_edit_toolbar.h"

class RecordTextEditWidget : public QWidget {
public:
	RecordTextEditWidget(QWidget *parent = 0);

	void addTextToolBar(RecordTextEditToolBar::ToolSize in_size);

	RecordTextEdit* textEditor(){return _m_text_edit;}
	RecordTextEditToolBar* toolBar(){return _m_text_edit_tool_bar;}

private:
	QVBoxLayout				*_m_vertical_layout;
	RecordTextEdit		*_m_text_edit;
	RecordTextEditToolBar *_m_text_edit_tool_bar;

};


#endif /* RECORD_TEXT_EDIT_WIDGET_H_ */

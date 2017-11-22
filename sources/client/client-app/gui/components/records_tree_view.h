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

#ifndef RECORDS_TREE_VIEW_H
#define RECORDS_TREE_VIEW_H

#include <QtGui>
#include "hierarchy.h"

class TreeViewStyle : public QProxyStyle
{
public:
    TreeViewStyle(QStyle *baseStyle = 0) : QProxyStyle(baseStyle) {}

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
    {
	if (element == QStyle::PE_IndicatorItemViewItemDrop)
	{
	    //?? do nothing or do custom painting here
	}
	else
	{
	    QProxyStyle::drawPrimitive(element, option, painter, widget);
	}
    }
};

class RecordsTreeView : public QTreeView
{
    Q_OBJECT

public:
    RecordsTreeView(QWidget * parent = 0);

    void moveDown();
    QList<Hierarchy*> selectedRecords();

    template <class T>
    QList<T*> selectedRecords()
    {
	QList<T*>     tmp_list;
	T             *tmp_record = NULL;

	foreach(QModelIndex tmp_index, selectedIndexes())
	{
	    tmp_record = static_cast<T*>(tmp_index.internalPointer());
	    if (tmp_record != NULL)
		tmp_list.append(tmp_record);
	}

	return tmp_list;
    }

    void expandIndex(const QModelIndex &in_model_index);
    void setExpandedIndex(const QModelIndex &in_parent_index, bool expanded, bool recursively = true);

Q_SIGNALS:
    void delKeyPressed(QList<Hierarchy*>);
    void userEnterIndex(QModelIndex);

protected:
    void keyPressEvent ( QKeyEvent * event );
    void dropEvent ( QDropEvent * event );
    void mousePressEvent ( QMouseEvent * event );
    void mouseReleaseEvent ( QMouseEvent * event );

private:
    bool mUserPushLeftMouseButton;
};

#endif // RECORDS_TREE_VIEW_H

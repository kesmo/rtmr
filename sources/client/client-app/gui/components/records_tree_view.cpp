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

#include "records_tree_view.h"
#include <QtGui>

RecordsTreeView::RecordsTreeView(QWidget * parent) : QTreeView(parent),
  mUserPushLeftMouseButton(false)
{
    setDefaultDropAction(Qt::MoveAction);
    setUniformRowHeights(true);
}

void RecordsTreeView::keyPressEvent ( QKeyEvent * event )
{

    if (selectionModel() != NULL)
    {
        if (event != NULL)
        {
            switch (event->key())
            {
            case Qt::Key_Delete:
                emit delKeyPressed(selectedRecords());
                break;
            case Qt::Key_Enter:
            case Qt::Key_Return:
                emit userEnterIndex(currentIndex());
                break;
            }
        }
    }

    QTreeView::keyPressEvent(event);
}


void RecordsTreeView::dropEvent ( QDropEvent * event )
{
    if (event->source() != this)
    {
        event->setDropAction(Qt::CopyAction);
    }

    QTreeView::dropEvent(event);
    if (!event->isAccepted())
    {
        event->setDropAction(Qt::IgnoreAction);
    }
}


void RecordsTreeView::moveDown()
{
    moveCursor(MoveNext, Qt::NoModifier);
}


QList<Hierarchy*> RecordsTreeView::selectedRecords()
{
    return selectedRecords<Hierarchy>();
}


void RecordsTreeView::expandIndex(const QModelIndex &in_model_index)
{
    QModelIndex tmp_parent_index;

    if (in_model_index.isValid())
    {
        tmp_parent_index = in_model_index.parent();
        while (tmp_parent_index.isValid())
        {
            expand(tmp_parent_index);
            tmp_parent_index = tmp_parent_index.parent();
        }
    }
}


void RecordsTreeView::mousePressEvent(QMouseEvent *event)
{
    mUserPushLeftMouseButton = false;

    QModelIndex tmp_index = indexAt(event->pos());
    if (tmp_index.isValid())
    {
        if (event->buttons() & Qt::LeftButton)
        {
            if (visualRect(tmp_index).contains(event->pos().x(), event->pos().y()))
                mUserPushLeftMouseButton = true;
        }
    }
    QTreeView::mousePressEvent(event);
}


void RecordsTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    if(mUserPushLeftMouseButton){
        QModelIndex tmp_index = indexAt(event->pos());
        if (tmp_index.isValid())
        {
            emit userEnterIndex(tmp_index);
        }

    }

    QTreeView::mouseReleaseEvent(event);
}

void RecordsTreeView::setExpandedIndex(const QModelIndex &in_parent_index, bool expanded, bool recursively)
{
    if (in_parent_index.isValid())
    {
        setExpanded(in_parent_index, expanded);
        if (recursively)
        {
            QModelIndex tmp_child = in_parent_index.child(0, 0);
            while (tmp_child.isValid())
            {
                setExpandedIndex(tmp_child, expanded, recursively);
                tmp_child = in_parent_index.child(tmp_child.row() + 1, 0);
            }
        }
    }
}

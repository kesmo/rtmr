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

#include "records_table_widget.h"
#include <QtGui>
#include "genericrecord.h"

RecordsTableWidget::RecordsTableWidget ( QWidget * parent) : QTableWidget(parent)
{
    setMinimumHeight(200);

    _m_mime_type = DEFAULT_MIME_TYPE;
    _m_remove_on_keypress_event = true;

    createActions();
    setUpContextMenu();
}


RecordsTableWidget::~RecordsTableWidget()
{
    delete _m_copy_act;
    delete _m_cut_act;
    delete _m_paste_act;
    delete _m_paste_plain_text_act;
}


QStringList RecordsTableWidget::mimeTypes() const
{
    QStringList types;
    types << _m_mime_type;
    return types;
}

QMimeData* RecordsTableWidget::mimeData(const QList<QTableWidgetItem*> items) const
{
    QMimeData       *tmp_mime_data = new QMimeData();
    QByteArray      tmp_byte_array;
    Record        *tmp_record = NULL;

    foreach (QTableWidgetItem *tmp_current_item, items)
    {
        tmp_record = (Record*)tmp_current_item->data(Qt::UserRole).value<void*>();
        if (tmp_record != NULL)
        {
	    tmp_byte_array.append(tmp_record->serialize());
	    tmp_byte_array.append('\n');
	}
    }

    tmp_mime_data->setData(_m_mime_type, tmp_byte_array);

    return tmp_mime_data;
}


Qt::DropActions RecordsTableWidget::supportedDropActions () const
{
    return Qt::CopyAction | Qt::MoveAction;
}


void RecordsTableWidget::dropEvent ( QDropEvent * event )
{
    if (event->source() != this)
    {
	event->setDropAction(Qt::CopyAction);
    }

    QTableWidget::dropEvent(event);
}


bool RecordsTableWidget::dropMimeData ( int row, int /*column*/, const QMimeData * data, Qt::DropAction /*action*/ )
{
    QByteArray          tmp_bytes_array;
    QList<QString>      tmp_serialized_records_list;
    int                 tmp_index = 0;

    entity_def		*tmp_entity_def = NULL;
    int                 tmp_start_index = 0;

    GenericRecord       *tmp_generic_record = NULL;
    QList<Record*>    tmp_records_list;
    QList<QUrl>		tmp_urls;
    QList<QString>	tmp_filenames_list;
    QString		tmp_filename;

    if (data->hasUrls())
    {
	tmp_urls = data->urls();
	if (tmp_urls.isEmpty())
            return false;

	foreach(QUrl tmp_file_url,  tmp_urls)
	{
	    tmp_filename = tmp_file_url.toLocalFile();
	    if (!tmp_filename.isEmpty())
	    {
		tmp_filenames_list.append(tmp_filename);
	    }
	}

	emit urlsListDrop(tmp_filenames_list, row);
	return true;
    }
    else if (_m_mime_type.compare("application/records.list") == 0)
    {
    	tmp_bytes_array = data->data(_m_mime_type);

	if (tmp_bytes_array.isEmpty() == false)
	{
	    tmp_serialized_records_list = QString(tmp_bytes_array).split('\n');
	    for (tmp_index = 0; tmp_index < tmp_serialized_records_list.count(); tmp_index++)
	    {
		tmp_start_index = tmp_serialized_records_list[tmp_index].indexOf(SEPARATOR_CHAR);
		if (tmp_start_index >= 0)
		{
		    std::string tmp_std_string = tmp_serialized_records_list[tmp_index].section(SEPARATOR_CHAR, 0, 0).toStdString();
		    if (get_table_def(atoi(tmp_std_string.c_str()), &tmp_entity_def) == NOERR)
		    {
			tmp_generic_record = new GenericRecord(tmp_entity_def);
			if (tmp_generic_record != NULL)
			{
			    tmp_generic_record->deserialize(tmp_serialized_records_list[tmp_index]);
			    if (tmp_generic_record != NULL)
			    {
				tmp_records_list.append(tmp_generic_record);
			    }
			}
		    }
		}
	    }

	    emit recordslistDrop(tmp_records_list, row);
	    return true;
	}
    }

    return false;
}


bool RecordsTableWidget::focusNextPrevChild(bool next)
{
    // check if current column is the last column of the last row
    if (sender() == NULL && tabKeyNavigation() && next && currentRow() == rowCount()-1 &&  currentColumn() == columnCount()-1)
    {
        emit lastRowAndColumnReach();
        return true;
    }
    return QTableWidget::focusNextPrevChild(next);
}


void RecordsTableWidget::insertRow(int row)
{
    QTableWidget::insertRow(row);
}


void RecordsTableWidget::keyPressEvent ( QKeyEvent * event )
{
    QModelIndexList tmp_selected_indexes = selectionModel()->selectedRows();
    int             tmp_row = 0;

    if (event != NULL && event->key() == Qt::Key_Delete)
    {
	if (_m_remove_on_keypress_event)
	{
	    while (tmp_selected_indexes.count() > 0)
	    {
		if (tmp_selected_indexes[0].isValid())
		{
		    tmp_row = tmp_selected_indexes[0].row();
		    removeRow(tmp_row);
		    emit rowRemoved(tmp_row);
		}
		tmp_selected_indexes = selectionModel()->selectedRows();
	    }
	}
	else
	{
	    emit delKeyPressed(selectedRecords());
	}

        return;
    }

    QTableWidget::keyPressEvent(event);
}


void RecordsTableWidget::setMimeType(QString in_mime_type)
{
    _m_mime_type = in_mime_type;
}


void RecordsTableWidget::setRemoveSelectedRowsOnKeypressEvent(bool in_remove)
{
    _m_remove_on_keypress_event = in_remove;
}


QList<Record*> RecordsTableWidget::selectedRecords()
{
    QList<Record*>	    tmp_list;
    Record		    *tmp_record = NULL;
    QTableWidgetItem	    *tmp_item = NULL;

    foreach(QModelIndex tmp_index, selectedIndexes())
    {
	tmp_record = static_cast<Record*>(tmp_index.internalPointer());
	if (tmp_record == NULL)
	{
	    tmp_item = itemFromIndex(tmp_index);
	    if (tmp_item != NULL)
	    {
		tmp_record = (Record*)tmp_item->data(Qt::UserRole).value<void*>();
		if (tmp_record != NULL && tmp_list.indexOf(tmp_record) < 0)
		    tmp_list.append(tmp_record);
	    }
	}
	else if (tmp_list.indexOf(tmp_record) < 0)
	    tmp_list.append(tmp_record);
    }

    return tmp_list;
}


void RecordsTableWidget::createActions()
{
        _m_copy_act = new QAction(QIcon(":/images/22x22/editcopy.png"), tr("&Copier"), this);
	_m_copy_act->setShortcut(tr("Ctrl+C"));
	_m_copy_act->setStatusTip(tr("Copier"));
	connect(_m_copy_act, SIGNAL(triggered()), this, SLOT(copy()));

        _m_cut_act = new QAction(QIcon(":/images/22x22/editcut.png"), tr("Co&uper"), this);
        _m_cut_act->setShortcut(tr("Ctrl+X"));
        _m_cut_act->setStatusTip(tr("Couper"));
        connect(_m_cut_act, SIGNAL(triggered()), this, SLOT(cut()));

        _m_paste_act = new QAction(QIcon(":/images/22x22/editpaste.png"), tr("C&oller"), this);
	_m_paste_act->setShortcut(tr("Ctrl+V"));
	_m_paste_act->setStatusTip(tr("Coller"));
	connect(_m_paste_act, SIGNAL(triggered()), this, SLOT(paste()));

        _m_paste_plain_text_act = new QAction(QIcon(":/images/22x22/editpaste.png"), tr("Co&ller sans mise en forme"), this);
	_m_paste_plain_text_act->setShortcut(tr("Ctrl+B"));
	_m_paste_plain_text_act->setStatusTip(tr("Coller sans mise en forme"));
	connect(_m_paste_plain_text_act, SIGNAL(triggered()), this, SLOT(pastePlainText()));

}

void RecordsTableWidget::setUpContextMenu()
{
	verticalHeader()->addAction(_m_copy_act);
        verticalHeader()->addAction(_m_cut_act);
        verticalHeader()->addAction(_m_paste_act);
	verticalHeader()->addAction(_m_paste_plain_text_act);
	verticalHeader()->setContextMenuPolicy(Qt::ActionsContextMenu);

	setContextMenuPolicy(Qt::ActionsContextMenu);
}


void RecordsTableWidget::closeEditor ( QWidget * editor, QAbstractItemDelegate::EndEditHint hint )
{
    if (hint == QAbstractItemDelegate::EditNextItem && currentRow() == rowCount() - 1 && currentColumn() == columnCount() - 1)
    {
	QTableWidget::closeEditor(editor, QAbstractItemDelegate::SubmitModelCache);
	insertRow(rowCount());
	this->setCurrentCell(rowCount() - 1, 0);
	return;
    }

    QTableWidget::closeEditor(editor, hint);
}


void RecordsTableWidget::copy()
{   
	emit linesCopied();
}


void RecordsTableWidget::cut()
{
        emit linesCut();
}


void RecordsTableWidget::paste()
{
	emit linesPaste();
}


void RecordsTableWidget::pastePlainText()
{
	emit linesPastePlainText();
}



/*
void RecordsTableWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/records.list"))
	event->acceptProposedAction();
}



void RecordsTableWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/records.list"))
    {
	event->acceptProposedAction();
    }
}
*/

void RecordsTableWidget::dragMoveEvent ( QDragMoveEvent * event )
{
    if (event->mimeData()->hasFormat("application/records.list"))
    {
	// Sélectionner la ligne
	QModelIndex tmp_model_index = indexAt(event->pos());
	if (tmp_model_index.isValid())
	{
	    if (tmp_model_index.row() < rowCount())
	    {
		selectRow(tmp_model_index.row());
	    }
	}
	event->acceptProposedAction();
    }
}


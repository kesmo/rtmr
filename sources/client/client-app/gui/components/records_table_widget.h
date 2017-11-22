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

#ifndef RECORDS_TABLE_WIDGET_H
#define RECORDS_TABLE_WIDGET_H

#include <QTableWidget>
#include "record.h"

#define DEFAULT_MIME_TYPE "application/records.list"

class RecordsTableWidget : public QTableWidget
{
    Q_OBJECT

private:
    QString					_m_mime_type;

    bool					_m_remove_on_keypress_event;

    QAction *_m_copy_act;
    QAction *_m_cut_act;
    QAction *_m_paste_act;
    QAction *_m_paste_plain_text_act;

public:
    RecordsTableWidget(QWidget * parent = 0);
    ~RecordsTableWidget();

    void setMimeType(QString in_mime_type);
    void setRemoveSelectedRowsOnKeypressEvent(bool in_remove);
    QList<Record*> selectedRecords();

signals:
    void urlsListDrop(QList<QString>, int);
    void recordslistDrop(QList<Record*>, int);

    void rowRemoved(int in_row);
    void delKeyPressed(QList<Record*> in_records_list);

    void linesCopied();
    void linesCut();
    void linesPaste();
    void linesPastePlainText();

    void lastRowAndColumnReach();

protected:
    QStringList mimeTypes() const;
    QMimeData *mimeData(const QList<QTableWidgetItem*> items) const;
    bool dropMimeData ( int row, int column, const QMimeData * data, Qt::DropAction action );
    Qt::DropActions supportedDropActions () const;
    void dropEvent ( QDropEvent * event );

    bool focusNextPrevChild(bool next);
    void keyPressEvent ( QKeyEvent * event );

    void createActions();
    void setUpContextMenu();

    /*
    virtual void dropEvent ( QDropEvent * event );
    virtual void dragEnterEvent ( QDragEnterEvent * event );
    */
    virtual void dragMoveEvent ( QDragMoveEvent * event );

public slots:
	void copy();
        void cut();
        void paste();
	void pastePlainText();

        void insertRow(int row);

protected slots:
    void closeEditor ( QWidget * editor, QAbstractItemDelegate::EndEditHint hint );

};

#endif // RECORDS_TABLE_WIDGET_H

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
#include "combobox_delegate.h"

#include <QComboBox>


ComboBoxDelegate::ComboBoxDelegate(const QList<Record *> &records, const char* in_visible_column, QObject *parent) : QStyledItemDelegate(parent)
{
    mRecordsList = records;
    mVisibleColumn = in_visible_column;
}


void ComboBoxDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyledItemDelegate::paint(painter, option, index);
}


QSize ComboBoxDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    return QStyledItemDelegate::sizeHint(option, index);
}


QWidget *ComboBoxDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const

{
    QComboBox *comboBox = new QComboBox(parent);

    foreach (Record* record, mRecordsList) {
        comboBox->addItem(record->getValueForKey(mVisibleColumn), record->getIdentifier());
    }

    return comboBox;
}

void ComboBoxDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    QVariant value = index.model()->data(index, Qt::EditRole);

    comboBox->setCurrentIndex(comboBox->findData(value));
}


void ComboBoxDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
    model->setData(index, comboBox->itemData(comboBox->currentIndex()));
}

void ComboBoxDelegate::commitAndCloseEditor()
{
    QComboBox *editor = qobject_cast<QComboBox*>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

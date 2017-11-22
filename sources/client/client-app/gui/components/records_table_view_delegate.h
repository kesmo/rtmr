#ifndef RECORDS_TABLE_VIEW_DELEGATE_H
#define RECORDS_TABLE_VIEW_DELEGATE_H

#include "record.h"
#include "parent.h"
#include "gui/components/abstractdataselector.h"
#include <QItemDelegate>
#include <QPainter>
#include <QMenu>
#include <QPushButton>
#include <QEvent>


class RecordsTableViewEditor : public QWidget
{
    Q_OBJECT

public:
    RecordsTableViewEditor(AbstractDataSelector* in_data_selector, const QList<Record*>& records, const char *in_visible_column, QWidget *parent = 0);

    void setColumnsMap(QMap<QString, QString> in_columns_map);

    const QList<Record*>& getRecords() const;

Q_SIGNALS:
    void recordAdded(Record*);
    void recordRemoved(Record*);
    void recordMoved(Record*, int, int);

public Q_SLOTS:
    void addRecordButtonClicked();
    void removeRecordButtonClicked();
    void moveUpRecordButtonClicked();
    void moveDownRecordButtonClicked();

private:
    QList<Record*> mRecordsList;
    const char* mVisibleColumn;
    AbstractDataSelector* mDataSelector;

    void updateContent();

};

template <class T>
class RecordsTableViewDelegate : public QItemDelegate
{

public:

    RecordsTableViewDelegate(AbstractDataSelector* in_data_selector, const char* in_visible_column, QObject *parent)
        : QItemDelegate(parent),
          mDataSelector(in_data_selector),
          mVisibleColumn(in_visible_column)
    {
    }


    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return NULL;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        RecordsTableViewEditor *tableViewEditor = qobject_cast<RecordsTableViewEditor*>(editor);
        QVariant value = index.model()->data(index, Qt::EditRole);

    }


    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QMenu *tmp_menu = qobject_cast<QMenu*>(editor);
        Parent<T>*  tmp_parent = dynamic_cast< Parent<T>* >((Record*)index.internalPointer());
    }


    virtual bool editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
    {
        if (event->type() == QEvent::MouseButtonPress){
            QMenu *tmp_menu = NULL;
            QMenu *tmp_function_menu = NULL;
            QAction *tmp_action = NULL;
            Parent<T>*  tmp_parent = dynamic_cast< Parent<T>* >((Record*)index.internalPointer());
            T* tmp_child = NULL;
            QList<T*> tmp_childs = tmp_parent->getChilds();
            int tmp_index = 0;

            tmp_menu = new QMenu();
            for(tmp_index = 0; tmp_index < tmp_childs.count(); ++tmp_index){

                tmp_child = tmp_childs[tmp_index];
                tmp_function_menu = tmp_menu->addMenu(tmp_child->getValueForKey(mVisibleColumn));
                tmp_action = tmp_function_menu->addAction(tr("Supprimer"));
                tmp_action->setProperty("record", QVariant::fromValue<void*>(tmp_child));
                tmp_action->setProperty("type", 0);

                if (tmp_index > 0){
                    tmp_action = tmp_function_menu->addAction(tr("Monter"));
                    tmp_action->setProperty("record", QVariant::fromValue<void*>(tmp_child));
                    tmp_action->setProperty("type", 1);
                }

                if (tmp_index < tmp_childs.count() - 1){
                    tmp_action = tmp_function_menu->addAction(tr("Déscendre"));
                    tmp_action->setProperty("record", QVariant::fromValue<void*>(tmp_child));
                    tmp_action->setProperty("type", 2);
                }
            }

            tmp_menu->addAction(tr("Ajouter une fonction de validation..."));
            tmp_action = tmp_menu->exec(QCursor::pos());
            if (tmp_action){
                T* tmp_child = reinterpret_cast<T*>(tmp_action->property("record").value<void*>());
                if (tmp_child){
                    int tmp_child_index = tmp_parent->getChilds().indexOf(tmp_child);
                    switch (tmp_action->property("type").toInt()) {
                    case 0:
                        tmp_parent->removeChild(tmp_child);
                        break;
                    case 1:
                        tmp_parent->moveChild(tmp_child_index, tmp_child_index - 1);
                        break;
                    case 2:
                        tmp_parent->moveChild(tmp_child_index, tmp_child_index + 1);
                        break;
                    default:
                        break;
                    }
                }else{
                    Record* tmp_record = mDataSelector->exec();

                    if (tmp_record){
                        tmp_parent->addChild(dynamic_cast<T*>(tmp_record));
                    }
                }
            }
        }

        return true;
    }

    /*
    void updateEditorGeometry( QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        QRect rect = option.rect;
        QSize sizeHint = editor->sizeHint();

        if (rect.width()<sizeHint.width()) rect.setWidth(sizeHint.width());
        if (rect.height()<sizeHint.height()) rect.setHeight(sizeHint.height());

        editor->setGeometry(rect);
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        RecordsTableViewEditor *tableViewEditor = NULL;

        Parent<T>*  tmp_parent = dynamic_cast< Parent<T>* >((Record*)index.internalPointer());
        if (tmp_parent){
            tableViewEditor = new RecordsTableViewEditor(mDataSelector, castToRecordsList<T>(tmp_parent->getChilds()), mVisibleColumn, parent);
            return tableViewEditor;
        }

        return QItemDelegate::createEditor(parent, option, index);
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        RecordsTableViewEditor *tableViewEditor = qobject_cast<RecordsTableViewEditor*>(editor);
        QVariant value = index.model()->data(index, Qt::EditRole);

    }


    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        RecordsTableViewEditor *tableViewEditor = qobject_cast<RecordsTableViewEditor*>(editor);
        Parent<T>*  tmp_parent = dynamic_cast< Parent<T>* >((Record*)index.internalPointer());
        const QList<Record*>& tmp_records = tableViewEditor->getRecords();
        QList<T*> tmp_childs = tmp_parent->getChilds();
        QList<T*> tmp_removed_childs;
        QList<T*> tmp_new_childs;
        foreach(T* tmp_child, tmp_childs){
            if (!tmp_records.contains(tmp_child))
                tmp_removed_childs.append(tmp_child);
        }
        foreach(T* tmp_child, tmp_removed_childs){
            tmp_parent->removeChild(tmp_child);
        }

        foreach(Record* tmp_record, tmp_records){
            T* tmp_child = dynamic_cast<T*>(tmp_record);
            tmp_new_childs.append(tmp_child);
        }
        tmp_parent->setChilds(tmp_new_childs);
    }
*/
    /*
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
        Parent<T>*  tmp_parent = dynamic_cast< Parent<T>* >((Record*)index.internalPointer());
        if (tmp_parent && !(option.state & QStyle::State_Editing)){
            QList<T*> tmp_childs = tmp_parent->getChilds();
            QString tmp_text;
            foreach(T* tmp_child, tmp_childs){
                tmp_text += tmp_child->getValueForKey(mVisibleColumn);
                tmp_text += ", ";
            }
            if (tmp_parent && (option.state & QStyle::State_Selected)) {
                painter->fillRect(option.rect, option.palette.highlight());
                painter->setPen(option.palette.highlightedText().color());
            } else {
                painter->setPen(option.palette.text().color());
            }
            painter->drawText(option.rect, Qt::TextWordWrap, tmp_text);
        }
    }
    */

private:
    AbstractDataSelector* mDataSelector;
    const char* mVisibleColumn;
};

#endif // RECORDS_TABLE_VIEW_DELEGATE_H

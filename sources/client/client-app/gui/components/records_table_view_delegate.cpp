#include "records_table_view_delegate.h"

#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

RecordsTableViewEditor::RecordsTableViewEditor(AbstractDataSelector *in_data_selector, const QList<Record *> &records, const char *in_visible_column, QWidget *parent) :
    QWidget(parent),
    mRecordsList(records),
    mVisibleColumn(in_visible_column),
    mDataSelector(in_data_selector)
{
    setAutoFillBackground(true);
    updateContent();
}


const QList<Record*>& RecordsTableViewEditor::getRecords() const
{
  return mRecordsList;
}


void RecordsTableViewEditor::addRecordButtonClicked()
{
  Record*   tmp_record = mDataSelector->exec();

  if (tmp_record){
      mRecordsList.append(tmp_record);
  }
  updateContent();
}


void RecordsTableViewEditor::removeRecordButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());

    mRecordsList.removeOne((Record*)button->property("record").value<void*>());

    emit recordRemoved((Record*)button->property("record").value<void*>());
    updateContent();
}



void RecordsTableViewEditor::moveUpRecordButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    int tmp_index = 0;

    foreach(Record* record, mRecordsList){
        if (record == (Record*)button->property("record").value<void*>()){
            mRecordsList.move(tmp_index, tmp_index - 1);
            emit recordMoved(record, tmp_index, tmp_index - 1);
            break;
        }
        ++tmp_index;
    }
    updateContent();
}


void RecordsTableViewEditor::moveDownRecordButtonClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    int tmp_index = 0;

    foreach(Record* record, mRecordsList){
        if (record == (Record*)button->property("record").value<void*>()){
            mRecordsList.move(tmp_index, tmp_index + 1);
            emit recordMoved(record, tmp_index, tmp_index + 1);
            break;
        }
        ++tmp_index;
    }

    updateContent();
}


void RecordsTableViewEditor::updateContent()
{
    QGridLayout* layout = NULL;
    QLabel* label = NULL;
    QPushButton* button = NULL;
    int tmp_index = 0;

    if (this->layout()){
        layout = dynamic_cast<QGridLayout*>(this->layout());
        for(int tmp_row_index = 0; tmp_row_index < layout->rowCount(); ++tmp_row_index)
        {
            for(int tmp_index = 0; tmp_index < layout->columnCount(); ++tmp_index)
            {
                layout->removeItem(layout->itemAt((tmp_row_index * layout->columnCount()) + tmp_index));
            }
        }
        delete this->layout();
    }

    layout = new QGridLayout();
    layout->setMargin(0);
    layout->setSpacing(0);

    foreach(Record* record, mRecordsList){
        label = new QLabel(record->getValueForKey(mVisibleColumn), this);

        // Ajout bouton [-]
        button = new QPushButton(this);
        button->setIcon(QIcon(":/images/minus.png"));
        button->setFlat(true);
        button->setProperty("record", QVariant::fromValue<void*>(record));
        connect(button, SIGNAL(clicked()), this, SLOT(removeRecordButtonClicked()));

        layout->addWidget(label, tmp_index, 0);
        layout->addWidget(button, tmp_index, 1);

        ++tmp_index;
    }

    // Ajout bouton [+]
    button = new QPushButton(this);
    button->setIcon(QIcon(":/images/plus.png"));
    button->setFlat(true);
    connect(button, SIGNAL(clicked()), this, SLOT(addRecordButtonClicked()));

    layout->addWidget(button, tmp_index, 1);

    setLayout(layout);

    update();
}

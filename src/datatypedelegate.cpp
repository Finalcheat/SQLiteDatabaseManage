#include "datatypedelegate.h"
#include <QPainter>
#include <QComboBox>
#include <QEvent>
#include <QDebug>

DataTypeDelegate::DataTypeDelegate(int dataTypeColumn, int primary, int isNull, QObject *parent)
    : QItemDelegate(parent), primaryPixmap(QPixmap(":images/primary.png")), isNullPixmap(QPixmap(":images/isnull.png"))
{
    this->dataTypeColumn = dataTypeColumn;
    this->primary = primary;
    this->isNull = isNull;
    typeList << tr("INTEGER") << tr("REAL")
             << tr("TEXT") << tr("BLOB");
}

void DataTypeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == dataTypeColumn)
    {
        QString text = index.model()->data(index, Qt::DisplayRole).toString();
        drawDisplay(painter, option, option.rect, text);
        drawFocus(painter, option, option.rect);
    }
    else if (index.column() == isNull)
    {
        bool is = index.model()->data(index, Qt::DisplayRole).toBool();
        if (is)
        {
            int x = option.rect.x() + (option.rect.width() / 2) - (isNullPixmap.width() / 2);
            int y = option.rect.y() + (option.rect.height() / 2) - (isNullPixmap.height() / 2);
            painter->drawPixmap(x, y, isNullPixmap);
        }
        else
            painter->drawText(option.rect, "");
    }
    else if (index.column() == primary)
    {
        bool is = index.model()->data(index, Qt::DisplayRole).toBool();
        if (is)
        {
            int x = option.rect.x() + (option.rect.width() / 2) - (primaryPixmap.width() / 2);
            int y = option.rect.y() + (option.rect.height() / 2) - (primaryPixmap.height() / 2);
            painter->drawPixmap(x, y, primaryPixmap);
        }
        else
            painter->drawText(option.rect, "");
    }
    else
    {
        QItemDelegate::paint(painter, option, index);
    }
}

QWidget* DataTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.column() == dataTypeColumn)
    {
        QComboBox *comboBox = new QComboBox(parent);
        comboBox->addItems(typeList);
        QString text = index.data(Qt::DisplayRole).toString();
        int index = comboBox->findText(text);
        comboBox->setCurrentIndex(index);
        connect(comboBox, SIGNAL(editTextChanged(QString)), this, SLOT(comboBoxEditTextChanged()));
        return comboBox;
    }
    else
    {
        return QItemDelegate::createEditor(parent, option, index);
    }
}

void DataTypeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (index.column() == dataTypeColumn)
    {
        QString text = index.model()->data(index, Qt::DisplayRole).toString();
        QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
        comboBox->setCurrentIndex(comboBox->findText(text));
    }
    else
    {
        QItemDelegate::setEditorData(editor, index);
    }
}

void DataTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (index.column() == dataTypeColumn)
    {
        QComboBox *comboBox = qobject_cast<QComboBox*>(editor);
        QString text = comboBox->currentText();

        model->setData(index, text);
    }
    else
    {
        QItemDelegate::setModelData(editor, model, index);
    }
}

bool DataTypeDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (index.column() == 0 || index.column() == 1)
        return QItemDelegate::editorEvent(event, model, option, index);

    if (event->type() == QEvent::MouseButtonPress)
    {
        bool is = !model->data(index, Qt::DisplayRole).toBool();
        model->setData(index, is);
//        if (index.column() == 2 && is)
//        {
//            emit primaryChanged(index.row());
//        }
        return false;         //so that the selection can change
    }

    return true;
}

void DataTypeDelegate::comboBoxEditTextChanged()
{
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());
    emit commitData(comboBox);
    emit closeEditor(comboBox);
}

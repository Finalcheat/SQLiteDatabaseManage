#ifndef DATATYPEDELEGATE_H
#define DATATYPEDELEGATE_H

#include <QItemDelegate>


class DataTypeDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    DataTypeDelegate(int dataTypeColumn, int primary, int isNull, QObject *parent = 0);

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QWidget* createEditor(QWidget *parent,
                          const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

    void setComboBoxItem(const QStringList &itemList) { typeList = itemList; }

//signals:
//    void primaryChanged(int row);

private slots:
    void comboBoxEditTextChanged();

private:
    int dataTypeColumn;
    int primary;
    int isNull;

    QPixmap primaryPixmap;
    QPixmap isNullPixmap;

    QStringList typeList;
};

#endif // DATATYPEDELEGATE_H

#include "systemtableview.h"
#include <QSqlTableModel>
#include <QSqlRecord>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>

SystemTableView::SystemTableView(const QString &connectName, const QString &tableName, QWidget *parent) :
    QTableView(parent)
{
    QSqlDatabase db = QSqlDatabase::database(connectName);
    model = new QSqlTableModel(0, db);
    model->setTable(tableName);
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

//    QSqlRecord record = model->record();
//    for (int i = 0; i < record.count(); ++i)
//    {
//        model->setHeaderData(i, Qt::Horizontal, record.fieldName(i));
//    }

    this->setModel(model);
    this->resizeColumnsToContents();
    this->setEditTriggers(QAbstractItemView::NoEditTriggers);

    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setIcon(QIcon(":/images/copy.png"));
    copyAction->setStatusTip(tr("Copy the current selection's contents "
                                "to the clipboard"));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));
    addAction(copyAction);
    setContextMenuPolicy(Qt::ActionsContextMenu);

}

SystemTableView::~SystemTableView()
{
//    qDebug() << "~SystemTableView";
    delete model;
    delete copyAction;
}

void SystemTableView::copy()
{
    QItemSelectionModel *selectModel = selectionModel();
    if (!selectModel)
        return;

    QString str;
    QModelIndexList indexList = selectModel->selectedIndexes();
    QMap<int, QString> strMap;

    for (int i = indexList.size() - 1; i >= 0; --i)
    {
        strMap.insertMulti(indexList[i].row(), indexList[i].data(Qt::DisplayRole).toString());
    }

    QList<int> rowList = strMap.uniqueKeys();

    for (int i = 0; i < rowList.size(); ++i)
    {
        if (i > 0)
            str += "\n";

        QMap<int, QString>::iterator it = strMap.lowerBound(rowList[i]);
        QMap<int, QString>::iterator itUpper = strMap.upperBound(rowList[i]);
        while (it != itUpper)
        {
            str += it.value();
            if (++it != itUpper)
                str += "\t";
        }
    }

    QApplication::clipboard()->setText(str);
}

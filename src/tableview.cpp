#include "tableview.h"
#include "mainwindow.h"
#include <QSqlTableModel>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QMessageBox>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QSqlError>
#include <QSqlIndex>
#include <QSqlRecord>
#include <QDebug>

TableView::TableView(const QString &connectName, const QString &tableName, QWidget *parent) :
    QTableView(parent)
{
    QSqlDatabase db = QSqlDatabase::database(connectName);
//    qDebug() << db.primaryIndex(tableName).fieldName(0) << endl;
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
    this->setSelectionMode(QAbstractItemView::ContiguousSelection);

//    setSortingEnabled(true);

    createMenu();
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showMenu(QPoint)));
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged()));
}

TableView::~TableView()
{
//    qDebug() << "~TableView";
    delete model;
//    delete cutAction;
//    delete copyAction;
//    delete pasteAction;
//    delete deleteAction;
//    delete insertRowAction;
//    delete deleteRowAction;
//    delete tableMenu;
}

void TableView::createMenu()
{
    cutAction = new QAction(tr("Cu&t"), this);
    cutAction->setIcon(QIcon(":/images/cut.png"));
    cutAction->setShortcut(tr("Ctrl+X"));
    cutAction->setStatusTip(tr("Cut the current selection's contents "
                               "to the clipboard"));
    connect(cutAction, SIGNAL(triggered()), this, SLOT(cut()));

    copyAction = new QAction(tr("&Copy"), this);
    copyAction->setIcon(QIcon(":/images/copy.png"));
    copyAction->setShortcut(tr("Ctrl+C"));
    copyAction->setStatusTip(tr("Copy the current selection's contents "
                                "to the clipboard"));
    connect(copyAction, SIGNAL(triggered()), this, SLOT(copy()));

    pasteAction = new QAction(tr("&Paste"), this);
    pasteAction->setIcon(QIcon(":/images/paste.png"));
    pasteAction->setShortcut(tr("Ctrl+V"));
    pasteAction->setStatusTip(tr("Paste the clipboard's contents into "
                                 "the current selection"));
    connect(pasteAction, SIGNAL(triggered()), this, SLOT(paste()));

    deleteAction = new QAction(tr("&Delete"), this);
    deleteAction->setIcon(QIcon(":/images/delete.png"));
    deleteAction->setShortcut(tr("Del"));
    deleteAction->setStatusTip(tr("Delete the current selection's "
                                  "contents"));
    connect(deleteAction, SIGNAL(triggered()), this, SLOT(del()));

    insertRowAction = new QAction(tr("&Insert row"), this);
    insertRowAction->setIcon(QIcon(":/images/insertrow.png"));
    insertRowAction->setStatusTip(tr("Insert a row in current Table"));
    connect(insertRowAction, SIGNAL(triggered()), this, SLOT(insertRow()));

    deleteRowAction = new QAction(tr("&Delete rows"), this);
    deleteRowAction->setIcon(QIcon(":/images/deleterow.png"));
    deleteRowAction->setStatusTip(tr("Delete the current selection's rows"));
    connect(deleteRowAction, SIGNAL(triggered()), this, SLOT(delRow()));


    tableMenu = new QMenu(this);
    tableMenu->addAction(cutAction);
    tableMenu->addAction(copyAction);
    tableMenu->addAction(pasteAction);
    tableMenu->addAction(deleteAction);
    tableMenu->addSeparator();
    tableMenu->addAction(insertRowAction);
    tableMenu->addAction(deleteRowAction);
    this->setContextMenuPolicy(Qt::CustomContextMenu);
}

void TableView::showMenu(const QPoint &)
{
    tableMenu->exec(QCursor::pos());
}

void TableView::cut()
{
    if (MainWindow::getCurrentReadOnly())
    {
        QMessageBox::information(this, tr("SQLite GUI"),
                                 tr("The information cannot be cut because the "
                                    "current settings is Read Only."));
    }
    else
    {
        copy();
        del();
    }
}

void TableView::copy()
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

void TableView::paste()
{
    if (MainWindow::getCurrentReadOnly())
    {
        QMessageBox::information(this, tr("SQLite GUI"),
                                 tr("The information cannot be pasted because the "
                                    "current settings is Read Only."));
        return;
    }

    QItemSelectionModel *selectModel = selectionModel();
    if (!selectModel)
        return;

    QString str = QApplication::clipboard()->text();
    QStringList rows = str.split('\n');
    int numRows = rows.count();
    int numColumns = rows.first().count('\t') + 1;
    QModelIndexList indexList = selectModel->selectedIndexes();

    if (numRows * numColumns != indexList.size())
    {
        QMessageBox::information(this, tr("SQLite GUI"),
                                 tr("The information cannot be pasted because the copy "
                                    "and paste areas aren't the same size."));
        return;
    }

    QMap<int, QString> strMap;
    for (int i = numRows - 1; i >= 0; --i)
    {
        QStringList strList = rows[i].split('\t');
        for (int j = 0; j < numColumns; ++j)
        {
            strMap.insertMulti(j, strList[j]);
        }
    }

    QList<QString> strValue = strMap.values();

    for (int i = 0; i < indexList.size(); ++i)
    {
        model->setData(indexList[i], strValue[i]);
    }
}

void TableView::del()
{
    if (MainWindow::getCurrentReadOnly())
    {
        QMessageBox::information(this, tr("SQLite"),
                                 tr("The row cannot be delete because the "
                                    "current settings is Read Only."));
        return;
    }

    QItemSelectionModel *selectModel = selectionModel();
    if (!selectModel)
        return;

    QModelIndexList indexList = selectModel->selectedIndexes();
    for (int i = 0; i < indexList.size(); ++i)
    {
        model->setData(indexList[i], tr(""));
    }
}

void TableView::insertRow()
{
    if (MainWindow::getCurrentReadOnly())
    {
        QMessageBox::information(this, tr("SQLite GUI"),
                                 tr("The row cannot be insert because the "
                                    "current settings is Read Only."));
        return;
    }

    int row = model->rowCount();
    model->insertRow(row);

    QModelIndex index = model->index(row, 0);
    setCurrentIndex(index);
    edit(index);
}

void TableView::delRow()
{
    if (MainWindow::getCurrentReadOnly())
    {
        QMessageBox::information(this, tr("SQLite GUI"),
                                 tr("The row cannot be delete because the "
                                    "current settings is Read Only."));
        return;
    }

    int ok = QMessageBox::warning(this, tr("SQLite GUI"),
                                  tr("Are you sure delete the current rows"),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No,
                                  QMessageBox::Cancel | QMessageBox::Escape);

    if (ok == QMessageBox::Yes)
    {
//        qDebug() << selectionModel()->selectedRows(0).count();
//        qDebug() << selectionModel()->selectedRows(0).begin()->row();

        const int rowCount = selectionModel()->selectedRows(0).count();
        int row = selectionModel()->selectedRows(0).begin()->row();
        for (int i = 0; i < rowCount; ++i)
        {
            model->removeRow(row++);
        }
//        model->removeRow(currentIndex().row());
        model->submitAll();
    }
}

void TableView::setRead(bool b)
{
    if (b)
    {
        setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    else
    {
        setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
    }
}

void TableView::dataChanged()
{
    setWindowModified(true);
}

void TableView::save()
{
    model->database().transaction();
    if (model->submitAll())
    {
        model->database().commit();
        setWindowModified(false);
    }
    else
    {
        model->database().rollback();
        QMessageBox::warning(this, tr("Cached Table"),
                             tr("The database reported an error: %1")
                             .arg(model->lastError().text()));
    }
}

void TableView::findNext(const QString &str, Qt::CaseSensitivity cs)
{
    int row = currentIndex().row();
    int column = currentIndex().column() + 1;
    int RowCount = model->rowCount();
    int ColumnCount = model->columnCount();

    while (row < RowCount)
    {
        while (column < ColumnCount)
        {
            QModelIndex findIndex = model->index(row, column);
            if (findIndex.data(Qt::DisplayRole).toString().contains(str, cs))
            {
                clearSelection();
                setCurrentIndex(findIndex);
                activateWindow();
                return;
            }
            ++column;
        }
        column = 0;
        ++row;
    }

    QApplication::beep();
}

void TableView::findPrevious(const QString &str, Qt::CaseSensitivity cs)
{
    int row = currentIndex().row();
    int column = currentIndex().column() - 1;
    int ColumnCount = model->columnCount();

    while (row >= 0)
    {
        while (column >= 0)
        {
            QModelIndex findIndex = model->index(row, column);
            if (findIndex.data(Qt::DisplayRole).toString().contains(str, cs))
            {
                clearSelection();
                setCurrentIndex(findIndex);
                activateWindow();
                return;
            }
            --column;
        }
        column = ColumnCount - 1;
        --row;
    }
    QApplication::beep();
}

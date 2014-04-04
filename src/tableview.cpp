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
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSqlQuery>
#include <QDebug>

TableView::TableView(const QString &connectName, const QString &tableName, QWidget *parent) :
    QWidget(parent)
{
    QSqlDatabase db = QSqlDatabase::database(connectName);
    QSqlQuery query(db);
    query.exec(tr("select count(*) from %1").arg(tableName));
    uint rowCount = 0;
    while (query.next())
    {
       rowCount = query.value(0).toUInt();
    }
    //qDebug() << rowCount;
    uint page = rowCount / rowNumber;
    pageCount = rowCount % rowNumber ? page + 1 : page;
//    qDebug() << db.primaryIndex(tableName).fieldName(0) << endl;
    model = new QSqlTableModel(this, db);
    model->setTable(tableName);
    model->setFilter(tr(" 1=1 limit 0,%1").arg(rowNumber));
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();

//    QSqlRecord record = model->record();
//    for (int i = 0; i < record.count(); ++i)
//    {
//        model->setHeaderData(i, Qt::Horizontal, record.fieldName(i));
//    }

    view = new QTableView(this);
    view->setModel(model);
    view->resizeColumnsToContents();
    view->setSelectionMode(QAbstractItemView::ContiguousSelection);

    QHBoxLayout *bottomLayout = new QHBoxLayout();
    firstButton = new QPushButton(tr("First"));
    firstButton->setEnabled(false);
    prevButton = new QPushButton(tr("Previous"));
    prevButton->setEnabled(false);
    //prevButton->setIcon();
    pageNumberlineEdit = new QLineEdit();
    pageNumberlineEdit->setEnabled(false);
    pageNumberlineEdit->setText("1");
    pageNumberlineEdit->setFixedWidth(50);
    pageNumberlineEdit->setAlignment(Qt::AlignHCenter);
    QLabel *label = new QLabel(tr("/"));
    QLabel *countLabel = new QLabel(QString::number(pageCount));
    nextButton = new QPushButton(tr("Next"));
    nextButton->setEnabled(pageCount > 1);
    endButton = new QPushButton(tr("End"));
    endButton->setEnabled(pageCount > 1);
    bottomLayout->addStretch();
    bottomLayout->addWidget(firstButton);
    bottomLayout->addWidget(prevButton);
    bottomLayout->addWidget(pageNumberlineEdit);
    bottomLayout->addWidget(label);
    bottomLayout->addWidget(countLabel);
    bottomLayout->addWidget(nextButton);
    bottomLayout->addWidget(endButton);
    bottomLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->addWidget(view);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);

//    setSortingEnabled(true);

    createMenu();
    connect(this, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showMenu(QPoint)));
    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged()));
    connect(firstButton, SIGNAL(clicked()), this, SLOT(firstButtonClicked()));
    connect(prevButton, SIGNAL(clicked()), this, SLOT(prevButtonClicked()));
    connect(nextButton, SIGNAL(clicked()), this, SLOT(nextButtonClicked()));
    connect(endButton, SIGNAL(clicked()), this, SLOT(endButtonClicked()));

    setWindowTitle(tr("%1[*]").arg(tableName));
    setWindowModified(false);

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
        QMessageBox::information(this, tr("SQLite Database Manage"),
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
    QItemSelectionModel *selectModel = view->selectionModel();
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
        QMessageBox::information(this, tr("SQLite Database Manage"),
                                 tr("The information cannot be pasted because the "
                                    "current settings is Read Only."));
        return;
    }

    QItemSelectionModel *selectModel = view->selectionModel();
    if (!selectModel)
        return;

    QString str = QApplication::clipboard()->text();
    QStringList rows = str.split('\n');
    int numRows = rows.count();
    int numColumns = rows.first().count('\t') + 1;
    QModelIndexList indexList = selectModel->selectedIndexes();

    if (numRows * numColumns != indexList.size())
    {
        QMessageBox::information(this, tr("SQLite Database Manage"),
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
        QMessageBox::information(this, tr("SQLite Database Manage"),
                                 tr("The row cannot be delete because the "
                                    "current settings is Read Only."));
        return;
    }

    QItemSelectionModel *selectModel = view->selectionModel();
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
        QMessageBox::information(this, tr("SQLite Database Manage"),
                                 tr("The row cannot be insert because the "
                                    "current settings is Read Only."));
        return;
    }

    int row = model->rowCount();
    model->insertRow(row);

    QModelIndex index = model->index(row, 0);
    view->setCurrentIndex(index);
    view->edit(index);

}

void TableView::delRow()
{
    if (MainWindow::getCurrentReadOnly())
    {
        QMessageBox::information(this, tr("SQLite Database Manage"),
                                 tr("The row cannot be delete because the "
                                    "current settings is Read Only."));
        return;
    }

    int ok = QMessageBox::warning(this, tr("SQLite Database Manage"),
                                  tr("Are you sure delete the current rows"),
                                  QMessageBox::Yes | QMessageBox::Default,
                                  QMessageBox::No,
                                  QMessageBox::Cancel | QMessageBox::Escape);

    if (ok == QMessageBox::Yes)
    {
//        qDebug() << selectionModel()->selectedRows(0).count();
//        qDebug() << selectionModel()->selectedRows(0).begin()->row();

        const int rowCount = view->selectionModel()->selectedRows(0).count();
        int row = view->selectionModel()->selectedRows(0).begin()->row();
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
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    else
    {
        view->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
    }
}

void TableView::setShowGrid(bool b)
{
    view->setShowGrid(b);
}

void TableView::dataChanged()
{   
    if (!isWindowModified())
    {
        setWindowModified(true);
        emit tableDataChanged(model->database().connectionName(), model->tableName(), true);
    }
}

void TableView::save()
{
    model->database().transaction();
    if (model->submitAll())
    {
        model->database().commit();
        setWindowModified(false);
        emit tableDataChanged(model->database().connectionName(), model->tableName(), false);
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
    int row = view->currentIndex().row();
    int column = view->currentIndex().column() + 1;
    int RowCount = model->rowCount();
    int ColumnCount = model->columnCount();

    while (row < RowCount)
    {
        while (column < ColumnCount)
        {
            QModelIndex findIndex = model->index(row, column);
            if (findIndex.data(Qt::DisplayRole).toString().contains(str, cs))
            {
                view->clearSelection();
                view->setCurrentIndex(findIndex);
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
    int row = view->currentIndex().row();
    int column = view->currentIndex().column() - 1;
    int ColumnCount = model->columnCount();

    while (row >= 0)
    {
        while (column >= 0)
        {
            QModelIndex findIndex = model->index(row, column);
            if (findIndex.data(Qt::DisplayRole).toString().contains(str, cs))
            {
                view->clearSelection();
                view->setCurrentIndex(findIndex);
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

void TableView::firstButtonClicked()
{
    model->setFilter(tr(" 1=1 limit 0,%1").arg(rowNumber));
    pageNumberlineEdit->setText("1");

    firstButton->setEnabled(false);
    prevButton->setEnabled(false);
    nextButton->setEnabled(pageCount > 1);
    endButton->setEnabled(pageCount > 1);
}

void TableView::prevButtonClicked()
{
    bool b = false;
    uint number = pageNumberlineEdit->text().toUInt(&b);
    if (b)
    {
        --number;
        int startNumber = (number - 1) * rowNumber;
        model->setFilter(tr(" 1=1 limit %1,%2").arg(startNumber).arg(rowNumber));
        pageNumberlineEdit->setText(QString::number(number));

        firstButton->setEnabled(number > 1);
        prevButton->setEnabled(number > 1);
        nextButton->setEnabled(number < pageCount);
        endButton->setEnabled(number < pageCount);
    }
}

void TableView::nextButtonClicked()
{
    bool b = false;
    uint number = pageNumberlineEdit->text().toUInt(&b);
    if (b)
    {
        int startNumber = number * rowNumber;
        ++number;
        model->setFilter(tr(" 1=1 limit %1,%2").arg(startNumber).arg(rowNumber));
        pageNumberlineEdit->setText(QString::number(number));

        firstButton->setEnabled(number > 1);
        prevButton->setEnabled(number > 1);
        nextButton->setEnabled(number < pageCount);
        endButton->setEnabled(number < pageCount);
    }
}

void TableView::endButtonClicked()
{
    model->setFilter(tr(" 1=1 limit %1,%2").arg((pageCount - 1) * rowNumber).arg(rowNumber));
    pageNumberlineEdit->setText(QString::number(pageCount));
    firstButton->setEnabled(pageCount > 1);
    prevButton->setEnabled(pageCount > 1);
    nextButton->setEnabled(false);
    endButton->setEnabled(false);
}

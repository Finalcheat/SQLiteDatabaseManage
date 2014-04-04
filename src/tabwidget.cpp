#include "tabwidget.h"
#include "mainwindow.h"
#include "querywidget.h"
#include "newtablewidgett.h"
#include "modifytablefieldwidget.h"
#include "systemtableview.h"
#include "createindexwidget.h"
#include "tableview.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlField>
#include <QtSql/QSqlError>
#include <QMap>
#include <QList>
#include <iterator>
#include <QDebug>

TabWidget::TabWidget(QWidget *parent) :
    QTabWidget(parent)
{
    setTabsClosable(true);
    setMovable(true);
    connect(this, SIGNAL(tabCloseRequested(int)), this, SLOT(tableClose(int)));
    connect(this, SIGNAL(currentChanged(int)), this, SLOT(currentWidgetChanged(int)));
}

void TabWidget::createNewTableView(const QString &connectName, const QString &tableName)
{
//    if (findText(title))
//        return;

//    QSqlDatabase db = QSqlDatabase::database(connectName);

//    QTableView *view = new QTableView;
//    setReadOnlyPrivate(view, MainWindow::getcurrentReadOnly());
//    view->setShowGrid(MainWindow::getcurrentShow());

//    QSqlTableModel *model = new QSqlTableModel(this, db);
//    model->setTable(title);
//    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
//    model->select();

//    QSqlRecord record = model->record();

//    for (int i = 0; i < record.count(); ++i)
//    {
//        model->setHeaderData(i, Qt::Horizontal, record.fieldName(i));
////        qDebug() << record.fieldName(i) << endl;
////        int r = model->record(i).field(i).type();
////        qDebug() << r << endl;
////        if (r == QVariant::Int)
////            qDebug() << "Int" << endl;
////        if (r == QVariant::Invalid)
////            qDebug() << "NULL" << endl;
//    }
////    qDebug() << title << connectName << endl;
//    view->setModel(model);
//    view->resizeColumnsToContents();

//    view->setSelectionMode(QAbstractItemView::ContiguousSelection);

////    connect(model, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(dataChanged()));

////    tableViewList.push_back(newTableView);
////    tableModelList.push_back(model);

    int index = connectName.lastIndexOf("\\");
    QString name = connectName.mid(index + 1) + '-' + tableName;

    if (findText(name) != -1)
        return;

    TableView *view = new TableView(connectName, tableName, this);
    //setReadOnlyPrivate(view, MainWindow::getCurrentReadOnly());
    view->setRead(MainWindow::getCurrentReadOnly());
    view->setShowGrid(MainWindow::getCurrentShow());
    connect(view, SIGNAL(tableDataChanged(QString,QString,bool)), this, SLOT(tableDataChanged(QString,QString,bool)));

    addTab(view, name);
    setCurrentWidget(view);
}

void TabWidget::tableDataChanged(const QString &connectName, const QString &tableName, bool b)
{
    int index = connectName.lastIndexOf("\\");
    QString name = connectName.mid(index + 1) + '-' + tableName;

    index = findText(name);
    if (index != -1)
    {
        QString temp = "";
        if (b)
            temp = "*";

        setTabText(index, name + temp);
    }
}

void TabWidget::createNewQuery(const QString &connectName)
{
//    QSqlDatabase db = QSqlDatabase::database(connectName);

//    QSqlTableModel *model = new QSqlTableModel(this, db);

//    model->QSqlQueryModel::setQuery(query, db);
//    if (model->lastError().isValid())
//    {
//        QMessageBox::warning(this, tr("SQLite GUI"), model->lastError().text());
//        delete model;
//        return;
//    }

//    QTableView *newTableView = new QTableView;
//    newTableView->setModel(model);
//    newTableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    newTableView->resizeColumnsToContents();
//    newTableView->setSelectionMode(QAbstractItemView::ContiguousSelection);

//    addTab(newTableView, query);
//    setCurrentWidget(newTableView);
//    tableViewList.push_back(newTableView);
//    tableModelList.push_back(model);
//    qDebug() << connectName << endl;

    int index = connectName.lastIndexOf("\\");
    QString title = connectName.mid(index + 1) + tr("-SQL");

    if (findText(title) != -1)
        return;

    QueryWidget *queryWidget = new QueryWidget(connectName, this);
    addTab(queryWidget, title);
    setCurrentWidget(queryWidget);
}

void TabWidget::onSqlExecute()
{
//    QWidget *w = currentWidget();
//    if (w)
//    {
//        QString className = w->metaObject()->className();
//        if (className == tr("QueryWidget"))
//        {
//            QueryWidget *queryWidget = (QueryWidget*)w;
//            queryWidget->onSqlExecute();
//        }
//    }
    QString className = getCurrentClassName();
    if (className == "QueryWidget")
    {
        static_cast<QueryWidget*>(currentWidget())->onSqlExecute();
    }
}

void TabWidget::createNewTable(const QString &connectName)
{
    int index = connectName.lastIndexOf("\\");
    QString title = connectName.mid(index + 1) + tr("-New Table");

    if (findText(title) != -1)
        return;

    NewTableWidget *widget = new NewTableWidget(connectName, this);
    addTab(widget, title);
    setCurrentWidget(widget);
}

void TabWidget::createSystemTable(const QString &connectName, const QString &tableName)
{
    int index = connectName.lastIndexOf("\\");
    QString name = connectName.mid(index + 1) + '-' + tableName;

    if (findText(name) != -1)
        return;

    SystemTableView *systemTable = new SystemTableView(connectName, tableName, this);
    addTab(systemTable, name);
    setCurrentWidget(systemTable);
}

void TabWidget::createModifyTableField(const QString &connectName, const QString &tableName)
{
    int index = connectName.lastIndexOf("\\");
    QString title = connectName.mid(index + 1) + "-" + tableName + tr("-Modify Field");

    if (findText(title) != -1)
        return;

    ModifyTableFieldWidget *w = new ModifyTableFieldWidget(connectName, tableName, this);
    addTab(w, title);
    setCurrentWidget(w);

}

void TabWidget::createIndex(const QString &connectName, const QString &tableName)
{
    int index = connectName.lastIndexOf("\\");
    QString title = connectName.mid(index + 1) + "-" + tableName + tr("-Create Index");
    if (findText(title) != -1)
        return;

    CreateIndexWidget *w = new CreateIndexWidget(connectName, tableName, this);
    addTab(w, title);
    setCurrentWidget(w);
}

void TabWidget::tableClose(int index)
{
    QWidget *w = widget(index);
//    QString className = w->metaObject()->className();
//    if (className == tr("NewTableWidget"))
//    {
//        NewTableWidget *newTableWidget = (NewTableWidget*)w;
//        newTableWidget->save();
//    }
    if (w->isWindowModified())
    {
        int r = QMessageBox::warning(this, tr("SQLite Database Manage"),
                                     tr("The document has been modified.\n"
                                        "Do you want to save your changes?"),
                                     QMessageBox::Yes | QMessageBox::Default,
                                     QMessageBox::No,
                                     QMessageBox::Cancel | QMessageBox::Escape);

        if (r == QMessageBox::Cancel)
        {
            return;
        }
        else if (r == QMessageBox::Yes)
        {
            static_cast<TableView*>(w)->save();
        }
    }
    if (!w->close())
        return;

    removeTab(index);
    delete w;
//    QTableView *tableView = (QTableView*)widget(index);
//    if (tableView->isWindowModified())
//    {
//        int r = QMessageBox::warning(this, tr("SQLite GUI"),
//                                     tr("The document has been modified.\n"
//                                        "Do you want to save your changes?"),
//                                     QMessageBox::Yes | QMessageBox::Default,
//                                     QMessageBox::No,
//                                     QMessageBox::Cancel | QMessageBox::Escape);

//        if (r == QMessageBox::Cancel)
//        {
//            return;
//        }
//        else if (r == QMessageBox::Yes)
//        {
//            save(index);
//        }
//    }
//    removeTab(index);
//    QTableView *view = tableViewList.value(index);
//    view->model();
//    delete view;
//    QSqlTableModel *model = tableModelList.value(index);
//    delete model;
//    tableViewList.removeAt(index);
//    tableModelList.removeAt(index);
}

void TabWidget::save()
{
//    QSqlTableModel* model = tableModelList[index];

//    model->database().transaction();
//    if (model->submitAll())
//    {
//        model->database().commit();
//    }
//    else
//    {
//        model->database().rollback();
//        QMessageBox::warning(this, tr("Cached Table"),
//                             tr("The database reported an error: %1")
//                             .arg(model->lastError().text()));
//        return false;
//    }
//    return true;

    QWidget *w = currentWidget();
    if (w && w->isWindowModified())
    {
        static_cast<TableView*>(w)->save();
    }
}

void TabWidget::setReadOnly(bool isRead)
{
//    for (int i = 0; i < tableViewList.size(); ++i)
//    {
//        setReadOnlyPrivate(tableViewList[i], isRead);
//    }

    for (int i = 0; i < count(); ++i)
    {
        QWidget *w = widget(i);
        QString className = w->metaObject()->className();
        if (className == "TableView")
        {
            static_cast<TableView*>(w)->setRead(isRead);
        }
    }
}

void TabWidget::setReadOnlyPrivate(QTableView *view, bool isRead)
{
    if (isRead)
    {
        view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    else
    {
        view->setEditTriggers(
                    QAbstractItemView::DoubleClicked | QAbstractItemView::AnyKeyPressed);
    }
}

void TabWidget::setShowGrid(bool isShow)
{
//    for (int i = 0; i < tableViewList.size(); ++i)
//    {
//        tableViewList[i]->setShowGrid(isShow);
//    }

    for (int i = 0; i < count(); ++i)
    {
        QWidget *w = widget(i);
        QString className = w->metaObject()->className();
        if (className == "TableView")
        {
            static_cast<TableView*>(w)->setShowGrid(isShow);
        }
    }
}

void TabWidget::copy()
{
    QString className = getCurrentClassName();
    if (className == "TableView")
    {
        static_cast<TableView*>(currentWidget())->copy();
    }
    else if (className == "QueryWidget")
    {
        static_cast<QueryWidget*>(currentWidget())->copy();
    }
    else
    {
        static_cast<SystemTableView*>(currentWidget())->copy();
    }
}

void TabWidget::paste()
{
//    if (MainWindow::getcurrentReadOnly())
//    {
//        QMessageBox::information(this, tr("SQLite GUI"),
//                                 tr("The information cannot be pasted because the "
//                                    "current settings is Read Only."));
//        return;
//    }


    QString className = getCurrentClassName();
    if (className == "TableView")
    {
        static_cast<TableView*>(currentWidget())->paste();
    }
    else
    {
        static_cast<QueryWidget*>(currentWidget())->paste();
    }
}

void TabWidget::cut()
{
    if (getCurrentClassName() == "TableView")
    {
        static_cast<TableView*>(currentWidget())->cut();
    }
    else
    {
        static_cast<QueryWidget*>(currentWidget())->cut();
    }
}

void TabWidget::del()
{
//    if (MainWindow::getcurrentReadOnly())
//    {
//        QMessageBox::information(this, tr("SQLite"),
//                                 tr("The row cannot be delete because the "
//                                    "current settings is Read Only."));
//        return;
//    }

    if (getCurrentClassName() == "TableView")
    {
        static_cast<TableView*>(currentWidget())->del();
    }
    else
    {
        static_cast<QueryWidget*>(currentWidget())->del();
    }
}


void TabWidget::insertRow()
{
    if (getCurrentClassName() == "TableView")
    {
        static_cast<TableView*>(currentWidget())->insertRow();
    }

}

void TabWidget::delRow()
{
    if (getCurrentClassName() == "TableView")
    {
        static_cast<TableView*>(currentWidget())->delRow();
    }
}

void TabWidget::findNext(const QString &str, Qt::CaseSensitivity cs)
{
    if (getCurrentClassName() == "TableView")
    {
        static_cast<TableView*>(currentWidget())->findNext(str, cs);
    }
    else
    {
        static_cast<QueryWidget*>(currentWidget())->findNext(str);
    }


//    QTableView *currentView = (QTableView*)currentWidget();

//    if (!currentView)
//        return;

//    QSqlTableModel *model = (QSqlTableModel*)currentView->model();

//    int row = currentView->currentIndex().row();
//    int column = currentView->currentIndex().column() + 1;
//    int RowCount = model->rowCount();
//    int ColumnCount = model->columnCount();

//    while (row < RowCount)
//    {
//        while (column < ColumnCount)
//        {
//            QModelIndex findIndex = model->index(row, column);
//            if (findIndex.data(Qt::DisplayRole).toString().contains(str, cs))
//            {
//                currentView->clearSelection();
//                currentView->setCurrentIndex(findIndex);
//                currentView->activateWindow();
//                return;
//            }
//            ++column;
//        }
//        column = 0;
//        ++row;
//    }

//    QApplication::beep();
}

void TabWidget::findPrevious(const QString &str, Qt::CaseSensitivity cs)
{
    if (getCurrentClassName() == "TableView")
    {
        static_cast<TableView*>(currentWidget())->findPrevious(str, cs);
    }
    else
    {
        static_cast<QueryWidget*>(currentWidget())->findPrevious(str);
    }

//    QTableView *currentView = (QTableView*)currentWidget();

//    if (!currentView)
//        return;

//    QSqlTableModel *model = (QSqlTableModel*)currentView->model();

//    int row = currentView->currentIndex().row();
//    int column = currentView->currentIndex().column() - 1;
//    int ColumnCount = model->columnCount();

//    while (row >= 0)
//    {
//        while (column >= 0)
//        {
//            QModelIndex findIndex = model->index(row, column);
//            if (findIndex.data(Qt::DisplayRole).toString().contains(str, cs))
//            {
//                currentView->clearSelection();
//                currentView->setCurrentIndex(findIndex);
//                currentView->activateWindow();
//                return;
//            }
//            --column;
//        }
//        column = ColumnCount - 1;
//        --row;
//    }
//    QApplication::beep();
}

void TabWidget::saveAll()
{
    for (int i = 0; i < count(); ++i)
    {
        QWidget *w = widget(i);
        if (w->isWindowModified())
        {
            static_cast<TableView*>(w)->save();
        }
    }
//    for (int i = 0; i < count(); ++i)
//    {
//        if (save(i))
//        {
//            QTableView *view = tableViewList[i];
//            view->setWindowModified(false);
//        }
//    }
}

void TabWidget::saveSqlAs()
{
    if (getCurrentClassName() == "QueryWidget")
    {
        static_cast<QueryWidget*>(currentWidget())->saveAs();
    }
}

void TabWidget::openSql(const QString &connectName)
{
    QWidget *w = currentWidget();
    if (w)
    {
        if (getCurrentClassName() != "QueryWidget")
            createNewQuery(connectName);

        static_cast<QueryWidget*>(currentWidget())->openSql();
    }
}

int TabWidget::findText(const QString &text)
{
    for (int i = 0; i < count(); ++i)
    {
        QString tabTextStr = tabText(i);
        if (tabTextStr.size() - 1 == text.size() && tabTextStr.endsWith('*'))
        {
            tabTextStr.remove(tabTextStr.size() - 1, 1);
        }
        if (tabTextStr.size() == text.size())
        {
            if (tabTextStr == text)
            {
                setCurrentIndex(i);
                return i;
            }
        }
    }
    return -1;
}

void TabWidget::runLineSql()
{
    if (getCurrentClassName() == "QueryWidget")
    {
        static_cast<QueryWidget*>(currentWidget())->runLineSql();
    }
}

void TabWidget::analysisSql()
{
    if (getCurrentClassName() == "QueryWidget")
    {
        static_cast<QueryWidget*>(currentWidget())->analysisSql();
    }
}

void TabWidget::queryChanged(const QString &database, const QString &connectName)
{
    QWidget *w = currentWidget();
    if (w)
    {
        if (findText(database) != -1)
        {
            QMessageBox::information(this, tr("SQLite Database Manage"), tr("The Query already exist"));
            return;
        }
        QString className = w->metaObject()->className();
        if (className == "QueryWidget")
        {
            QueryWidget *queryWidget = (QueryWidget*)w;
            queryWidget->setConnectName(connectName);
            setTabText(currentIndex(), database + "-SQL");
        }
    }
}

QString TabWidget::getCurrentClassName() const
{
    QWidget *w = currentWidget();
    if (w)
    {
        return w->metaObject()->className();
    }

    return "";
}

void TabWidget::currentWidgetChanged(int index)
{
//    qDebug() << index;
    if (index == -1)
    {
        emit widgetChanged(NOWIDGET);
        return;
    }

    QWidget *w = widget(index);
    QString className = w->metaObject()->className();
    WidgetType type;
    if (className == "TableView")
    {
        type = TABLEVIEW;
    }
    else if (className == "SystemTableView")
    {
        type = SYSTEMTABLEVIEW;
    }
    else if (className == "QueryWidget")
    {
        type = QUERYWIDGET;
    }
    else if (className == "CreateIndexWidget")
    {
        type = CREATEINDEXWIDGET;
    }
    else if (className == "NewTableWidget")
    {
        type = NEWTABLEWIDGET;
    }
    else
    {
        type = MODIFYTABLEFIELDWIDGET;
    }
    emit widgetChanged(type);
}


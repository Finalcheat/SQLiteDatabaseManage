#include "querywidget.h"
#include "codeeditor.h"
#include "highlighter.h"
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QtSql/QSqlRecord>
#include <QSqlTableModel>
#include <QMessageBox>
#include <QTableView>
#include <QFileDialog>
#include <QTextDocumentFragment>
#include <QListView>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QCursor>
#include <QApplication>
#include <QTime>
#include <QDebug>

QueryWidget::QueryWidget(const QString &c, QWidget *parent) :
    QWidget(parent), connectName(c), font("", 10)
{
    groupBox = new QGroupBox(tr("SQL"));
    sqlCodeEditor = new CodeEditor(parent);
//    font.setUnderline(true);

    sqlCodeEditor->setFont(font);
//    sqlCodeEditor->setToolTip(tr("SQL Statement"));
    highlighter = new Highlighter(sqlCodeEditor->document());

    QHBoxLayout *textLayout = new QHBoxLayout;
    textLayout->addWidget(sqlCodeEditor);
    groupBox = new QGroupBox(tr("SQL"));
    groupBox->setLayout(textLayout);

    tabWidget = new QTabWidget;
    resultText = new QPlainTextEdit;
    resultText->setReadOnly(true);
    tabWidget->addTab(resultText, tr("Result"));
//    tabWidget->setVisible(false);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout ->addWidget(groupBox);
    mainLayout->addWidget(tabWidget);

    setLayout(mainLayout);

    databaseInitialize();
//    sqlCodeEditor->setParent(this);

//    listWidget->addItems(QStringList() << "select" << "a" << "b" << "c" << "d" << "e" << "f");
//    connect(sqlCodeEditor, SIGNAL(textChanged()), this, SLOT(textChanged()));
}

QueryWidget::~QueryWidget()
{
//    qDebug() << "~QueryWidget";
//    delete highlighter;
//    delete sqlCodeEditor;
//    delete groupBox;
//    delete resultText;
//    delete tabWidget;
    for (int i = 0; i < tableView.size(); ++i)
    {
        delete tableModelList[i];
//        delete tableView[i];
    }
}

void QueryWidget::onSqlExecute()
{
    QSqlDatabase db = QSqlDatabase::database(connectName);

    QString str = sqlCodeEditor->toPlainText();
    QStringList sqlCodeList = str.split(';', QString::SkipEmptyParts);
//    QStringList sqlCodeList = QStringList(str);
//    QStringList sqlCodeList = str.split(QRegExp(//[^\n]*), QString::SkipEmptyParts);
//    qDebug() << sqlCodeList;
//    return;

//    QSqlQuery query(db);

//    Test insert data
//    QString sql = "insert into testTable values('";

//    time.restart();
//    db.transaction();
//    for (int i = 50000; i < 150000; ++i)
//    {
//        QString str = sql + QString::number(i) + "'";
//        str = str + tr(",'%1','%2','%3','%4','%5','%6','%7','%8','%9')").arg(i+1).arg(i+2)
//                .arg(i+3).arg(i+4).arg(i+5).arg(i+6).arg(i+7).arg(i+8).arg(i+9);
////        qDebug() << str;
////        query.exec(str);
//        db.exec(str);
//    }
//    db.commit();
//    resultText->setPlainText(tr("%1 ms").arg(time.elapsed()));
//    return;

    int cnt = 1;
    resultText->clear();
    for (int i = 0; i < tableView.size(); ++i)
    {
        delete tableView[i];
        delete tableModelList[i];
    }
    tableView.clear();
    tableModelList.clear();
    time.restart();
    db.transaction();
    for (int i = 0; i < sqlCodeList.size(); ++i)
    {
//        qDebug() << sqlCodeList[i] << endl;
//        QString sqlCode = sqlCodeList[i].toUpper();
//        query.exec(sqlCodeList[i]);
        QSqlQuery query = db.exec(sqlCodeList[i]);
        if (!query.isActive())
        {
            resultText->setPlainText(query.lastError().text());
            return;
        }
        if (query.isSelect())
        {
            createNewTable(cnt++, sqlCodeList[i]);
        }
    }
    db.commit();
    resultText->setPlainText(tr("Query Sucessfully\nTime elapsed: %1 ms").arg(time.elapsed()));
//    tabWidget->setVisible(true);
}

void QueryWidget::createNewTable(int num, const QString &query)
{
    QSqlDatabase db = QSqlDatabase::database(connectName);
    QSqlTableModel *model = new QSqlTableModel(this, db);
    model->QSqlQueryModel::setQuery(query, db);
    if (model->lastError().isValid())
    {
        delete model;
        return;
    }
    QTableView *view = new QTableView;
    view->setModel(model);
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->resizeColumnsToContents();
    tableView.push_back(view);
    tableModelList.push_back(model);
    tabWidget->addTab(view, tr("Table") + QString::number(num));
}

void QueryWidget::saveAs()
{
    QString sqlFileName = QFileDialog::getSaveFileName(this, tr("Save as"), "",
                                                       tr("Database files (*.sql)"));
    if (!sqlFileName.isEmpty())
    {
        QFile file(sqlFileName);
        file.open(QIODevice::WriteOnly);
        QString str = sqlCodeEditor->toPlainText();
        str.replace("\n", "\r\n");
        file.write(str.toAscii());
        file.close();
    }
}

void QueryWidget::openSql()
{
    QString sqlFileName = QFileDialog::getOpenFileName(this,
                                                       tr("Open SQL file"), ".",
                                                       tr("Sql files (*.sql | *.txt)"));
    if (!sqlFileName.isEmpty())
    {
        QFile file(sqlFileName);
        file.open(QIODevice::ReadOnly);
        QTextStream stream(&file);
        sqlCodeEditor->setPlainText(stream.readAll());
        file.close();
    }
}

void QueryWidget::closeEvent(QCloseEvent *event)
{
    int r = QMessageBox::warning(this, tr("Save New Query"),
                    tr("Do you want to save your changes?"),
                    QMessageBox::Yes | QMessageBox::Default,
                    QMessageBox::No,
                    QMessageBox::Cancel | QMessageBox::Escape);

    if (r == QMessageBox::Cancel)
    {
        event->ignore();
        return;
    }

    if (r == QMessageBox::Yes)
        saveAs();

    event->accept();
}

void QueryWidget::runLineSql()
{
    QString sqlStr = sqlCodeEditor->textCursor().selection().toPlainText();
    if (!sqlStr.isEmpty())
    {
        for (int i = 0; i < tableView.size(); ++i)
        {
            delete tableView[i];
            delete tableModelList[i];
        }
        tableView.clear();
        tableModelList.clear();

        resultText->clear();
        QSqlDatabase db = QSqlDatabase::database(connectName);
        QSqlQuery query(db);
        time.restart();
        query.exec(sqlStr);
        if (!query.isActive())
        {
            resultText->setPlainText(query.lastError().text());
            return;
        }
        if (query.isSelect())
        {
            createNewTable(1, sqlStr);
            QString str = tr("%1\nQuery Sucessfully\nTime elapsed: %2 ms").arg(sqlStr).arg(time.elapsed());
            resultText->setPlainText(str);
        }
        else
        {
            QString rows = QString::number(query.numRowsAffected());
            QString str = tr("%1\nQuery Sucessfully\nRow(s) affected: %2\nTime elapsed: %3 ms").arg(sqlStr).arg(rows).arg(time.elapsed());
            resultText->setPlainText(str);
        }
//        tabWidget->setVisible(true);
    }
}

void QueryWidget::analysisSql()
{
    QString sqlStr = sqlCodeEditor->textCursor().selection().toPlainText();
    if (!sqlStr.isEmpty())
    {
        for (int i = 0; i < tableView.size(); ++i)
        {
            delete tableView[i];
            delete tableModelList[i];
        }
        tableView.clear();
        tableModelList.clear();

        QSqlDatabase db = QSqlDatabase::database(connectName);
        QSqlQuery query(db);

        resultText->clear();
        if (!query.prepare(sqlStr))
        {
            resultText->setPlainText(query.lastError().text());
            return;
        }
        resultText->setPlainText(tr("Analysis Sucessfully"));
//        tabWidget->setVisible(true);
    }
}

void QueryWidget::setConnectName(const QString &connectName)
{
    this->connectName = connectName;
    databaseInitialize();
}

void QueryWidget::databaseInitialize()
{
    QSqlDatabase db = QSqlDatabase::database(connectName);
    QStringList tableName = db.tables(QSql::Tables);
    QSet<QString> fieldName;
//    qDebug() << "next" << db.tables(QSql::Views);
    for (int i = 0; i < tableName.size(); ++i)
    {
        QSqlRecord record = db.record(tableName[i]);
        for (int j = 0; j < record.count(); ++j)
        {
            QString name = record.fieldName(j);
            fieldName.insert(name);
        }
    }

    QSet<QString> str = fieldName + QSet<QString>::fromList(db.tables(QSql::AllTables));
//    qDebug() << str;
    sqlCodeEditor->addString(str);
    highlighter->addString(str);
}

void QueryWidget::copy()
{
    sqlCodeEditor->copy();
}

void QueryWidget::paste()
{
    sqlCodeEditor->paste();
}

void QueryWidget::cut()
{
    sqlCodeEditor->cut();
}

void QueryWidget::del()
{
    sqlCodeEditor->insertPlainText("");
}

void QueryWidget::findNext(const QString &str)
{
    if (sqlCodeEditor->find(str))
    {
        activateWindow();
    }
    else
    {
        QApplication::beep();
    }
    setFocus();
}

void QueryWidget::findPrevious(const QString &str)
{
    if (sqlCodeEditor->find(str, QTextDocument::FindBackward))
    {
        activateWindow();
    }
    else
    {
        QApplication::beep();
    }
    setFocus();
}

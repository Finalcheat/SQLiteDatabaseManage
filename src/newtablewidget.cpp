#include "newtablewidgett.h"
#include "datatypedelegate.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QCloseEvent>
#include <QDebug>

NewTableWidget::NewTableWidget(const QString &c, QWidget *parent) :
    QWidget(parent), connectName(c)
{
    tableWidget = new QTableWidget;
    tableWidget->setRowCount(1);
    tableWidget->setColumnCount(4);
    DataTypeDelegate *dataTypeDelegate = new DataTypeDelegate(1, 2, 3);
    tableWidget->setItemDelegate(dataTypeDelegate);
    tableWidget->setHorizontalHeaderLabels(
                QStringList() << tr("Field Name") << tr("Data Type")
                << tr("Primary") << tr("Allow Null"));

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(tableWidget);
    setLayout(mainLayout);

    connect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(tableItemChanged(QTableWidgetItem*)));
//    connect(dataTypeDelegate, SIGNAL(primaryChanged(int)), this, SLOT(setPrimary(int)));
}

NewTableWidget::~NewTableWidget()
{
//    delete tableWidget;
}

void NewTableWidget::tableItemChanged(QTableWidgetItem *item)
{
    if (item->column() != 0)
        return;

    if (item->text().isEmpty())
        return;

    int rows = tableWidget->rowCount();
    int count = 0;
    for (int i = 0; i < rows; ++i)
    {
        QTableWidgetItem *firstItem = tableWidget->item(i, 0);
        if (firstItem && !firstItem->data(Qt::DisplayRole).toString().isEmpty())
        {
            ++count;
        }
    }
    if (count == rows)
    {
        tableWidget->insertRow(rows);
    }
}

//void NewTableWidget::setPrimary(int row)
//{
//    for (int i = 0; i < tableWidget->rowCount(); ++i)
//    {
//        if (i != row)
//        {
//            QTableWidgetItem *item = tableWidget->item(i,2);
//            if (item)
//                item->setData(Qt::DisplayRole, QVariant(false));
//        }
//    }
//}

bool NewTableWidget::save()
{
    int r = QMessageBox::warning(this, tr("Save New Table"),
                    tr("Do you want to save your changes?"),
                    QMessageBox::Yes | QMessageBox::Default,
                    QMessageBox::No,
                    QMessageBox::Cancel | QMessageBox::Escape);

    if (r == QMessageBox::Cancel)
    {
        return false;
    }
    else if (r == QMessageBox::No)
    {
        return true;
    }


    bool ok = false;
    QString newTableName = QInputDialog::getText(this, tr("Edit Table Name"),
                                                 tr("Save new table name:"),
                                                 QLineEdit::Normal, tr("New Table"),
                                                 &ok, 0, Qt::ImhNoPredictiveText);

    ok = !newTableName.isEmpty() && ok;
    if (ok)
    {
        QString sqlStr = tr("create table '") + newTableName + "' (";
        int i = 0;
        int rowCount = tableWidget->rowCount();
        QString primaryField;
        while (i < rowCount - 1)
        {
            QString fieldName = tableWidget->item(i, 0)->data(Qt::DisplayRole).toString();

            QTableWidgetItem *item = tableWidget->item(i, 1);
            QString fieldType;
            if (item)
                fieldType = item->data(Qt::DisplayRole).toString();

            item = tableWidget->item(i, 2);
            bool isPrimary = false;
            if (item)
                isPrimary = tableWidget->item(i, 2)->data(Qt::DisplayRole).toBool();

            //        sqlStr += fieldName;
            sqlStr = sqlStr + "'" + fieldName + "'";
            sqlStr.append(tr(" ")).append(fieldType);

            if (isPrimary)
                primaryField = primaryField + "'"+ fieldName + "',";
            else
            {
                item = tableWidget->item(i, 3);
                bool isNull = false;
                if (item)
                    isNull = item->data(Qt::DisplayRole).toBool();

                if (!isNull)
                {
                    sqlStr += tr(" not null");
                }
            }

            sqlStr.append(tr(","));
            ++i;
        }
        int index;
        if (!primaryField.isEmpty())
        {
            primaryField = "primary key(" + primaryField;
            sqlStr += primaryField;
            index = sqlStr.lastIndexOf(',');
            sqlStr.replace(index, 2, "))");
//            qDebug() << sqlStr << endl;
        }
        else
        {
            index = sqlStr.lastIndexOf(',');
            sqlStr.replace(index, 1, ')');
        }

        QSqlDatabase db = QSqlDatabase::database(connectName);
        QSqlQuery query(db);
        query.exec(sqlStr);
        if (!query.isActive())
        {
            QMessageBox::information(this, tr("SQLite Database Manage"), query.lastError().text());
            return false;
        }
    }
    return ok;
}

void NewTableWidget::closeEvent(QCloseEvent *event)
{
    if (save())
        event->accept();
    else
        event->ignore();
}

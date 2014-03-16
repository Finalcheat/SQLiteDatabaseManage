#include "modifytablefieldwidget.h"
#include "datatypedelegate.h"
#include <QGroupBox>
#include <QTableWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QMessageBox>
#include <QSqlError>
#include <QSqlIndex>
#include <QDebug>

ModifyTableFieldWidget::ModifyTableFieldWidget(const QString &connectName, const QString &tableName, QWidget *parent) :
    QWidget(parent)
{
    this->connectName = connectName;
    this->tableName = tableName;

    groupBox = new QGroupBox(tableName, this);
    tableWidget = new QTableWidget;
    DataTypeDelegate *dataTypeDelegate = new DataTypeDelegate(1, 2, 3);
    tableWidget->setItemDelegate(dataTypeDelegate);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    QVBoxLayout *topLayout = new QVBoxLayout;
    topLayout->addWidget(tableWidget);
    groupBox->setLayout(topLayout);

    addButton = new QPushButton(tr("Add"));
    submitButton = new QPushButton(tr("Submit"));
    deleteButton = new QPushButton(tr("Delete"));
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addStretch();
    bottomLayout->addWidget(addButton);
    bottomLayout->addWidget(deleteButton);
    bottomLayout->addWidget(submitButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(groupBox);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);
//    setWindowTitle(tr("Set Table Field"));

    setTableWidget();

//    resize(550, 300);

//    connect(dataTypeDelegate, SIGNAL(primaryChanged(int)), this, SLOT(setPrimary(int)));
    connect(addButton, SIGNAL(clicked()), this, SLOT(addRow()));
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteRow()));
    connect(submitButton, SIGNAL(clicked()), this, SLOT(submit()));
}

void ModifyTableFieldWidget::setTableWidget()
{
    QSqlDatabase db = QSqlDatabase::database(connectName);
    QSqlRecord record = db.record(tableName);
//    for (int i = 0; i < record.count(); ++i)
//        qDebug() << record.field(i).requiredStatus() << endl;
//    record.isNull();
//    db.primaryIndex(tableName);
    tableWidget->setRowCount(record.count());
    tableWidget->setColumnCount(4);
    tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Field")
                                           << tr("Type") << tr("Primary")
                                           << tr("Allow Null"));


//    QSqlQuery query(db);
//    query.exec(tr("select sql from sqlite_master where tbl_name='%1'").arg(tableName));
//    if (!query.isActive())
//    {
//        QMessageBox::information(this, tr("SQLite GUI"), query.lastError().text());
//        return;
//    }
//    if (query.next())
//    {
//        QString sql = query.value(0).toString();
//        int firstIndex = sql.indexOf(tr("("));
//        QString str = sql.mid(firstIndex + 1, sql.size() - firstIndex - 2);
//        analysisFiedl(str, record);
//    }


    QSqlIndex primaryRecord = db.primaryIndex(tableName);
    for (int i = 0; i < record.count(); ++i)
    {
        QString fieldName = record.fieldName(i);
        QTableWidgetItem *fieldNameItem = new QTableWidgetItem(fieldName);
        tableWidget->setItem(i, 0, fieldNameItem);

        int value = record.field(i).type();
//        qDebug() << record.fieldName(i) << record.field(i).type();
        QString fieldType;
        switch (value)
        {
        case QVariant::String :
            fieldType = "TEXT";
            break;
        case QVariant::ByteArray :
            fieldType = "BLOB";
            break;
        case QVariant::Int :
            fieldType = "INTEGER";
            break;
        case QVariant::Double :
            fieldType = "REAL";
            break;
        default:
            fieldType = "TEXT";
        }
        QTableWidgetItem *fieldTypeItem = new QTableWidgetItem(fieldType);
        tableWidget->setItem(i, 1, fieldTypeItem);

        bool primary = false;
        if (primaryRecord.contains(fieldName))
            primary = true;
        QTableWidgetItem *primaryItem = new QTableWidgetItem;
        primaryItem->setData(Qt::DisplayRole, primary);
        tableWidget->setItem(i, 2, primaryItem);

        bool isNull = !record.field(i).requiredStatus();
        QTableWidgetItem *isNullItem = new QTableWidgetItem;
        isNullItem->setData(Qt::DisplayRole, isNull);
        tableWidget->setItem(i, 3, isNullItem);

        originalStr.push_back(Field(fieldName, fieldType, primary, isNull));
    }

}

//void ModifyTableFieldWidget::analysisFiedl(const QString &str, const QSqlRecord &record)
//{
//    QStringList strList = str.split(tr(","));

//    for (int i = 0; i < strList.size(); ++i)
//    {
//        QStringList singleField = strList[i].split(tr(" "), QString::SkipEmptyParts);

////        QString fieldName = singleField[0];
//        QString fieldName = record.fieldName(i);
//        QTableWidgetItem *fieldNameItem = new QTableWidgetItem(fieldName);
//        tableWidget->setItem(i, 0, fieldNameItem);
//        Field recordField(fieldName);

//        QTableWidgetItem *isNullItem = new QTableWidgetItem;
//        isNullItem->setData(Qt::DisplayRole, true);
//        tableWidget->setItem(i, 3, isNullItem);

//        for (int j = 1; j < singleField.size(); ++j)
//        {
//            QString fieldStr = singleField[j].toUpper();
//            if (fieldStr == tr("INTEGER") ||
//                    fieldStr == tr("TEXT") ||
//                    fieldStr == tr("BLOB") ||
//                    fieldStr == tr("REAL"))
//            {
//                QTableWidgetItem *typeItem = new QTableWidgetItem(fieldStr);
//                tableWidget->setItem(i, 1, typeItem);
//                recordField.type = fieldStr;
//            }
//            else if (fieldStr == tr("PRIMARY"))
//            {
//                QTableWidgetItem *primaryItem = new QTableWidgetItem;
//                primaryItem->setData(Qt::DisplayRole, true);
//                tableWidget->setItem(i, 2, primaryItem);
//                recordField.primary = true;
//                break;
//            }
//            else if (fieldStr == tr("NOT"))
//            {
//                QTableWidgetItem *item = tableWidget->item(i, 3);
//                item->setData(Qt::DisplayRole, false);
//                recordField.isNull = false;
//                break;
//            }
//        }
//        originalStr.push_back(recordField);
//    }
//}

//void ModifyTableFieldWidget::setPrimary(int row)
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

void ModifyTableFieldWidget::addRow()
{
    int row = tableWidget->rowCount();
    tableWidget->insertRow(row);
    QTableWidgetItem *item = new QTableWidgetItem;
    QTableWidgetItem *isNullItem = new QTableWidgetItem;
    isNullItem->setData(Qt::DisplayRole, true);
    tableWidget->setItem(row, 0, item);
    tableWidget->setItem(row, 3, isNullItem);
    tableWidget->editItem(item);
}

void ModifyTableFieldWidget::deleteRow()
{
    QList<QTableWidgetItem*> item = tableWidget->selectedItems();

    if (!item.isEmpty())
    {
        int row = item.first()->row();
        tableWidget->removeRow(row);
        originalStr.removeAt(row);
    }
}

void ModifyTableFieldWidget::submit()
{
    int r = QMessageBox::warning(this, tr("SQLite GUI"),
                                 tr("Are you sure save your changes?"),
                                 QMessageBox::Yes | QMessageBox::Default,
                                 QMessageBox::No,
                                 QMessageBox::Cancel | QMessageBox::Escape);

    if (r == QMessageBox::No || r == QMessageBox::Cancel)
        return;


    QList<Field> newFieldList;
    for (int i = 0; i < tableWidget->rowCount(); ++i)
    {
        Field field;
        QTableWidgetItem *fieldNameItem = tableWidget->item(i, 0);
        if (fieldNameItem)
        {
            QString fieldName = fieldNameItem->text();
            if (fieldName.isEmpty())
                return;

            field.name = fieldName;
        }

        QTableWidgetItem *fieldTypeItem = tableWidget->item(i, 1);
        if (fieldTypeItem)
        {
            QString fieldType = fieldTypeItem->text();
            if (fieldType.isEmpty())
                fieldType = "TEXT";
            field.type = fieldType;
        }

        QTableWidgetItem *primaryItem = tableWidget->item(i, 2);
        if (primaryItem)
        {
            field.primary = primaryItem->data(Qt::DisplayRole).toBool();
        }

        QTableWidgetItem *isNullItem = tableWidget->item(i, 3);
        if (isNullItem)
        {
            field.isNull = isNullItem->data(Qt::DisplayRole).toBool();
        }

        newFieldList.push_back(field);
    }

//    for (int i = 0; i < newFieldList.size(); ++i)
//    {
//        Field temp = newFieldList[i];
//        qDebug() << temp.name << " " << temp.type << " " << temp.primary << temp.isNull << endl;
//        INSERT INTO fieldtest2(int,text,blob) select int,text,blob FROM fieldtest
//    }

    QString str = createNewTableStr(newFieldList);
    QSqlDatabase db = QSqlDatabase::database(connectName);
//    QSqlQuery query(db);
//    query.exec(str);
    db.transaction();
    db.exec(str);
    str = oldTableInsertToNewTableStr(newFieldList);
//    query.exec(str);
    db.exec(str);
    newFieldInsertToNewTable(db, newFieldList);

    db.exec(tr("drop table %1").arg(tableName));
    db.exec(tr("alter table temp%1temp rename to %2").arg(tableName).arg(tableName));
    db.commit();
}

QString ModifyTableFieldWidget::createNewTableStr(const QList<Field> &newFieldList)
{
    QString str = tr("create table temp%1temp(").arg(tableName);
    QString primaryField;
    for (int i = 0; i < newFieldList.size(); ++i)
    {
        str = str + "'" + newFieldList[i].name + "' " + newFieldList[i].type;
//        str += newFieldList[i].name;
//        str += tr(" ");
//        str += newFieldList[i].type;
        if (newFieldList[i].primary)
        {
            primaryField = primaryField + "'" + newFieldList[i].name + "',";
//            str += tr(" primary key");
        }
        else if (!newFieldList[i].isNull)
        {
            str += tr(" not null");
        }
        str += ",";
    }

    int index;
    if (!primaryField.isEmpty())
    {
        primaryField = "primary key(" + primaryField;
        str += primaryField;
        index = str.lastIndexOf(',');
        str.replace(index, 2, "))");
    }
    else
    {
        index = str.lastIndexOf(',');
        str.replace(index, 1, ')');
    }
//    qDebug() << str << endl;

    return str;
}

QString ModifyTableFieldWidget::oldTableInsertToNewTableStr(const QList<Field> &newFieldList)
{
//        INSERT INTO fieldtest2(int,text,blob) select int,text,blob FROM fieldtest
    QString oriStr;
    QString newFieldStr = "(";
//    qDebug() << "o\t" << originalStr.size() << endl << "n\t" << newFieldList.size() << endl;
    for (int i = 0; i < originalStr.size(); ++i)
    {
//        oriStr = oriStr + "'" + originalStr[i].name + "',";
        oriStr += originalStr[i].name;
        oriStr += ",";

        newFieldStr = newFieldStr + "'" + newFieldList[i].name + "',";
//        newFieldStr += newFieldList[i].name;
//        newFieldStr += ",";
    }
    int index = oriStr.lastIndexOf(",");
    oriStr.replace(index, 1, " ");
    index = newFieldStr.lastIndexOf(",");
    newFieldStr.replace(index, 1, ")");

    QString str = tr("insert into temp%1temp%2 select %3from %4").arg(tableName).arg(newFieldStr).arg(oriStr).arg(tableName);
//    qDebug() << str << endl;
    return str;
}

void ModifyTableFieldWidget::newFieldInsertToNewTable(QSqlDatabase &db, const QList<Field> &newFieldList)
{
    const QString str = tr("alter table temp%1temp add column ").arg(tableName);
    QString queryStr;
    for (int i = originalStr.size(); i < newFieldList.size(); ++i)
    {
        queryStr = str + "'" + newFieldList[i].name + "' ";
//        queryStr += tr(" ");
        queryStr += newFieldList[i].type;
        if (!queryStr.isNull())
        {
            queryStr += tr(" not null default ''");
        }
        db.exec(queryStr);
//        if (!query.isActive())
//        {
//            QMessageBox::information(this, tr("SQLite GUI"), query.lastError().text());
//        }
    }
}

#include "createindexwidget.h"
#include "datatypedelegate.h"
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTableWidget>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QGroupBox>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlError>

CreateIndexWidget::CreateIndexWidget(const QString &cName, const QString &tName,
                                     QWidget *parent) :
    QWidget(parent), connectName(cName), tableName(tName)
{
    typeList << tr("Ascending") << tr("Descending");

    tableWidget = new QTableWidget;
    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(tableWidget);
    QGroupBox *groupBox = new QGroupBox(tableName);
    groupBox->setLayout(topLayout);

    indexNameLabel = new QLabel(tr("In&dex Name:"));
    indexNameLineEdit = new QLineEdit;
    indexNameLabel->setBuddy(indexNameLineEdit);
    QRegExp regExp("[A-Za-z].*");
    indexNameLineEdit->setValidator(new QRegExpValidator(regExp, this));
    createButton = new QPushButton(tr("Create"));
    createButton->setEnabled(false);

    QHBoxLayout *hboxLayout = new QHBoxLayout;
    hboxLayout->addWidget(indexNameLabel);
    hboxLayout->addWidget(indexNameLineEdit);
    hboxLayout->addStretch();
    hboxLayout->addWidget(createButton);

    createInformation = new QPlainTextEdit;
    createInformation->setReadOnly(true);
//    createInformation->setMaximumHeight(100);
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(createInformation);
    QGroupBox *resultBox = new QGroupBox(tr("Result"));
    resultBox->setLayout(bottomLayout);
    resultBox->setMaximumHeight(200);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(groupBox);
    mainLayout->addLayout(hboxLayout);
    mainLayout->addWidget(resultBox);

    setLayout(mainLayout);

    setTableWidget();

//    setWindowTitle(tr("Create Index"));

//    resize(570, 410);

    connect(indexNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(lineEditChanged(const QString&)));
    connect(tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), this, SLOT(tableItemChanged(QTableWidgetItem*)));
    connect(createButton, SIGNAL(clicked()), this, SLOT(createButtonClicked()));
}

void CreateIndexWidget::setTableWidget()
{
    DataTypeDelegate *dataTypeDelegate = new DataTypeDelegate(1, 2, 3);
    dataTypeDelegate->setComboBoxItem(typeList);
    tableWidget->setItemDelegate(dataTypeDelegate);
    tableWidget->setColumnCount(4);
    tableWidget->setHorizontalHeaderLabels(QStringList() << tr("Field")
                                           << tr("Asc/Desc") << tr("Primary")
                                           << tr("IsUse"));
    tableWidget->setColumnHidden(2, true);
    QSqlDatabase db = QSqlDatabase::database(connectName);
    QSqlRecord record = db.record(tableName);
    tableWidget->setRowCount(record.count());
    for (int i = 0; i < record.count(); ++i)
    {
        QString fieldName = record.fieldName(i);
        QTableWidgetItem *item = new QTableWidgetItem(fieldName);
        tableWidget->setItem(i, 0, item);
    }

//    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void CreateIndexWidget::lineEditChanged(const QString &str)
{
    somethingChanged(str);
}

void CreateIndexWidget::tableItemChanged(QTableWidgetItem*)
{
    QString str = indexNameLineEdit->text();
    somethingChanged(str);
}

void CreateIndexWidget::somethingChanged(const QString &str)
{
    bool b = false;
    for (int i = 0; i < tableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *item = tableWidget->item(i, 3);
        QTableWidgetItem *typeItem = tableWidget->item(i, 1);
        if (item && typeItem && item->data(Qt::DisplayRole).toBool() && !typeItem->text().isEmpty())
        {
            b = true;
            break;
        }
    }
    createButton->setEnabled(!str.isEmpty() && b);
}

void CreateIndexWidget::createButtonClicked()
{
    QString str = tr("create index %1 on %2 (").arg(indexNameLineEdit->text())
            .arg(tableName);
    for (int i = 0; i < tableWidget->rowCount(); ++i)
    {
        QTableWidgetItem *item = tableWidget->item(i, 3);
        QTableWidgetItem *typeItem = tableWidget->item(i, 1);
        if (item && typeItem && item->data(Qt::DisplayRole).toBool() && !typeItem->text().isEmpty())
        {
            str += tableWidget->item(i, 0)->text();
            str += " ";
            QString typeText = typeItem->text();
            if (typeText == "Ascending")
                str += "ASC";
            else
                str += "DESC";

            str += ",";
        }
    }
    int index = str.lastIndexOf(',');
    str.replace(index, 1, ')');

    QSqlDatabase db = QSqlDatabase::database(connectName);
    QSqlQuery query(db);
    query.exec(str);
    QString informationStr;
    informationStr = createInformation->toPlainText();
    if (query.isActive())
    {
        informationStr += tr("Index %1 created successfully.\n").arg(indexNameLineEdit->text());
        createInformation->setPlainText(informationStr);
        indexNameList.push_back(indexNameLineEdit->text());
    }
    else
    {
        informationStr += query.lastError().text();
        createInformation->setPlainText(informationStr + "\n");
    }

}

#include "importwizard.h"
#include "qexcel.h"
#include "mylistmodel.h"
#include "workthread.h"
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QRadioButton>
#include <QComboBox>
#include <QCheckBox>
#include <QProgressBar>
#include <QFileDialog>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlIndex>
#include <QSqlError>
#include <QtXml>
#include <QMessageBox>
#include <QListView>
#include <QStringListModel>
#include <QDebug>

ImportWizard::ImportWizard(const QMap<QString, QString> &str,QWidget *parent) :
    QWizard(parent), strMap(str)
{
//    addPage(new ImportIntroPage);
//    addPage(new ImportInfoPage);
//    addPage(new ImportTargetInfoPage(str));
//    progressPage = new ImportProgressInfoPage;
//    addPage(progressPage);
    setPage(PageIntro, new ImportIntroPage);
    setPage(PageSourceInfo, new ImportInfoPage);
    setPage(PageSourceTxtInfo, new ImportTxtInfoPage);
    setPage(PageSoruceExcelInfo, new ImportExcelInfoPage);
    setPage(PageSourceAccessInfo, new ImportAccessInfoPage);
    setPage(PageSourceSQLServerInfo, new ImportSQLServerInfoPage);
    setPage(PageSourceMySQLInfo, new ImportMySQLInfoPage);
    setPage(PageSourceXmlInfo, new ImportXmlInfoPqge);
    importTable = new ImportTableInfoPage;
    setPage(PageTableInfo, importTable);
    importTarget = new ImportTargetInfoPage(str);
    setPage(PageTargetInfo, importTarget);
    progressPage = new ImportProgressInfoPage;
    setPage(PageFinishInfo, progressPage);
    setWindowTitle(tr("Import Wizard"));
}

void ImportWizard::accept()
{
    QString finishText = buttonText(QWizard::FinishButton);
    if (finishText == tr("Finish"))
    {
        QDialog::accept();
        return;
    }

    QSqlDatabase target = getTargetDatabase();

    if (field("accessRadio").toBool())
    {
        QString accessName = field("accessName").toString();
        {
            QSqlDatabase source = QSqlDatabase::addDatabase("QODBC", "QODBCTempConnect");
            source.setDatabaseName("DRIVER={Microsoft Access driver (*.mdb, *.accdb)};FIL={MS Access};DBQ=" + accessName);

            if (!source.open())
                return;

//            importAccess(source, target);
            importData(source, target);
        }
//        QSqlDatabase::removeDatabase(accessName);
    }
    else if (field("excelRadio").toBool())
    {
        QString excelName = field("excelName").toString();
        importExcel(excelName, target);
    }
    else if (field("txtRadio").toBool())
    {
        QString txtName = field("txtName").toString();
        importTxt(txtName, target);
    }
    else if (field("sqlserverRadio").toBool())
    {
        QString databaseName = field("sqlserverDatabaseName").toString();
        QString hostName = field("sqlserverHostName").toString();
        QString dsn = "Driver={sql server};server=" + hostName + ";database=" + databaseName;
        {
            QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "QODBCTempConnect");
            db.setDatabaseName(dsn);
            QString userName = field("sqlserverUserName").toString();
            if (!userName.isEmpty())
                db.setUserName(userName);

            QString password = field("sqlserverPassword").toString();
            if (!password.isEmpty())
                db.setPassword(password);

            QString port = field("sqlserverPort").toString();
            if (!port.isEmpty())
            {
                bool ok;
                int i = port.toInt(&ok);
                if (ok)
                    db.setPort(i);
            }

            if (db.open())
                importData(db, target);
        }
//        QSqlDatabase::removeDatabase("temp");
    }
    else if (field("xmlRadio").toBool())
    {
        QString fileName = field("xmlName").toString();
        importXml(fileName, target);
    }

//    QDialog::accept();
}

void ImportWizard::importTxt(const QString &source, QSqlDatabase &target)
{
    setOptions(QWizard::DisabledBackButtonOnLastPage);
    button(QWizard::FinishButton)->setEnabled(false);
    txtWorkThread = new ImportTxtWorkThread(source, target);
    connect(txtWorkThread, SIGNAL(finished()), this, SLOT(txtWorkFinish()));
    connect(txtWorkThread, SIGNAL(finishPos(int)), this, SLOT(setProgress(int)));
    QFile file(source);
    file.open(QIODevice::ReadOnly);
    int num = file.readAll().count("\n");
//    qDebug() << num;
    file.close();
    progressPage->progressBar->setMaximum(num - 1);
    txtWorkThread->start();
}

void ImportWizard::txtWorkFinish()
{
    QAbstractButton *finishButton = button(QWizard::FinishButton);
    finishButton->setEnabled(true);
    setButtonText(QWizard::FinishButton, tr("Finish"));
    setOption(QWizard::DisabledBackButtonOnLastPage, false);
    delete txtWorkThread;
}

void ImportWizard::importData(QSqlDatabase &source, QSqlDatabase &target)
{
    setOptions(QWizard::DisabledBackButtonOnLastPage);
    button(QWizard::FinishButton)->setEnabled(false);
    QStringList tableNameList = importTable->getUseTableList();
    workThread = new ImportWorkThread(source, tableNameList, target);
    connect(workThread, SIGNAL(finished()), this, SLOT(workFinish()));
    connect(workThread, SIGNAL(finishPos(int)), this, SLOT(setProgress(int)));
    progressPage->progressBar->setMaximum(tableNameList.size());
    workThread->start();
}

void ImportWizard::importXml(const QString &filePath, QSqlDatabase &target)
{
    setOption(QWizard::DisabledBackButtonOnLastPage);
    button(QWizard::FinishButton)->setEnabled(false);
    QStringList tableNameList = importTable->getUseTableList();
    xmlWorkThread = new ImportXmlWorkThread(filePath, tableNameList, target);
    connect(xmlWorkThread, SIGNAL(maxProgress(int)), this, SLOT(setMaxProgress(int)));
    connect(xmlWorkThread, SIGNAL(finishPos(int)), this, SLOT(setProgress(int)));
    connect(xmlWorkThread, SIGNAL(finished()), this, SLOT(xmlWorkFinish()));
    xmlWorkThread->start();
}

void ImportWizard::xmlWorkFinish()
{
    QAbstractButton *finishButton = button(QWizard::FinishButton);
    finishButton->setEnabled(true);
    setButtonText(QWizard::FinishButton, tr("Finish"));
    setOption(QWizard::DisabledBackButtonOnLastPage, false);
    delete xmlWorkThread;
}

void ImportWizard::workFinish()
{
    QAbstractButton *finishButton = button(QWizard::FinishButton);
    finishButton->setEnabled(true);
    setButtonText(QWizard::FinishButton, tr("Finish"));
    setOption(QWizard::DisabledBackButtonOnLastPage, false);
//    progressPage->progressBar->setMaximum(1);
    delete workThread;
    QSqlDatabase::removeDatabase("QODBCTempConnect");
//    connect(finishButton, SIGNAL(clicked()), this, SLOT(close()));
}

void ImportWizard::setProgress(int value)
{
    progressPage->progressBar->setValue(value);
}

void ImportWizard::setMaxProgress(int maxValue)
{
    progressPage->progressBar->setMaximum(maxValue);
}

void ImportWizard::importExcel(const QString &source, QSqlDatabase &target)
{   
    setOptions(QWizard::DisabledBackButtonOnLastPage);
    button(QWizard::FinishButton)->setEnabled(false);
    QStringList tableNameList = importTable->getUseTableList();
//    static QExcel excel(source);
    excelWorkThread = new ImportExcelWorkThread(source, tableNameList, target);
    connect(excelWorkThread, SIGNAL(finished()), this, SLOT(excelWorkFinish()));
    connect(excelWorkThread, SIGNAL(finishPos(int)), this, SLOT(setProgress(int)));
    progressPage->progressBar->setMaximum(tableNameList.size());
    excelWorkThread->start();
}

void ImportWizard::excelWorkFinish()
{
    QAbstractButton *finishButton = button(QWizard::FinishButton);
    finishButton->setEnabled(true);
    setButtonText(QWizard::FinishButton, tr("Finish"));
    setOption(QWizard::DisabledBackButtonOnLastPage, false);
    QSqlDatabase::removeDatabase("ImportExcel");
    delete excelWorkThread;
}

QSqlDatabase ImportWizard::getTargetDatabase()
{
    QSqlDatabase db;
    QString connectName;
    if (!field("checkBox").toBool())
    {
        QString key = importTarget->getTarget();
        connectName = strMap.value(key);
//        qDebug() << connectName;
        db = QSqlDatabase::database(connectName);
    }
    else
    {
        connectName = field("newDatabaseName").toString();
        db = QSqlDatabase::addDatabase("QSQLITE", connectName);
        db.setDatabaseName(connectName);
        db.open();
    }

    return db;
}

ImportIntroPage::ImportIntroPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Introduction"));

    label = new QLabel(tr("This wizard will import Excel or Xml or Txt or Access or SQL Server or MySql "
                          "to table data."));
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}

ImportInfoPage::ImportInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Source Information"));
    setSubTitle(tr("Specify source information about the source type for which you "
                   "want to import."));

//    sourceLabel = new QLabel(tr("Source:"));
//    sourceLineEdit = new QLineEdit;
//    sourceLabel->setBuddy(sourceLineEdit);
//    sourceButton = new QPushButton("Select");

//    QHBoxLayout *topLayout = new QHBoxLayout;
//    topLayout->addWidget(sourceLabel);
//    topLayout->addWidget(sourceLineEdit);
//    topLayout->addWidget(sourceButton);

    groupBox = new QGroupBox(tr("Import File Ty&pe"));

    excelRadioButton = new QRadioButton(tr("&Excel"));
    registerField("excelRadio", excelRadioButton);
    xmlRadioButton = new QRadioButton(tr("Xm&l"));
    registerField("xmlRadio", xmlRadioButton);
    txtRadioButton = new QRadioButton(tr("Txt"));
    registerField("txtRadio", txtRadioButton);
    accessRadioButton = new QRadioButton(tr("&Access"));
    registerField("accessRadio", accessRadioButton);
    sqlserverRadioButton = new QRadioButton(tr("SQL &Server"));
    registerField("sqlserverRadio", sqlserverRadioButton);
    mysqlRadioButton = new QRadioButton(tr("Mysql"));
    registerField("mysqlRadio", mysqlRadioButton);
    mysqlRadioButton->setVisible(false);
    excelRadioButton->setChecked(true);

    QVBoxLayout *groupBoxLayout = new QVBoxLayout;
    groupBoxLayout->addWidget(excelRadioButton);
    groupBoxLayout->addWidget(xmlRadioButton);
    groupBoxLayout->addWidget(txtRadioButton);
    groupBoxLayout->addWidget(accessRadioButton);
    groupBoxLayout->addWidget(sqlserverRadioButton);
    groupBoxLayout->addWidget(mysqlRadioButton);

    groupBox->setLayout(groupBoxLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
//    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(groupBox);

    setLayout(mainLayout);

//    registerField("sourceName*", sourceLineEdit);

//    connect(sourceButton, SIGNAL(clicked()), this, SLOT(selectFilePath()));
}


int ImportInfoPage::nextId() const
{
    if (accessRadioButton->isChecked())
    {
        return ImportWizard::PageSourceAccessInfo;
    }
    else if (excelRadioButton->isChecked())
    {
        return ImportWizard::PageSoruceExcelInfo;
    }
    else if (txtRadioButton->isChecked())
    {
        return ImportWizard::PageSourceTxtInfo;
    }
    else if (sqlserverRadioButton->isChecked())
    {
        return ImportWizard::PageSourceSQLServerInfo;
    }
    else if (xmlRadioButton->isChecked())
    {
        return ImportWizard::PageSourceXmlInfo;
    }
    else
    {
        return ImportWizard::PageSourceMySQLInfo;
    }
}

ImportTxtInfoPage::ImportTxtInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Txt Information"));
    setSubTitle(tr("Please select source txt information."));

    sourceLabel = new QLabel(tr("Source:"));
    sourceLineEdit = new QLineEdit;
    registerField("txtName*", sourceLineEdit);
    sourceLabel->setBuddy(sourceLineEdit);
    sourceButton = new QPushButton("Select");

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(sourceLabel);
    topLayout->addWidget(sourceLineEdit);
    topLayout->addWidget(sourceButton);

    groupBox = new QGroupBox(tr("Column Separators"));
    commaRadioButton = new QRadioButton(tr("Comma"));
    commaRadioButton->setChecked(true);

    QGridLayout *gridLayout = new QGridLayout;
    gridLayout->addWidget(commaRadioButton, 0, 0);

    groupBox->setLayout(gridLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addWidget(groupBox);
//    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(sourceButton, SIGNAL(clicked()), this, SLOT(selectPath()));
}

int ImportTxtInfoPage::nextId() const
{
    return ImportWizard::PageTargetInfo;
}

void ImportTxtInfoPage::selectPath()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select Txt File"), ".",
                                                    tr("Txt File (*.txt)"));

    if (!fileName.isEmpty())
    {
        sourceLineEdit->setText(fileName);
    }
}

ImportAccessInfoPage::ImportAccessInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Access Information"));
    setSubTitle(tr("Specify source access information."));

    sourceLabel = new QLabel(tr("Source:"));
    sourceLineEdit = new QLineEdit;
    registerField("accessName*", sourceLineEdit);
    sourceLabel->setBuddy(sourceLineEdit);
    sourceButton = new QPushButton("Select");

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(sourceLabel);
    topLayout->addWidget(sourceLineEdit);
    topLayout->addWidget(sourceButton);

    connectInfoLabel = new QLabel(tr("Connect Information"));
    userNameLabel = new QLabel(tr("Us&ername:"));
    userNameLineEdit = new QLineEdit("Admin");
    userNameLabel->setBuddy(userNameLineEdit);
    passwordLabel = new QLabel(tr("Password:"));
    passwordLineEdit = new QLineEdit;
    passwordLineEdit->setEnabled(false);
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    passwordLabel->setBuddy(passwordLineEdit);
    passwordCheckBox = new QCheckBox(tr("Use Password"));
    testConnectButton = new QPushButton(tr("Test Connect"));

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(connectInfoLabel, 0, 0);
    layout->addWidget(userNameLabel, 1, 0);
    layout->addWidget(userNameLineEdit, 1, 1);
    layout->addWidget(passwordLabel, 2, 0);
    layout->addWidget(passwordLineEdit, 2, 1);
    layout->addWidget(passwordCheckBox, 3, 0);

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(testConnectButton);
    bottomLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(layout);
    mainLayout->addStretch();
    mainLayout->addLayout(bottomLayout);


    setLayout(mainLayout);

    connect(passwordCheckBox, SIGNAL(clicked(bool)), passwordLineEdit, SLOT(setEnabled(bool)));
    connect(sourceButton, SIGNAL(clicked()), this, SLOT(selectPath()));
    connect(testConnectButton, SIGNAL(clicked()), this, SLOT(databaseConnect()));
}

void ImportAccessInfoPage::selectPath()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select Access File"), ".",
                                                    tr("Access File (*.mdb || *.accdb)"));

    if (!fileName.isEmpty())
    {
        sourceLineEdit->setText(fileName);
    }
}

void ImportAccessInfoPage::databaseConnect()
{
    QString str = field("accessName").toString();
//    qDebug() << str;
    {
        QSqlDatabase source = QSqlDatabase::addDatabase("QODBC", str);
        source.setDatabaseName("DRIVER={Microsoft Access driver (*.mdb, *.accdb)};FIL={MS Access};DBQ=" + str);
        if (passwordCheckBox->isChecked())
        {
            source.setUserName(userNameLineEdit->text());
            source.setPassword(passwordLineEdit->text());
        }
        if (source.open())
        {
            QMessageBox::information(this, tr("Import"), tr("Connect Successful."));
        }
        else
        {
            QMessageBox::information(this, tr("Import"), source.lastError().text());
        }
    }
    QSqlDatabase::removeDatabase(str);
}

int ImportAccessInfoPage::nextId() const
{
    return ImportWizard::PageTableInfo;
}

ImportExcelInfoPage::ImportExcelInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Excel Information"));
    setSubTitle(tr("Specify Excel File Path."));
    sourceLabel = new QLabel(tr("&Source:"));
    sourceLineEdit = new QLineEdit;
    registerField("excelName*", sourceLineEdit);
    sourceLabel->setBuddy(sourceLineEdit);
    sourceButton = new QPushButton(tr("Select"));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(sourceLabel);
    topLayout->addWidget(sourceLineEdit);
    topLayout->addWidget(sourceButton);

    testConnectButton = new QPushButton(tr("Test Connect"));
    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(testConnectButton);
    bottomLayout->addStretch();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addStretch();
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);

    connect(sourceButton, SIGNAL(clicked()), this, SLOT(selectPath()));
    connect(testConnectButton, SIGNAL(clicked()), this, SLOT(excelConnect()));
}

void ImportExcelInfoPage::selectPath()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select Excel File"), ".",
                                                    tr("Excel File (*.xls || *.xlsx)"));

    if (!fileName.isEmpty())
    {
        sourceLineEdit->setText(fileName);
    }
}

void ImportExcelInfoPage::excelConnect()
{
    QString filePath = sourceLineEdit->text();
    if (!QFile::exists(filePath))
    {
        QMessageBox::information(this, tr("Information"), tr("File Not Exist."));
        return;
    }

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "ExcelConnect");
        QString dsn = "Driver={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};READONLY=FALSE;Dbq=" + filePath;
        db.setDatabaseName(dsn);

        if (db.open())
        {
            QMessageBox::information(this, tr("Information"), tr("Connect Successful."));
        }
        else
        {
            QMessageBox::information(this, tr("Information"), tr("Connect Fail."));
        }
    }
    QSqlDatabase::removeDatabase("ExcelConnect");
}

int ImportExcelInfoPage::nextId() const
{
    return ImportWizard::PageTableInfo;
}

ImportSQLServerInfoPage::ImportSQLServerInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("SQL Server Information"));
    setSubTitle(tr("Specify SQL Server Connect Information."));

    databaseNameLabel = new QLabel(tr("&Database Name:"));
    databaseNameLineEdit = new QLineEdit;
    registerField("sqlserverDatabaseName*", databaseNameLineEdit);
    databaseNameLabel->setBuddy(databaseNameLineEdit);
    hostNameLabel = new QLabel(tr("&Host Name:"));
    hostNameLineEdit = new QLineEdit;
    registerField("sqlserverHostName", hostNameLineEdit);
    hostNameLabel->setBuddy(hostNameLineEdit);
    userNameLabel = new QLabel(tr("&User Name:"));
    userNameLineEdit = new QLineEdit;
    registerField("sqlserverUserName", userNameLineEdit);
    userNameLabel->setBuddy(userNameLineEdit);
    passwordLabel = new QLabel(tr("&Password:"));
    passwordLineEdit = new QLineEdit;
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    registerField("sqlserverPassword", passwordLineEdit);
    passwordLabel->setBuddy(passwordLineEdit);
    portLabel = new QLabel(tr("Port:"));
    portLineEdit = new QLineEdit;
    registerField("sqlserverPort", portLineEdit);
    portLabel->setBuddy(portLineEdit);

    testConnectButton = new QPushButton(tr("Test Connect"));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(databaseNameLabel, 0, 0);
    mainLayout->addWidget(databaseNameLineEdit, 0, 1);
    mainLayout->addWidget(hostNameLabel, 1, 0);
    mainLayout->addWidget(hostNameLineEdit, 1, 1);
    mainLayout->addWidget(userNameLabel, 2, 0);
    mainLayout->addWidget(userNameLineEdit, 2, 1);
    mainLayout->addWidget(passwordLabel, 3, 0);
    mainLayout->addWidget(passwordLineEdit, 3, 1);
    mainLayout->addWidget(portLabel, 4, 0);
    mainLayout->addWidget(portLineEdit, 4, 1);

    mainLayout->addWidget(testConnectButton, 5, 0);

    setLayout(mainLayout);

    connect(testConnectButton, SIGNAL(clicked()), this, SLOT(testConnect()));
}

int ImportSQLServerInfoPage::nextId() const
{
    return ImportWizard::PageTableInfo;
}

void ImportSQLServerInfoPage::testConnect()
{
    QString databaseName = databaseNameLineEdit->text();
    if (databaseName.isEmpty())
    {
        QMessageBox::information(this, tr("Information"), tr("Database Name Empty."));
        return;
    }
    QString hostName = hostNameLineEdit->text();
    QString dsn = "Driver={sql server};server=" + hostName + ";database=" + databaseName;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "temp");
        db.setDatabaseName(dsn);
        QString userName = userNameLineEdit->text();
        if (!userName.isEmpty())
            db.setUserName(userName);

        QString password = passwordLineEdit->text();
        if (!password.isEmpty())
            db.setPassword(password);

        QString port = portLineEdit->text();
        if (!port.isEmpty())
        {
            bool ok;
            int i = port.toInt(&ok);
            if (ok)
                db.setPort(i);
        }

        if (db.open())
        {
            QMessageBox::information(this, tr("Information"), tr("Connect Successful."));
        }
        else
        {
            QMessageBox::information(this, tr("Information"), db.lastError().text());
        }
    }
    QSqlDatabase::removeDatabase("temp");
}

ImportMySQLInfoPage::ImportMySQLInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("MySQL Information"));
    setSubTitle(tr("Specify MySQL Connect Information."));

    databaseNameLabel = new QLabel(tr("&Database Name:"));
    databaseNameLineEdit = new QLineEdit;
    registerField("mysqlName*", databaseNameLineEdit);
    databaseNameLabel->setBuddy(databaseNameLineEdit);
    hostNameLabel = new QLabel(tr("&Host Name:"));
    hostNameLineEdit = new QLineEdit;
    registerField("mysqlHostName", hostNameLineEdit);
    hostNameLabel->setBuddy(hostNameLineEdit);
    userNameLabel = new QLabel(tr("&User Name:"));
    userNameLineEdit = new QLineEdit;
    registerField("mysqlUserName", userNameLineEdit);
    userNameLabel->setBuddy(userNameLineEdit);
    passwordLabel = new QLabel(tr("&Password:"));
    passwordLineEdit = new QLineEdit;
    passwordLineEdit->setEchoMode(QLineEdit::Password);
    registerField("mysqlPassword", passwordLineEdit);
    passwordLabel->setBuddy(passwordLineEdit);
    portLabel = new QLabel(tr("Port:"));
    portLineEdit = new QLineEdit;
    registerField("mysqlPort", portLineEdit);
    portLabel->setBuddy(portLineEdit);

    testConnectButton = new QPushButton(tr("Test Connect"));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(databaseNameLabel, 0, 0);
    mainLayout->addWidget(databaseNameLineEdit, 0, 1);
    mainLayout->addWidget(hostNameLabel, 1, 0);
    mainLayout->addWidget(hostNameLineEdit, 1, 1);
    mainLayout->addWidget(userNameLabel, 2, 0);
    mainLayout->addWidget(userNameLineEdit, 2, 1);
    mainLayout->addWidget(passwordLabel, 3, 0);
    mainLayout->addWidget(passwordLineEdit, 3, 1);
    mainLayout->addWidget(portLabel, 4, 0);
    mainLayout->addWidget(portLineEdit, 4, 1);

    mainLayout->addWidget(testConnectButton, 5, 0);

    setLayout(mainLayout);

    connect(testConnectButton, SIGNAL(clicked()), this, SLOT(testConnect()));
}

int ImportMySQLInfoPage::nextId() const
{
    return ImportWizard::PageTableInfo;
}

void ImportMySQLInfoPage::testConnect()
{
    QString databaseName = databaseNameLineEdit->text();
    if (databaseName.isEmpty())
    {
        QMessageBox::information(this, tr("Information"), tr("Database Name Empty."));
        return;
    }

    QString hostName = hostNameLineEdit->text();
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL", "temp");
        db.setHostName(hostName);
        db.setDatabaseName(databaseName);
        QString userName = userNameLineEdit->text();
        qDebug() << userName;
        if (!userName.isEmpty())
            db.setUserName(userName);

        QString password = passwordLineEdit->text();
        qDebug() << password;
        if (!password.isEmpty())
            db.setPassword(password);

        QString port = portLineEdit->text();
        if (!port.isEmpty())
        {
            bool ok;
            int i = port.toInt(&ok);
            if (ok)
                db.setPort(i);
        }

        if (db.open())
        {
            QMessageBox::information(this, tr("Information"), tr("Connect Successful."));
        }
        else
        {
            QMessageBox::information(this, tr("Information"), db.lastError().text());
        }
    }
    QSqlDatabase::removeDatabase("temp");
}

ImportXmlInfoPqge::ImportXmlInfoPqge(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Xml Information"));
    setSubTitle(tr("Please select source xml information."));

    sourceLabel = new QLabel(tr("S&ource:"));
    sourceLineEdit = new QLineEdit;
    registerField("xmlName", sourceLineEdit);
    sourceLabel->setBuddy(sourceLineEdit);
    selectButton = new QPushButton(tr("Select"));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(sourceLabel);
    topLayout->addWidget(sourceLineEdit);
    topLayout->addWidget(selectButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(selectButton, SIGNAL(clicked()), this, SLOT(selectPath()));
}

void ImportXmlInfoPqge::selectPath()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Select Xml File"), ".",
                                                    tr("Xml File (*.xml)"));

    if (!fileName.isEmpty())
    {
        sourceLineEdit->setText(fileName);
    }
}

int ImportXmlInfoPqge::nextId() const
{
    return ImportWizard::PageTargetInfo;
}

ImportTableInfoPage::ImportTableInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Table Information"));
    setSubTitle(tr("Specify table information about the target database for which you "
                   "want to import."));
    listView = new QListView;
    listView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    model = new MyStringListModel;
    listView->setModel(model);

    selectAllButton = new QPushButton(tr("Select All"));
    unSelectAllButton = new QPushButton(tr("Unselect All"));

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(selectAllButton);
    rightLayout->addWidget(unSelectAllButton);
    rightLayout->addStretch();

    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(listView);
    mainLayout->addLayout(rightLayout);

    setLayout(mainLayout);

    connect(selectAllButton, SIGNAL(clicked()), this, SLOT(selectAllClicked()));
    connect(unSelectAllButton, SIGNAL(clicked()), this, SLOT(unselectAllClicked()));
}

void ImportTableInfoPage::selectAllClicked()
{
    model->selectAll();
    listView->setFocus();
}

void ImportTableInfoPage::unselectAllClicked()
{
    model->unselectAll();
    listView->setFocus();
}

void ImportTableInfoPage::initializePage()
{
    if (field("accessRadio").toBool())
    {
        initAccessTable();
    }
    else if (field("excelRadio").toBool())
    {
        initExcelTable();
    }
    else if (field("sqlserverRadio").toBool())
    {
        initSqlserverTable();
    }
    else if (field("xmlRadio").toBool())
    {
        initXmlTable();
    }
}

QStringList ImportTableInfoPage::getUseTableList() const
{
    return model->getUseStringList();
}

void ImportTableInfoPage::initAccessTable()
{
    QString str = field("accessName").toString();
    {
        QSqlDatabase source = QSqlDatabase::addDatabase("QODBC", str);
        source.setDatabaseName("DRIVER={Microsoft Access driver (*.mdb, *.accdb)};FIL={MS Access};DBQ=" + str);

        if (source.open())
        {
//            QStringList tableNameList;
//            QSqlQuery query = source.exec("select name from Msysobjects where type=1 and flags=0");
//            qDebug() << source.lastError().text();
//            while (query.next())
//                tableNameList.push_back(query.value(0).toString());

            model->setStringList(source.tables());
//            listView->setModel(model);
        }
    }
    QSqlDatabase::removeDatabase(str);
}

void ImportTableInfoPage::initExcelTable()
{
    QString filePath = field("excelName").toString();
    if (!QFile::exists(filePath))
        return;

    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "InitExcel");
        QString dsn = "Driver={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};READONLY=FALSE;Dbq=" + filePath;
        db.setDatabaseName(dsn);

        if (db.open())
        {
//            qDebug() << db.tables(QSql::AllTables);
            if (filePath.endsWith(".xls"))
            {
                model->setStringList(db.tables());
            }
            else
            {
                model->setStringList(db.tables(QSql::AllTables).replaceInStrings("$", ""));
            }
        }
    }
    QSqlDatabase::removeDatabase("InitExcel");
}

void ImportTableInfoPage::initSqlserverTable()
{
    QString databaseName = field("sqlserverDatabaseName").toString();
    QString hostName = field("sqlserverHostName").toString();
    QString dsn = "Driver={sql server};server=" + hostName + ";database=" + databaseName;
    {
        QSqlDatabase db = QSqlDatabase::addDatabase("QODBC", "temp");
        db.setDatabaseName(dsn);
        QString userName = field("sqlserverUserName").toString();
        if (!userName.isEmpty())
            db.setUserName(userName);

        QString password = field("sqlserverPassword").toString();
        if (!password.isEmpty())
            db.setPassword(password);

        QString port = field("sqlserverPort").toString();
        if (!port.isEmpty())
        {
            bool ok;
            int i = port.toInt(&ok);
            if (ok)
                db.setPort(i);
        }

        if (db.open())
        {
//            QTextCodec *code = QTextCodec::codecForLocale();
//            QTextCodec::setCodecForCStrings(code);

//            QStringList temp = db.tables();
////            QString t = code->toUnicode(temp);
//            for (int i = 0; i < temp.size(); ++i)
//            {
//                QByteArray b = temp[i].toLocal8Bit();
//                QTextStream stream(&b);
//                qDebug() << stream.readAll();
//            }
            QStringList tableNameList;
            QSqlQuery query = db.exec("select name from sysobjects where xtype = 'U'");
            while (query.next())
                tableNameList.push_back(query.value(0).toString());

//            qDebug() << tableNameList;

            model->setStringList(tableNameList);
//            listView->setModel(model);
        }
    }
    QSqlDatabase::removeDatabase("temp");
}

void ImportTableInfoPage::initXmlTable()
{
    QString fileName = field("xmlName").toString();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QDomDocument doc("tempDoc");
    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }

    QDomElement docElem = doc.documentElement();
    QDomNode n = docElem.firstChild();
    if (!n.isNull())
    {
        QDomElement e = n.toElement();

        if (!e.isNull())
        {
//            qDebug() << e.tagName() << ":" << e.text();
            QDomNode temp = e.firstChild();
            QStringList tableNameList;
            while (!temp.isNull())
            {
                QDomElement etemp = temp.toElement();
                if (!e.isNull())
                {
//                    qDebug() << etemp.tagName() << ":" << etemp.text();
                    tableNameList.push_back(etemp.tagName());
                }
                temp = temp.nextSibling();
            }

            model->setStringList(tableNameList);
        }

//        n = n.nextSibling();
    }
}

ImportTargetInfoPage::ImportTargetInfoPage(const QMap<QString, QString> &str, QWidget *parent) :
    QWizardPage(parent)
{

    setTitle(tr("Target Information"));
    setSubTitle(tr("Specify target information about the target database for which you "
                   "want to import."));

    targetLabel = new QLabel(tr("Target Database:"));
    targetComboBox = new QComboBox;
    targetComboBox->addItems(str.keys());
    targetComboBox->setCurrentIndex(-1);

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(targetLabel);
    topLayout->addWidget(targetComboBox);

    checkBox = new QCheckBox(tr("Use New Database"));

    newDatabaseNameLabel = new QLabel(tr("New Database Name:"));
    newDatabaseNameLineEdit = new QLineEdit;
    newDatabasePathButton = new QPushButton(tr("Select"));
    newDatabaseNameLabel->setEnabled(false);
    newDatabaseNameLineEdit->setEnabled(false);
    newDatabasePathButton->setEnabled(false);

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->addWidget(targetLabel, 0, 0);
    mainLayout->addWidget(targetComboBox, 0, 1);
    mainLayout->addWidget(newDatabaseNameLabel, 1, 0);
    mainLayout->addWidget(newDatabaseNameLineEdit, 1, 1);
    mainLayout->addWidget(newDatabasePathButton, 1, 2);
    mainLayout->addWidget(checkBox, 2, 0);

    //    QVBoxLayout *mainLayout = new QVBoxLayout;
    //    mainLayout->addLayout(topLayout);
    //    mainLayout->addWidget(checkBox);
    //    mainLayout->addWidget(groupBox);

    setLayout(mainLayout);

    connect(checkBox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxChanged(int)));
    connect(newDatabasePathButton, SIGNAL(clicked()), this, SLOT(selectPath()));
    connect(newDatabaseNameLineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged(QString)));

    registerField("targetComboBox*", targetComboBox);
    registerField("checkBox", checkBox);
    registerField("newDatabaseName", newDatabaseNameLineEdit);

}

void ImportTargetInfoPage::textChanged(const QString &str)
{
    wizard()->button(QWizard::NextButton)->setEnabled(checkBox->isChecked() && !str.isEmpty());
}

void ImportTargetInfoPage::selectPath()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save as"), "",
                                                    tr("Database files (*.db)"));
    if (!fileName.isEmpty())
    {
        newDatabaseNameLineEdit->setText(fileName);
    }
}

void ImportTargetInfoPage::checkBoxChanged(int state)
{
    if (state == Qt::Unchecked)
    {
        targetLabel->setEnabled(true);
        targetComboBox->setEnabled(true);
        newDatabaseNameLabel->setEnabled(false);
        newDatabaseNameLineEdit->setEnabled(false);
        newDatabasePathButton->setEnabled(false);
        wizard()->button(QWizard::NextButton)->setEnabled(targetComboBox->count());
    }
    else
    {
        targetLabel->setEnabled(false);
        targetComboBox->setEnabled(false);
        newDatabaseNameLabel->setEnabled(true);
        newDatabaseNameLineEdit->setEnabled(true);
        newDatabasePathButton->setEnabled(true);
        wizard()->button(QWizard::NextButton)->setEnabled(!newDatabaseNameLineEdit->text().isEmpty());
    }
}

QString ImportTargetInfoPage::getTarget() const
{
    return targetComboBox->currentText();
}

ImportProgressInfoPage::ImportProgressInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Import Progress Information"));
    progressBar = new QProgressBar;
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(progressBar);
    setLayout(mainLayout);

    registerField("progress", progressBar);
}

void ImportProgressInfoPage::initializePage()
{
    wizard()->setButtonText(QWizard::FinishButton, tr("Start"));
    progressBar->setValue(0);
}



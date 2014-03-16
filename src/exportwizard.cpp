#include "exportwizard.h"
#include "mylistmodel.h"
#include "workthread.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QRadioButton>
#include <QLineEdit>
#include <QSqlDatabase>
#include <QSqlRecord>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QFileDialog>
#include <QListView>
#include <QProgressBar>
#include <QMessageBox>
#include <QSqlError>
#include <QDebug>

ExportWizard::ExportWizard(const QMap<QString, QString> &str,QWidget *parent) :
    QWizard(parent), strMap(str)
{
    setPage(PageIntro, new ExportIntroPage);
    exportInfo =  new ExportInfoPage(str);
    setPage(PageSourceInfo, exportInfo);
    setPage(PageTargetInfo, new ExportTragetInfoPage);
    setPage(PageTargetTxtInfo, new ExportTxtInfoPage);
    setPage(PageTargetXmlInfo, new ExportXmlInfoPage);
    setPage(PageTargetExcelInfo, new ExportExcelInfoPage);
    setPage(PageTargetAccessInfo, new ExportAccessInfoPage);
    setPage(PageTargetSQLServerInfo, new ExportSQLServerInfoPage);
    exportFinishInfo = new ExportFinishInfoPage;
    setPage(PageFinishInfo, exportFinishInfo);
    setWindowTitle(tr("Export Wizard"));
}

void ExportWizard::accept()
{

    QString finishText = buttonText(QWizard::FinishButton);
    if (finishText == tr("Finish"))
    {
        QDialog::accept();
        return;
    }

    QString key = exportInfo->getSource();
    QSqlDatabase source = QSqlDatabase::database(strMap.value(key));

    if (field("txtRadio").toBool())
    {
        QString directory = field("txtName").toString();
        exportTxt(source, directory);
    }
    else if (field("accessRadio").toBool())
    {
        QString accessName = field("accessName").toString();
        QSqlDatabase target = QSqlDatabase::addDatabase("QODBC", "QODBCTempConnect");
        target.setDatabaseName("DRIVER={Microsoft Access driver (*.mdb, *.accdb)};FIL={MS Access};DBQ=" + accessName);

        if (!target.open())
            return;

        exportData(source, target);
    }
    else if (field("sqlserverRadio").toBool())
    {
        QString databaseName = field("sqlserverDatabaseName").toString();
        QString hostName = field("sqlserverHostName").toString();
        QString dsn = "Driver={sql server};server=" + hostName + ";database=" + databaseName;
        QSqlDatabase target = QSqlDatabase::addDatabase("QODBC", "QODBCTempConnect");
        target.setDatabaseName(dsn);
        QString userName = field("sqlserverUserName").toString();
        if (!userName.isEmpty())
            target.setUserName(userName);

        QString password = field("sqlserverPassword").toString();
        if (!password.isEmpty())
            target.setPassword(password);

        QString port = field("sqlserverPort").toString();
        if (!port.isEmpty())
        {
            bool ok;
            int i = port.toInt(&ok);
            if (ok)
                target.setPort(i);
        }

        if (target.open())
        {
            exportData(source, target);
        }

    }
    else if (field("excelRadio").toBool())
    {
        QString excelName = field("excelName").toString();
        exportExcel(source, excelName);
    }
    else
    {
        QString directory = field("xmlName").toString();
        exportXml(source, directory);
    }

}

void ExportWizard::exportData(QSqlDatabase &source, QSqlDatabase &target)
{
    setOptions(QWizard::DisabledBackButtonOnLastPage);
    button(QWizard::FinishButton)->setEnabled(false);
    QStringList tableNameList = exportInfo->getUseTableList();
    workThread = new ExportWorkThread(source, tableNameList, target);
    connect(workThread, SIGNAL(finished()), this, SLOT(workFinish()));
    connect(workThread, SIGNAL(finishPos(int)), this, SLOT(setProgress(int)));
    exportFinishInfo->progressBar->setMaximum(tableNameList.size());
    workThread->start();
}

void ExportWizard::workFinish()
{
    QAbstractButton *finishButton = button(QWizard::FinishButton);
    finishButton->setEnabled(true);
    setButtonText(QWizard::FinishButton, tr("Finish"));
    setOption(QWizard::DisabledBackButtonOnLastPage, false);
//    progressPage->progressBar->setMaximum(1);
    delete workThread;
    QSqlDatabase::removeDatabase("QODBCTempConnect");
}

void ExportWizard::setProgress(int value)
{
    exportFinishInfo->progressBar->setValue(value);
}

void ExportWizard::exportExcel(QSqlDatabase &source, const QString &target)
{
    setOptions(QWizard::DisabledBackButtonOnLastPage);
    button(QWizard::FinishButton)->setEnabled(false);
    QStringList tableNameList = exportInfo->getUseTableList();
    excelWorkThread = new ExportExcelWorkThread(source, tableNameList, target);
    connect(excelWorkThread, SIGNAL(finished()), this, SLOT(excelWorkFinish()));
    connect(excelWorkThread, SIGNAL(finishPos(int)), this, SLOT(setProgress(int)));
    exportFinishInfo->progressBar->setMaximum(tableNameList.size());
    excelWorkThread->start();
}

void ExportWizard::excelWorkFinish()
{
    QAbstractButton *finishButton = button(QWizard::FinishButton);
    finishButton->setEnabled(true);
    setButtonText(QWizard::FinishButton, tr("Finish"));
    setOption(QWizard::DisabledBackButtonOnLastPage, false);
    QSqlDatabase::removeDatabase("ExportExcel");
    delete excelWorkThread;  
}

void ExportWizard::exportTxt(QSqlDatabase &source, const QString &directory)
{
    setOptions(QWizard::DisabledBackButtonOnLastPage);
    button(QWizard::FinishButton)->setEnabled(false);
    QStringList tableNameList = exportInfo->getUseTableList();
    txtWorkThread = new ExportTxtWorkThread(source, tableNameList, directory);
    connect(txtWorkThread, SIGNAL(finished()), this, SLOT(txtWorkFinish()));
    connect(txtWorkThread, SIGNAL(finishPos(int)), this, SLOT(setProgress(int)));
    exportFinishInfo->progressBar->setMaximum(tableNameList.size());
    txtWorkThread->start();
}

void ExportWizard::txtWorkFinish()
{
    QAbstractButton *finishButton = button(QWizard::FinishButton);
    finishButton->setEnabled(true);
    setButtonText(QWizard::FinishButton, tr("Finish"));
    setOption(QWizard::DisabledBackButtonOnLastPage, false);
    delete txtWorkThread;
}

void ExportWizard::exportXml(QSqlDatabase &source, const QString &directory)
{
    setOptions(QWizard::DisabledBackButtonOnLastPage);
    button(QWizard::FinishButton)->setEnabled(false);
    QStringList tableNameList = exportInfo->getUseTableList();
    xmlWorkThread = new ExportXmlWorkThread(source, tableNameList, directory);
    connect(xmlWorkThread, SIGNAL(finished()), this, SLOT(xmlWorkFinish()));
    connect(xmlWorkThread, SIGNAL(finishPos(int)), this, SLOT(setProgress(int)));
    exportFinishInfo->progressBar->setMaximum(tableNameList.size());
    xmlWorkThread->start();
}

void ExportWizard::xmlWorkFinish()
{
    QAbstractButton *finishButton = button(QWizard::FinishButton);
    finishButton->setEnabled(true);
    setButtonText(QWizard::FinishButton, tr("Finish"));
    setOption(QWizard::DisabledBackButtonOnLastPage, false);
    delete xmlWorkThread;
}

ExportIntroPage::ExportIntroPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Introduction"));

    label = new QLabel(tr("This wizard will export table data "
                          "to Excel or Xml or Txt or Access or SQL Server."));
    label->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(label);
    setLayout(layout);
}

ExportInfoPage::ExportInfoPage(const QMap<QString, QString> &str, QWidget *parent)
    : QWizardPage(parent), strMap(str)
{
    setTitle(tr("Source Information"));
    setSubTitle(tr("Specify source information about the source type for which you "
                   "want to export."));

    sourceDatabaseLabel = new QLabel(tr("Source Database:"));
    sourceDatabaseComcoBox = new QComboBox;
//    sourceTableLabel = new QLabel(tr("Source Table:"));
//    sourceTableComcoBox = new QComboBox;
    listView = new QListView;
    model = new MyStringListModel;
    listView->setModel(model);
    selectAllButton = new QPushButton(tr("Select All"));
    unSelectAllButton = new QPushButton(tr("Unselect All"));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(sourceDatabaseLabel);
    topLayout->addWidget(sourceDatabaseComcoBox);
    topLayout->addStretch();

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(selectAllButton);
    rightLayout->addWidget(unSelectAllButton);
    rightLayout->addStretch();

    QHBoxLayout *bottomLayout = new QHBoxLayout;
    bottomLayout->addWidget(listView);
    bottomLayout->addLayout(rightLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);

    setLayout(mainLayout);

    connect(selectAllButton, SIGNAL(clicked()), this, SLOT(selectAllClicked()));
    connect(unSelectAllButton, SIGNAL(clicked()), this, SLOT(unselectAllClicked()));
    connect(sourceDatabaseComcoBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(databaseChanged(QString)));
    sourceDatabaseComcoBox->addItems(str.keys());
//    sourceDatabaseComcoBox->setCurrentIndex(-1);
//    sourceTableComcoBox->setCurrentIndex(-1);

//    registerField("sourceName*", sourceDatabaseComcoBox);
}

void ExportInfoPage::initializePage()
{
    if (strMap.isEmpty())
        return;

    QString connectName = strMap.begin().value();
    QSqlDatabase db = QSqlDatabase::database(connectName);
    model->setStringList(db.tables());
}

QStringList ExportInfoPage::getUseTableList() const
{
    return model->getUseStringList();
}

QString ExportInfoPage::getSource() const
{
    return sourceDatabaseComcoBox->currentText();
}

void ExportInfoPage::databaseChanged(const QString &text)
{
    if (text.isEmpty())
        return;

    QString connectName = strMap.value(text);
    QSqlDatabase db = QSqlDatabase::database(connectName);
    model->setStringList(db.tables());
    listView->setFocus();
}

void ExportInfoPage::selectAllClicked()
{
    model->selectAll();
    listView->setFocus();
}

void ExportInfoPage::unselectAllClicked()
{
    model->unselectAll();
    listView->setFocus();
}

ExportTragetInfoPage::ExportTragetInfoPage(QWidget *parent) : QWizardPage(parent)
{
    setTitle(tr("Target Information"));
    setSubTitle(tr("Specify target information about the target file for which you "
                   "want to export."));

//    targetLabel = new QLabel(tr("T&arget File:"));
//    targetLineEdit = new QLineEdit;
//    targetLabel->setBuddy(targetLineEdit);
//    targetButton = new QPushButton(tr("Sele&ct"));

//    QHBoxLayout *topLayout = new QHBoxLayout;
//    topLayout->addWidget(targetLabel);
//    topLayout->addWidget(targetLineEdit);
//    topLayout->addWidget(targetButton);

    groupBox = new QGroupBox(tr("Export File Ty&pe"));
    excelRadioButton = new QRadioButton(tr("Excel"));
    registerField("excelRadio", excelRadioButton);
    excelRadioButton->setChecked(true);
    xmlRadioButton = new QRadioButton(tr("Xml"));
    registerField("xmlRadio", xmlRadioButton);
    txtRadioButton = new QRadioButton(tr("Txt"));
    registerField("txtRadio", txtRadioButton);
    accessRadioButton = new QRadioButton(tr("Access"));
    registerField("accessRadio", accessRadioButton);
    sqlserverRadioButton = new QRadioButton(tr("SQL Server"));
    registerField("sqlserverRadio", sqlserverRadioButton);

    QGridLayout *groupBoxLayout = new QGridLayout;
    groupBoxLayout->addWidget(excelRadioButton, 0, 0);
    groupBoxLayout->addWidget(xmlRadioButton, 1, 0);
    groupBoxLayout->addWidget(txtRadioButton, 2, 0);
    groupBoxLayout->addWidget(accessRadioButton, 3, 0);
    groupBoxLayout->addWidget(sqlserverRadioButton, 4, 0);

    groupBox->setLayout(groupBoxLayout);


    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(groupBox);

    setLayout(mainLayout);

}

int ExportTragetInfoPage::nextId() const
{
    if (excelRadioButton->isChecked())
    {
        return ExportWizard::PageTargetExcelInfo;
    }
    else if (accessRadioButton->isChecked())
    {
        return ExportWizard::PageTargetAccessInfo;
    }
    else if (txtRadioButton->isChecked())
    {
        return ExportWizard::PageTargetTxtInfo;
    }
    else if (sqlserverRadioButton->isChecked())
    {
        return ExportWizard::PageTargetSQLServerInfo;
    }
    else if (xmlRadioButton->isChecked())
    {
        return ExportWizard::PageTargetXmlInfo;
    }
    return ExportWizard::PageFinishInfo;
}

ExportTxtInfoPage::ExportTxtInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Txt Information"));
    setSubTitle(tr("Please Select Target Txt Information."));

    targetLabel = new QLabel(tr("Target:"));
    targetLineEdit = new QLineEdit;
    registerField("txtName*", targetLineEdit);
    targetLabel->setBuddy(targetLineEdit);
    targetButton = new QPushButton("Select");

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(targetLabel);
    topLayout->addWidget(targetLineEdit);
    topLayout->addWidget(targetButton);

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

    connect(targetButton, SIGNAL(clicked()), this, SLOT(selectPath()));
}

int ExportTxtInfoPage::nextId() const
{
    return ExportWizard::PageFinishInfo;
}

void ExportTxtInfoPage::selectPath()
{
//    QString fileName = QFileDialog::getSaveFileName(this,
//                                                    tr("Save Txt File"), ".",
//                                                    tr("Txt File (*.txt)"));

    QString directory = QFileDialog::getExistingDirectory(this, tr("Select exist directory"));

    if (!directory.isEmpty())
        targetLineEdit->setText(directory);
}

ExportXmlInfoPage::ExportXmlInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Xml Information"));
    setSubTitle(tr("Please Select Target Xml Information."));

    targetLabel = new QLabel(tr("Target:"));
    targetLineEdit = new QLineEdit;
    registerField("xmlName*", targetLineEdit);
    targetButton = new QPushButton(tr("Select"));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(targetLabel);
    topLayout->addWidget(targetLineEdit);
    topLayout->addWidget(targetButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(targetButton, SIGNAL(clicked()), this, SLOT(selectPath()));
}

int ExportXmlInfoPage::nextId() const
{
    return ExportWizard::PageFinishInfo;
}

void ExportXmlInfoPage::selectPath()
{
    QString directory = QFileDialog::getExistingDirectory(this, tr("Select exist directory"));
    if (!directory.isEmpty())
        targetLineEdit->setText(directory);
}

ExportExcelInfoPage::ExportExcelInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Excel Information"));
    setSubTitle(tr("Specify Excel File Path."));
    targetLabel = new QLabel(tr("&Target:"));
    targetLineEdit = new QLineEdit;
    registerField("excelName*", targetLineEdit);
    targetLabel->setBuddy(targetLineEdit);
    targetButton = new QPushButton(tr("Select"));

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(targetLabel);
    topLayout->addWidget(targetLineEdit);
    topLayout->addWidget(targetButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(targetButton, SIGNAL(clicked()), this, SLOT(selectPath()));
}

int ExportExcelInfoPage::nextId() const
{
    return ExportWizard::PageFinishInfo;
}

void ExportExcelInfoPage::selectPath()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Excel File"), ".",
                                                    tr("Excel File (*.xls)"));

    targetLineEdit->setText(fileName);
}

ExportAccessInfoPage::ExportAccessInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Access Information"));
    setSubTitle(tr("Specify Target Access Information."));

    targetLabel = new QLabel(tr("Target:"));
    targetLineEdit = new QLineEdit;
    registerField("accessName*", targetLineEdit);
    targetLabel->setBuddy(targetLineEdit);
    targetButton = new QPushButton("Select");

    QHBoxLayout *topLayout = new QHBoxLayout;
    topLayout->addWidget(targetLabel);
    topLayout->addWidget(targetLineEdit);
    topLayout->addWidget(targetButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(topLayout);
    mainLayout->addStretch();

    setLayout(mainLayout);

    connect(targetButton, SIGNAL(clicked()), this, SLOT(selectPath()));
}

int ExportAccessInfoPage::nextId() const
{
    return ExportWizard::PageFinishInfo;
}

void ExportAccessInfoPage::selectPath()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open Access File"), ".",
                                                    tr("Access File (*.mdb);;Access File (*.accdb)"));
    if (!fileName.isEmpty())
    {
        targetLineEdit->setText(fileName);
    }
}

ExportSQLServerInfoPage::ExportSQLServerInfoPage(QWidget *parent) :
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

int ExportSQLServerInfoPage::nextId() const
{
    return ExportWizard::PageFinishInfo;
}

void ExportSQLServerInfoPage::testConnect()
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

ExportFinishInfoPage::ExportFinishInfoPage(QWidget *parent) :
    QWizardPage(parent)
{
    setTitle(tr("Export Progress Information"));
    progressBar = new QProgressBar;
    QHBoxLayout *mainLayout = new QHBoxLayout;
    mainLayout->addWidget(progressBar);
    setLayout(mainLayout);

    registerField("progress", progressBar);
}

void ExportFinishInfoPage::initializePage()
{
    wizard()->setButtonText(QWizard::FinishButton, tr("Start"));
    progressBar->setValue(0);
}

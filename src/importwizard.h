#ifndef IMPORTWIZARD_H
#define IMPORTWIZARD_H

#include <QWizard>
#include <QThread>
#include <QStringListModel>
#include <QSqlDatabase>
#include "qexcel.h"
class QLabel;
class QLineEdit;
class QGroupBox;
class QRadioButton;
class QComboBox;
class QCheckBox;
class QProgressBar;
class ImportProgressInfoPage;
class ImportTableInfoPage;
class ImportTargetInfoPage;
class QSqlDatabase;
class QListView;
class MyStringListModel;
class ImportWorkThread;
class ImportExcelWorkThread;
class ImportTxtWorkThread;
class ImportXmlWorkThread;

class ImportWizard : public QWizard
{
    Q_OBJECT
public:
    explicit ImportWizard(const QMap<QString, QString> &str, QWidget *parent = 0);
    void accept();

public:
    enum
    {
        PageIntro,
        PageSourceInfo,
        PageSourceTxtInfo,
        PageSoruceExcelInfo,
        PageSourceAccessInfo,
        PageSourceSQLServerInfo,
        PageSourceMySQLInfo,
        PageSourceXmlInfo,
        PageTableInfo,
        PageTargetInfo,
        PageFinishInfo
    };

private slots:
    void workFinish();
    void excelWorkFinish();
    void xmlWorkFinish();
    void txtWorkFinish();
    void setProgress(int);
    void setMaxProgress(int);

private:
    void importExcel(const QString &source, QSqlDatabase &target);
    void importTxt(const QString &source, QSqlDatabase &target);
    void importXml(const QString &filePath, QSqlDatabase &target);
    QSqlDatabase getTargetDatabase();
    void importData(QSqlDatabase &source, QSqlDatabase &target);

private:
    ImportTableInfoPage *importTable;
    ImportTargetInfoPage *importTarget;
    ImportProgressInfoPage *progressPage;
    const QMap<QString, QString> &strMap;

    ImportWorkThread *workThread;
    ImportTxtWorkThread *txtWorkThread;
    ImportExcelWorkThread *excelWorkThread;
    ImportXmlWorkThread *xmlWorkThread;
};

class ImportIntroPage : public QWizardPage
{
    Q_OBJECT

public:
    ImportIntroPage(QWidget *parent = 0);

private:
    QLabel *label;
};

class ImportInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ImportInfoPage(QWidget *parent = 0);

public:
    int nextId() const;

private:
//    QLabel *sourceLabel;
//    QLineEdit *sourceLineEdit;
//    QPushButton *sourceButton;

    QGroupBox *groupBox;
    QRadioButton *excelRadioButton;
    QRadioButton *xmlRadioButton;
    QRadioButton *txtRadioButton;
    QRadioButton *accessRadioButton;
    QRadioButton *sqlserverRadioButton;
    QRadioButton *mysqlRadioButton;
};

class ImportTxtInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ImportTxtInfoPage(QWidget *parent = 0);

public:
    int nextId() const;

private slots:
    void selectPath();

private:
    QLabel *sourceLabel;
    QLineEdit *sourceLineEdit;
    QPushButton *sourceButton;

    QGroupBox *groupBox;
    QRadioButton *commaRadioButton;
};

class ImportAccessInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ImportAccessInfoPage(QWidget *parent = 0);

    int nextId() const;

private slots:
    void databaseConnect();
    void selectPath();

private:
    QLabel *sourceLabel;
    QLineEdit *sourceLineEdit;
    QPushButton *sourceButton;

    QLabel *connectInfoLabel;
    QLabel *userNameLabel;
    QLineEdit *userNameLineEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordLineEdit;
    QCheckBox *passwordCheckBox;

    QPushButton *testConnectButton;
};

class ImportExcelInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ImportExcelInfoPage(QWidget *parent = 0);

public:
    int nextId() const;

private slots:
    void selectPath();
    void excelConnect();

private:
    QLabel *sourceLabel;
    QLineEdit *sourceLineEdit;
    QPushButton *sourceButton;

    QPushButton *testConnectButton;
};

class ImportSQLServerInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ImportSQLServerInfoPage(QWidget *parent = 0);

public:
    int nextId() const;

private slots:
    void testConnect();

private:
    QLabel *databaseNameLabel;
    QLineEdit *databaseNameLineEdit;
    QLabel *hostNameLabel;
    QLineEdit *hostNameLineEdit;
    QLabel *userNameLabel;
    QLineEdit *userNameLineEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordLineEdit;
    QLabel *portLabel;
    QLineEdit *portLineEdit;

    QPushButton *testConnectButton;
};

class ImportMySQLInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ImportMySQLInfoPage(QWidget *parent = 0);

public:
    int nextId() const;

private slots:
    void testConnect();

private:
    QLabel *databaseNameLabel;
    QLineEdit *databaseNameLineEdit;
    QLabel *hostNameLabel;
    QLineEdit *hostNameLineEdit;
    QLabel *userNameLabel;
    QLineEdit *userNameLineEdit;
    QLabel *passwordLabel;
    QLineEdit *passwordLineEdit;
    QLabel *portLabel;
    QLineEdit *portLineEdit;

    QPushButton *testConnectButton;
};

class ImportXmlInfoPqge : public QWizardPage
{
    Q_OBJECT

public:
    ImportXmlInfoPqge(QWidget *parent = 0);

public:
    int nextId() const;

private slots:
    void selectPath();

private:
    QLabel *sourceLabel;
    QLineEdit *sourceLineEdit;
    QPushButton *selectButton;
};

class ImportTableInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ImportTableInfoPage(QWidget *parent = 0);

public:
    void initializePage();
    QStringList getUseTableList() const;

private:
    void initAccessTable();
    void initExcelTable();
    void initSqlserverTable();
    void initXmlTable();

private slots:
    void selectAllClicked();
    void unselectAllClicked();

private:
    QListView *listView;
    QPushButton *selectAllButton;
    QPushButton *unSelectAllButton;
    MyStringListModel *model;
};

class ImportTargetInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ImportTargetInfoPage(const QMap<QString, QString> &str, QWidget *parent = 0);
    QString getTarget() const;

private slots:
    void checkBoxChanged(int state);
    void textChanged(const QString&);
    void selectPath();

private:
    QLabel *targetLabel;
    QComboBox *targetComboBox;

    QCheckBox *checkBox;

    QLabel *newDatabaseNameLabel;
    QLineEdit *newDatabaseNameLineEdit;
    QPushButton *newDatabasePathButton;
};

class ImportProgressInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ImportProgressInfoPage(QWidget *parent = 0);

    void initializePage();

public:
    QProgressBar *progressBar;
};

#endif // IMPORTWIZARD_H

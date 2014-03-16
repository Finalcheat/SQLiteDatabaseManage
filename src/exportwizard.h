#ifndef EXPORTWIZARD_H
#define EXPORTWIZARD_H

#include <QWizard>
class QLabel;
class QLineEdit;
class QComboBox;
class QPushButton;
class QGroupBox;
class QRadioButton;
class ExportInfoPage;
class QSqlDatabase;
class QListView;
class MyStringListModel;
class QProgressBar;
class ImportWorkThread;
class ExportFinishInfoPage;
class ExportWorkThread;
class ExportExcelWorkThread;
class ExportTxtWorkThread;
class ExportXmlWorkThread;

class ExportWizard : public QWizard
{
    Q_OBJECT
public:
    explicit ExportWizard(const QMap<QString, QString> &str, QWidget *parent = 0);

public:
    void accept();

public:
    enum
    {
        PageIntro,
        PageSourceInfo,
        PageTargetInfo,
        PageTargetTxtInfo,
        PageTargetXmlInfo,
        PageTargetExcelInfo,
        PageTargetAccessInfo,
        PageTargetSQLServerInfo,
        PageFinishInfo
    };

private slots:
    void workFinish();
    void excelWorkFinish();
    void txtWorkFinish();
    void xmlWorkFinish();
    void setProgress(int);

private:
    void exportData(QSqlDatabase &source, QSqlDatabase &target);
    void exportExcel(QSqlDatabase &source, const QString &target);
    void exportTxt(QSqlDatabase &source, const QString &directory);
    void exportXml(QSqlDatabase &source, const QString &directory);
    
private:
    const QMap<QString, QString> &strMap;
    ExportInfoPage *exportInfo;
    ExportFinishInfoPage *exportFinishInfo;

    ExportWorkThread *workThread;
    ExportExcelWorkThread *excelWorkThread;
    ExportTxtWorkThread *txtWorkThread;
    ExportXmlWorkThread *xmlWorkThread;
};

class ExportIntroPage : public QWizardPage
{
    Q_OBJECT

public:
    ExportIntroPage(QWidget *parent = 0);

private:
    QLabel *label;
};

class ExportInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ExportInfoPage(const QMap<QString, QString> &str, QWidget *parent = 0);

public:
    void initializePage();
    QStringList getUseTableList() const;
    QString getSource() const;

private slots:
    void databaseChanged(const QString &text);
    void selectAllClicked();
    void unselectAllClicked();

private:
    QLabel *sourceDatabaseLabel;
    QComboBox *sourceDatabaseComcoBox;
//    QLabel *sourceTableLabel;
//    QComboBox *sourceTableComcoBox;

    QListView *listView;
    QPushButton *selectAllButton;
    QPushButton *unSelectAllButton;
    MyStringListModel *model;

    const QMap<QString, QString> &strMap;
};

class ExportTragetInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ExportTragetInfoPage(QWidget *parent = 0);

public:
    int nextId() const;

private:
    QGroupBox *groupBox;
    QRadioButton *excelRadioButton;
    QRadioButton *xmlRadioButton;
    QRadioButton *txtRadioButton;
    QRadioButton *accessRadioButton;
    QRadioButton *sqlserverRadioButton;
};

class ExportTxtInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ExportTxtInfoPage(QWidget *parent = 0);

public:
    int nextId() const;

private slots:
    void selectPath();

private:
    QLabel *targetLabel;
    QLineEdit *targetLineEdit;
    QPushButton *targetButton;

    QGroupBox *groupBox;
    QRadioButton *commaRadioButton;
};

class ExportXmlInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ExportXmlInfoPage(QWidget *parent = 0);

public:
    int nextId() const;

private slots:
    void selectPath();

private:
    QLabel *targetLabel;
    QLineEdit *targetLineEdit;
    QPushButton *targetButton;

};

class ExportExcelInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ExportExcelInfoPage(QWidget *parent = 0);

public:
    int nextId() const;

private slots:
    void selectPath();

private:
    QLabel *targetLabel;
    QLineEdit *targetLineEdit;
    QPushButton *targetButton;
};

class ExportAccessInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ExportAccessInfoPage(QWidget *parent = 0);

    int nextId() const;

private slots:
    void selectPath();

private:
    QLabel *targetLabel;
    QLineEdit *targetLineEdit;
    QPushButton *targetButton;
};

class ExportSQLServerInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ExportSQLServerInfoPage(QWidget *parent = 0);

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

class ExportFinishInfoPage : public QWizardPage
{
    Q_OBJECT

public:
    ExportFinishInfoPage(QWidget *parent = 0);

public:
    void initializePage();

public:
    QProgressBar *progressBar;
};

#endif // EXPORTWIZARD_H

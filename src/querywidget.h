#ifndef QUERYWIDGET_H
#define QUERYWIDGET_H

#include <QWidget>
#include <QTime>
class QGroupBox;
class CodeEditor;
class Highlighter;
class QTabWidget;
class QPlainTextEdit;
class QTableView;
class QSqlTableModel;

class QueryWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QueryWidget(const QString &, QWidget *parent = 0);
    ~QueryWidget();

public:
    void onSqlExecute();
    void createNewTable(int num, const QString &query);
    void saveAs();
    void analysisSql();
    void openSql();
    void runLineSql();
    void setConnectName(const QString &connectName);
    void copy();
    void paste();
    void cut();
    void del();
    void findNext(const QString &str);
    void findPrevious(const QString &str);

protected:
    void closeEvent(QCloseEvent *event);

private:
    void databaseInitialize();

private:
    QString connectName;
    
    QGroupBox *groupBox;
    CodeEditor *sqlCodeEditor;
    Highlighter *highlighter;

    QTabWidget *tabWidget;
    QPlainTextEdit *resultText;
    QList<QTableView*> tableView;
    QList<QSqlTableModel*> tableModelList;

    QFont font;

    QTime time;
};

#endif // QUERYWIDGET_H

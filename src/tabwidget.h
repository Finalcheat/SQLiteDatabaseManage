#ifndef TABWIDGET_H
#define TABWIDGET_H

#include <QTabWidget>
#include <QTableWidget>
class QSqlTableModel;

enum WidgetType
{
    NOWIDGET = -1,
    TABLEVIEW = 0,
    SYSTEMTABLEVIEW = 1,
    QUERYWIDGET = 2,
    CREATEINDEXWIDGET = 3,
    NEWTABLEWIDGET = 4,
    MODIFYTABLEFIELDWIDGET = 5
};

class TabWidget : public QTabWidget
{
    Q_OBJECT
public:
    explicit TabWidget(QWidget *parent = 0);

signals:
    void widgetChanged(WidgetType type);

public:
    void setReadOnly(bool isRead);
    void setShowGrid(bool isShow);
    void createNewTableView(const QString &connectName, const QString &tableName);
    void createNewQuery(const QString &connectName);
    void createSystemTable(const QString &connectName, const QString &tableName);
    void createModifyTableField(const QString &connectName, const QString &tableName);
    void createIndex(const QString &connectName, const QString &tableName);
    void createNewTable(const QString &connectName);
    void openSql(const QString &connectName);
    void queryChanged(const QString &database, const QString &connectName);

public slots:
    void analysisSql();
    void onSqlExecute();
    void runLineSql();
    void save();
    void saveSqlAs();
    void saveAll();
    void cut();
    void copy();
    void paste();
    void del();
    void insertRow();
    void delRow();
    void findNext(const QString &str, Qt::CaseSensitivity cs);
    void findPrevious(const QString &str, Qt::CaseSensitivity cs);

private slots:
    void tableClose(int index);
    void currentWidgetChanged(int index);
    void tableDataChanged(const QString &connectName, const QString &tableName, bool b);

private:
    void setReadOnlyPrivate(QTableView *view, bool isRead);
    int findText(const QString &text);
    QString getCurrentClassName() const;
};

#endif // TABWIDGET_H

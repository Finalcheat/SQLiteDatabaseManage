#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QTableView>
class QSqlTableModel;

class TableView : public QTableView
{
    Q_OBJECT
public:
    explicit TableView(const QString &connectName, const QString &tableName, QWidget *parent = 0);
    ~TableView();

public:
    void setRead(bool);
    void save();
    void findNext(const QString &str, Qt::CaseSensitivity cs);
    void findPrevious(const QString &str, Qt::CaseSensitivity cs);

public slots:
    void cut();
    void copy();
    void paste();
    void del();
    void insertRow();
    void delRow();

private slots:
    void showMenu(const QPoint &);
    void dataChanged();

private:
    void createMenu();

private:
    QSqlTableModel *model;

    QMenu *tableMenu;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *deleteAction;
    QAction *insertRowAction;
    QAction *deleteRowAction;
};

#endif // TABLEVIEW_H

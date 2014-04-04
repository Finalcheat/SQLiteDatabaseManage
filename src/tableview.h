#ifndef TABLEVIEW_H
#define TABLEVIEW_H

#include <QWidget>
class QSqlTableModel;
class QTableView;
class QMenu;
class QLineEdit;
class QPushButton;

class TableView : public QWidget
{
    Q_OBJECT
public:
    explicit TableView(const QString &connectName, const QString &tableName, QWidget *parent = 0);
    ~TableView();

signals:
    void tableDataChanged(const QString&,const QString&, bool);

public:
    void setRead(bool);
    void setShowGrid(bool);
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
    void firstButtonClicked();
    void prevButtonClicked();
    void nextButtonClicked();
    void endButtonClicked();

private:
    void createMenu();

private:
    QSqlTableModel *model;
    QTableView *view;
    QMenu *tableMenu;
    QAction *cutAction;
    QAction *copyAction;
    QAction *pasteAction;
    QAction *deleteAction;
    QAction *insertRowAction;
    QAction *deleteRowAction;
    QLineEdit *pageNumberlineEdit;
    QPushButton *firstButton;
    QPushButton *prevButton;
    QPushButton *nextButton;
    QPushButton *endButton;
    static const uint rowNumber = 100;
    uint pageCount;
};

#endif // TABLEVIEW_H

#ifndef SYSTEMTABLEVIEW_H
#define SYSTEMTABLEVIEW_H

#include <QTableView>
class QSqlTableModel;

class SystemTableView : public QTableView
{
    Q_OBJECT
public:
    explicit SystemTableView(const QString &connectName, const QString &tableName, QWidget *parent = 0);
    ~SystemTableView();

public slots:
    void copy();
    
private:
    QSqlTableModel *model;

    QAction *copyAction;
};

#endif // SYSTEMTABLEVIEW_H

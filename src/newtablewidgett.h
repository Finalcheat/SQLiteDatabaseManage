#ifndef FIELDWIDGET_H
#define FIELDWIDGET_H

#include <QWidget>

class QTableWidget;
class QTableWidgetItem;

class NewTableWidget : public QWidget
{
    Q_OBJECT
public:
    NewTableWidget(const QString&, QWidget *parent = 0);
    ~NewTableWidget();

private:
    bool save();

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void tableItemChanged(QTableWidgetItem*);
//    void setPrimary(int);

private:
    const QString connectName;

    QTableWidget *tableWidget;
};

#endif // FIELDWIDGET_H

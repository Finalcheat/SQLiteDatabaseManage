#ifndef MODIFYTABLEFIELDWIDGET_H
#define MODIFYTABLEFIELDWIDGET_H

#include <QWidget>

class QGroupBox;
class QTableWidget;
class QPushButton;
class QTableWidgetItem;
class QSqlQuery;
class QSqlRecord;
class QSqlDatabase;

struct Field
{
    Field(const QString &n = "", const QString &t = "", bool p = false, bool i = true)
        : name(n), type(t), primary(p), isNull(i) {}
    QString name;
    QString type;
    bool primary;
    bool isNull;
};

class ModifyTableFieldWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ModifyTableFieldWidget(const QString &connectName, const QString &tableName, QWidget *parent = 0);

private slots:
//    void setPrimary(int row);
    void addRow();
    void deleteRow();
    void submit();

private:
    void setTableWidget();
//    void analysisFiedl(const QString &str, const QSqlRecord &record);
    QString createNewTableStr(const QList<Field> &newFieldList);
    QString oldTableInsertToNewTableStr(const QList<Field> &newFieldList);
    void newFieldInsertToNewTable(QSqlDatabase &db, const QList<Field> &newFieldList);

private:
    QGroupBox *groupBox;
    QTableWidget *tableWidget;
    QPushButton *addButton;
    QPushButton *submitButton;
    QPushButton *deleteButton;

    QString connectName;
    QString tableName;

    QList<Field> originalStr;
};

#endif // MODIFYTABLEFIELDWIDGET_H

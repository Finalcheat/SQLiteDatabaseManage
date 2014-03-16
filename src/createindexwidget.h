#ifndef CREATEINDEXWIDGET_H
#define CREATEINDEXWIDGET_H

#include <QWidget>
class QTableWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class CodeEditor;
class QPlainTextEdit;
class QTableWidgetItem;

class CreateIndexWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CreateIndexWidget(const QString &cName, const QString &tName,
                               QWidget *parent = 0);

public:
    QStringList getIndexName() { return indexNameList; }

private slots:
    void lineEditChanged(const QString&);
    void tableItemChanged(QTableWidgetItem*);
    void createButtonClicked();

private:
    void setTableWidget();
    void somethingChanged(const QString &str);

private:
    QString connectName;
    QString tableName;
    QStringList typeList;
    QStringList indexNameList;

    QTableWidget *tableWidget;
    QLabel *indexNameLabel;
    QLineEdit *indexNameLineEdit;
    QPushButton *createButton;
    QPlainTextEdit *createInformation;
};

#endif // CREATEINDEXWIDGET_H

#ifndef SQLTEXTEDIT_H
#define SQLTEXTEDIT_H

#include <QTextEdit>

class SqlTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit SqlTextEdit(QWidget *parent = 0);

private slots:
    void somethingChanged();
};

#endif // SQLTEXTEDIT_H

#include "sqltextedit.h"

QString sqlList[] = {"select", "from", "insert", "\0"};

SqlTextEdit::SqlTextEdit(QWidget *parent) :
    QTextEdit(parent)
{
    connect(this, SIGNAL(textChanged()), this, SLOT(somethingChanged()));

}

void SqlTextEdit::somethingChanged()
{
//    QString str = toPlainText();
}

#include "mydockwidget.h"

MyDockWidget::MyDockWidget(const QString &title, QWidget *parent) :
    QDockWidget(title, parent)
{
}

void MyDockWidget::closeEvent(QCloseEvent*)
{
    hide();
    emit dockWidgetShow(false);
}

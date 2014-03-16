#ifndef MYDOCKWIDGET_H
#define MYDOCKWIDGET_H

#include <QDockWidget>

class MyDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit MyDockWidget(const QString &title, QWidget *parent = 0);

signals:
    void dockWidgetShow(bool);

protected:
    void closeEvent(QCloseEvent*);
    
};

#endif // MYDOCKWIDGET_H

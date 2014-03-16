#include <QtGui/QApplication>
#include <QTranslator>
#include <QFile>
#include <QTextCodec>
#include <QSettings>
#include <QDebug>
#include <QString>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QSettings settings("SQLite Database Manage.ini", QSettings::IniFormat);
    int b = settings.value("Language").toInt();

    QTranslator translator(0);
    if (b == 0)
    {
        translator.load( ":/translations/SQLiteDatabaseManage.qm", "." );
        app.installTranslator(&translator);
    }

    QFile file(":/qss/SQLite GUI.qss");
    file.open(QFile::ReadOnly);
    app.setStyleSheet(file.readAll());
//    QTextCodec::codecForName("GB2313");
//    if (QTextCodec::codecForName("UTF-16"))
//        qDebug() << "yes";
//    QTextCodec::setCodecForLocale(QTextCodec::codecForLocale());
//    QTextCodec::setCodecForCStrings(QTextCodec::codecForLocale());

//    qDebug() << QTextCodec::codecForLocale()->name();
    MainWindow w;
//    w.setWindowFlags(Qt::FramelessWindowHint);
    w.show();
//    w.resize(1300, 600);
//    qDebug() << QApplication::style()->metaObject()->className();
    
    return app.exec();
}

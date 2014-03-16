#ifndef WORKTHREAD_H
#define WORKTHREAD_H

#include <QThread>
#include <QObject>
#include <QSqlDatabase>
#include <QStringList>

class ImportWorkThread : public QThread
{
    Q_OBJECT

public:
    ImportWorkThread(QSqlDatabase &s, const QStringList &l, QSqlDatabase &t);

signals:
    void finishPos(int value);

public:
    void run();

private:
    QSqlDatabase source;
    const QStringList tableNameList;
    QSqlDatabase target;
};

class ExportWorkThread : public QThread
{
    Q_OBJECT

public:
    ExportWorkThread(QSqlDatabase &s, const QStringList &l, QSqlDatabase &t);

signals:
    void finishPos(int value);

public:
    void run();

private:
    QSqlDatabase source;
    const QStringList tableNameList;
    QSqlDatabase target;
};

class ImportTxtWorkThread : public QThread
{
    Q_OBJECT

public:
    ImportTxtWorkThread(const QString &s, QSqlDatabase &t);

signals:
    void finishPos(int value);

public:
    void run();

private:
    const QString source;
    QSqlDatabase target;
};

class ExportTxtWorkThread : public QThread
{
    Q_OBJECT

public:
    ExportTxtWorkThread(QSqlDatabase &s, const QStringList &t, const QString &d);

signals:
    void finishPos(int value);

public:
    void run();

private:
    QSqlDatabase source;
    const QStringList tableNameList;
    const QString directory;
};

class ImportExcelWorkThread : public QThread
{
    Q_OBJECT

public:
    ImportExcelWorkThread(const QString &f, const QStringList &l, QSqlDatabase &t);

signals:
    void finishPos(int value);

public:
    void run();

private:
    const QString filePath;
    const QStringList tableNameList;
    QSqlDatabase target;
};

class ExportExcelWorkThread : public QThread
{
    Q_OBJECT

public:
    ExportExcelWorkThread(QSqlDatabase &s, const QStringList &l, const QString &f);

signals:
    void finishPos(int value);

public:
    void run();

private:
    QSqlDatabase source;
    const QStringList tableNameList;
    const QString filePath;
};

class ImportXmlWorkThread : public QThread
{
    Q_OBJECT

public:
    ImportXmlWorkThread(const QString &f, const QStringList &l, QSqlDatabase &t);

signals:
    void finishPos(int value);
    void maxProgress(int maxValue);

public:
    void run();

private:
    const QString filePath;
    const QStringList tableNameList;
    QSqlDatabase target;
};

class ExportXmlWorkThread : public QThread
{
    Q_OBJECT

public:
    ExportXmlWorkThread(QSqlDatabase &s, const QStringList &t, const QString &d);

signals:
    void finishPos(int value);

public:
    void run();

private:
    QSqlDatabase source;
    const QStringList tableNameList;
    const QString directory;
};

#endif // WORKTHREAD_H

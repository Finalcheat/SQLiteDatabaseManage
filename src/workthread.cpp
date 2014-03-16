#include "workthread.h"
#include "qt_windows.h"
#include "qexcel.h"
#include <QSqlRecord>
#include <QSqlIndex>
#include <QSqlQuery>
#include <QVariant>
#include <QFile>
#include <QTextStream>
#include <QDomDocument>
#include <QAxObject>
#include <QDir>
#include <QSqlError>
#include <QDebug>

ImportWorkThread::ImportWorkThread(QSqlDatabase &s, const QStringList &l, QSqlDatabase &t) :
    source(s), tableNameList(l), target(t)
{

}

void ImportWorkThread::run()
{
    source.transaction();
    target.transaction();
    for (int i = 0; i < tableNameList.size(); ++i)
    {
        QString sqlStr = "create table '" + tableNameList[i] + "'(";
        QSqlRecord record = source.record(tableNameList[i]);

        QSqlIndex primaryIndex = source.primaryIndex(tableNameList[i]);
        QString primaryName;

        for (int j = 0; j < record.count(); ++j)
        {
            QString fieldName = record.fieldName(j);
            if (primaryIndex.contains(fieldName))
            {
                primaryName += "'";
                primaryName += fieldName;
                primaryName += "',";
            }
            sqlStr += "'";
            sqlStr += fieldName;
            sqlStr += "',";
        }

        int index;
        if (primaryName.isEmpty())
        {
            index = sqlStr.lastIndexOf(',');
            sqlStr.replace(index, 1, ')');
        }
        else
        {
            sqlStr = sqlStr + "primary key(" + primaryName;
            index = sqlStr.lastIndexOf(',');
            sqlStr.replace(index, 2, "))");
        }

//        qDebug() << sqlStr;
//        targetQuery.exec(sqlStr);
        target.exec(sqlStr);

        QSqlQuery sourceQuery = source.exec("select * from " + tableNameList[i]);

        while (sourceQuery.next())
        {
            sqlStr.clear();
            sqlStr = "insert into '" + tableNameList[i] + "' values (";
            for (int column = 0; column < record.count(); ++column)
            {
                sqlStr += "'";
                QString fieldValue = sourceQuery.value(column).toString();
                sqlStr += fieldValue;
                sqlStr += "'";
                sqlStr += ",";
            }
            index = sqlStr.lastIndexOf(',');
            sqlStr.replace(index, 1, ')');
//            qDebug() << sqlStr;
//            targetQuery.exec(sqlStr);
            target.exec(sqlStr);
        }
        emit finishPos(i + 1);
    }
    source.commit();
    target.commit();
}

ExportWorkThread::ExportWorkThread(QSqlDatabase &s, const QStringList &l, QSqlDatabase &t) :
    source(s), tableNameList(l), target(t)
{

}

void ExportWorkThread::run()
{
    source.transaction();
    target.transaction();
    for (int i = 0; i < tableNameList.size(); ++i)
    {
        QString sqlStr = "create table \"" + tableNameList[i] + "\"(";
        QSqlRecord record = source.record(tableNameList[i]);

        for (int j = 0; j < record.count(); ++j)
        {
            QString fieldName = record.fieldName(j);
            sqlStr += "\"";
            sqlStr += fieldName;
            sqlStr += "\" text,";
        }

        int index = sqlStr.lastIndexOf(',');
        sqlStr.replace(index, 1, ')');


//        qDebug() << sqlStr;
//        targetQuery.exec(sqlStr);
        target.exec(sqlStr);

        QSqlQuery sourceQuery = source.exec("select * from " + tableNameList[i]);

        while (sourceQuery.next())
        {
            sqlStr.clear();
            sqlStr = "insert into \"" + tableNameList[i] + "\" values (";
            for (int column = 0; column < record.count(); ++column)
            {
                sqlStr += "'";
                QString fieldValue = sourceQuery.value(column).toString();
                sqlStr += fieldValue;
                sqlStr += "'";
                sqlStr += ",";
            }
            index = sqlStr.lastIndexOf(',');
            sqlStr.replace(index, 1, ')');
//            qDebug() << sqlStr;
//            targetQuery.exec(sqlStr);
            target.exec(sqlStr);
        }
        emit finishPos(i + 1);
    }
    source.commit();
    target.commit();
}

ImportTxtWorkThread::ImportTxtWorkThread(const QString &s, QSqlDatabase &t) :
    source(s), target(t)
{

}

void ImportTxtWorkThread::run()
{
    if (!QFile::exists(source))
        return;

    QFile file(source);
    file.open(QIODevice::ReadOnly);

    QTextStream stream(&file);
//    stream.setCodec(QTextCodec::codecForName("GB2313"));
//    stream.readAll();
    QString str = stream.readAll();
//    qDebug() << str;
//    return;
    int index = source.lastIndexOf("/");
    QString tableName = source.mid(index + 1, source.size() - index - 5);
    QStringList strList = str.split("\n", QString::SkipEmptyParts);
    if (!strList.empty())
    {
//        QStringList fieldList = strList[0].spit(",");
        QString sql = "create table '" + tableName + "'(";
        sql += strList[0];
        sql += ')';
//        qDebug() << sql;

//        QSqlQuery query(target);
        target.transaction();
        target.exec(sql);

        sql = "insert into '" + tableName + "' values(";
        for (int i = 1; i < strList.size(); ++i)
        {
            QString insertSql = sql + strList[i] + ')';
//            qDebug() << insertSql;
            target.exec(insertSql);
            emit finishPos(i);
        }
        target.commit();
    }
    file.close();
}

ExportTxtWorkThread::ExportTxtWorkThread(QSqlDatabase &s, const QStringList &t, const QString &d) :
    source(s), tableNameList(t),directory(d)
{
}

void ExportTxtWorkThread::run()
{
    source.transaction();
//    qDebug() << directory;
    QSqlQuery query;
    for (int i = 0; i < tableNameList.size(); ++i)
    {
        query = source.exec("select * from '" + tableNameList[i] + "'");
        QString str;
        QSqlRecord record = query.record();
        for (int j = 0; j < record.count(); ++j)
        {
            str += "\"";
            str += record.fieldName(j);
            str += "\",";
        }

        int index = str.lastIndexOf(",");
        str.replace(index, 1, "\r\n");

        while (query.next())
        {
            for (int k = 0; k < record.count(); ++k)
            {
                str += "\"";
                str += query.value(k).toString();
                str += "\",";
            }
            index = str.lastIndexOf(",");
            str.replace(index, 1, "\r\n");
        }

        QFile file(directory + "\\" + tableNameList[i] + ".txt");
        file.open(QIODevice::WriteOnly);
        file.write(str.toAscii());
        file.close();
        emit finishPos(i + 1);
    }
    source.commit();
}


ImportExcelWorkThread::ImportExcelWorkThread(const QString &f, const QStringList &l, QSqlDatabase &t) :
    filePath(f), tableNameList(l), target(t)
{
//    HRESULT r = OleInitialize(0);
//    if (r != S_OK && r != S_FALSE)
//    {
//        qWarning("Qt: Could not initialize OLE (error %x)", (unsigned int)r);
//    }
//    if (excel.open(filePath))
//        qDebug() << "yes";
}

void ImportExcelWorkThread::run()
{
//        qDebug() << source;
//    QExcel excel(source);

//    if (excel.isNull())
//        return;

//    QStringList tableNameList = importTable->getUseTableList();

    //    QSqlQuery query(target);

//    HRESULT r = OleInitialize(0);
//    if (r != S_OK && r != S_FALSE)
//    {
//        qWarning("Qt: Could not initialize OLE (error %x)", (unsigned int)r);
//        return;
//    }

//    {
//        QExcel excel;
//        excel.open(filePath);

//        target.transaction();
//        for (int i = 0; i < tableNameList.size(); ++i)
//        {
//            QString sqlStr = "create table '" + tableNameList[i] + "'(";
//            excel.selectSheet(tableNameList[i]);
////                    qDebug() << sqlStr;
//            int topLeftRow, topLeftColumn, bottomRightRow, bottomRightColumn;
//            excel.getUsedRange(&topLeftRow, &topLeftColumn, &bottomRightRow, &bottomRightColumn);
//            //        qDebug() << topLeftRow << endl << topLeftColumn << endl << bottomRightRow <<endl << bottomRightColumn << endl;
//            for (int j = topLeftColumn; j <= bottomRightColumn; ++j)
//            {
//                QString fieldName = excel.getCellValue(1, j).toString();
//                if (!fieldName.isEmpty())
//                {
//                    sqlStr += "'";
//                    sqlStr += fieldName;
//                    sqlStr += "',";
//                }
//            }
//            int index = sqlStr.lastIndexOf(',');
//            sqlStr.replace(index, 1, ')');
//            //        qDebug() << sqlStr << endl;

//            //        query.exec(sqlStr);
//            target.exec(sqlStr);

//            for (int row = topLeftRow + 1; row <= bottomRightRow; ++row)
//            {
//                sqlStr = "insert into '" + tableNameList[i] + "' values (";
//                for (int column = topLeftColumn; column <= bottomRightColumn; ++column)
//                {
//                    QString field = excel.getCellValue(row, column).toString();
//                    if (!field.isEmpty())
//                    {
//                        sqlStr += "'";
//                        sqlStr += field;
//                        sqlStr += "',";
//                    }
//                }
//                index = sqlStr.lastIndexOf(',');
//                sqlStr.replace(index, 1, ')');
//                //            qDebug() << sqlStr << endl;
//                //            query.exec(sqlStr);
//                target.exec(sqlStr);
//            }
//            emit finishPos(i + 1);
//        }
//        target.commit();
//    }

//    OleUninitialize();


    QSqlDatabase source = QSqlDatabase::addDatabase("QODBC", "ImportExcel");
    QString dsn = "Driver={Microsoft Excel Driver (*.xls, *.xlsx, *.xlsm, *.xlsb)};READONLY=FALSE;Dbq=" + filePath;
    source.setDatabaseName(dsn);
    if (source.open())
    {
        source.transaction();
        target.transaction();
        for (int i = 0; i < tableNameList.size(); ++i)
        {
            QSqlQuery query = source.exec("select * from [" + tableNameList[i] + "$]");
            QSqlRecord record = query.record();
            QString sql = "create table '" + tableNameList[i] + "'(";
            for (int j = 0; j < record.count(); ++j)
            {
                sql += "'";
                sql += record.fieldName(j);
                sql += "',";
            }
            int index = sql.lastIndexOf(",");
            sql.replace(index, 1, ")");

            target.exec(sql);

            while (query.next())
            {
                sql = "insert into '" + tableNameList[i] + "' values(";
                for (int k = 0; k < record.count(); ++k)
                {
                    sql += "'";
                    sql += query.value(k).toString();
                    sql += "',";
                }
                index = sql.lastIndexOf(",");
                sql.replace(index, 1, ")");
                target.exec(sql);
            }
            emit finishPos(i + 1);
        }
        source.commit();
        target.commit();
    }

}

ExportExcelWorkThread::ExportExcelWorkThread(QSqlDatabase &s, const QStringList &l, const QString &f) :
    source(s), tableNameList(l), filePath(f)
{

}

void ExportExcelWorkThread::run()
{

    QSqlDatabase target = QSqlDatabase::addDatabase("QODBC", "ExportExcel");
//    QString fileName = "d:\\temp\\book1.xls";
//    QString dsn = "Driver={Microsoft Excel Driver (*.xls)};Readonly=0;DriverId=790;Dbq=d:\\temp\\book1.xls";
    QString dsn = QString("DRIVER={Microsoft Excel Driver (*.xls)};READONLY=FALSE;CREATE_DB=\"%1\";DBQ=%2").
                          arg(filePath).arg(filePath);
//    qDebug() << filePath;
    target.setDatabaseName(dsn);
    if (target.open())
    {
        source.transaction();
        target.transaction();
        for (int i = 0; i < tableNameList.size(); ++i)
        {
            QSqlQuery query = source.exec("select * from '" + tableNameList[i] + "'");
            QSqlRecord record = query.record();
            QString sql = "create table " + tableNameList[i] + "(";
            for (int j = 0; j < record.count(); ++j)
            {
                sql += record.fieldName(j);
                sql += " text,";
            }
            int index = sql.lastIndexOf(",");
            sql.replace(index, 1, ")");
//            qDebug() << sql;
            target.exec(sql);

            while (query.next())
            {
                sql = "insert into [" + tableNameList[i] + "$] values(";
                for (int k = 0; k < record.count(); ++k)
                {
                    sql += "'";
                    sql += query.value(k).toString();
                    sql += "',";
                }
                index = sql.lastIndexOf(",");
                sql.replace(index, 1, ")");
                target.exec(sql);
            }
            emit finishPos(i + 1);
        }
        source.commit();
        target.commit();
    }
    else
    {
        qDebug() << target.lastError().text();
    }
}

ImportXmlWorkThread::ImportXmlWorkThread(const QString &f, const QStringList &l, QSqlDatabase &t) :
    filePath(f), tableNameList(l), target(t)
{

}

void ImportXmlWorkThread::run()
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return;

    QDomDocument doc("tempDoc");
    if (!doc.setContent(&file))
    {
        file.close();
        return;
    }

    QDomElement docElem = doc.documentElement();
//    qDebug() << docElem.childNodes().size();
    QDomNodeList NodeList = docElem.childNodes();

    target.transaction();

    // create table
    if (NodeList.size())
    {
        QDomElement e = NodeList.at(0).toElement();
        QString tableName = e.tagName();
        QString sqlStr = "create table '" + tableName + "'(";

        if (!e.isNull())
        {
            QDomNode temp = e.firstChild();
            while (!temp.isNull())
            {
                QDomElement etemp = temp.toElement();
                if (!e.isNull())
                {
                    sqlStr = sqlStr + "'" + etemp.tagName() + "',";
                }
                temp = temp.nextSibling();
            }
        }

        int index = sqlStr.lastIndexOf(",");
        sqlStr.replace(index, 1, ')');
//        qDebug() << sqlStr;
        target.exec(sqlStr);
    }

    emit maxProgress(NodeList.size());

    // insert into data
    for (int i = 0; i < NodeList.size(); ++i)
    {
//        QDomElement e = n.toElement();
        QDomElement e = NodeList.at(i).toElement();

        if (!e.isNull())
        {
//            qDebug() << e.tagName();
            QDomNode temp = e.firstChild();
//            qDebug() << temp.columnNumber();
            QString sqlStr = "insert into '" + e.tagName() + "' values(";
            while (!temp.isNull())
            {
                QDomElement etemp = temp.toElement();
//                qDebug() << etemp.attribute(tableNameList[0]);
                if (!e.isNull())
                {
                    sqlStr = sqlStr + "'" + etemp.text() + "',";
//                    qDebug() << etemp.tagName() << ":" << etemp.text();
                }
                temp = temp.nextSibling();
            }

            int index = sqlStr.lastIndexOf(",");
            sqlStr.replace(index, 1, ')');
//            qDebug() << sqlStr;
            target.exec(sqlStr);
        }

        emit finishPos(i + 1);
    }

    target.commit();
}

ExportXmlWorkThread::ExportXmlWorkThread(QSqlDatabase &s, const QStringList &t, const QString &d) :
    source(s), tableNameList(t), directory(d)
{
}

void ExportXmlWorkThread::run()
{

    source.transaction();
    for (int i = 0; i < tableNameList.size(); ++i)
    {
        QDomDocument doc;
        doc.appendChild(doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\""));
        QDomElement root = doc.createElement("Table");
        doc.appendChild(root);

        QSqlQuery query = source.exec("select * from '" + tableNameList[i] + "'");
        QSqlRecord record = query.record();
        while (query.next())
        {
            QDomElement table = doc.createElement(tableNameList[i]);
            root.appendChild(table);

            for (int j = 0; j < record.count(); ++j)
            {
                QDomElement tag = doc.createElement(record.fieldName(j));
                table.appendChild(tag);

                QDomText text = doc.createTextNode(query.value(j).toString());
                tag.appendChild(text);
            }
        }

        QFile file(directory + "\\" + tableNameList[i] + ".xml");
        file.open(QIODevice::WriteOnly);
        file.write(doc.toByteArray());
        file.close();
        emit finishPos(i + 1);
    }
    source.commit();
}

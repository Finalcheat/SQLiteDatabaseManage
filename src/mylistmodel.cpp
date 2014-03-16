#include "mylistmodel.h"
#include <QIcon>
#include <QDebug>

MyListModel::MyListModel(QObject *parent) :
    QAbstractListModel(parent)
{

}

int MyListModel::rowCount(const QModelIndex& /* parent */) const
{
    return str.size();
}

int MyListModel::columnCount(const QModelIndex& /* parent */) const
{
    return 1;
}

QVariant MyListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        return *(str.begin() + index.row());
    }
//    else if (role == Qt::DecorationRole)
//    {
//        return QIcon(":/images/columns.png");
//    }

    return QVariant();
}

void MyListModel::setSourceString()
{
    str  << "select" << "insert" << "table" << "create" << "from" << "primary" << "key"
         << "drop" << "view" << "index" << "trigger" << "delete" << "alter" << "where"
         << "and" << "or" << "in" << "between" << "on" << "values" << "group" << "by"
         << "asc" << "desc" << "into" << "rename";
}

void MyListModel::setCurrentSet(const QSet<QString> &s)
{
    str.clear();
    setSourceString();
    str.unite(s);
//    qDebug() << str;
}

int MyListModel::count() const
{
    return str.count();
}

bool MyListModel::isMatch(const QString &s) const
{
    return str.contains(s);
}




MyStringListModel::MyStringListModel(QObject *parent) :
    QAbstractListModel(parent)
{
}

void MyStringListModel::setStringList(const QStringList &source)
{
    str = source;
    isUseList.clear();
    for (int i = 0; i < source.size(); ++i)
        isUseList.push_back(Qt::Checked);

    reset();
//    qDebug() << str;
}

int MyStringListModel::rowCount(const QModelIndex &/*parent*/) const
{
    return str.size();
}

int MyStringListModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 1;
}

QVariant MyStringListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        return *(str.begin() + index.row());
    }
    else if (role == Qt::CheckStateRole)
    {
        return isUseList[index.row()];
//        return Qt::Checked;
    }

    return QVariant();
}

bool MyStringListModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::CheckStateRole)
    {
        isUseList[index.row()] = value.toInt();
        return true;
    }

    return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags MyStringListModel::flags(const QModelIndex &/*index*/) const
{
    return Qt::NoItemFlags | Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable;
}

void MyStringListModel::selectAll()
{
    for (int i = 0; i < isUseList.size(); ++i)
    {
        isUseList[i] = Qt::Checked;
    }
}

void MyStringListModel::unselectAll()
{
    for (int i = 0; i < isUseList.size(); ++i)
    {
        isUseList[i] = Qt::Unchecked;
    }
}

QStringList MyStringListModel::getUseStringList() const
{
    QStringList strList;
    for (int i = 0; i < isUseList.size(); ++i)
    {
        if (isUseList[i] == Qt::Checked)
            strList += str[i];
    }
    return strList;
}

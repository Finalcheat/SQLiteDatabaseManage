#ifndef MYLISTMODEL_H
#define MYLISTMODEL_H

#include <QAbstractListModel>
#include <QSet>
#include <QStringList>
#include <QVector>

class MyListModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit MyListModel(QObject *parent = 0);

public:
    void setCurrentSet(const QSet<QString> &s);
    int count() const;
    bool isMatch(const QString &s) const;

public:
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;

private:
    void setSourceString();
    
private:
    QSet<QString> str;
};

class MyStringListModel : public QAbstractListModel
{
     Q_OBJECT

public:
    explicit MyStringListModel(QObject *parent = 0);

public:
    void setStringList(const QStringList &);
    void selectAll();
    void unselectAll();
    QStringList getUseStringList() const;

public:
    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);
    Qt::ItemFlags flags(const QModelIndex & index) const ;

private:
    QStringList str;
    QVector<int> isUseList;
};

#endif // MYLISTMODEL_H

#ifndef CUSTOM_SQL_TABLE_MODEL_H
#define CUSTOM_SQL_TABLE_MODEL_H

#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>

namespace dbi {
    class DBTInfo;
}

class CustomSqlTableModel : public QSqlRelationalTableModel
{
    Q_OBJECT

public:
    explicit CustomSqlTableModel(QObject *parent = 0, QSqlDatabase db = QSqlDatabase());
    void setTable(const QString &tableName);
    QVariant data(const QModelIndex &item, int role) const;
    bool setData(const QModelIndex &item, const QVariant &value, int role);
};

class CustomSqlRelationalDelegate : public QSqlRelationalDelegate
{
public:
    CustomSqlRelationalDelegate(QObject *parent);
    ~CustomSqlRelationalDelegate();
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif // CUSTOM_SQL_TABLE_MODEL_H

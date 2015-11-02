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
    void setDataWithSavings();
    void setTable(const QString &tableName);
    QVariant data(const QModelIndex &item, int role) const;
    bool setData(const QModelIndex &item, const QVariant &value, int role);

private:
    void saveData(const QModelIndex &currentIndex, int role);
    void restoreData(int currentRow, int role);

    QMap<int, QVariant> m_mSavedData;
    bool m_bNeedSave;
};

class CustomSqlRelationalDelegate : public QSqlRelationalDelegate
{
public:
    CustomSqlRelationalDelegate(QObject *parent);
    ~CustomSqlRelationalDelegate();
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private:
    void setDataToSimpleDBT(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
};

#endif // CUSTOM_SQL_TABLE_MODEL_H

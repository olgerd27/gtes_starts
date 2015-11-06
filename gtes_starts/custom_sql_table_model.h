#ifndef CUSTOM_SQL_TABLE_MODEL_H
#define CUSTOM_SQL_TABLE_MODEL_H

#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>

namespace dbi {
    class DBTInfo;
}

/* Base strategy class */
struct IQueryPreparer
{
    virtual void finalPrepare(QStringList &listSelect, QStringList &listFrom, QStringList &listWhere) = 0;
    virtual ~IQueryPreparer() { }
};

class DisplayDataGenerator
{
public:
    DisplayDataGenerator();
    ~DisplayDataGenerator();
    int generate(const QString &complexTableName); /* return the mask items quantity */
    inline void setQueryPreparer(IQueryPreparer *qp);
    inline QString mask() const { return m_strMask; }
    inline QString query() const { return m_strQuery; }
private:
    /* Class for generation the query expression */
    class QueryGenerator
    {
    public:
        inline void addSelect(const QString &str) { m_listSelect.push_back(str); }
        inline void addFrom(const QString &str) { m_listFrom.push_back(str); }
        inline void addWhere(const QString &str) { m_listWhere.push_back(str); }
        inline int quantityResultData() const { return m_quantityRes; }
        QString generateQuery(IQueryPreparer *queryPrep); // there are ability to get only one time the particular generated query string
    private:
        void flush();
        QStringList m_listSelect, m_listFrom, m_listWhere;
        int m_quantityRes;
    };

    void generate_Mask_QueryData(dbi::DBTInfo *table, int &fieldCounter);
    void flush();

    QueryGenerator m_queryGen;
    IQueryPreparer *m_queryPrep;
    QString m_strMask, m_strQuery;
};

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
    void fillTheStorage();
    void defineFieldsRelations();
    void setRelationsWithSimpleDBT(int fieldIndex);
    void saveDisplayData(const QString &strMask, const QString &strQuery, int resDataQuantity);

    void saveData(const QModelIndex &currentIndex, int role);
    void restoreData(int currentRow, int role);

    DisplayDataGenerator m_dataGen;
    QVector<int> m_indexSimple;
    QVector<int> m_indexComplex;
    QMap<int, QVector<QVariant>> m_storage;
    QMap<int, QVariant> m_saveRestore;
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

#ifndef DBT_DATA_GENERATOR_H
#define DBT_DATA_GENERATOR_H

#include <QStringList>

namespace dbi {
    class DBTInfo; // TODO: delete?
    class DBTFieldInfo;
}
class IQueryPreparer;

class DBTDataGenerator
{
public:
    typedef struct {
        int idPrim;
        QString genData;
    } T_resData;
    typedef QVector<T_resData> T_arrResData;

    DBTDataGenerator();
    ~DBTDataGenerator();
    void setQueryPreparer(IQueryPreparer *pr);
    void setForeignFieldName(const QString &name); // TODO: do not need, maybe delete?
    void generate(const dbi::DBTFieldInfo &foreignFieldInf); /* return generated data */
    bool hasNextResultData() const;
    DBTDataGenerator::T_resData nextResultData();

private:
    /* Class for generation the query expression */
    class QueryGenerator
    {
    public:
        QueryGenerator();
        ~QueryGenerator();
        void setQueryPreparer(IQueryPreparer *qp);
        inline IQueryPreparer * queryPreparer() { return m_queryPrep; }
        inline void addSelect(const QString &str) { m_listSelect.push_back(str); }
        inline void addFrom(const QString &str) { m_listFrom.push_back(str); }
        inline void addWhere(const QString &str) { m_listWhere.push_back(str); }
        inline int quantityResultData() const { return m_quantityRes; }
        QString generateQuery(); // there are ability to get only one time the particular generated query string

    private:
        QString concatWhere() const;
        void flush();

        IQueryPreparer *m_queryPrep;
        QStringList m_listSelect, m_listFrom, m_listWhere;
        int m_quantityRes;
    };

    enum { INIT_RES_INDEX = 0 };

    void generate_Mask_QueryData(const dbi::DBTFieldInfo &ffield, int &fieldCounter);
    void generateResultData();
    bool isReadyToGeneration();
    void flush();

    QueryGenerator m_queryGen;
    QString m_strMask;
    T_arrResData m_resData;
    int m_indexResData {INIT_RES_INDEX};
};

/*
 * Base abstract strategy class for preparing queries
 */
struct IQueryPreparer
{
    virtual ~IQueryPreparer() { }
    virtual void finalPrepare(QStringList &listSelect, QStringList &listFrom, QStringList &listWhere) = 0;
};

/*
 * Strategy class, that help to prepare a query for obtaining data from database for all id's of a some main table.
 * This class allow obtain all data, stored in the main table.
 * This may be usefully for application effectiveness work - minimize the access to DB when need to extract all data from DB for some DBT field.
 */
struct QuePrepPrimaryAllId : public IQueryPreparer
{
    QuePrepPrimaryAllId(const QString &mainTableName = QString(), const QString &foreignFieldName = QString());
    virtual ~QuePrepPrimaryAllId();
    virtual void finalPrepare(QStringList &listSelect, QStringList &listFrom, QStringList &listWhere);
    inline void setMainTableName(const QString &mtName) { m_mTableName = mtName; }
    inline void setForeignFieldName(const QString &ffName) { m_fFieldName = ffName; }

protected:
    QString m_mTableName, m_fFieldName;
};

/*
 * Strategy class, that help to prepare a query for obtaining data from database for particular one id value
 * of a some main table.
 */
struct QuePrepPrimaryOneId : public QuePrepPrimaryAllId
{
    enum { NOT_SET = -1 };
    QuePrepPrimaryOneId(int id = QuePrepPrimaryOneId::NOT_SET, const QString &mainTableName = QString(), const QString &foreignFieldName = QString());
    virtual ~QuePrepPrimaryOneId();
    virtual void finalPrepare(QStringList &listSelect, QStringList &listFrom, QStringList &listWhere);
    inline void setId(int id) { m_id = id; }
    void setId(const QVariant &varId);

protected:
    int m_id;
};

#endif // DBT_DATA_GENERATOR_H

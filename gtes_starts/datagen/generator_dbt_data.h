#ifndef GENERATOR_DBT_DATA_H
#define GENERATOR_DBT_DATA_H

#include <QStringList>

namespace dbi {
    class DBTFieldInfo;
}

class GeneratorDBTData
{
public:
    /*
     * Base abstract strategy class for queries generations. The finalPrepare() method is the Template Method design pattern.
     */
    class QueryGenerator
    {
    public:
        virtual ~QueryGenerator();
        inline void addSelect(const QString &str) { m_listSelect.push_back(str); }
        inline void addFrom(const QString &str) { m_listFrom.push_back(str); }
        inline void addWhere(const QString &str) { m_listWhere.push_back(str); }
        inline int quantityResultData() const { return m_quantityRes; }
        inline QString resultQuery() const { return m_resQuery; }
        void generateQuery();

    private:
        QString concatWhere() const;
        void flush();\
        QString m_resQuery;

    protected:
        void finalPrepare();
        virtual void doFinalPrepare() = 0;

        QStringList m_listSelect, m_listFrom, m_listWhere;
        int m_quantityRes;
    };

    typedef struct {
        int idPrim;
        QString genData;
    } T_resData;
    typedef QVector<T_resData> T_arrResData;

    GeneratorDBTData();
    ~GeneratorDBTData();
    void setQueryGenerator(QueryGenerator *pr);
    void generate(const dbi::DBTFieldInfo &foreignFieldInf);
    bool hasNextResultData() const;
    GeneratorDBTData::T_resData nextResultData();

private:
    enum { INIT_RES_INDEX = 0 };

    void generate_Mask_QueryData(const dbi::DBTFieldInfo &ffield, int &fieldCounter);
    void generateResultData();
    bool isReadyToGeneration();
    void flush();

    QueryGenerator *m_queryDataPrep;
    QString m_strMask;
    T_arrResData m_resData;
    int m_indexResData {INIT_RES_INDEX};
};


/*
 * Query generator foreign key one id - is a strategy class, that generate a query for obtaining data from database
 * for a particular one id value of a some foreign key field
 */
class QueryGenForeignOneId : public GeneratorDBTData::QueryGenerator
{
public:
    QueryGenForeignOneId(int idFor);
    virtual ~QueryGenForeignOneId();
protected:
    virtual void doFinalPrepare();
    int m_idF;
};

/*
 * Query generator primary key all id - is a strategy class, that generate a query for obtaining data from database
 * for all id's of a some main table (all values of the primary key).   This class allow obtain all data, stored in the main table.
 * This class grows up the application work effectiveness - minimize the access to DB when need to extract all data from DB.
 */
class QueryGenPrimaryAllId : public GeneratorDBTData::QueryGenerator
{
public:
    QueryGenPrimaryAllId(const QString &mainTableName = QString(), const QString &foreignFieldName = QString());
    virtual ~QueryGenPrimaryAllId();
    inline void setMainTableName(const QString &mtName) { m_mTableName = mtName; }
    inline void setForeignFieldName(const QString &ffName) { m_fFieldName = ffName; }
protected:
    virtual void doFinalPrepare();
    QString m_mTableName, m_fFieldName;
};

/*
 * Query data generator primary key one id - is a strategy class, that generate a query for obtaining data from database
 * for a particular one id value of a some primary key field
 */
struct QueryGenPrimaryOneId : public QueryGenPrimaryAllId
{
    enum { NOT_SET = -1 };
    QueryGenPrimaryOneId(int idPrim = QueryGenPrimaryOneId::NOT_SET, const QString &mainTableName = QString(), const QString &foreignFieldName = QString());
    virtual ~QueryGenPrimaryOneId();
    inline void setId(int id) { m_idP = id; }
    void setId(const QVariant &varId);
protected:
    virtual void doFinalPrepare();
    int m_idP;
};

#endif // GENERATOR_DBT_DATA_H

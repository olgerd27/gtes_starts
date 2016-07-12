#ifndef GENERATOR_DBT_DATA_H
#define GENERATOR_DBT_DATA_H

#include <memory>
#include <QStringList>
#include "../common/common_defines.h"

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
        /* Types of query generators */
        enum TypeQueryGenerator {
              tqg_notype
            , tqg_foreignOneId
            , tqg_primaryAllId
            , tqg_primaryOneId
        };

        typedef GeneratorDBTData::QueryGenerator::TypeQueryGenerator T_typeQG;

        QueryGenerator(TypeQueryGenerator typeqg = tqg_notype);
        virtual ~QueryGenerator();
        inline void addSelect(const QString &str) { m_listSelect.push_back(str); }
        inline void addFrom(const QString &str) { m_listFrom.push_back(str); }
        inline void addWhere(const QString &str) { m_listWhere.push_back(str); }
        inline int quantityResultData() const { return m_quantityRes; }
        inline QString lastGeneratedQuery() const { return m_resQuery; }
        void generateQuery();
        inline T_typeQG typeQG() const { return m_type; }

    private:
        QString concatWhere() const;
        void flush();
        QString m_resQuery;

    protected:
        void finalPrepare();
        virtual void doFinalPrepare() = 0;

        QStringList m_listSelect, m_listFrom, m_listWhere;
        int m_quantityRes;
        T_typeQG m_type;
    };

    typedef cmmn::T_id T_id;
    typedef struct {
        T_id idPrim;
        QString genData;
    } T_resData;
    typedef QVector<T_resData> T_arrResData;

    GeneratorDBTData();
    ~GeneratorDBTData();
    QueryGenerator::TypeQueryGenerator typeQueryGenerator() const;
    void setQueryGenerator(QueryGenerator *gen);
    void generate(const dbi::DBTFieldInfo &foreignFieldInf);
    bool hasNextResultData() const;
    const GeneratorDBTData::T_resData &nextResultData();

private:
    enum { INIT_RES_INDEX = 0 };

    void generate_Mask_QueryData(const dbi::DBTFieldInfo &ffield, int &fieldCounter);
    void generateResultData();
    bool isReadyToGeneration();
    void flush();

    QueryGenerator *m_queryGen; // use std::unique_ptr<> here
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
    typedef cmmn::T_id T_id;
    QueryGenForeignOneId(QueryGenForeignOneId::T_id idFor, QueryGenForeignOneId::T_id idPrim,
                         TypeQueryGenerator typeqg = tqg_foreignOneId);
    virtual ~QueryGenForeignOneId();
protected:
    virtual void doFinalPrepare();
    T_id m_idFor, m_idPrim; // foreign and primary keys id's. Primary key need for support the algorithm of a DBT data generation
};

/*
 * Query generator primary key all id - is a strategy class, that generate a query for obtaining data from database
 * for all id's of a some main table (all values of the primary key).   This class allow obtain all data, stored in the main table.
 * This class grows up the effectiveness of application work - minimize the access to DB when need to extract all data from DB.
 */
class QueryGenPrimaryAllId : public GeneratorDBTData::QueryGenerator
{
public:
    QueryGenPrimaryAllId(const QString &mainTableName = QString(), const QString &foreignFieldName = QString(),
                         TypeQueryGenerator typeqg = tqg_primaryAllId);
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
    typedef cmmn::T_id T_id;
    enum { NOT_SET = -1 };
    QueryGenPrimaryOneId(QueryGenPrimaryOneId::T_id idPrim = QueryGenPrimaryOneId::NOT_SET,
                         const QString &mainTableName = QString(),
                         const QString &foreignFieldName = QString(),
                         TypeQueryGenerator typeqg = tqg_primaryOneId);
    virtual ~QueryGenPrimaryOneId();
    inline void setId(QueryGenPrimaryOneId::T_id id) { m_idPrim = id; }
    void setId(const QVariant &varId);
protected:
    virtual void doFinalPrepare();
    T_id m_idPrim;
};

#endif // GENERATOR_DBT_DATA_H

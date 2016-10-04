#include <QSqlQuery>
#include <QSqlError>
#include <QVector>
#include <QDebug>
#include "generator_dbt_data.h"
#include "../common/db_info.h"

/* QueryGenerator */
GeneratorDBTData::QueryGenerator::QueryGenerator(TypeQueryGenerator typeqg)
    : m_type(typeqg)
{ }

GeneratorDBTData::QueryGenerator::~QueryGenerator()
{ }

void GeneratorDBTData::QueryGenerator::generateQuery()
{
    ASSERT_DBG( !m_listFrom.isEmpty() && !m_listSelect.isEmpty(),
                cmmn::MessageException::type_critical, QObject::tr("Error query generation"),
                QObject::tr("Too many attempts to get the query string, used for generation displayed data"),
                QString("GeneratorDBTData::QueryGenerator::generateQuery()") );

//    qDebug() << "Not ready query data:\nSELECT:" << m_listSelect << "\nFROM:" << m_listFrom << "\nWHERE:" << m_listWhere;
    finalPrepare(); // the Template Method
    m_quantityRes = m_listSelect.size(); // save quantity of the result data
//    qDebug() << "Ready query data:\nSELECT:" << m_listSelect.join(", ") << "\nFROM:" << m_listFrom.join(", ") << "\nWHERE:" << m_listWhere.join(", ");
    m_resQuery = QString("SELECT %1 FROM %2 WHERE %3;")
                 .arg( m_listSelect.join(", ") ).arg( m_listFrom.join(", ") ).arg( concatWhere() );
    flush();
    //    qDebug() << "query:" << m_resQuery;
}

QString GeneratorDBTData::QueryGenerator::concatWhere() const
{
    // Concatenate the WHERE statement with adding the "AND" and the "=" items
    QString str("");
    for (auto it = m_listWhere.cbegin(); it != m_listWhere.cend(); it += 2) {
        if (it != m_listWhere.cbegin()) str += "AND ";
        str += ( *it + " = " + *(it + 1) + " " );
    }
    return str;
}

void GeneratorDBTData::QueryGenerator::flush()
{
    m_listSelect.clear();
    m_listFrom.clear();
    m_listWhere.clear();
}

// The Template Method design pattern. The virtual doFinalPrepare() method is a hook operation (GoF, p.312-313)
void GeneratorDBTData::QueryGenerator::finalPrepare()
{
    m_listFrom.removeDuplicates();
    m_listWhere.push_back(m_listFrom.first() + ".id");
    doFinalPrepare();
}

/* QueryGenForeignOneId */
QueryGenForeignOneId::QueryGenForeignOneId(T_id idFor, T_id idPrim, TypeQueryGenerator typeqg)
    : GeneratorDBTData::QueryGenerator(typeqg)
    , m_idFor(idFor)
    , m_idPrim(idPrim)
{ }

QueryGenForeignOneId::~QueryGenForeignOneId()
{ }

void QueryGenForeignOneId::doFinalPrepare()
{
    m_listSelect.push_front(QString::number(m_idPrim));
    m_listWhere.push_front(QString::number(m_idFor));
}

/* QueryGenPrimaryAllId */
QueryGenPrimaryAllId::QueryGenPrimaryAllId(const QString &mainTableName, const QString &foreignFieldName, TypeQueryGenerator typeqg)
    : GeneratorDBTData::QueryGenerator(typeqg)
    , m_mTableName(mainTableName)
    , m_fFieldName(foreignFieldName)
{ }

QueryGenPrimaryAllId::~QueryGenPrimaryAllId()
{ }

void QueryGenPrimaryAllId::doFinalPrepare()
{
    if (m_mTableName.isEmpty() || m_fFieldName.isEmpty())
        throw std::logic_error("QueryGenPrimaryAllId::doFinalPrepare(), main table name or foreign field name was not setted.");
    m_listSelect.push_front(m_mTableName + ".id");
    m_listFrom.push_front(m_mTableName);
    m_listWhere.push_front(m_mTableName + "." + m_fFieldName);
}

/* QueryGenPrimaryOneId */
QueryGenPrimaryOneId::QueryGenPrimaryOneId(T_id idPrim, const QString &mainTableName, const QString &foreignFieldName, TypeQueryGenerator typeqg)
    : QueryGenPrimaryAllId(mainTableName, foreignFieldName, typeqg)
    , m_idPrim(idPrim)
{
}

QueryGenPrimaryOneId::~QueryGenPrimaryOneId()
{ }

void QueryGenPrimaryOneId::doFinalPrepare()
{
    if (m_idPrim == NOT_SET)
        throw std::logic_error("QueryGenPrimaryOneId::doFinalPrepare(), primary id value was not setted");
    QueryGenPrimaryAllId::doFinalPrepare();
    m_listWhere.push_front(m_mTableName + ".id");
    m_listWhere.push_front(QString::number(m_idPrim));
}

void QueryGenPrimaryOneId::setId(const QVariant &varId)
{
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, m_idPrim), varId );
}

/*****************************************************************************************************************************/
/*
 * GeneratorDBTData
 */
GeneratorDBTData::GeneratorDBTData()
{ }

GeneratorDBTData::~GeneratorDBTData()
{
    delete m_queryGen;
}

GeneratorDBTData::QueryGenerator::TypeQueryGenerator GeneratorDBTData::typeQueryGenerator() const
{
    return m_queryGen->typeQG();
}

void GeneratorDBTData::setQueryGenerator(QueryGenerator *gen)
{
//    m_queryGen.reset(gen); // use in release version
    m_queryGen = gen;
}

void GeneratorDBTData::generate(const dbi::DBTFieldInfo &foreignFieldInf)
{
    flush();
    ASSERT_DBG( isReadyToGeneration(),
                cmmn::MessageException::type_critical, QObject::tr("Error data generating"),
                QObject::tr("The functor for database query was not setted"),
                QString("GeneratorDBTData::generate()") );
    int fieldsCounter = 0;
//    qDebug() << "generate() 1, field:" << foreignFieldInf.m_nameInDB;
    generate_Mask_QueryData( foreignFieldInf, fieldsCounter );
//    qDebug() << "generate() 2";
//    qDebug() << "MASK:" << m_strMask;
    generateResultData();
//    qDebug() << "generate() 3";
}

/* Generate data mask and data for generation query string */
void GeneratorDBTData::generate_Mask_QueryData(const dbi::DBTFieldInfo &ffield, int &fieldCounter)
{
    dbi::DBTInfo *table = dbi::relatedDBT(ffield);
    const auto &idnFieldsArr = table->m_idnFields;
    for (const auto &idnField : idnFieldsArr) {
        m_strMask += idnField.m_strBefore;

        if ( dbi::DBTInfo::isUseStringAfter(idnField.m_NField) )
            continue; // in this case a "string before" become a "string after" after the previous "field number"

        const dbi::DBTFieldInfo &fieldInf = table->fieldByIndex( idnField.m_NField );
        if (fieldInf.isForeign()) {
            m_queryGen->addWhere( table->m_nameInDB + ".id" );
            m_queryGen->addWhere( table->m_nameInDB + "." + fieldInf.m_nameInDB );
            generate_Mask_QueryData( fieldInf, fieldCounter ); /* recursive calling */
        }
        else {
            // when current field isn't a foreign key -> exit from recursion
            m_strMask += QString("%%1").arg(++fieldCounter);
            m_queryGen->addSelect( table->m_nameInDB + "." + fieldInf.m_nameInDB );
            m_queryGen->addFrom( table->m_nameInDB );
        }
    }
}

void GeneratorDBTData::generateResultData()
{
    T_id idPrim = -1;
    QString strRes;
    m_queryGen->generateQuery();
//    qDebug() << "generateResultData(), last generated query:" << m_queryGen->lastGeneratedQuery();
    QSqlQuery query(m_queryGen->lastGeneratedQuery());
    m_resData.reserve(query.size()); // allocate the storage memory for effective adding data to the storage
    qDebug() << "Generate result data, MASK:" << m_strMask;
    while (query.next()) {
        // get the primary id value
        const QVariant &varId = query.value(0);
        CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, idPrim), varId );
        strRes = m_strMask;
        for (int i = 1; i < m_queryGen->quantityResultData(); ++i)
            strRes = strRes.arg( query.value(i).toString() ); // forming result data with using mask and QString::arg()
        m_resData.push_back( {idPrim, strRes} );
        qDebug() << "Generate result data, id:" << idPrim << ", data:" << strRes;
    }
    ASSERT_DBG( idPrim != -1,
                cmmn::MessageException::type_critical, QObject::tr("Error data getting"),
                QObject::tr("Cannot get a data from the database table for generation. ") +
                QObject::tr("The database error: ") + query.lastError().text(),
                QString("GeneratorDBTData::generateResultData()") );
//    qDebug() << "-----------------------------";
}

bool GeneratorDBTData::hasNextResultData() const
{
    return m_resData.size() > m_indexResData;
}

const GeneratorDBTData::T_resData &GeneratorDBTData::nextResultData()
{
    return m_resData.at(m_indexResData++);
}

bool GeneratorDBTData::isReadyToGeneration()
{
    return m_queryGen != 0;
}

void GeneratorDBTData::flush()
{
    m_indexResData = INIT_RES_INDEX;
    m_strMask.clear();
    m_resData.clear();
}

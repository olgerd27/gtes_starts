#include <QSqlQuery>
#include <QSqlError>
#include <QVector>
#include <QDebug>
#include "dbt_data_generator.h"
#include "db_info.h"
#include "common_defines.h"

/* Query preparer - prepare by all "id" of a some main table */
QuePrepPrimaryAllId::QuePrepPrimaryAllId(const QString &mainTableName, const QString &foreignFieldName)
    : m_mTableName(mainTableName)
    , m_fFieldName(foreignFieldName)
{ }

QuePrepPrimaryAllId::~QuePrepPrimaryAllId()
{ }

void QuePrepPrimaryAllId::finalPrepare(QStringList &listSelect, QStringList &listFrom, QStringList &listWhere)
{
    if (m_mTableName.isEmpty() || m_fFieldName.isEmpty())
        throw std::logic_error("QPMainTableAllId::finalPrepare(), main table name or foreign field name was not setted.");
    listSelect.push_front(m_mTableName + ".id");
    listFrom.push_front(m_mTableName);
    listWhere.push_front(m_mTableName + "." + m_fFieldName);
}

/* Query preparer - prepare by the one particular "id" of a some main table */
QuePrepPrimaryOneId::QuePrepPrimaryOneId(int id, const QString &mainTableName, const QString &foreignFieldName)
    : QuePrepPrimaryAllId(mainTableName, foreignFieldName)
    , m_id(id)
{ }

QuePrepPrimaryOneId::~QuePrepPrimaryOneId()
{ }

void QuePrepPrimaryOneId::finalPrepare(QStringList &listSelect, QStringList &listFrom, QStringList &listWhere)
{
    if (m_id == NOT_SET)
        throw std::logic_error("QPMainTableOneId::finalPrepare(), id value was not setted");
    QuePrepPrimaryAllId::finalPrepare(listSelect, listFrom, listWhere);
    // add additional SELECT query statement - id value of the main table record
    listWhere.push_front(m_mTableName + ".id");
    listWhere.push_front(QString::number(m_id));
}

void QuePrepPrimaryOneId::setId(const QVariant &varId)
{
    m_id = cmmn::safeQVariantToInt(varId);
}

/*****************************************************************************************************************************/
/*
 * DBTDataGenerator
 */
DBTDataGenerator::DBTDataGenerator()
{ }

DBTDataGenerator::~DBTDataGenerator()
{ }

void DBTDataGenerator::setQueryPreparer(IQueryPreparer *pr)
{
    m_queryGen.setQueryPreparer(pr);
}

void DBTDataGenerator::setForeignFieldName(const QString &name)
{
    if ( QuePrepPrimaryAllId *pr = dynamic_cast<QuePrepPrimaryAllId *>(m_queryGen.queryPreparer()) )
         pr->setForeignFieldName(name);
}

void DBTDataGenerator::generate(const dbi::DBTFieldInfo &foreignFieldInf)
{
    flush();
    if (!isReadyToGeneration())
        throw cmmn::MessageException(cmmn::MessageException::type_critical,
                                     QObject::tr("Error data generating"),
                                     QObject::tr("The functor for database query was not setted"), "DBTDataGenerator::generate");
    int fieldsCounter = 0;
    generate_Mask_QueryData( foreignFieldInf, fieldsCounter );
    generateResultData();
}

/* Generate data mask and data for generation query string */
void DBTDataGenerator::generate_Mask_QueryData(const dbi::DBTFieldInfo &ffield, int &fieldCounter)
{
    dbi::DBTInfo *table = dbi::relatedDBT(ffield);
    const auto &idnFieldsArr = table->m_idnFields;
    for (const auto &idnField : idnFieldsArr) {
        const dbi::DBTFieldInfo &fieldInf = table->fieldByIndex( idnField.m_NField );
        m_strMask += idnField.m_strBefore;
        if (fieldInf.isValid() && fieldInf.isForeign()) {
            m_queryGen.addWhere( table->m_nameInDB + ".id" );
            m_queryGen.addWhere( table->m_nameInDB + "." + fieldInf.m_nameInDB );
            generate_Mask_QueryData( fieldInf, fieldCounter ); /* recursive calling */
        }
        else {
            m_strMask += QString("%%1").arg(++fieldCounter); /* when current field isn't a foreign key -> exit from recursion */
            m_queryGen.addSelect( table->m_nameInDB + "." + fieldInf.m_nameInDB );
            m_queryGen.addFrom( table->m_nameInDB );
        }
    }
}

void DBTDataGenerator::generateResultData()
{
    int id = -1;
    QString strRes;
    QSqlQuery query(m_queryGen.generateQuery());
    m_resData.reserve(query.size());
    while (query.next()) {
        strRes = m_strMask;
        id = cmmn::safeQVariantToInt(query.value(0));
        for (int i = 1; i < m_queryGen.quantityResultData(); ++i)
            strRes = strRes.arg( query.value(i).toString() ); // forming result data
        m_resData.push_back( {id, strRes} );
    }
    if (id == -1) {
         // TODO: make correct error message - replace "Some DB table" to a some another string.
        throw cmmn::MessageException( cmmn::MessageException::type_critical,
                                      QObject::tr("Error data getting"),
                                      QObject::tr("Cannot get a data from the database table for generation.") + "\n"
                                      + QObject::tr("The database error:") + query.lastError().text(),
                                      "DBTDataGenerator::generateResultData()" );
    }
}

bool DBTDataGenerator::hasNextResultData() const
{
    return m_resData.size() > m_indexResData;
}

DBTDataGenerator::T_resData DBTDataGenerator::nextResultData()
{
    return m_resData.at(m_indexResData++);
}

bool DBTDataGenerator::isReadyToGeneration()
{
    return m_queryGen.queryPreparer() != 0;
}

void DBTDataGenerator::flush()
{
    m_indexResData = INIT_RES_INDEX;
    m_strMask.clear();
    m_resData.clear();
}

/*****************************************************************************************************************************/
/*
 * DBTDataGenerator::QueryGenerator
 */
DBTDataGenerator::QueryGenerator::QueryGenerator()
    : m_queryPrep(0)
{
}

DBTDataGenerator::QueryGenerator::~QueryGenerator()
{
    delete m_queryPrep;
}

void DBTDataGenerator::QueryGenerator::setQueryPreparer(IQueryPreparer *qp)
{
    delete m_queryPrep;
    m_queryPrep = qp;
}

QString DBTDataGenerator::QueryGenerator::generateQuery()
{
    if (m_listFrom.isEmpty() || m_listSelect.isEmpty()) {
        throw cmmn::MessageException( cmmn::MessageException::type_critical,
                                      QObject::tr("Error query generation"),
                                      QObject::tr("Too many attempts to get the query, used for generation displayed data"),
                                      "DBTDataGenerator::QueryGenerator::generateQuery" );
    }

//    qDebug() << "Not ready query data:\nSELECT:" << m_listSelect;
//    qDebug() << "FROM:" << m_listFrom;
//    qDebug() << "WHERE:" << m_listWhere;

    /* preparing the FROM statement */
    m_listFrom.removeDuplicates();

    /* preparing the WHERE statements */
    m_listWhere.push_back(m_listFrom.first() + ".id");

    /* final preparing with help of strategy function */
    m_queryPrep->finalPrepare(m_listSelect, m_listFrom, m_listWhere);
    m_quantityRes = m_listSelect.size(); // save quantity of the result data

//    qDebug() << "Ready query data:\nSELECT:" << m_listSelect.join(", ");
//    qDebug() << "FROM:" << m_listFrom.join(", ");
//    qDebug() << "WHERE:" << m_listWhere.join(", ");

    QString strQuery = QString("SELECT %1 FROM %2 WHERE %3;")
            .arg( m_listSelect.join(", ") ).arg( m_listFrom.join(", ") ).arg( concatWhere() );
    flush();
    return strQuery;
}

QString DBTDataGenerator::QueryGenerator::concatWhere() const
{
    // Concatenate the WHERE statement with adding the "AND" and the "=" items
    QString str("");
    for (auto it = m_listWhere.cbegin(); it != m_listWhere.cend(); it += 2) {
        if (it != m_listWhere.cbegin()) str += "AND ";
        str += ( *it + " = " + *(it + 1) + " " );
    }
    return str;
}

void DBTDataGenerator::QueryGenerator::flush()
{
    m_listSelect.clear();
    m_listFrom.clear();
    m_listWhere.clear();
}

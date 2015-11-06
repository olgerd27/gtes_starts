#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "custom_sql_table_model.h"
#include "db_info.h"
#include "common_defines.h"

/*
 * Strategy functors for final preparing query data. They used by the DisplayDataGenerator::QueryGenerator class methods.
 * This functors must implement the interface, defining in the DisplayDataGenerator::pfFinalPrepQueryData function pointer.
 */
/*
 * Strategy class, that help to prepare a query for obtaining data from database for particular id value
 */
struct QueryPreparerOne : public IQueryPreparer
{
    QueryPreparerOne(int id)
        : m_id(id)
    { }

    virtual ~QueryPreparerOne() { }

    virtual void finalPrepare(QStringList &listSelect, QStringList &listFrom, QStringList &listWhere)
    {
        Q_UNUSED(listSelect)
        Q_UNUSED(listFrom)
        listWhere.push_front(QString::number(m_id));
    }

private:
    int m_id;
};

/*
 * Strategy class, that help to prepare a query for obtaining all data from database for particular table and its field
 */
struct QueryPreparerAll : public IQueryPreparer
{
    QueryPreparerAll(const QString &mainTableName, const QString &foreignFieldName)
        : m_tableName(mainTableName)
        , m_fieldName(foreignFieldName)
    { }

    virtual ~QueryPreparerAll() { }

    virtual void finalPrepare(QStringList &listSelect, QStringList &listFrom, QStringList &listWhere)
    {
        listSelect.push_front(m_tableName + ".id");
        listFrom.push_front(m_tableName);
        listWhere.push_front(m_tableName + "." + m_fieldName);
    }

private:
    QString m_tableName, m_fieldName;
};

/*****************************************************************************************************************************/

/*
 * DisplayDataGenerator
 */
DisplayDataGenerator::DisplayDataGenerator()
    : m_queryPrep(0)
{ }

DisplayDataGenerator::~DisplayDataGenerator()
{
    delete m_queryPrep;
}

int DisplayDataGenerator::generate(const QString &complexTableName)
{
    if (!m_queryPrep)
        throw cmmn::MessageException(cmmn::MessageException::type_critical,
                                     QObject::tr("Error data generating"),
                                     QObject::tr("The functor for database query was not setted"), "DisplayDataGenerator::generate");
    flush();
    int fieldsCounter = 0;
    generate_Mask_QueryData( DBINFO.tableByName(complexTableName), fieldsCounter );
    m_strQuery = m_queryGen.generateQuery(m_queryPrep);
    return m_queryGen.quantityResultData();
}

void DisplayDataGenerator::setQueryPreparer(IQueryPreparer *qp)
{
    delete m_queryPrep;
    m_queryPrep = qp;
}

/* Generate data mask and data for generation query string */
void DisplayDataGenerator::generate_Mask_QueryData(dbi::DBTInfo *table, int &fieldCounter)
{
    const auto &idnFieldsArr = table->m_idnFields;
    for (const auto &idnField : idnFieldsArr) {
        const dbi::DBTFieldInfo &field = table->fieldByIndex( idnField.m_NField );
        m_strMask += idnField.m_strBefore;
        if (field.isValid() && field.isForeign()) {
            m_queryGen.addWhere( table->m_nameInDB + ".id" );
            m_queryGen.addWhere( table->m_nameInDB + "." + field.m_nameInDB );
            generate_Mask_QueryData( dbi::relatedDBT(field), fieldCounter ); /* recursive calling */
        }
        else {
            m_strMask += QString("%%1").arg(++fieldCounter); /* when current field isn't a foreign key -> exit from recursion */
            m_queryGen.addSelect( table->m_nameInDB + "." + field.m_nameInDB );
            m_queryGen.addFrom( table->m_nameInDB );
        }
    }
}

void DisplayDataGenerator::flush()
{
    m_strMask.clear();
    m_strQuery.clear();
}

/*
 * DisplayDataGenerator::QueryGenerator
 */
QString DisplayDataGenerator::QueryGenerator::generateQuery(IQueryPreparer *queryPrep)
{
    if (m_listFrom.isEmpty() || m_listSelect.isEmpty()) {
        throw cmmn::MessageException( cmmn::MessageException::type_critical,
                                      QObject::tr("Error query generation"),
                                      QObject::tr("Too many attempts to get the query, used for generation displayed data"),
                                      "DisplayDataGenerator::QueryGenerator::generateQuery" );
    }

//    qDebug() << "SELECT 1:" << m_listSelect;
//    qDebug() << "FROM 1:" << m_listFrom;
//    qDebug() << "WHERE 1:" << m_listWhere;

    /* preparing the FROM statement */
    m_listFrom.removeDuplicates();

    /* preparing the WHERE statements */
    m_listWhere.push_back(m_listFrom.first() + ".id");

    /* final preparing with help of strategy function */
    queryPrep->finalPrepare(m_listSelect, m_listFrom, m_listWhere);
    m_quantityRes = m_listSelect.size();

    // add "AND" and "=" to the query expression
    QString strWhere("");
    for (auto it = m_listWhere.cbegin(); it != m_listWhere.cend(); it += 2) {
        if (it != m_listWhere.cbegin()) strWhere += "AND ";
        strWhere += ( *it + " = " + *(it + 1) + " " );
    }

//    qDebug() << "SELECT 2:" << m_listSelect.join(", ");
//    qDebug() << "FROM 2:" << m_listFrom.join(", ");
//    qDebug() << "WHERE 2:" << strWhere;

    QString strQuery = QString("SELECT %1 FROM %2 WHERE %3;")
            .arg( m_listSelect.join(", ") ).arg( m_listFrom.join(", ") ).arg( strWhere );
    flush();
    return strQuery;
}

void DisplayDataGenerator::QueryGenerator::flush()
{
    m_listSelect.clear();
    m_listFrom.clear();
    m_listWhere.clear();
}

/*****************************************************************************************************************************/
/*
 * CustomSqlTableModel
 * Description of the spike #1.
 * When user change a some field that is a foreign key (in particular, a foreign key of a related complex DB table), there are performs
 * a calling of the QSqlRelationalTableModel::setData() method from the CustomSqlTableModel::setData().
 * The QSqlRelationalTableModel::setData() method (or maybe the QSqlTableModel::setData() method (called from the
 * QSqlRelationalTableModel::setData()) ) set some incorrect data to the items, that is different from present (current item).
 * This setted data are: to the DisplayRole - a QString-type already generated data, to the EditRole - a QString-type empty instance.
 * This invalid behaviour of data settings, which is protected from influent, need to prevent. This is achieved by usage saving and
 * following restoring data, that must not change.
 * Turn on the run of this operations performs by calling the public method setDataWithSavings(). Turn off the run of this operations
 * performs without assistance from outside in the restoreData() method.
 */
CustomSqlTableModel::CustomSqlTableModel(QObject *parent, QSqlDatabase db)
    : QSqlRelationalTableModel(parent, db)
    , m_bNeedSave(false) /* Spike #1 */
{ }

void CustomSqlTableModel::setDataWithSavings()
{
    m_bNeedSave = true; /* Spike #1 */
}

void CustomSqlTableModel::setTable(const QString &tableName)
{
    QSqlRelationalTableModel::setTable(tableName);
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    try {
        if (m_storage.isEmpty()) fillTheStorage();
    }
    catch (const cmmn::MessageException &me) {
        QString strPlace = QObject::tr("Error placement") + ": " + me.placement();
        qDebug() << "[ERROR custom]: Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
    }
    catch (const std::exception &ex) {
        qDebug() << "[ERROR standard]:" << ex.what();
    }
}

void CustomSqlTableModel::fillTheStorage()
{
    defineFieldsRelations();
    for (int field = 0; field < columnCount(QModelIndex()); ++field) {
        const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), field);
        if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_complex)) {
            m_dataGen.setQueryPreparer( new QueryPreparerAll(tableName(), fieldInf.m_nameInDB) );
            int resDataQnt = m_dataGen.generate(fieldInf.m_relationDBTable);
            saveDisplayData(m_dataGen.mask(), m_dataGen.query(), resDataQnt);
        }
    }
}

/* Define quantity of fields, that is foreign keys to the complex database table */
void CustomSqlTableModel::defineFieldsRelations()
{
    dbi::DBTInfo *tableInf = DBINFO.tableByName(tableName());
    for (int col = 0; (unsigned)col < tableInf->m_fields.size(); ++col) {
        const dbi::DBTFieldInfo &fieldInf = tableInf->fieldByIndex(col);
        if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_simple)) {
            m_indexSimple.push_back(col);
            setRelationsWithSimpleDBT(col);
        }
        else if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_complex))
            m_indexComplex.push_back(col);
    }
}

void CustomSqlTableModel::setRelationsWithSimpleDBT(int fieldIndex)
{
    const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), fieldIndex);
    dbi::DBTInfo *tableInf = dbi::relatedDBT(fieldInf);
    setRelation( fieldIndex, QSqlRelation( tableInf->m_nameInDB,
                                           tableInf->m_fields.at(0).m_nameInDB, tableInf->m_fields.at(1).m_nameInDB ) );
}

int safeQVariantIdToInt(const QVariant &value)
{
    bool b = false;
    int id = value.toInt(&b);
    if (!b)
        throw cmmn::MessageException( cmmn::MessageException::type_critical,
                                      QObject::tr("Conversion error"),
                                      QObject::tr("Cannot convert the foreign key value \"%1\" from the QVariant to the integer type.")
                                      .arg(value.toString()), "safeQVariantIdToInt");
    return id;
}

void CustomSqlTableModel::saveDisplayData(const QString &strMask, const QString &strQuery, int resDataQuantity)
{
    int id = -1;
    QString strRes;
    QSqlQuery query(strQuery);
    while (query.next()) {
        strRes = strMask;
        id = safeQVariantIdToInt(query.value(0));
        for (int i = 1; i < resDataQuantity; ++i)
            strRes = strRes.arg( query.value(i).toString() );
//        qDebug() << "id:" << id << ", data:" << strRes;
//        if (m_storage.contains(id))
//        m_storage.insert(id, strRes);
    }
    if (id == -1) {
        throw cmmn::MessageException( cmmn::MessageException::type_critical,
                                      QObject::tr("Error data getting"),
                                      QObject::tr("Cannot get a data from the \"%1\" database table for displaying.").arg(tableName())
                                      + "\n" + QObject::tr("The database error:") + query.lastError().text(),
                                      "CustomSqlTableModel::generateDisplayData" );
    }
}

QVariant CustomSqlTableModel::data(const QModelIndex &item, int role) const
{
    /*
     * Rules of data getting:
     * - in time of initial running of a view performs getting from the DB big data amount, save it in the map and filling view by one index;
     * - next calling of the data() method must only get data from the map and put it in the view by index (index is a row number);
     * - if user performs the UPDATE or INSERT a some record (row) operations, changes sends to the DB and after this updates
     *   the map and view with appropriate index (row number);
     * - if user performs the DELETE a some record (row) operation, changes sends to the DB and at the same time deletes appropriate
     *   value in the map;
     * - the next getting a big amount of data performs only if the rows number of data in DB is bigger then items number in the map with saved data.
     */
    // TODO: use try-catch
    QVariant data = QSqlRelationalTableModel::data(item, role);
    //    qDebug() << "data(), [" << item.row() << "," << item.column() << "], role =" << role << ", data =" << data.toString();
    int colNumb = item.column();
    if ( colNumb != 2 && role == Qt::EditRole ) {
        const dbi::DBTFieldInfo &fieldInfo = dbi::fieldByNameIndex(tableName(), colNumb);
        if ( fieldInfo.isValid() && fieldInfo.isForeign() && (DBINFO.tableByName( fieldInfo.m_relationDBTable )->m_type == dbi::DBTInfo::ttype_complex) ) {
            /* TODO: generation must calls from the setData() method. When the storage is empty, there are need performs filling the storage by all
             * available data from the DB by one referencing to the DB. This operation performs in the setData() method, and this method must be called
             * from the setTable() method. In this method performs only getting data from the storage.
             */
//            data = DisplayDataGenerator().generate( fieldInfo.m_relationDBTable, data ); // TODO: temporary generation -> delete later
            // TODO: convert the item.column() {1, 3, 4} values to the indexes of stored data {0, 1, 2}
//            data = m_mStorage.value(QSqlRelationalTableModel::data( this->index(item.row(), 0), Qt::DisplayRole ).toInt()).at(item.column());
        }
    }
    return data;
}

bool CustomSqlTableModel::setData(const QModelIndex &item, const QVariant &value, int role)
{
    if (!item.isValid()) return false;
    // TODO: use try-catch
//    qDebug() << "setData(), [" << item.row() << "," << item.column() << "], role =" << role << ", data =" << value.toString();
    if (m_bNeedSave) saveData(item, role); // Spike #1
    bool bSucc = QSqlRelationalTableModel::setData(item, value, role);
    if (m_bNeedSave) restoreData(item.row(), role); // Spike #1
    if (bSucc) emit dataChanged(item, item);
    return bSucc;
}

/* Save data before calling the QSqlRelationalTableModel::setData() method. Spike #1 */
void CustomSqlTableModel::saveData(const QModelIndex &currentIndex, int role)
{
//    qDebug() << "saveData(), role =" << role;
    const dbi::DBTInfo::T_arrDBTFieldsInfo &fieldsInf = DBINFO.tableByName(tableName())->m_fields;
    for (int col = 0; (unsigned)col < fieldsInf.size(); ++col) {
        /* if a field is not the current field and it is a foreign key (related with a complex DB table) -> save its data */
        if ( (col != currentIndex.column()) && (fieldsInf.at(col).relationDBTtype() == dbi::DBTInfo::ttype_complex) ) {
            m_saveRestore.insert( col, QSqlRelationalTableModel::data( this->index(currentIndex.row(), col), role ) );
        }
    }
}

/* Restore saved data after calling the QSqlRelationalTableModel::setData() method. Spike #1 */
void CustomSqlTableModel::restoreData(int currentRow, int role)
{
    for (auto it = m_saveRestore.cbegin(); it != m_saveRestore.cend(); ++it) {
//        qDebug() << "data restoring. col =" << it.key() << ", value =" << it.value().toString();
        QSqlRelationalTableModel::setData( index(currentRow, it.key()), it.value(), role );
    }
    m_saveRestore.clear();
    m_bNeedSave = false;
}

/*****************************************************************************************************************************/
/*
 * CustomSqlRelationalDelegate
 */
CustomSqlRelationalDelegate::CustomSqlRelationalDelegate(QObject *parent)
    : QSqlRelationalDelegate(parent)
{ }

CustomSqlRelationalDelegate::~CustomSqlRelationalDelegate()
{ }

void CustomSqlRelationalDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    if (!index.isValid())
        return;

    qDebug() << "delegate, row =" << index.row() << ", col =" << index.column();

    QSqlTableModel *sqlTable = qobject_cast<QSqlTableModel *>(model);
    if (!sqlTable) return;
    const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(sqlTable->tableName(), index.column());
    bool isForeign = fieldInf.isValid() && fieldInf.isForeign();
    if (!isForeign)
        QItemDelegate::setModelData(editor, model, index);
    else if ( isForeign && (DBINFO.tableByName( fieldInf.m_relationDBTable )->m_type == dbi::DBTInfo::ttype_simple) )
        setDataToSimpleDBT(editor, model, index);
}

void CustomSqlRelationalDelegate::setDataToSimpleDBT(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    /*
     * Set data to the QSqlRelationalTableModel if the current field has relation with a simple DB table
     * (with help of the QSqlRelation class instance and used a combo box for choosing records of a simple DB table).
     * This code was taked from the QSqlRelationalDelegate::setModelData() method.
     */
    QSqlRelationalTableModel *sqlModel = qobject_cast<QSqlRelationalTableModel *>(model);
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (sqlModel && childModel && combo) {
        int currentItem = combo->currentIndex();
        int childColIndex = childModel->fieldIndex(sqlModel->relation(index.column()).displayColumn());
        int childEditIndex = childModel->fieldIndex(sqlModel->relation(index.column()).indexColumn());
        sqlModel->setData(index,
                          childModel->data(childModel->index(currentItem, childColIndex), Qt::DisplayRole),
                          Qt::DisplayRole);
        sqlModel->setData(index,
                          childModel->data(childModel->index(currentItem, childEditIndex), Qt::EditRole),
                          Qt::EditRole);
    }
}

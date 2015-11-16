#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "custom_sql_table_model.h"
#include "db_info.h"
#include "common_defines.h"

/*
 * Strategy class, that help to prepare a query for obtaining all data from database for particular table and its field
 */
struct QueryPreparerAllId : public IQueryPreparer
{
    QueryPreparerAllId(const QString &mainTableName = QString(), const QString &foreignFieldName = QString())
        : m_mTableName(mainTableName)
        , m_fFieldName(foreignFieldName)
    { }

    virtual ~QueryPreparerAllId() { }

    virtual void finalPrepare(QStringList &listSelect, QStringList &listFrom, QStringList &listWhere)
    {
        if (m_mTableName.isEmpty() || m_fFieldName.isEmpty())
            throw std::logic_error("QueryPreparerAllId::finalPrepare(), main table name or foreign field name was not setted.");
        listSelect.push_front(m_mTableName + ".id");
        listFrom.push_front(m_mTableName);
        listWhere.push_front(m_mTableName + "." + m_fFieldName);
    }

    inline void setMainTableName(const QString &mtName) { m_mTableName = mtName; }
    inline void setForeignFieldName(const QString &ffName) { m_fFieldName = ffName; }

protected:
    QString m_mTableName, m_fFieldName;
};

/*
 * Strategy class, that help to prepare a query for obtaining data from database for particular one id value
 */
struct QueryPreparerOneId : public QueryPreparerAllId
{
    QueryPreparerOneId(int id, const QString &mainTableName = QString(), const QString &foreignFieldName = QString())
        : QueryPreparerAllId(mainTableName, foreignFieldName)
        , m_id(id)
    { }

    virtual ~QueryPreparerOneId() { }

    virtual void finalPrepare(QStringList &listSelect, QStringList &listFrom, QStringList &listWhere)
    {
        QueryPreparerAllId::finalPrepare(listSelect, listFrom, listWhere);
        // add additional SELECT query statement - id value of the main table record
        listWhere.push_front(m_mTableName + ".id");
        listWhere.push_front(QString::number(m_id));
    }

protected:
    int m_id;
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

int DisplayDataGenerator::generate(const dbi::DBTFieldInfo &foreignFieldInf)
{
    if (!m_queryPrep)
        throw cmmn::MessageException(cmmn::MessageException::type_critical,
                                     QObject::tr("Error data generating"),
                                     QObject::tr("The functor for database query was not setted"), "DisplayDataGenerator::generate");
    flush();
    int fieldsCounter = 0;
    generate_Mask_QueryData( foreignFieldInf, fieldsCounter );

//    qDebug() << "mask:" << m_strMask;
//    m_queryGen.print();

    m_strQuery = m_queryGen.generateQuery(m_queryPrep);
    return m_queryGen.quantityResultData();
}

void DisplayDataGenerator::setQueryPreparer(IQueryPreparer *qp)
{
    delete m_queryPrep;
    m_queryPrep = qp;
}

/* Generate data mask and data for generation query string */
void DisplayDataGenerator::generate_Mask_QueryData(const dbi::DBTFieldInfo &ffield, int &fieldCounter)
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
    m_quantityRes = m_listSelect.size(); // save quantity of the result data

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
class StorageChanger
{
public:
    virtual ~StorageChanger() {}
    virtual void changeStorage(CustomSqlTableModel::T_storage &storage, int id, const QVariant &data) = 0;
};

class StorageUpdater : public StorageChanger
{
public:
    StorageUpdater(int paramIndex)
        : m_paramIndex(paramIndex)
    {}

    ~StorageUpdater() {}
    virtual void changeStorage(CustomSqlTableModel::T_storage &storage, int id, const QVariant &data)
    {
        storage[id][m_paramIndex] = data;
    }
private:
    int m_paramIndex;
};

class StorageBackInserter : public StorageChanger
{
public:
    ~StorageBackInserter() {}
    virtual void changeStorage(CustomSqlTableModel::T_storage &storage, int id, const QVariant &data)
    {
        storage[id].push_back(data);
    }
};

/*****************************************************************************************************************************/
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

/*
 * CustomSqlTableModel
 *
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
 *
 * Rules of data setting and getting:
 * - in time of initial running of a view performs getting from the DB all data and saving it in the storage;
 * - calling of the data() method only get data from the storage and put it in the view by current index (index is a row number);
 * - if user performs the UPDATE or INSERT a some record (row) operations, changes sends to the DB and after this updates
 *   the storage and view with appropriate index (row number);
 * - if user performs the DELETE a some record (row) operation, changes sends to the DB and immediately deletes appropriate value in the storage;
 * - if user click to the Refresh action, the custom model clear storage and get all data again.
 */
CustomSqlTableModel::CustomSqlTableModel(QObject *parent, QSqlDatabase db)
    : QSqlRelationalTableModel(parent, db)
    , m_storageChanger(0)
    , m_bNeedSave(false) /* Spike #1 */
{ }

CustomSqlTableModel::~CustomSqlTableModel()
{
    delete m_storageChanger;
}

void CustomSqlTableModel::setDataWithSavings()
{
    m_bNeedSave = true; /* Spike #1 */
}

void CustomSqlTableModel::setTable(const QString &tableName)
{
    QSqlRelationalTableModel::setTable(tableName);
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    try {
        if (m_storage.isEmpty()) slotFillTheStorage();
    }
    catch (const cmmn::MessageException &me) {
        QString strPlace = QObject::tr("Error placement") + ": " + me.placement();
        qDebug() << "[ERROR custom]: Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
    }
    catch (const std::exception &ex) {
        qDebug() << "[ERROR standard]:" << ex.what();
    }
}

void CustomSqlTableModel::changeComplexDBTData(int colFrom, int colTo)
{
    bool needSaveComplexIndex = m_indexComplex.empty(); // true - if this is a filling or refilling all storage
    for (int col = colFrom; col < colTo; ++col) {
        const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), col);
        if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_simple))
            setRelationsWithSimpleDBT(col);
        else if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_complex)) {
            if (needSaveComplexIndex) m_indexComplex.push_back(col); // save index of the fields, that is a foreign key to a some complex database table
            if ( QueryPreparerAllId *pr = dynamic_cast<QueryPreparerAllId *>(m_dataGen.queryPreparer()) )
                 pr->setForeignFieldName(fieldInf.m_nameInDB);
            int resDataQnt = m_dataGen.generate(fieldInf);
            saveDisplayData(m_dataGen.mask(), m_dataGen.query(), resDataQnt);
        }
    }
}

void CustomSqlTableModel::setRelationsWithSimpleDBT(int fieldIndex)
{
    const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), fieldIndex);
    dbi::DBTInfo *tableInf = dbi::relatedDBT(fieldInf);
    setRelation( fieldIndex, QSqlRelation( tableInf->m_nameInDB,
                                           tableInf->m_fields.at(0).m_nameInDB, tableInf->m_fields.at(1).m_nameInDB ) );
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
            strRes = strRes.arg( query.value(i).toString() ); // forming result data
//        qDebug() << "id:" << id << ", data:" << strRes;
        m_storageChanger->changeStorage(m_storage, id, strRes); // save results data to the storage
    }
    if (id == -1) {
        throw cmmn::MessageException( cmmn::MessageException::type_critical,
                                      QObject::tr("Error data getting"),
                                      QObject::tr("Cannot get a data from the \"%1\" database table for displaying.").arg(tableName())
                                      + "\n" + QObject::tr("The database error:") + query.lastError().text(),
                                      "CustomSqlTableModel::generateDisplayData" );
    }
}

void CustomSqlTableModel::flush()
{
    m_storage.clear();
}

//QVariant CustomSqlTableModel::data(const QModelIndex &item, int role) const
//{
//    /*
//     * Rules of data getting:
//     * - in time of initial running of a view performs getting from the DB big data amount, save it in the storage and filling view by one index;
//     * - next calling of the data() method must only get data from the storage and put it in the view by index (index is a row number);
//     * - if user performs the UPDATE or INSERT a some record (row) operations, changes sends to the DB and after this updates
//     *   the storage and view with appropriate index (row number);
//     * - if user performs the DELETE a some record (row) operation, changes sends to the DB and immediately deletes appropriate value in the storage;
//     * - the next getting a big amount of data performs only if the rows number of data in DB is bigger then items number in the storage.
//     */
//    QVariant data = QSqlRelationalTableModel::data(item, role);
//    //    qDebug() << "data(), [" << item.row() << "," << item.column() << "], role =" << role << ", data =" << data.toString();
//    int colNumb = item.column();
//    if ( role == Qt::EditRole ) {
//        const dbi::DBTFieldInfo &fieldInfo = dbi::fieldByNameIndex(tableName(), colNumb);
//        if ( fieldInfo.isValid() && fieldInfo.isForeign() && (DBINFO.tableByName( fieldInfo.m_relationDBTable )->m_type == dbi::DBTInfo::ttype_complex) ) {
//            /* TODO: generation must calls from the setData() method. When the storage is empty, there are need performs filling the storage by all
//             * available data from the DB by one referencing to the DB. This operation performs in the setData() method, and this method must be called
//             * from the setTable() method. In this method performs only getting data from the storage.
//             */
//            data = DisplayDataGenerator().generate( fieldInfo.m_relationDBTable, data ); // TODO: temporary generation -> delete later

//            // TODO: convert the item.column() {1, 3, 4} values to the indexes of stored data {0, 1, 2}
////            data = m_storage.value(QSqlRelationalTableModel::data( this->index(item.row(), 0), Qt::DisplayRole ).toInt()).at(item.column());
//        }
//    }
//    return data;
//}

QVariant CustomSqlTableModel::data(const QModelIndex &item, int role) const
{
    // TODO: use try-catch
    QVariant data = QSqlRelationalTableModel::data(item, role);
    if ( role == Qt::EditRole ) {
        int indexStorage = m_indexComplex.indexOf( safeQVariantIdToInt(item.column()) ); // converting the column number to the storage index
        if (indexStorage != -1) {
            int id = safeQVariantIdToInt( QSqlRelationalTableModel::data( this->index(item.row(), 0), Qt::DisplayRole ) );
            data = m_storage.value(id).at(indexStorage);
//            qDebug() << "column =" << item.column() << ", index =" << indexStorage << ", data:" << data;
        }
    }
    return data;
}

bool CustomSqlTableModel::setData(const QModelIndex &item, const QVariant &value, int role)
{
    if (!item.isValid()) return false;
    // TODO: use try-catch
    qDebug() << "setData(), [" << item.row() << "," << item.column() << "], role =" << role << ", data =" << value.toString();
//    if (m_bNeedSave) saveData(item, role); // Spike #1
    bool bSucc = QSqlRelationalTableModel::setData(item, value, role);
//    if (m_bNeedSave) restoreData(item.row(), role); // Spike #1
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

void CustomSqlTableModel::setStorageChanger(StorageChanger *changer)
{
    delete m_storageChanger;
    m_storageChanger = changer;
}

void CustomSqlTableModel::slotFillTheStorage()
{
    flush(); // delete previous results
    select();
    m_dataGen.setQueryPreparer( new QueryPreparerAllId(tableName()) );
    setStorageChanger( new StorageBackInserter );
    changeComplexDBTData( 0, this->columnCount(QModelIndex()) );
}

void CustomSqlTableModel::slotInsertToTheStorage(int id)
{
    // TODO: maybe need to call setData?
    m_dataGen.setQueryPreparer( new QueryPreparerOneId(id, tableName()) );
    setStorageChanger( new StorageBackInserter );
    changeComplexDBTData( 0, this->columnCount(QModelIndex()) );
}

void CustomSqlTableModel::slotUpdateTheStorage(int id, int colNumb)
{
    m_dataGen.setQueryPreparer( new QueryPreparerOneId(id, tableName()) );
    int indexComplex = m_indexComplex.indexOf( safeQVariantIdToInt(colNumb) ); // converting the column number to the storage index
    if (indexComplex == -1) return;
    setStorageChanger( new StorageUpdater(indexComplex) );
    changeComplexDBTData( colNumb, colNumb );
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

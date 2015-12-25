#include <QSqlQuery>
#include <QDebug>
#include "custom_sql_table_model.h"
#include "dbt_data_generator.h"
#include "gen_data_storage.h"
#include "../common/db_info.h"
#include "../common/common_defines.h"

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
 * Rules of data setting and getting (v.1):
 * - in time of initial running of a view performs getting from the DB all data and saving it in the storage;
 * - calling of the data() method only get data from the storage and put it in the view by current index (index is a row number);
 * - if user performs the UPDATE or INSERT a some record (row) operations, changes sends to the DB and after this updates
 *   the storage and view with appropriate index (row number);
 * - if user performs the DELETE a some record (row) operation, changes sends to the DB and immediately deletes appropriate value in the storage;
 * - if user click to the Refresh action, the custom model clear storage and get all data again.
 *
 * Rules of data setting and getting (v.2):
 * - in time of set model table name, performs filling the model with data with help of the setData() method.
 *   For this, performs extraction from the DB all data for every column, that is foreign key to the complex DB table.
 *   Every value, extracted from the DB, used for generation display data. In the setData() method performs savings the DB data and
 *   the generated display data {manual} {user, edit role};
 * - views extracts any data from the model {automatic} {edit/display role};
 * - if there are need to get a some value from the model -> get it with the {user} role {manual};
 * - if there are need to set a some value to the model -> set it with {edit} role {manual}.
 *
 * Using primary and foreign keys for changing data in the model:
 * - refreshing all data (filling storage) - primary key id;
 * - inserting a new row in the DBT - primary key id;
 * - deleting a some row in the DBT - primary key id;
 * - updating a some item - foreign key (for generating data), primary key (for saving generated data to the storage).
 */
CustomSqlTableModel::CustomSqlTableModel(QObject *parent, QSqlDatabase db)
    : QSqlRelationalTableModel(parent, db)
    , m_dataGenerator(new DBTDataGenerator)
    , m_genDataStorage(new GenDataStorage)
    , m_bNeedSave(false) /* Spike #1 */
{ }

CustomSqlTableModel::~CustomSqlTableModel()
{
    delete m_dataGenerator;
    delete m_genDataStorage;
}

void CustomSqlTableModel::setDataWithSavings()
{
    m_bNeedSave = true; /* Spike #1 */
}

void CustomSqlTableModel::setTable(const QString &tableName)
{
    QSqlRelationalTableModel::setTable(tableName);
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (m_genDataStorage->isEmpty()) {
        defineSimpleDBTAndComplexIndex();
        slotRefreshTheModel();
    }
}

void CustomSqlTableModel::defineSimpleDBTAndComplexIndex()
{
    /* Model initialization: define relations with simple DBT columns number and save complex DBT columns numbers (indexes) */
    // Check - is this a first calling
    if (!m_genDataStorage->isNotSetted()) {
        // TODO: generate warning about: trying to initialize already initialized model
        qDebug() << "trying to initialize already initialized model";
        return;
    }
    // Processing
    for (int col = 0; col < this->columnCount(QModelIndex()); ++col) {
        const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), col);
        if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_simple)) {
            dbi::DBTInfo *tableInf = dbi::relatedDBT(fieldInf);
            setRelation( col, QSqlRelation( tableInf->m_nameInDB,
                                            tableInf->m_fields.at(0).m_nameInDB, tableInf->m_fields.at(1).m_nameInDB ) );
        }
        else if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_complex))
            m_genDataStorage->addFieldIndex(col); // save index of the fields, that is a foreign key to a some complex database table
    }
}

void CustomSqlTableModel::changeComplexDBTData(int colFrom, int colTo)
{
    for (int col = colFrom; col < colTo; ++col) {
        const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), col);
        if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_complex)) {
            m_dataGenerator->setForeignFieldName(fieldInf.m_nameInDB);
            m_dataGenerator->generate(fieldInf);
        }
    }
}

QVariant CustomSqlTableModel::data(const QModelIndex &item, int role) const
{
    // TODO: use try-catch
    QVariant data = QSqlRelationalTableModel::data(item, role);
    int storageDataIndex = -1;
    if ( (role == Qt::EditRole || role == Qt::DisplayRole) && m_genDataStorage->isComplexDBTFieldIndex(item.column(), storageDataIndex) ) {
        int idPrim = cmmn::safeQVariantToInt( QSqlRelationalTableModel::data( this->index(item.row(), 0), Qt::DisplayRole ) ); // primary id
        if (m_genDataStorage->isStorageContainsId(idPrim))
            data = m_genDataStorage->data(idPrim, storageDataIndex); // get generated data
//        qDebug() << "data(), [" << item.row() << "," << item.column() << "], role:" << role << ", data:" << data.toString();
    }
    else if (role == Qt::UserRole && m_genDataStorage->isComplexDBTFieldIndex(item.column(), storageDataIndex)) {
        data = QSqlRelationalTableModel::data(item, Qt::DisplayRole);
//        qDebug() << "data(), [" << item.row() << "," << item.column() << "], role:" << role << ", data:" << data.toString();
    }
    return data;
}

bool CustomSqlTableModel::setData(const QModelIndex &item, const QVariant &value, int role)
{
    /*
     * You cannot call the QSqlRelationalTableModel::setData() method with the Qt::UserRole to save some data in the existent model.
     * In this case the QSqlRelationalTableModel::setData() method return false.
     * The setData() method successfully works only with Qt::EditRole.
     * If you wanna save data with the Qt::UserRole (ot UserRole + 1, +2, ...), you must implement saving in a some custom storage.
     */
    if (!item.isValid()) return false;
    // TODO: use try-catch
//    if (m_bNeedSave) saveData(item, role); // Spike #1
    bool bSetted = false;
    int storageComplexIndex = -1;
    if (role == Qt::EditRole && m_genDataStorage->isComplexDBTFieldIndex(item.column(), storageComplexIndex) ) {
        bSetted = QSqlRelationalTableModel::setData(item, value, role);
//        qDebug() << "setData(), [" << item.row() << "," << item.column() << "], role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;

        try {
            updateDataInStorage(item, storageComplexIndex);
        }
        catch (const cmmn::MessageException &me) {
            // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
            QString strPlace = QObject::tr("Error placement") + ": " + me.placement();
            qDebug() << "[ERROR custom] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
            return false;
        }
        catch (const std::exception &ex) {
            // TODO: generate a message box with critical error
            qDebug() << "[ERROR standard] Message: " << ex.what();
            return false;
        }

        if (bSetted) emit dataChanged(item, item);
    }
//    if (m_bNeedSave) restoreData(item.row(), role); // Spike #1
    return bSetted;
}

/* generate string data and update the storage */
void CustomSqlTableModel::updateDataInStorage(const QModelIndex &index, int storageComplexIndex)
{
    /*
     * TODO:
     * - get foreign key id value - [index.row(), index.column()] (not primary key id value - [index.row(), 0])
     * - create the QuePrepForeignOneId class for preparing the query, that used for data generation
     * - there are need to change the DBTDataGenerator class in part of generation result data
     *   (now, the primary key id value not used, only foreign key id value)
     */
    int idPrim = cmmn::safeQVariantToInt( QSqlRelationalTableModel::data( this->index(index.row(), 0), Qt::DisplayRole ) ); // primary id
//    qDebug() << "updateDataInStorage(), primary id =" << idPrim;
    const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), index.column());
    QuePrepPrimaryOneId *qp = new QuePrepPrimaryOneId(idPrim, tableName(), fieldInf.m_nameInDB);
    m_dataGenerator->setQueryPreparer(qp);
    m_dataGenerator->generate(fieldInf);
    if (m_dataGenerator->hasNextResultData()) {
        const auto &resData = m_dataGenerator->nextResultData();
        m_genDataStorage->updateData(resData.idPrim, storageComplexIndex, resData.genData);
    }
    else {
        // TODO: generate error message
        qDebug() << "!--Error. Cannot set a data to the model storage. No one data was generated. "
                    "Please check the query for generating data for the item: [" << index.row() << "," << index.column() << "]";
        return;
    }
    if (m_dataGenerator->hasNextResultData()) {
        // TODO: generate error message
        qDebug() << "!--Error. Cannot set a data to the model storage. Too many data was generated. "
                    "Please check the query for generating data for the item: [" << index.row() << "," << index.column() << "]";
        return;
    }
}

void CustomSqlTableModel::flush()
{
    m_genDataStorage->clear();
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

void CustomSqlTableModel::printData() const
{
    QString strPrint;
    for(int row = 0; row < rowCount(); ++row) {
        for(int col = 0; col < columnCount(); ++col ) {
            QModelIndex index = QSqlRelationalTableModel::index(row, col);
            strPrint += (QSqlRelationalTableModel::data(index).toString() + "  ");
        }
        strPrint += "\n";
    }
    qDebug() << "Model data after select() calling:\n" << strPrint;
}

void CustomSqlTableModel::slotRefreshTheModel()
{
    try {
        flush(); // delete previous results - need when refreshing all data
        select();
        fillTheStorage();
        m_genDataStorage->flushToGetFIndex();
    }
    catch (const cmmn::MessageException &me) {
        // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
        QString strPlace = QObject::tr("Error placement") + ": " + me.placement();
        qDebug() << "[ERROR custom] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
        return;
    }
    catch (const std::exception &ex) {
        // TODO: generate a message box with critical error
        qDebug() << "[ERROR standard] Message: " << ex.what();
        return;
    }
}

void CustomSqlTableModel::fillTheStorage()
{
//    qDebug() << "fill the storage";
    QuePrepPrimaryAllId *qp = new QuePrepPrimaryAllId(tableName());
    m_dataGenerator->setQueryPreparer(qp);
    while ( m_genDataStorage->hasNextFieldIndex() ) {
        auto complexFIndex = m_genDataStorage->nextFieldIndex();
        const dbi::DBTFieldInfo &fieldInfo = dbi::fieldByNameIndex(tableName(), complexFIndex);
        qp->setForeignFieldName(fieldInfo.m_nameInDB);
        m_dataGenerator->generate(fieldInfo);
        while (m_dataGenerator->hasNextResultData()) {
            const auto &resData = m_dataGenerator->nextResultData();
            m_genDataStorage->addData(resData.idPrim, resData.genData);
        }
        emit dataChanged( index(0, complexFIndex), index(rowCount() - 1, complexFIndex) ); // at first time - don't need, but maybe need when performs refreshing?
//        qDebug() << "dataChanged() was called";
    }
}

void CustomSqlTableModel::slotInsertToTheStorage(int idPrim)
{
//    m_dataGen->setQueryPreparer( new QPMainTableOneId(idPrim, tableName()) );
//    changeComplexDBTData( 0, this->columnCount(QModelIndex()) );
}

void CustomSqlTableModel::slotUpdateTheStorage(int idPrim, int colNumb)
{
//    m_dataGen->setQueryPreparer( new QPMainTableOneId(idPrim, tableName()) );
//    int indexComplex = m_indexComplex.indexOf( cmmn::safeQVariantToInt(colNumb) ); // converting the column number to the storage index
//    if (indexComplex == -1) return;
//    changeComplexDBTData( colNumb, colNumb );
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

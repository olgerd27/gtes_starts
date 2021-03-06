#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug> // TODO: delete later
#include <QMessageBox> // TODO: delete later
#include "custom_sql_table_model.h"
#include "generator_dbt_data.h"
#include "storage_gen_data.h"
#include "../../common/db_info.h"

/*
 * Spike1
 * Description of the spike #1.
 * When user set a new data in the foreign key field, the others foreign fields in the current row assigns generated string-type values,
 * but here must remain the int-type id values. This not expected behaviour performs in the QSqlRelationalTableModel::setData()
 * (or QSqlTableModel::setData()) method, which work process is private from outside. Most probably, this string-type generated values was readed
 * from the mapper widgets.
 * To prevent this invalid behaviour, it was added the spike #1. The Spike #1 performs data saving from the foreign key fields, before calling the
 * QSqlRelationalTableModel::setData() and restore saved data after calling the QSqlRelationalTableModel::setData(). It allows remains the int-type id values
 * in the foreign keys fields.
 * Turn on the Spike #1 performs by calling the public method useSpike1().
 * Turn off the Spike #1 performs automatically (without assistance from outside) in the restoreData_spike1() method.
 */
// TODO: rename this class
class Spike1
{
public:
    Spike1(CustomSqlTableModel *model);
    void save(const QModelIndex &index, const QVariant &data);
    void restore(const QModelIndex &index);
    inline void turnOn() { m_isNeedUse = true; }

private:
    typedef QMap<int, QVariant> T_storage;

    CustomSqlTableModel *m_model;
    T_storage m_storage;
    bool m_isNeedUse;
};

Spike1::Spike1(CustomSqlTableModel *model)
    : m_model(model)
    , m_isNeedUse(false)
{}

// Save data of the foreign keys in the current row and the current item data
void Spike1::save(const QModelIndex &index, const QVariant &data)
{
    if (!m_isNeedUse) return;
//    static int counter = 0;
    const dbi::DBTInfo *tableInf = DBINFO.tableByName( m_model->tableName() );
    for (int field = 0; field < tableInf->tableDegree(); ++field)
        if ( tableInf->fieldByIndex(field).isForeign() )
            m_storage.insert( field, ( field == index.column()
                                       ? data
                                       : m_model->data( m_model->index(index.row(), field), Qt::UserRole ) ) );
//    qDebug() << "Spike1::save() #" << counter++ << ":";
//    for (auto it = m_storage.cbegin(); it != m_storage.cend(); ++it)
//        qDebug() << it.key() << ":" << it.value().toString();
}

/* Restore saved data after calling the QSqlRelationalTableModel::setData() method. Spike #1 */
void Spike1::restore(const QModelIndex &index)
{
    if (!m_isNeedUse) return;
//    qDebug() << "Spike1::restore()";
    for (auto it = m_storage.cbegin(); it != m_storage.cend(); ++it) {
//        qDebug() << "[" << index.row() << "," << it.key() << "], replace:"
//                 << m_model->data( m_model->index(index.row(), it.key()), Qt::DisplayRole ).toString()
//                 << "to the:" << it.value().toString();
        m_model->setParentData( m_model->index(index.row(), it.key()), it.value(), Qt::EditRole );
    }
    m_storage.clear();
    m_isNeedUse = false;
}

/*
 * CustomSqlTableModel
 *
 * Rules of data setting and getting:
 * - in time of initial running of a view (and/or mapper) performs getting from the DB all data, generate the data of the foreign keys fields
 *   and save it in the storage. This operations performs when user choose REFRESH command;
 * - calling of the data() method get data from the model or storage (depending on field type - foreign key or not) and put it in the view
 *   by current index (index is a row number). Views extracts data (generated, string-type) from the model automatically with
 *   the Qt::EditRole and Qt::DisplayRole. User can extract int-type foreign-keys data from the model with Qt::UserRole;
 * - if a user choose the INSERT command of a some record (row), a new record inserts at the end of the model with the id = max id + 1. Next, user
 *   populate this row with data and only after this he can save changes in the DB (after populating fields, that is mandatory for populating);
 * - if user performs the DELETE a some record (row) operation, deletes appropriate value in the storage and model and only after calling SAVE
 *   this changes sends to the DB;
 * - user or view set data to the model and the storage with Qt::EditRole role - UPDATE data command.
 *
 * Using primary and foreign keys for changing data in the model:
 * - refreshing all data (filling storage) - primary key id;
 * - inserting a new row in the DBT - primary key id;
 * - deleting a some row in the DBT - primary key id;
 * - updating a some item - foreign key (for generating data), primary key (for saving generated data to the storage).
 */
CustomSqlTableModel::CustomSqlTableModel(QObject *parent, QSqlDatabase db)
    : QSqlRelationalTableModel(parent, db)
    , m_dataGenerator(new GeneratorDBTData)
    , m_genDataStorage(new StorageGenData)
    , m_spike1(new Spike1(this))
{ }

CustomSqlTableModel::~CustomSqlTableModel()
{
    delete m_dataGenerator;
    delete m_genDataStorage;
    delete m_spike1;
}

void CustomSqlTableModel::spike1_turnOn()
{
    m_spike1->turnOn();
}

void CustomSqlTableModel::setTable(const QString &tableName)
{
    QSqlRelationalTableModel::setTable(tableName);
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    setSort(0, Qt::AscendingOrder);
    try {
        if (m_genDataStorage->isEmpty()) {
            defineForeignFields();
            slotRefreshModel();
        }
    }
    catch (const cmmn::MessageException &me) {
        // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
        QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
        qCritical() << "[CRITICAL ERROR] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
        exit(2);
    }
    catch (const std::exception &ex) {
        // TODO: generate a message box with critical error
        qCritical() << "[CRITICAL ERROR] Message: " << ex.what();
        exit(3);
    }
}

QVariant CustomSqlTableModel::data(const QModelIndex &idx, int role) const
{
    QVariant data;
    if (!idx.isValid()) return data;
    int storageDataIndex = -1;
    if ( (role == Qt::EditRole || role == Qt::DisplayRole) && m_genDataStorage->isForeignField(idx.column(), storageDataIndex) ) {
        try {
//            qDebug() << "source model, data(), is foreign, BEFORE getDataFromStorage(), index: [" << idx.row() << "," << idx.column() << "]";

            data = getDataFromStorage(idx, storageDataIndex);

//            qDebug() << "source model, data(), is foreign, AFTER getDataFromStorage(), index: [" << idx.row() << "," << idx.column() << "]"
//                     << ",  data:" << data.toString();

//            qDebug() << "data(), foreign field,  [" << idx.row() << "," << idx.column() << "], role =" << role
//                     << ", storageDataIndex =" << storageDataIndex << ", data:" << data;
        }
        catch (const cmmn::MessageException &me) {
            // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
            QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
            qCritical() << "[CRITICAL ERROR] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
            return QVariant();
        }
        catch (const std::exception &ex) {
            // TODO: generate a message box with critical error
            qCritical() << "[CRITICAL ERROR] Message: " << ex.what();
            return QVariant();
        }
    }
    else if (role == Qt::UserRole) {
        data = QSqlRelationalTableModel::data(idx, Qt::DisplayRole); // take data from the display role and neightbor's cell (provides by the baseIdx index)
//        qDebug() << "source model, data(), UserRole, [" << idx.row() << "," << idx.column() << "], role:" << role << ", data:" << data.toString();
    }
    else
        data = QSqlRelationalTableModel::data(idx, role);
    return data;
}

bool CustomSqlTableModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    /*
     * There are impossible to call the QSqlRelationalTableModel::setData() method with the Qt::UserRole to save some data in the existent model.
     * In this case the QSqlRelationalTableModel::setData() method return false. The setData() method successfully works only with Qt::EditRole.
     * Saving data with the Qt::UserRole (or UserRole + 1, +2, ...) can be achieved by means of data saving in a some custom storage.
     */
    qDebug() << "Source model setData(), [" << idx.row() << "," << idx.column() << "],  data:" << value.toString();

    if (!idx.isValid()) return false;
    bool bSetted = false;
    int storageDataIndex = -1;
    m_spike1->save(idx, value); // spike 1: save data and restore at the end of this method
    if (role == Qt::EditRole && value == NOT_SETTED) {
        // fill the model's new row by the empty values.
        // The storages new rows filling performs earlier by calling the CustomSqlTableModel::slotInsertRecord()
        bSetted = QSqlRelationalTableModel::setData(idx, QVariant(), role);
//        qDebug() << "source setData(), NOT_SETTED: [" << idx.row() << "," << idx.column() << "],"
//                 << "role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
    }
    else if (role == Qt::EditRole && m_genDataStorage->isForeignField(idx.column(), storageDataIndex) ) {
        qDebug() << "source setData(), is foreign, START, index: [" << idx.row() << "," << idx.column() << "],  data:" << value.toString();
        bSetted = QSqlRelationalTableModel::setData(idx, value, role);

        try {
            updateDataInStorage(idx, storageDataIndex);
        }
        catch (const cmmn::MessageException &me) {
            // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
            QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
            qCritical() << "[CRITICAL ERROR] [Data setting custom ERROR] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
            return false;
        }
        catch (const std::exception &ex) {
            // TODO: generate a message box with critical error
            qCritical() << "[CRITICAL ERROR] [Data setting standard ERROR] Message: " << ex.what();
            return false;
        }
        if (bSetted) emit dataChanged(idx, idx);
    }
    else {
        bSetted = QSqlRelationalTableModel::setData(idx, value, role);
//        qDebug() << "source setData(), ELSE case: [" << idx.row() << "," << idx.column() << "],"
//                 << "role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
    }
    m_spike1->restore(idx);
//    qDebug() << "setData(), [" << item.row() << "," << item.column() << "], role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
    return bSetted;
}

QVariant CustomSqlTableModel::primaryIdInRow(int row) const
{
    return QSqlRelationalTableModel::data(index(row, 0), Qt::DisplayRole);
}

bool CustomSqlTableModel::getIdRow(const QVariant &idPrim, int &rRowValue) const
{
    for (int row = 0; row < rowCount(); ++row) {
        if (primaryIdInRow(row) == idPrim) {
            rRowValue = row;
            return true;
        }
    }
    return false;
}

// Save the foreign keys fields (columns numbers)
void CustomSqlTableModel::defineForeignFields()
{
    // Check - is this the first calling
    if (!m_genDataStorage->isNotSettedForeignFields()) {
        // TODO: generate warning about: Initialization of the earlier initialized model -> delete foreign fields previous data
        qWarning() << "Initialization of the earlier initialized model -> delete foreign fields previous data";
        m_genDataStorage->clearForeignFields();
    }
    // Processing
    const dbi::DBTInfo *tableInfo = DBINFO.tableByName(tableName());
    for (int col = 0; col < columnCount(); ++col) {
        if (tableInfo->fieldByIndex(col).isForeign()) {
            m_genDataStorage->addFieldIndex(col);
        }
    }
}

void CustomSqlTableModel::setHeader()
{
    for (int col = 0; col < columnCount(); ++col) {
        const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), col);
        setHeaderData(col, Qt::Horizontal, fieldInf.m_nameInUI, Qt::EditRole);
    }
}

void CustomSqlTableModel::fillTheStorage()
{
    QueryGenPrimaryAllId *qgen = new QueryGenPrimaryAllId(tableName()); // the foreign fields will be setted in the while loop below
    m_dataGenerator->setQueryGenerator(qgen);
//    QString strPrint;
    while ( m_genDataStorage->hasNextFieldIndex() ) {
        auto foreignFieldIndex = m_genDataStorage->nextFieldIndex();
        const dbi::DBTFieldInfo &fieldInfo = dbi::fieldByNameIndex(tableName(), foreignFieldIndex);
        qgen->setForeignFieldName(fieldInfo.m_nameInDB);
        m_dataGenerator->generate(fieldInfo);
//        strPrint += QString("field %1: %2, data:\n").arg(foreignFieldIndex).arg(fieldInfo.m_nameInDB);
        while (m_dataGenerator->hasNextResultData()) {
            const auto &resData = m_dataGenerator->nextResultData();
//            strPrint += QString("  id = %1, value = %2\n").arg(resData.idPrim).arg(resData.genData);
            m_genDataStorage->addData(resData.idPrim, resData.genData);
        }
//        emit dataChanged( index(0, foreignFieldIndex), index(rowCount() - 1, foreignFieldIndex) ); // NOTE: Is this need or not?
    }
//    qDebug() << strPrint;
}

cmmn::T_id CustomSqlTableModel::insertNewRecord()
{
    int Nrows = rowCount(); // quantity of the existing rows and number of the next row, that will be inserting
    // TODO: maybe create strategies, that performs different definition of a new id.
    // Now exists the only one definition: new_id = last_id + 1
    const QVariant &varId = primaryIdInRow(Nrows - 1);
    cmmn::T_id newIdPrim;
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, newIdPrim), varId );
    ++newIdPrim; // obtaining the new (next) primary id

    ASSERT_DBG( insertRow(Nrows),
                cmmn::MessageException::type_critical, QObject::tr("Insertion error"),
                QObject::tr("The new row cannot be inserted"),
                QString("CustomSqlTableModel::insertNewRecord") );

    // set initial values in the inserted row
    for (int col = 0; col < columnCount(); ++col) {
        if ( dbi::fieldByNameIndex(tableName(), col).isForeign() )
            m_genDataStorage->addData(newIdPrim); // init the data storage -> must be before the model::setData() calling
        setData( this->index(Nrows, col), (col == 0 ? newIdPrim : (int)NOT_SETTED), Qt::EditRole );
    }
    return newIdPrim;
}

QVariant CustomSqlTableModel::getDataFromStorage(const QModelIndex &index, int storageComplexIndex) const
{
    QVariant resData;
    cmmn::T_id idPrim; // primary id
    const QVariant &varIdPrim = QSqlRelationalTableModel::data( this->index(index.row(), 0), Qt::DisplayRole ); // get the primary id
//    qDebug() << "getDataFromStorage() 1, [" << index.row() << "," << 1 << "], varId:" << varIdPrim << ", data:";
//    printDataDB(Qt::DisplayRole);
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varIdPrim, idPrim), varIdPrim );

    if (m_genDataStorage->isStorageContainsId(idPrim)) {
        resData = m_genDataStorage->data(idPrim, storageComplexIndex);
    }
    return resData;
}

/* generate a data and update the storage */
void CustomSqlTableModel::updateDataInStorage(const QModelIndex &frgnIndex, int storageComplexIndex)
{
//    printDataDB(Qt::DisplayRole);
    // getting the primary and foreign id's values
    const QVariant &varIdPrim = QSqlRelationalTableModel::data( this->index(frgnIndex.row(), 0), Qt::DisplayRole );
    const QVariant &varIdFor = QSqlRelationalTableModel::data( frgnIndex, Qt::DisplayRole );

    cmmn::T_id idPrim, idFor; // primary and foreign keys values
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varIdPrim, idPrim), varIdPrim );
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varIdFor, idFor), varIdFor );

    // TODO: maybe create the Abstract Factory or other construction for prevent creating QueryGenForeignOneId if it is already setted
    m_dataGenerator->setQueryGenerator( new QueryGenForeignOneId(idFor, idPrim) );

    m_dataGenerator->generate( dbi::fieldByNameIndex(tableName(), frgnIndex.column()) );
    if (m_dataGenerator->hasNextResultData()) {
        const auto &resData = m_dataGenerator->nextResultData();
        m_genDataStorage->updateData(resData.idPrim, storageComplexIndex, resData.genData);
    }
    else {
        // TODO: generate error message
        qCritical() << "[CRITICAL ERROR] Cannot set a data to the model storage. Data wasn't generated.\n"
                    "Please check the query for generating data for the item: [" << frgnIndex.row() << "," << frgnIndex.column() << "]";
        return;
    }
    if (m_dataGenerator->hasNextResultData()) {
        // TODO: generate error message
        qCritical() << "[CRITICAL ERROR] Cannot set a data to the model storage. Too many data was generated.\n"
                    "Please check the query for generating data for the item: [" << frgnIndex.row() << "," << frgnIndex.column() << "]";
        return;
    }
}

void CustomSqlTableModel::flushGenData()
{
    m_genDataStorage->clear();
}

void CustomSqlTableModel::setParentData(const QModelIndex &idx, const QVariant &value, int role)
{
    QSqlRelationalTableModel::setData( idx, value, role );
}

void CustomSqlTableModel::printDataDB(int role) const
{
    QString strData = "\n";
    for(int row = 0; row < QSqlRelationalTableModel::rowCount(); ++row) {
        for(int col = 0; col < QSqlRelationalTableModel::columnCount(); ++col ) {
            const QModelIndex &index = QSqlRelationalTableModel::index(row, col);
            strData += QString("|%1|  ").arg(QSqlRelationalTableModel::data(index, role).toString());
        }
        strData += "\n";
    }
    qDebug() << "The source base model data with role #" << role << ":" << strData;
}

void CustomSqlTableModel::printHeader(int role) const
{
    QString str = "\n";
    for (int sect = 0; sect < QSqlRelationalTableModel::columnCount(); ++sect) {
        str += QString("|") += QSqlRelationalTableModel::headerData(sect, Qt::Horizontal, role).toString() += QString("| ");
    }
    qDebug() << "horizontal source header data, role =" << role << ", data:" << str;
}

void CustomSqlTableModel::slotRefreshModel()
{
    try {
        flushGenData(); // delete previous results

//        qDebug() << "before slotRefreshModel()::select(), column count =" << columnCount();
//        printHeader(Qt::DisplayRole);
//        printDataDB(Qt::DisplayRole);

        if (!select()) { // populate the model
            // TODO: generate the error
            qCritical() << "[CRITICAL ERROR] Cannot refresh the model - error data populating.\nThe DB error text: " << lastError().text();
        }

//        qDebug() << "after slotRefreshModel()::select(), column count =" << columnCount();
//        printHeader(Qt::DisplayRole);
//        printDataDB(Qt::DisplayRole);

        setHeader(); // must calls only after calling the select method

//        qDebug() << "after slotRefreshModel()::insertColumn() and setHeader, column count =" << columnCount();
//        printHeader(Qt::DisplayRole);
//        printDataDB(Qt::DisplayRole);

//        m_genDataStorage->flushFieldIndex(); // reset field index
        fillTheStorage(); // populate the storage
        m_genDataStorage->flushFieldIndex(); // reset field index
//        printDataDB(Qt::EditRole); // TODO: delete later

//        printHeader(Qt::DisplayRole);
//        printDataDB(Qt::DisplayRole);

        emit sigModelRefreshed();
    }
    catch (const cmmn::MessageException &me) {
        // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
        QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
        qCritical() << "[CRITICAL ERROR] [Refresh model custom ERROR] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
        return;
    }
    catch (const std::exception &ex) {
        // TODO: generate a message box with critical error
        qCritical() << "[CRITICAL ERROR] [Refresh model standard ERROR] Message: " << ex.what();
        return;
    }
}

void CustomSqlTableModel::slotInsertRecord()
{
    try {
        auto newId = insertNewRecord();
        emit sigRecordInserted(rowCount() - 1, newId);
    }
    catch (const cmmn::MessageException &me) {
        // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
        QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
        qCritical() << "[CRITICAL ERROR] [Insert data custom ERROR] Title:" << me.title()
                    << "\nMessage:" << me.message() << "\n" << strPlace;
        return;
    }
    catch (const std::exception &ex) {
        // TODO: generate a message box with critical error
        qCritical() << "[CRITICAL ERROR] [Insert data standard ERROR] Message: " << ex.what();
        return;
    }
}

void CustomSqlTableModel::slotDeleteRecord(int row)
{
    // getting the primary id value
    cmmn::T_id idPrimary;
    const QVariant &varId = primaryIdInRow(row);
    qDebug() << "delete from the source model, row =" << row << ", id =" << varId.toString();
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, idPrimary), varId );

    ASSERT_DBG( removeRow(row), cmmn::MessageException::type_critical, tr("Delete record error"),
                tr("Cannot delete the record from the model of DB table %1. "
                   "Primary id = %2, row = %3").arg(tableName()).arg(idPrimary).arg(row),
                QString("CustomSqlTableModel::slotDeleteRecord") );
    ASSERT_DBG( m_genDataStorage->deleteData(idPrimary), cmmn::MessageException::type_critical, tr("Delete record error"),
                tr("Cannot delete the record from the custom data storage of the DB table %1. "
                   "Primary id = %2, row = %3").arg(tableName()).arg(idPrimary).arg(row),
                QString("CustomSqlTableModel::slotDeleteRecord") );
    qDebug() << "The record successfully deleted. DBT:" << tableName() << ", primary id =" << idPrimary << ", row =" << row;
    emit sigRecordDeleted(row, idPrimary);
}

void CustomSqlTableModel::slotSaveInDB()
{
    qDebug() << "Custom model. SAVE 1";
    ASSERT_DBG( database().transaction(), cmmn::MessageException::type_warning, tr("Database transaction error"),
                tr("The database driver do not support the transactions operations"),
                QString("CustomSqlTableModel::slotSaveInDB") );
    if (submitAll())
        database().commit();
    else {
        database().rollback();
        ASSERT_DBG( false, cmmn::MessageException::type_warning, tr("Error data submit to the database"),
                    tr("Cannot submit data to the database. The database report an ERROR #%1: %2")
                    .arg( lastError().nativeErrorCode() ).arg( lastError().text() ),
                    QString("CustomSqlTableModel::slotSaveInDB") );
    }
    emit sigSavedInDB();
    qDebug() << "Custom model. SAVE 10";
}

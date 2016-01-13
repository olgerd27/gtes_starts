#include <QSqlQuery>
#include <QSqlRecord>
#include <QDebug>
#include "custom_sql_table_model.h"
#include "generator_dbt_data.h"
#include "storage_gen_data.h"
#include "../common/db_info.h"

/*
 * CustomSqlTableModel
 *
 * Description of the spike #1.
 * When user set a new data in a some field, that is a foreign key related with the complex DB table, to others similar fields in current row assigns
 * generated string-type values, but here must remain the int-type id values. This not expected behaviour performs in the QSqlRelationalTableModel::setData()
 * method, which work process is private from outside. Most probably, this string-type generated values was readed from the mapper widgets.
 * To prevent this invalid behaviour there was added the spike #1. Spike #1 save data, that doesn't must changes, before calling the
 * QSqlRelationalTableModel::setData() and restore saved data after calling the QSqlRelationalTableModel::setData(). It allows remains the int-type id values
 * in the foreign keys, that is related with the complex DB table.
 * Turn on the spike #1 performs by calling the public method spike1_turnOn(). Turn off the spike #1 performs automatically (without assistance from outside)
 * in the restoreData_spike1() method.
 *
 * Rules of data setting and getting:
 * - in time of initial running of a view (and/or mapper) performs getting from the DB all data, generate data of the foreign keys related with
 *   complex DB tables and saving it in the storage. This operations performs when user choose REFRESH command;
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
    , m_bNeedSave_spike1(false) /* Spike #1 */
{ }

CustomSqlTableModel::~CustomSqlTableModel()
{
    delete m_dataGenerator;
    delete m_genDataStorage;
}

void CustomSqlTableModel::spike1_turnOn(bool bOn)
{
    m_bNeedSave_spike1 = bOn; /* Spike #1 */
}

void CustomSqlTableModel::setTable(const QString &tableName)
{
    QSqlRelationalTableModel::setTable(tableName);
    setEditStrategy(QSqlTableModel::OnManualSubmit);
//    setEditStrategy(QSqlTableModel::OnRowChange);
    if (m_genDataStorage->isEmpty()) {
        defineSimpleDBTAndComplexIndex();
        slotRefreshTheModel();
    }
}

QVariant CustomSqlTableModel::data(const QModelIndex &item, int role) const
{
    QVariant data;
//    qDebug() << "data() 1, [" << item.row() << "," << item.column() << "], role:" << role << ", data:" << data.toString();
    int storageDataIndex = -1;
    if ( (role == Qt::EditRole || role == Qt::DisplayRole) && m_genDataStorage->isComplexDBTField(item.column(), storageDataIndex) ) {
        try {
            getDataFromStorage(data, item, storageDataIndex);
        }
        catch (const cmmn::MessageException &me) {
            // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
            QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
            qCritical() << "[Data getting custom ERROR] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
            return false;
        }
        catch (const std::exception &ex) {
            // TODO: generate a message box with critical error
            qCritical() << "[Data getting standard ERROR] Message: " << ex.what();
            return false;
        }
    }
    else if ( role == Qt::UserRole )
        data = QSqlRelationalTableModel::data(item, Qt::DisplayRole); // take data from the display role
    else
        data = QSqlRelationalTableModel::data(item, role);
//    qDebug() << "data() 2, [" << item.row() << "," << item.column() << "], role:" << role << ", data:" << data.toString();
    return data;
}

bool CustomSqlTableModel::setData(const QModelIndex &item, const QVariant &value, int role)
{
    /*
     * There are not exist an ability to call the QSqlRelationalTableModel::setData() method with the Qt::UserRole to save some data in the existent model.
     * In this case the QSqlRelationalTableModel::setData() method return false. The setData() method successfully works only with Qt::EditRole.
     * Saving data with the Qt::UserRole (ot UserRole + 1, +2, ...) can be achieved by means of data saving in a some custom storage.
     */
    if (!item.isValid()) return false;
    bool bSetted = false;
    int storageComplexIndex = -1;
    if (role == Qt::EditRole && value == NOT_SETTED) {
        // fill the model's new row by the empty values. The storages new rows filling performs earlier by calling the CustomSqlTableModel::slotInsertToTheModel()
        bSetted = QSqlRelationalTableModel::setData(item, QVariant(), role);
//        qDebug() << "setData(), NOT_SETTED, [" << item.row() << "," << item.column() << "], role:" << role << ", set data: " << ", bSetted:" << bSetted;
    }
    else if (role == Qt::EditRole && m_genDataStorage->isComplexDBTField(item.column(), storageComplexIndex) ) {
        if (m_bNeedSave_spike1) saveData_spike1(item); // Spike #1
        bSetted = QSqlRelationalTableModel::setData(item, value, role);
//        qDebug() << "setData(), Complex DBT, [" << item.row() << "," << item.column() << "], role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
        if (m_bNeedSave_spike1) restoreData_spike1(item.row()); // Spike #1

        try {
            updateDataInStorage(item, storageComplexIndex);
            printData(Qt::EditRole); // TODO: delete later
        }
        catch (const cmmn::MessageException &me) {
            // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
            QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
            qCritical() << "[Data setting custom ERROR] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
            return false;
        }
        catch (const std::exception &ex) {
            // TODO: generate a message box with critical error
            qCritical() << "[Data setting standard ERROR] Message: " << ex.what();
            return false;
        }
        if (bSetted) emit dataChanged(item, item);
    }
    else {
        bSetted = QSqlRelationalTableModel::setData(item, value, role);
//        qDebug() << "setData(), NOT Complex DBT, [" << item.row() << "," << item.column() << "], role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
    }
    return bSetted;
}

void CustomSqlTableModel::defineSimpleDBTAndComplexIndex()
{
    /* Model initialization: define relations with simple DBT columns number and save complex DBT columns numbers (indexes) */
    // Check - is this a first calling
    if (!m_genDataStorage->isNotSetted()) {
        // TODO: generate warning about: trying to initialize already initialized model
        qWarning() << "trying to initialize already initialized model";
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

void CustomSqlTableModel::fillTheStorage()
{
    QueryGenPrimaryAllId *qgen = new QueryGenPrimaryAllId(tableName());
    m_dataGenerator->setQueryGenerator(qgen);
    while ( m_genDataStorage->hasNextFieldIndex() ) {
        auto complexFIndex = m_genDataStorage->nextFieldIndex();
        const dbi::DBTFieldInfo &fieldInfo = dbi::fieldByNameIndex(tableName(), complexFIndex);
        qgen->setForeignFieldName(fieldInfo.m_nameInDB);
        m_dataGenerator->generate(fieldInfo);
        while (m_dataGenerator->hasNextResultData()) {
            const auto &resData = m_dataGenerator->nextResultData();
            m_genDataStorage->addData(resData.idPrim, resData.genData);
        }
//        emit dataChanged( index(0, complexFIndex), index(rowCount() - 1, complexFIndex) );
    }
}

void CustomSqlTableModel::insertNewRecord()
{
    // version #1
    int Nrows = rowCount();
    QSqlRecord rec = record(Nrows - 1); // get the last existent record
    auto newIdPrim = cmmn::safeQVariantToIdType(rec.value(0)) + 1; // generate the new primary key id value
    rec.setValue(0, newIdPrim); // set a new primary key id value
    for (int i = 1; i < rec.count(); ++i) {
        rec.setValue(i, NOT_SETTED);
        m_genDataStorage->addData(newIdPrim); /* fill the storages new row by the empty values. The model filling of this row performs
                                                 in the setData() method, that calls by calling the insertRecord() method. */
    }
    qDebug() << "Record will insert:";
    for (int i = 0; i < rec.count(); ++i)
        qDebug() << i << ":" << rec.value(i).toString();

    qDebug() << "All records before inserting:\n" << printRecords();
    /* this one perform a calling of the CustomSqlTableModel::setData() method.
     * The "-1" row number allow to insert record to the end of the model table. */
    qDebug() << "Before record inserting, rows count =" << rowCount();
    if (!insertRecord(Nrows, rec)) {
        // TODO: generate the error
        qCritical() << "The new empty record cannot be inserted to the end of DB table" << tableName() << "model";

//        if (!insertRow(Nrows)) qDebug() << "  Cannot be inserted the row #" << Nrows;
//        else qDebug() << "  The row #" << Nrows << "was inserted";
//        qDebug() << "  after row #" << Nrows << "inserting, rows count =" << rowCount();

//        if (!setRecord(Nrows, rec)) qDebug() << "  Cannot set record to the row #" << Nrows;
//        else qDebug() << "  The record was setted to the row #" << Nrows;
//        qDebug() << "  after record setting to the row #" << Nrows << ", rows count =" << rowCount();
    }
//    qDebug() << "After record inserting, rows count =" << rowCount();
    qDebug() << "All records after inserting:\n" << printRecords();

    // version #2
//    int Nrows = rowCount();
//    QSqlRecord rec = record(Nrows - 1); // get the last existent record
//    auto newIdPrim = cmmn::safeQVariantToIdType(rec.value(0)) + 1; // generate the new primary key id value
////    rec.setValue(0, newIdPrim); // set a new primary key id value
//    for (int i = 1; i < rec.count(); ++i) {
////        rec.setValue(i, NOT_SETTED);
//        m_genDataStorage->addData(newIdPrim); // fill the storages new row by the empty values
//    }
//    if (!insertRow(Nrows)) {
//        // TODO: generate error
//        qDebug() << "Cannot be inserted the row #" << Nrows;
//    }
//    else qDebug() << "The row #" << Nrows << "was inserted";
//    QSqlRelationalTableModel::setData( index(Nrows, 0), newIdPrim, Qt::EditRole ); // set the new primary id value to the model
//    qDebug() << "All records after inserting:\n" << printRecords();
}

void CustomSqlTableModel::getDataFromStorage(QVariant &data, const QModelIndex &index, int storageComplexIndex) const
{
    auto idPrim = cmmn::safeQVariantToIdType( QSqlRelationalTableModel::data( this->index(index.row(), 0), Qt::DisplayRole ) ); // primary id
    if (m_genDataStorage->isStorageContainsId(idPrim))
        data = m_genDataStorage->data(idPrim, storageComplexIndex);
}

/* generate a data and update the storage */
void CustomSqlTableModel::updateDataInStorage(const QModelIndex &index, int storageComplexIndex)
{
    auto idPrim = cmmn::safeQVariantToIdType( QSqlRelationalTableModel::data( this->index(index.row(), 0), Qt::DisplayRole ) ); // primary key id
    auto idFor = cmmn::safeQVariantToIdType( QSqlRelationalTableModel::data( index, Qt::DisplayRole ) ); // foreign key id
    m_dataGenerator->setQueryGenerator( new QueryGenForeignOneId(idFor, idPrim) ); // TODO: maybe create the Abstract Factory or other construction for prevent creating QueryGenForeignOneId if it is already setted
    m_dataGenerator->generate( dbi::fieldByNameIndex(tableName(), index.column()) );
    if (m_dataGenerator->hasNextResultData()) {
        const auto &resData = m_dataGenerator->nextResultData();
        m_genDataStorage->updateData(resData.idPrim, storageComplexIndex, resData.genData);
    }
    else {
        // TODO: generate error message
        qCritical() << "!--Error. Cannot set a data to the model storage. Data wasn't generated.\n"
                    "Please check the query for generating data for the item: [" << index.row() << "," << index.column() << "]";
        return;
    }
    if (m_dataGenerator->hasNextResultData()) {
        // TODO: generate error message
        qCritical() << "!--Error. Cannot set a data to the model storage. Too many data was generated.\n"
                    "Please check the query for generating data for the item: [" << index.row() << "," << index.column() << "]";
        return;
    }
}

void CustomSqlTableModel::flush()
{
    m_genDataStorage->clear();
}

/* Spike #1. Save data (EditRole) of the foreign keys, that related with complex DBT's in the current row without index currIndex */
void CustomSqlTableModel::saveData_spike1(const QModelIndex &currIndex)
{
    while (m_genDataStorage->hasNextFieldIndex()) {
        const auto &fieldIndex = m_genDataStorage->nextFieldIndex();
        if (fieldIndex != currIndex.column())
            m_saveRestore.insert( fieldIndex, this->data( index(currIndex.row(), fieldIndex), Qt::UserRole ) );
    }
    m_genDataStorage->flushFieldIndex(); // restore the field index for the nexts its gettings
}

/* Restore saved data after calling the QSqlRelationalTableModel::setData() method. Spike #1 */
void CustomSqlTableModel::restoreData_spike1(int currRow)
{
    for (auto it = m_saveRestore.cbegin(); it != m_saveRestore.cend(); ++it) {
//        qDebug() << "data restoring. col =" << it.key() << ", value =" << it.value().toString();
        QSqlRelationalTableModel::setData( index(currRow, it.key()), it.value(), Qt::EditRole );
    }
    m_saveRestore.clear();
    m_bNeedSave_spike1 = false;
}

void CustomSqlTableModel::printData(int role) const
{
    QString strRelatTableModel;
    for(int row = 0; row < rowCount(); ++row) {
        for(int col = 0; col < columnCount(); ++col ) {
            QModelIndex index = QSqlRelationalTableModel::index(row, col);
            strRelatTableModel += (QSqlRelationalTableModel::data(index, role).toString() + "  ");
        }
        strRelatTableModel += "\n";
    }
    qDebug() << "Relational table model data with role #" << role << ":\n" << strRelatTableModel;
}

QString CustomSqlTableModel::printRecords() const
{
    QSqlRecord r;
    QString strPrint;
    for (int row = 0; row < rowCount(); ++row) {
        r = record(row);
        for (int col = 0; col < r.count(); ++col)
            strPrint += (r.value(col).toString() + "  ");
        strPrint += "\n";
    }
    return strPrint;
}

void CustomSqlTableModel::slotRefreshTheModel()
{
    try {
        flush(); // delete previous results.
        // TODO: maybe need to clear data in the model at here? For example, if there was added a new record (row) in a the DBT.
        select();
        fillTheStorage();
        m_genDataStorage->flushFieldIndex();
        printData(Qt::EditRole); // TODO: delete later
    }
    catch (const cmmn::MessageException &me) {
        // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
        QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
        qCritical() << "[Refresh model custom ERROR] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
        return;
    }
    catch (const std::exception &ex) {
        // TODO: generate a message box with critical error
        qCritical() << "[Refresh model standard ERROR] Message: " << ex.what();
        return;
    }
}

void CustomSqlTableModel::slotInsertToTheModel()
{
    try {
        insertNewRecord();
        printData(Qt::EditRole);
        emit sigNewRecordInserted(rowCount() - 1);
    }
    catch (const cmmn::MessageException &me) {
        // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
        QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
        qCritical() << "[Insert data custom ERROR] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
        return;
    }
    catch (const std::exception &ex) {
        // TODO: generate a message box with critical error
        qCritical() << "[Insert data standard ERROR] Message: " << ex.what();
        return;
    }
}

#include <QMessageBox>
void CustomSqlTableModel::slotDeleteFromTheModel(int row)
{
    StorageGenData::T_id idPrimary = cmmn::safeQVariantToIdType( QSqlRelationalTableModel::data( index(row, 0) ) );
    if (!removeRow(row)) {
        // TODO: generate the critical error
        qCritical() << "Cannot delete the record from the data model of the DB table" << tableName()
                    << "with the primary key id =" << idPrimary << "(row #" << row << ")";
    }
    qDebug() << "The record successfully deleted. DBT:" << tableName() << ", primary id =" << idPrimary << ", row =" << row;
    if (!m_genDataStorage->deleteData(idPrimary)) {
        // TODO: generate the critical error
        qCritical() << "Cannot delete the record from the custom data storage of the DB table" << tableName()
                    << "with the primary key id =" << idPrimary << "(row #" << row << ")";
    }
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

//    qDebug() << "delegate, [" << index.row() << "," << index.column() << "]";

    QSqlTableModel *sqlTable = qobject_cast<QSqlTableModel *>(model);
    if (!sqlTable) return;
    const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(sqlTable->tableName(), index.column());
    bool isForeign = fieldInf.isForeign();
    if (!isForeign)
        QItemDelegate::setModelData(editor, model, index);
    else if ( isForeign && dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_simple) )
        setDataToSimpleDBT(editor, model, index);
}

void CustomSqlRelationalDelegate::setDataToSimpleDBT(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    /*
     * Set data to the QSqlRelationalTableModel if the current field has relation with a simple DB table
     * (with help of the QSqlRelation class instance and used a combo box for choosing records of a simple DB table).
     * This code was taked from the QSqlRelationalDelegate::setModelData() method.
     */
//    qDebug() << "set data to simple DBT - 1, [" << index.row() << "," << index.column() << "]";
    QSqlRelationalTableModel *sqlModel = qobject_cast<QSqlRelationalTableModel *>(model);
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (sqlModel && childModel && combo) {
//        qDebug() << "set data to simple DBT - 2, [" << index.row() << "," << index.column() << "]";
        int currentItem = combo->currentIndex();
        int childColIndex = childModel->fieldIndex( sqlModel->relation(index.column()).displayColumn() );
        int childEditIndex = childModel->fieldIndex( sqlModel->relation(index.column()).indexColumn() );
        sqlModel->setData(index,
                          childModel->data(childModel->index(currentItem, childColIndex), Qt::DisplayRole),
                          Qt::DisplayRole);
        sqlModel->setData(index,
                          childModel->data(childModel->index(currentItem, childEditIndex), Qt::EditRole),
                          Qt::EditRole);
    }
}

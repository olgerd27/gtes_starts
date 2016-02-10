#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QDebug> // TODO: delete later
#include <QMessageBox> // TODO: delete later
#include "custom_sql_table_model.h"
#include "generator_dbt_data.h"
#include "storage_gen_data.h"
#include "../common/db_info.h"

/*
 * Converter the model-database columns numbers (fields indexes), that can be used after inserting a new column in the model.
 */
class ConverterMDBIdx
{
public:
    enum Direction
    {
          ModelToDB
        , DBToModel
    };

    static ConverterMDBIdx &instance()
    {
        static ConverterMDBIdx theInstance;
        return theInstance;
    }

    void setInsertedColumn(int insertedColumn);
    int convColumn(int column, ConverterMDBIdx::Direction direction) const;
    QModelIndex convModelIndex(const QModelIndex &modelIndex, ConverterMDBIdx::Direction direction) const;

private:
    enum { NOT_SETTED = -1 };

    ConverterMDBIdx();
    ConverterMDBIdx(const ConverterMDBIdx &) = delete;
    ConverterMDBIdx & operator=(const ConverterMDBIdx &) = delete;

    int m_insertedColumn; // number of the inserted column
};

#define ConvMDBI ConverterMDBIdx::instance()

ConverterMDBIdx::ConverterMDBIdx()
    : m_insertedColumn(NOT_SETTED)
{ }

void ConverterMDBIdx::setInsertedColumn(int insertedColumn)
{
    m_insertedColumn = insertedColumn;
}

int ConverterMDBIdx::convColumn(int column, ConverterMDBIdx::Direction direction) const
{
    ASSERT_DBG( m_insertedColumn != NOT_SETTED,
                cmmn::MessageException::type_critical, QObject::tr("Error indexes conversion"),
                QObject::tr("Don't setted the inserted column number"),
                QString("ConverterMDBIdx::convColumn()"))
    return column >= m_insertedColumn ? (direction == ModelToDB ? --column : ++column) : column;
}

QModelIndex ConverterMDBIdx::convModelIndex(const QModelIndex &modelIndex,
                                            ConverterMDBIdx::Direction direction) const
{
    return modelIndex.model()->index( modelIndex.row(), convColumn(modelIndex.column(), direction) );
}

/*
 * CustomSqlTableModel
 *
 * Description of the spike #1.
 * When user set a new data in the foreign key field, the others foreign fields in the current row assigns generated string-type values,
 * but here must remain the int-type id values. This not expected behaviour performs in the QSqlRelationalTableModel::setData()
 * (or QSqlTableModel::setData()) method, which work process is private from outside. Most probably, this string-type generated values was readed
 * from the mapper widgets.
 * To prevent this invalid behaviour, it was added the spike #1. The Spike #1 performs data saving from the foreign key fields, before calling the
 * QSqlRelationalTableModel::setData() and restore saved data after calling the QSqlRelationalTableModel::setData(). It allows remains the int-type id values
 * in the foreign keys fields.
 * Turn on the Spike #1 performs by calling the public method spike1_turnOn(bool).
 * Turn off the Spike #1 performs automatically (without assistance from outside) in the restoreData_spike1() method.
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
    , m_selectedRow(NOT_SETTED)
    , m_selectIcon(":/images/ok.png")
    , m_spike1_bNeedSave(false) /* Spike #1 */
{
    ConvMDBI.setInsertedColumn(SELECT_ICON_COLUMN);
}

CustomSqlTableModel::~CustomSqlTableModel()
{ }

void CustomSqlTableModel::spike1_turnOn(bool bOn)
{
    m_spike1_bNeedSave = bOn; /* Spike #1 */
}

void CustomSqlTableModel::setTable(const QString &tableName)
{
    QSqlRelationalTableModel::setTable(tableName);
    setEditStrategy(QSqlTableModel::OnManualSubmit);
    setSort(0, Qt::AscendingOrder);
    try {
        if (m_genDataStorage->isEmpty()) {
            defineForeignFields();
            slotRefreshTheModel();
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

    // Convert the QModelIndex from the model to the QModelIndex that appropriate to the DB
    const QModelIndex &dbIdx = ConvMDBI.convModelIndex(idx, ConverterMDBIdx::ModelToDB);

    int storageDataIndex = -1;
    if ( (role == Qt::EditRole || role == Qt::DisplayRole) && m_genDataStorage->isForeignField(dbIdx.column(), storageDataIndex) ) {
        try {
            data = getDataFromStorage(dbIdx, storageDataIndex);
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
//        qDebug() << "data() UserRole, [" << item.row() << "," << item.column() << "], role:" << role << ", data:" << data.toString();
    }
    else if (role == Qt::DecorationRole && idx.column() == SELECT_ICON_COLUMN && idx.row() == m_selectedRow) {
        data = m_selectIcon;
//        qDebug() << "data(), decoration role, [" << idx.row() << "," << idx.column() << "], selected row =" << m_selectedRow;
    }
    else if (role == Qt::TextAlignmentRole) {
        if ( this->data(idx, Qt::DisplayRole).convert(QMetaType::Float) )
            data = Qt::AlignCenter; // center alignment of the numerical values
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
    if (!idx.isValid()) return false;
    // Convert the QModelIndex from the model to the QModelIndex that appropriate to the DB
    const QModelIndex &dbIdx = ConvMDBI.convModelIndex(idx, ConverterMDBIdx::ModelToDB);

    bool bSetted = false;
    int storageDataIndex = -1;
//    if (m_spike1_bNeedSave) spike1_saveData(idx, value); // TODO: use only this??
    if (role == Qt::EditRole && value == NOT_SETTED) {
        // fill the model's new row by the empty values. The storages new rows filling performs earlier by calling the CustomSqlTableModel::slotInsertToTheModel()
        bSetted = QSqlRelationalTableModel::setData(idx, QVariant(), role);
        qDebug() << "setData(), NOT_SETTED: [" << idx.row() << "," << idx.column() << "],"
                 << "role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
    }
    else if (role == Qt::EditRole && m_genDataStorage->isForeignField(dbIdx.column(), storageDataIndex) ) {
        if (m_spike1_bNeedSave) spike1_saveData(idx, value); // Spike #1 - TODO: delete?

        qDebug() << "setData(), before QSqlRelationalTableModel::setData()";
        printDataDB(Qt::DisplayRole);

        bSetted = QSqlRelationalTableModel::setData(idx, value, role);

        qDebug() << "After QSqlRelationalTableModel::setData()";
        qDebug() << "[" << idx.row() << "," << idx.column() << "]," << "role:" << role
                 << ", set data:" << value.toString() << ", bSetted:" << bSetted;
        printDataDB(Qt::DisplayRole);

        if (m_spike1_bNeedSave) spike1_restoreData(idx); // Spike #1 - TODO: delete?
        qDebug() << "setData(), after spike1_restoreData();";
        printDataDB(Qt::DisplayRole);

        try {
            updateDataInStorage(idx, storageDataIndex);

            qDebug() << "setData(), after updateDataInStorage()";
            printDataDB(Qt::DisplayRole);
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
    else if (role == Qt::DecorationRole && idx.column() == SELECT_ICON_COLUMN) {
        m_selectedRow = idx.row();
        qDebug() << "setData(), decoration role, [" << idx.row() << "," << idx.column() << "], selected row =" << m_selectedRow;
    }
    else {
        bSetted = QSqlRelationalTableModel::setData(idx, value, role);
        qDebug() << "setData(), else: [" << idx.row() << "," << idx.column() << "],"
                 << "role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
    }
//    if (m_spike1_bNeedSave) spike1_restoreData(idx); // TODO: use only this???
//    qDebug() << "setData(), [" << item.row() << "," << item.column() << "], role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
    return bSetted;
}

QVariant CustomSqlTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
//    QVariant data;
//    if (orientation == Qt::Horizontal) {
//        data = section > SELECT_ICON_COLUMN
//               ? QSqlRelationalTableModel::headerData( RegCI.convColNumber(section, RegisterCI::cd_customToBase), orientation, role )
//               : QVariant();
//        if (role == Qt::DisplayRole) {
//            qDebug() << "headerData(), section =" << section << ", orientation =" << orientation << ", role =" << role << ", data:" << data;
//            printHeader(Qt::DisplayRole);
//        }
//    }
//    return data;
    return QSqlRelationalTableModel::headerData(section, orientation, role);
}

int CustomSqlTableModel::columnCount(const QModelIndex &parent) const
{
//    return QSqlRelationalTableModel::columnCount(parent) + 1;
    return QSqlRelationalTableModel::columnCount(parent);
}

Qt::ItemFlags CustomSqlTableModel::flags(const QModelIndex &index) const
{
//    return index.column() == columnCount(index.parent()) - 1
//            ? QSqlRelationalTableModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable
//            : QSqlRelationalTableModel::flags(index);
    return QSqlRelationalTableModel::flags(index);
}

QVariant CustomSqlTableModel::primaryIdInRow(int row) const
{
    int colPrimId = ConvMDBI.convColumn(0, ConverterMDBIdx::DBToModel); // define the primary id's column
    return QSqlRelationalTableModel::data(index(row, colPrimId), Qt::DisplayRole);
}

bool CustomSqlTableModel::findPrimaryIdRow(const QVariant &idPrim, int &rRowValue) const
{
    for (int row = 0; row < rowCount(); ++row) {
        if (primaryIdInRow(row) == idPrim) {
            rRowValue = row;
            return true;
        }
    }
    return false;
}

cmmn::T_id CustomSqlTableModel::selectedId() const
{
    int colPrimId = ConvMDBI.convColumn(0, ConverterMDBIdx::DBToModel); // define the primary id's column
    cmmn::T_id id;
    const QVariant &varId = this->data(index(m_selectedRow, colPrimId), Qt::UserRole);
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, id), varId );
    return id;
}

// Save the foreign keys fields (columns numbers)
void CustomSqlTableModel::defineForeignFields()
{
    // Check - is this a first calling
    if (!m_genDataStorage->isNotSetted()) {
        // TODO: generate warning about: trying to initialize already initialized model
        qWarning() << "trying to initialize already initialized model";
        return;
    }
    // Processing
    for (int col = 0; col < columnCount(); ++col) {
        const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex( tableName(), col);
        if (fieldInf.isForeign()) {
            m_genDataStorage->addFieldIndex(col);
        }
    }
}

void CustomSqlTableModel::setHeader()
{
    for (int col = 0; col < columnCount(); ++col) {
        if (col == SELECT_ICON_COLUMN) {
            setHeaderData(col, Qt::Horizontal, "", Qt::EditRole);
            continue;
        }
        const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex( tableName(), ConvMDBI.convColumn(col, ConverterMDBIdx::ModelToDB) );
        setHeaderData(col, Qt::Horizontal, fieldInf.m_nameInUI, Qt::EditRole);
        // do not set the relations
//        if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_simple)) {
//            dbi::DBTInfo *relTableInf = dbi::relatedDBT(fieldInf);
//            setRelation( col,
//                         QSqlRelation( relTableInf->m_nameInDB, relTableInf->m_fields.at(0).m_nameInDB, relTableInf->m_fields.at(1).m_nameInDB ) );
//        }
    }
}

void CustomSqlTableModel::fillTheStorage()
{
    QueryGenPrimaryAllId *qgen = new QueryGenPrimaryAllId(tableName());
    m_dataGenerator->setQueryGenerator(qgen);
    qDebug() << "fill the storage";
    QString str;
    while ( m_genDataStorage->hasNextFieldIndex() ) {
        auto foreignFieldIndex = m_genDataStorage->nextFieldIndex();
        const dbi::DBTFieldInfo &fieldInfo = dbi::fieldByNameIndex(tableName(), foreignFieldIndex);
        qgen->setForeignFieldName(fieldInfo.m_nameInDB);
        m_dataGenerator->generate(fieldInfo);
        str += QString("field %1: %2, data:\n").arg(foreignFieldIndex).arg(fieldInfo.m_nameInDB);
        while (m_dataGenerator->hasNextResultData()) {
            const auto &resData = m_dataGenerator->nextResultData();
            str += QString("  id = %1, value = %2\n").arg(resData.idPrim).arg(resData.genData);
            m_genDataStorage->addData(resData.idPrim, resData.genData);
        }
//        emit dataChanged( index(0, foreignFieldIndex), index(rowCount() - 1, foreignFieldIndex) ); // NOTE: Is this need or not?
    }
    qDebug() << str;
}

cmmn::T_id CustomSqlTableModel::insertNewRecord()
{
    // version 1
//    int Nrows = rowCount();
//    QSqlRecord rec = record(Nrows - 1); // get the last existent record

//    qDebug() << "Record count:" << rec.count();
//    qDebug() << "The last existent record:";
//    for (int i = 0; i < rec.count(); ++i)
//        qDebug() << i << ":" << rec.value(i).toString();

//    /*
//     * define the new primary key id value
//     * TODO: maybe create strategies, that performs different definition of a new id (at now is only: new_id = (last_id + 1) definition).
//     */
//    int modelIdColumn = ConvMDBI.convColumn(0, ConverterMDBIdx::DBToModel); // the id's column number in the model
//    const QVariant &varId = rec.value(modelIdColumn);
//    cmmn::T_id newIdPrim;
//    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, newIdPrim), varId );
//    ++newIdPrim; // increment value for obtaining the new primary id

//    rec.setValue(modelIdColumn, newIdPrim); // set a new primary key id value
//    for (int i = modelIdColumn + 1; i < rec.count(); ++i) { /* start loop from the next column number, that follow after id */
////        int dbColNumb = ConvMDBI.convColumn(i, ConverterMDBIdx::ModelToDB); // the column number in the database
////        rec.setValue( i, dbi::isRelatedWithDBTType(tableName(), dbColNumb, dbi::DBTInfo::ttype_simple)
////                      ? 1 : NOT_SETTED ); // if this field related with a simple DB table -> set the 1-th value
////        if ( dbi::fieldByNameIndex(tableName(), dbColNumb).isForeign() )
//            rec.setValue(i, NOT_SETTED);
//        m_genDataStorage->addData(newIdPrim); /* fill the storages new row by the empty values. The model filling of this row performs
//                                                 in the setData() method, that calls by the insertRecord() method. */
//    }
//    qDebug() << "The record that will be inserted:";
//    for (int i = 0; i < rec.count(); ++i)
//        qDebug() << i << ":" << rec.value(i).toString();

//    ASSERT_DBG( insertRecord(Nrows, rec),
//                cmmn::MessageException::type_critical, QObject::tr("Insertion error"),
//                QObject::tr("The new record cannot be inserted"),
//                QString("CustomSqlTableModel::insertNewRecord()") )
//    return newIdPrim;

    // version 2
    int Nrows = rowCount();
    // TODO: maybe create strategies, that performs different definition of a new id (at now is only: new_id = (last_id + 1) definition).
    const QVariant &varId = primaryIdInRow(Nrows - 1);
    cmmn::T_id newIdPrim;
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, newIdPrim), varId );
    ++newIdPrim; // increment value for obtaining the new primary id

    ASSERT_DBG( insertRow(Nrows),
                cmmn::MessageException::type_critical, QObject::tr("Insertion error"),
                QObject::tr("The new row cannot be inserted"),
                QString("CustomSqlTableModel::insertNewRecord()") );

    int modelIdColumn = ConvMDBI.convColumn(0, ConverterMDBIdx::DBToModel); // the id's column number in the model
    for (int col = modelIdColumn; col < columnCount(); ++col) {
        int dbColNumb = ConvMDBI.convColumn(col, ConverterMDBIdx::ModelToDB); // the column number in the database
        if ( dbi::fieldByNameIndex(tableName(), dbColNumb).isForeign() )
            m_genDataStorage->addData(newIdPrim); // init the data storage -> must be before the model::setData() calling
        setData(this->index(Nrows, col), (col == modelIdColumn ? newIdPrim : (int)NOT_SETTED), Qt::EditRole);
    }
    return newIdPrim;
}

QVariant CustomSqlTableModel::getDataFromStorage(const QModelIndex &baseIndex, int storageComplexIndex) const
{

    QVariant resData;
    cmmn::T_id idPrim; // primary id
    const QVariant &varId = QSqlRelationalTableModel::data( this->index(baseIndex.row(), SELECT_ICON_COLUMN + 1), Qt::DisplayRole );
    qDebug() << "getDataFromStorage() 1, [" << baseIndex.row() << "," << (SELECT_ICON_COLUMN + 1) << "], varId:" << varId << ", data:";
    printDataDB(Qt::DisplayRole);
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, idPrim), varId );

    if (m_genDataStorage->isStorageContainsId(idPrim)) {
        resData = m_genDataStorage->data(idPrim, storageComplexIndex);
    }
    return resData;
}

/* generate a data and update the storage */
void CustomSqlTableModel::updateDataInStorage(const QModelIndex &index, int storageComplexIndex)
{
    qDebug() << "updateDataInStorage() 1, [" << index.row() << "," << index.column() << "]";
    printDataDB(Qt::DisplayRole);
    // getting the primary and foreign id's values
    const QVariant &varIdPrim = QSqlRelationalTableModel::data( this->index(index.row(), SELECT_ICON_COLUMN + 1), Qt::DisplayRole );
    const QVariant &varIdFor = QSqlRelationalTableModel::data( index, Qt::DisplayRole );

    qDebug() << "updateDataInStorage() 2, varIdPrim:" << varIdPrim << ", idFor:" << varIdFor;

    cmmn::T_id idPrim, idFor; // primary and foreign keys values
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varIdPrim, idPrim), varIdPrim );
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varIdFor, idFor), varIdFor );

    qDebug() << "updateDataInStorage() 3, idPrim:" << varIdPrim << "=" << idPrim << ", idFor:" << varIdFor << "=" << idFor;

    // TODO: maybe create the Abstract Factory or other construction for prevent creating QueryGenForeignOneId if it is already setted
    m_dataGenerator->setQueryGenerator( new QueryGenForeignOneId(idFor, idPrim) );

    qDebug() << "updateDataInStorage() 4, before generate()";
    m_dataGenerator->generate( dbi::fieldByNameIndex(tableName(), ConvMDBI.convColumn(index.column(), ConverterMDBIdx::ModelToDB) ) );
    qDebug() << "updateDataInStorage() 5, after generate()";
    int ii = 0;
    if (m_dataGenerator->hasNextResultData()) {
        qDebug() << "updateDataInStorage() 6" << ii << ", hasNextResultData";
        const auto &resData = m_dataGenerator->nextResultData();
        qDebug() << "updateDataInStorage() 7" << ii << ", hasNextResultData, getted data, id =" << resData.idPrim
                 << ", complex index =" << storageComplexIndex << ", data =" << resData.genData;
        m_genDataStorage->updateData(resData.idPrim, storageComplexIndex, resData.genData);
        ++ii;
    }
    else {
        // TODO: generate error message
        qCritical() << "[CRITICAL ERROR] Cannot set a data to the model storage. Data wasn't generated.\n"
                    "Please check the query for generating data for the item: [" << index.row() << "," << index.column() << "]";
        return;
    }
    if (m_dataGenerator->hasNextResultData()) {
        // TODO: generate error message
        qCritical() << "[CRITICAL ERROR] Cannot set a data to the model storage. Too many data was generated.\n"
                    "Please check the query for generating data for the item: [" << index.row() << "," << index.column() << "]";
        return;
    }
}

void CustomSqlTableModel::flush()
{
    m_genDataStorage->clear();
}

/* Spike #1. Save data (EditRole) of the foreign keys in the current row and the current item data */
void CustomSqlTableModel::spike1_saveData(const QModelIndex &modelIndex, const QVariant &setData)
{
    while (m_genDataStorage->hasNextFieldIndex()) {
        const auto &columnModelIndex = ConvMDBI.convColumn( m_genDataStorage->nextFieldIndex(), ConverterMDBIdx::DBToModel );
        // TODO: refactoring this code -> use ?:
        if (columnModelIndex == modelIndex.column()) {
            m_spike1_saveRestore.insert( columnModelIndex, setData);
        }
        else {
            m_spike1_saveRestore.insert( columnModelIndex,
                                         QSqlRelationalTableModel::data( this->index(modelIndex.row(), columnModelIndex), Qt::DisplayRole ) );
        }
    }
    m_genDataStorage->flushFieldIndex(); // restore the field index for the nexts data gettings
    qDebug() << "Spike #1, saved data:";
    for (auto it = m_spike1_saveRestore.cbegin(); it != m_spike1_saveRestore.cend(); ++it)
        qDebug() << it.key() << ":" << it.value();
}

/* Restore saved data after calling the QSqlRelationalTableModel::setData() method. Spike #1 */
void CustomSqlTableModel::spike1_restoreData(const QModelIndex &modelIndex)
{
    qDebug() << "spike1_restoreData() 1";
    int ii = 0;
    for (auto it = m_spike1_saveRestore.cbegin(); it != m_spike1_saveRestore.cend(); ++it) {
        qDebug() << "spike1_restoreData() 2" << ii << ", [" << modelIndex.row() << "," << it.key() << "], replace:"
                 << QSqlRelationalTableModel::data( this->index(modelIndex.row(), it.key()), Qt::DisplayRole ).toString()
                 << "to the:" << it.value().toString();
        QSqlRelationalTableModel::setData( this->index(modelIndex.row(), it.key()), it.value(), Qt::EditRole );
        qDebug() << "spike1_restoreData() 3" << ii;
        ++ii;
    }
    qDebug() << "spike1_restoreData() 4";
    m_spike1_saveRestore.clear();
    qDebug() << "spike1_restoreData() 5";
    m_spike1_bNeedSave = false;
}

void CustomSqlTableModel::printDataDB(int role) const
{
    QString strData = "\n";
    for(int row = 0; row < QSqlRelationalTableModel::rowCount(); ++row) {
        for(int col = 0; col < QSqlRelationalTableModel::columnCount(); ++col ) {
            const QModelIndex &index = QSqlRelationalTableModel::index(row, col);
            strData += (QSqlRelationalTableModel::data(index, role).toString() + "  ");
        }
        strData += "\n";
    }
    qDebug() << "The base model data with role #" << role << ":" << strData;
}

void CustomSqlTableModel::printDataModel(int role) const
{
    QString strData = "\n";
    for(int row = 0; row < rowCount(); ++row) {
        for(int col = 0; col < columnCount(); ++col ) {
            const QModelIndex &index = this->index(row, col);
            strData += (data(index, role).toString() + "  ");
        }
        strData += "\n";
    }
    qDebug() << "The custom model data with role #" << role << ":" << strData;
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

void CustomSqlTableModel::printHeader(int role) const
{
    QString str = "\n";
    for (int sect = 0; sect < QSqlRelationalTableModel::columnCount(); ++sect) {
        str += QString("|") += QSqlRelationalTableModel::headerData(sect, Qt::Horizontal, role).toString() += QString("| ");
    }
    qDebug() << "horizontal header data, role =" << role << ", data:" << str;
}

void CustomSqlTableModel::slotRefreshTheModel()
{
    try {
        flush(); // delete previous results

        qDebug() << "before slotRefreshTheModel()::select(), column count =" << columnCount();
        printHeader(Qt::DisplayRole);
        printDataDB(Qt::DisplayRole);

        if (!select()) { // populate the model
            // TODO: generate the error
            qCritical() << "[CRITICAL ERROR] Cannot refresh the model - error data populating.\nThe DB error text: " << lastError().text();
        }

        qDebug() << "after slotRefreshTheModel()::select(), column count =" << columnCount();
        printHeader(Qt::DisplayRole);
        printDataDB(Qt::DisplayRole);

        insertColumn(SELECT_ICON_COLUMN); // must calls only after calling the select() method
        setHeader(); // must calls only after inserting a new column

        qDebug() << "after slotRefreshTheModel()::insertColumn() and setHeader, column count =" << columnCount();
        printHeader(Qt::DisplayRole);
        printDataDB(Qt::DisplayRole);

        m_genDataStorage->flushFieldIndex(); // reset field index
        fillTheStorage(); // populate the storage
        m_genDataStorage->flushFieldIndex(); // reset field index
//        printDataDB(Qt::EditRole); // TODO: delete later

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

void CustomSqlTableModel::slotInsertToTheModel()
{
    try {
        auto newId = insertNewRecord();
        emit sigNewRecordInserted(rowCount() - 1, newId);
    }
    catch (const cmmn::MessageException &me) {
        // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
        QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
        qCritical() << "[CRITICAL ERROR] [Insert data custom ERROR] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
        return;
    }
    catch (const std::exception &ex) {
        // TODO: generate a message box with critical error
        qCritical() << "[CRITICAL ERROR] [Insert data standard ERROR] Message: " << ex.what();
        return;
    }
}

void CustomSqlTableModel::slotDeleteFromTheModel(int row)
{
    // getting the primary id value
    cmmn::T_id idPrimary;
    const QVariant &varId = primaryIdInRow(row);
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, idPrimary), varId );

    if (!removeRow(row)) {
        // TODO: generate the critical error
        qCritical() << "[CRITICAL ERROR] Cannot delete the record from the data model of the DB table" << tableName()
                    << "with the primary key id =" << idPrimary << "(row #" << row << ")";
        return;
    }
    if (!m_genDataStorage->deleteData(idPrimary)) {
        // TODO: generate the critical error
        qCritical() << "[CRITICAL ERROR] Cannot delete the record from the custom data storage of the DB table" << tableName()
                    << "with the primary key id =" << idPrimary << "(row #" << row << ")";
        return;
    }
    qDebug() << "The record successfully deleted. DBT:" << tableName() << ", primary id =" << idPrimary << ", row =" << row;
//    qDebug() << printRecords();
    emit sigRecordDeleted(row, idPrimary);
}

void CustomSqlTableModel::slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected)
{
    QItemSelectionModel *selectModel = qobject_cast<QItemSelectionModel *>(sender());
    QModelIndexList deselectedList = deselected.indexes();

    // catch a deselection of the first left item in current row and setting icons decoration on it
    if (deselectedList.size() == 1) {
        setData(deselectedList.first(), QVariant(), Qt::DecorationRole);
        return;
    }

    // update the first left items in the previous selected row for clearing icons decoration
    if (deselectedList.size() > 1) {
        QModelIndex someDeselected = deselectedList.first();
        QModelIndex firstDeselected = someDeselected.model()->index(someDeselected.row(), SELECT_ICON_COLUMN);
        emit sigNeedUpdateView(firstDeselected); // clear remained icons decoration
    }

    QModelIndex firstSelected = selected.indexes().first();
    selectModel->select(firstSelected, QItemSelectionModel::Deselect); // this make recursive calling of this slot
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
    if (!index.isValid()) return;

    /* Calling of base class method QSqlRelationalDelegate::setModelData() cause the error - the data for all roles of the fields,
     * that is foreign, sets to generated string (copy from the mapped widgets) and there are no ability to get the foreign key values for this fields.
     */
//    QSqlRelationalDelegate::setModelData(editor, model, index);

    qDebug() << "delegate, [" << index.row() << "," << index.column() << "], data:" << model->data(index);
    CustomSqlTableModel *sqlModel = qobject_cast<CustomSqlTableModel *>(model);
    if (!sqlModel) return;

    // convert the model index of the custom model to the model index of the base model
    const QModelIndex &baseIdx = ConvMDBI.convModelIndex(index, ConverterMDBIdx::ModelToDB);

    const auto &fieldInf = dbi::fieldByNameIndex(sqlModel->tableName(), baseIdx.column());

//    // set data for all field, except the fields that is foreign keys
    if ( !fieldInf.isForeign() /*|| dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_simple)*/ )
        QSqlRelationalDelegate::setModelData(editor, model, index);

    // old version - not use
//    bool isForeign = fieldInf.isForeign();
//    if (!isForeign)
//        QSqlRelationalDelegate::setModelData(editor, model, baseIdx);
//    else if ( isForeign && dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_simple) )
//        setDataToSimpleDBT(editor, model, baseIdx);
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

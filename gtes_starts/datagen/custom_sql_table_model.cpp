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
 * Register column insert.
 * Class that helps define model index, when there was inserted a some one column that do not exists in the DB.
 */
class RegisterCI
{
public:
    enum ConversionDirection
    {
        cd_baseToCustom,
        cd_customToBase
    };

    static RegisterCI &instance()
    {
        static RegisterCI theInstance;
        return theInstance;
    }

    void setInsertedColumn(int insertedColumn);
    int convColNumber(int column, RegisterCI::ConversionDirection direction) const;
    QModelIndex convModelIndex(const QModelIndex &customIndex,
                               const QSqlRelationalTableModel * const model,
                               RegisterCI::ConversionDirection direction) const;

private:
    enum { NOT_SETTED = -1 };

    RegisterCI();
    RegisterCI(const RegisterCI &) = delete;
    RegisterCI & operator=(const RegisterCI &) = delete;

    int m_insertedColumn; // number of the inserted column
};

#define RegCI RegisterCI::instance()
#define CHECK_SETTING(Func) \
    if (m_insertedColumn == NOT_SETTED) \
        throw std::runtime_error( std::string("The inserted column number is unknown [") + Func + "]" );

RegisterCI::RegisterCI()
    : m_insertedColumn(NOT_SETTED)
{ }

void RegisterCI::setInsertedColumn(int insertedColumn)
{
    m_insertedColumn = insertedColumn;
}

int RegisterCI::convColNumber(int column, RegisterCI::ConversionDirection direction) const
{
    CHECK_SETTING("RegisterCI::convColNumber()")
    return column >= m_insertedColumn ? (direction == cd_baseToCustom ? ++column : --column) : column;
}

QModelIndex RegisterCI::convModelIndex(const QModelIndex &customIndex,
                                       const QSqlRelationalTableModel * const model,
                                       RegisterCI::ConversionDirection direction) const
{
    return model->index( customIndex.row(), convColNumber(customIndex.column(), direction) );
}

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
    , m_selectedRow(NOT_SETTED)
    , m_selectIcon(":/images/ok.png")
    , m_spike1_bNeedSave(false) /* Spike #1 */
{
    RegCI.setInsertedColumn(SELECT_ICON_COLUMN);
    // Test the CHECK_ERROR_CONVERT_ID macro
//    cmmn::T_id id;
//    QVariant vId("tt");
//    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(vId, id), vId );
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
            defineSimpleDBTAndComplexIndex();
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

    // convert the model index of the custom model (CustomSqlTableModel) to the model index of the base model (QSqlRelationalTableModel)
    const QModelIndex &baseIdx = RegCI.convModelIndex(idx, this, RegisterCI::cd_customToBase);

    int storageDataIndex = -1;
    if ( (role == Qt::EditRole || role == Qt::DisplayRole) && m_genDataStorage->isComplexDBTField(idx.column(), storageDataIndex) ) {
        try {
            data = getDataFromStorage(idx, storageDataIndex);
//            qDebug() << "data(), complex field,  [" << idx.row() << "," << idx.column() << "], role =" << role
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
    // convert the model index of the custom model (CustomSqlTableModel) to the model index of the base model (QSqlRelationalTableModel)
    const QModelIndex &baseIdx = RegCI.convModelIndex(idx, this, RegisterCI::cd_customToBase);
    bool bSetted = false;
    int storageComplexIndex = -1;
//    if (m_spike1_bNeedSave) spike1_saveData(baseIdx); // TODO: use only this??
    if (role == Qt::EditRole && value == NOT_SETTED) {
        // fill the model's new row by the empty values. The storages new rows filling performs earlier by calling the CustomSqlTableModel::slotInsertToTheModel()
        bSetted = QSqlRelationalTableModel::setData(idx, QVariant(), role);
        qDebug() << "setData(), custom: [" << idx.row() << "," << idx.column() << "],"
                 << "base: [" << baseIdx.row() << "," << baseIdx.column() << "],"
                 << "role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
    }
    else if (role == Qt::EditRole && m_genDataStorage->isComplexDBTField(idx.column(), storageComplexIndex) ) {
        if (m_spike1_bNeedSave) spike1_saveData(idx); // Spike #1 - TODO: delete?

        qDebug() << "Before QSqlRelationalTableModel::setData()";
        printDataBase(Qt::DisplayRole);
//        QMessageBox::information(0, "", "Before QSqlRelationalTableModel::setData()");

        bSetted = QSqlRelationalTableModel::setData(idx, value, role);
//        bSetted = QSqlRelationalTableModel::setData( index(idx.row(), idx.column()), value, role );

        qDebug() << "setData(), custom: [" << idx.row() << "," << idx.column() << "],"
                 << "base: [" << baseIdx.row() << "," << baseIdx.column() << "],"
                 << "role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;

        qDebug() << "After QSqlRelationalTableModel::setData()";
        printDataBase(Qt::DisplayRole);
//        QMessageBox::information(0, "", "After QSqlRelationalTableModel::setData()");

        if (m_spike1_bNeedSave) spike1_restoreData(idx); // Spike #1 - TODO: delete?
        qDebug() << "After restore data, spike 1";
        printDataBase(Qt::DisplayRole);
//        QMessageBox::information(0, "", "After restore data, spike 1");

        try {
            updateDataInStorage(idx, storageComplexIndex);

            qDebug() << "After updateDataInStorage()";
            printDataBase(Qt::DisplayRole);
//            QMessageBox::information(0, "", "After updateDataInStorage()");
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
//        qDebug() << "setData(), decoration role, [" << idx.row() << "," << idx.column() << "], selected row =" << m_selectedRow;
    }
    else {
        bSetted = QSqlRelationalTableModel::setData(idx, value, role);
        qDebug() << "setData(), else: [" << idx.row() << "," << idx.column() << "],"
                 << "base: [" << baseIdx.row() << "," << baseIdx.column() << "],"
                 << "role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
    }
//    if (m_spike1_bNeedSave) spike1_restoreData(baseIdx); // TODO: use only this???
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
    return valueInBaseModel(row, 0 + 1); // the 1-th column store the primary id's values (the 0-th column store select image)
}

bool CustomSqlTableModel::findPrimaryIdRow(const QVariant &idPrim, int &rRowId) const
{
    return findValueRow(idPrim, 0 + 1, rRowId); // the 1-th column store the primary id's values (the 0-th column store select image)
}

bool CustomSqlTableModel::findValueRow(const QVariant &value, int column, int &rRowValue) const
{
    for (int row = 0; row < rowCount(); ++row) {
        if (valueInBaseModel(row, column) == value) {
            rRowValue = row;
            return true;
        }
    }
    return false;
}

QVariant CustomSqlTableModel::valueInBaseModel(int row, int col) const
{
    return QSqlRelationalTableModel::data(index(row, col), Qt::DisplayRole);
}

cmmn::T_id CustomSqlTableModel::selectedId() const
{
    int colPrimId = RegCI.convColNumber(SELECT_ICON_COLUMN, RegisterCI::cd_baseToCustom); // define the primary id's column
    cmmn::T_id id;
    const QVariant &varId = this->data(index(m_selectedRow, colPrimId), Qt::UserRole);
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, id), varId );
    return id;
}

/* Model initialization: define relations with simple DBT columns number and save complex DBT columns numbers (indexes) */
void CustomSqlTableModel::defineSimpleDBTAndComplexIndex()
{
    // Check - is this a first calling
    if (!m_genDataStorage->isNotSetted()) {
        // TODO: generate warning about: trying to initialize already initialized model
        qWarning() << "trying to initialize already initialized model";
        return;
    }
    // Processing
    qDebug() << "Recognize the simple and complex DBT:";
    for (int col = 0; col < columnCount(); ++col) {
        if (col == SELECT_ICON_COLUMN) continue;
        const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), col - 1);
//        if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_simple)) {
//            dbi::DBTInfo *relTableInf = dbi::relatedDBT(fieldInf);
//            setRelation( col,
//                         QSqlRelation( relTableInf->m_nameInDB, relTableInf->m_fields.at(0).m_nameInDB, relTableInf->m_fields.at(1).m_nameInDB ) );
//        }
        /*else */if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_complex)) {
            m_genDataStorage->addFieldIndex(col); // save index of the fields, that is a foreign key to a some complex database table
        }
    }
}

void CustomSqlTableModel::fillTheStorage()
{
    QueryGenPrimaryAllId *qgen = new QueryGenPrimaryAllId(tableName());
    m_dataGenerator->setQueryGenerator(qgen);
    qDebug() << "fill the storage";
//    QString str;
    while ( m_genDataStorage->hasNextFieldIndex() ) {
        auto complexFIndex = m_genDataStorage->nextFieldIndex() - 1; // need convert the field index
        const dbi::DBTFieldInfo &fieldInfo = dbi::fieldByNameIndex(tableName(), complexFIndex);
        qgen->setForeignFieldName(fieldInfo.m_nameInDB);
        m_dataGenerator->generate(fieldInfo);
//        str += QString("  field: %1, data:\n").arg(fieldInfo.m_nameInDB);
        while (m_dataGenerator->hasNextResultData()) {
            const auto &resData = m_dataGenerator->nextResultData();
//            str += QString("id = %1, value = %2\n").arg(resData.idPrim).arg(resData.genData);
            m_genDataStorage->addData(resData.idPrim, resData.genData);
        }
//        emit dataChanged( index(0, complexFIndex), index(rowCount() - 1, complexFIndex) );
    }
//    qDebug() << str;
}

cmmn::T_id CustomSqlTableModel::insertNewRecord()
{
    int Nrows = rowCount();
    QSqlRecord rec = record(Nrows - 1); // get the last existent record

    qDebug() << "The last existent record:";
    for (int i = 0; i < rec.count(); ++i)
        qDebug() << i << ":" << rec.value(i).toString();

    /*
     * define the new primary key id value
     * TODO: maybe create strategies, that performs different definition of a new id (at now is only: new_id = (last_id + 1) definition).
     */
    int customIdColNumb = RegCI.convColNumber(0, RegisterCI::cd_baseToCustom); // define the number of the id's column in the custom model
    const QVariant &varId = rec.value(customIdColNumb);
    cmmn::T_id newIdPrim;
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, newIdPrim), varId );
    ++newIdPrim; // increment value for obtaining the new primary id

    rec.setValue(customIdColNumb, newIdPrim); // set a new primary key id value
    for (int i = ++customIdColNumb; i < rec.count(); ++i) { /* start loop from the next value after id */
        int baseColNumb = RegCI.convColNumber(i, RegisterCI::cd_customToBase); // column number in the base model and in the DB too
        rec.setValue( i, dbi::isRelatedWithDBTType(tableName(), baseColNumb, dbi::DBTInfo::ttype_simple) ? 1 : NOT_SETTED );
        m_genDataStorage->addData(newIdPrim); /* fill the storages new row by the empty values. The model filling of this row performs
                                                 in the setData() method, that calls by the insertRecord() method. */
    }
    qDebug() << "Inserted record:";
    for (int i = 0; i < rec.count(); ++i)
        qDebug() << i << ":" << rec.value(i).toString();

    if (!insertRecord(Nrows, rec)) {
        // TODO: generate the error
        qCritical() << "[CRITICAL ERROR] The new empty record cannot be inserted to the end of DB table" << tableName() << "model";
    }
    return newIdPrim;
}

QVariant CustomSqlTableModel::getDataFromStorage(const QModelIndex &baseIndex, int storageComplexIndex) const
{
    QVariant resData;
    cmmn::T_id idPrim; // primary id
    const QVariant &varId = QSqlRelationalTableModel::data( this->index(baseIndex.row(), 0 + 1), Qt::DisplayRole );
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, idPrim), varId );
//    qDebug() << "getDataFromStorage(), [" << baseIndex.row() << "," << 1 << "], id =" << idPrim << ", data:";
//    printDataBase(Qt::DisplayRole);

    if (m_genDataStorage->isStorageContainsId(idPrim)) {
        resData = m_genDataStorage->data(idPrim, storageComplexIndex);
    }
    return resData;
}

/* generate a data and update the storage */
void CustomSqlTableModel::updateDataInStorage(const QModelIndex &index, int storageComplexIndex)
{
    // getting the primary and foreign id's values
    const QVariant &varIdPrim = QSqlRelationalTableModel::data( this->index(index.row(), 0 + 1), Qt::DisplayRole );
    const QVariant &varIdFor = QSqlRelationalTableModel::data( index, Qt::DisplayRole );
    cmmn::T_id idPrim, idFor; // primary and foreign keys values
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varIdPrim, idPrim), varIdPrim );
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varIdFor, idFor), varIdFor );

    qDebug() << "updateDataInStorage() 1, idPrim:" << varIdPrim << "=" << idPrim << ", idFor:" << varIdFor << "=" << idFor;

    // TODO: maybe create the Abstract Factory or other construction for prevent creating QueryGenForeignOneId if it is already setted
    m_dataGenerator->setQueryGenerator( new QueryGenForeignOneId(idFor, idPrim) );

    qDebug() << "updateDataInStorage() 2, before generate()";
    m_dataGenerator->generate( dbi::fieldByNameIndex(tableName(), index.column()) );
    qDebug() << "updateDataInStorage() 3, after generate()";
    int ii = 0;
    if (m_dataGenerator->hasNextResultData()) {
        qDebug() << "updateDataInStorage() 4" << ii << ", hasNextResultData";
        const auto &resData = m_dataGenerator->nextResultData();
        qDebug() << "updateDataInStorage() 5" << ii << ", hasNextResultData, getted data, id =" << resData.idPrim << ", data =" << resData.genData;
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

/* Spike #1. Save data (EditRole) of the foreign keys, that related with complex DBT's in the current row without index currIndex */
void CustomSqlTableModel::spike1_saveData(const QModelIndex &baseIndex)
{
    while (m_genDataStorage->hasNextFieldIndex()) {
        const auto &fieldIndex = m_genDataStorage->nextFieldIndex();
        if (fieldIndex != baseIndex.column())
            m_spike1_saveRestore.insert( fieldIndex, QSqlRelationalTableModel::data( this->index(baseIndex.row(), fieldIndex), Qt::DisplayRole ) );
    }
    m_genDataStorage->flushFieldIndex(); // restore the field index for the nexts data gettings
    qDebug() << "Spike #1, saved data:";
    for (auto it = m_spike1_saveRestore.cbegin(); it != m_spike1_saveRestore.cend(); ++it)
        qDebug() << it.key() << ":" << it.value();
}

/* Restore saved data after calling the QSqlRelationalTableModel::setData() method. Spike #1 */
void CustomSqlTableModel::spike1_restoreData(const QModelIndex &baseIndex)
{
    qDebug() << "spike1_restoreData() 1";
    int ii = 0;
    for (auto it = m_spike1_saveRestore.cbegin(); it != m_spike1_saveRestore.cend(); ++it) {
        qDebug() << "spike1_restoreData() 2" << ii << ", [" << baseIndex.row() << "," << it.key() << "], replace:"
                 << QSqlRelationalTableModel::data( this->index(baseIndex.row(), it.key()), Qt::DisplayRole ).toString()
                 << "to the:" << it.value().toString();
        QSqlRelationalTableModel::setData( this->index(baseIndex.row(), it.key()), it.value(), Qt::EditRole );
        qDebug() << "spike1_restoreData() 3" << ii;
        ++ii;
    }
    qDebug() << "spike1_restoreData() 4";
    m_spike1_saveRestore.clear();
    qDebug() << "spike1_restoreData() 5";
    m_spike1_bNeedSave = false;
}

void CustomSqlTableModel::printDataBase(int role) const
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

void CustomSqlTableModel::printDataCustom(int role) const
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
        printDataBase(Qt::DisplayRole);

        if (!select()) { // populate the model
            // TODO: generate the error
            qCritical() << "[CRITICAL ERROR] Cannot refresh the model - error data populating.\nThe DB error text: " << lastError().text();
        }

        qDebug() << "after slotRefreshTheModel()::select(), column count =" << columnCount();
        printHeader(Qt::DisplayRole);
        printDataBase(Qt::DisplayRole);

        insertColumn(0); // must be only after calling the select() method
        // setHeaderData() // must be only after calling the select() method

        qDebug() << "before FIND SIMPLE, column count =" << columnCount();
        for (int col = 0; col < columnCount(); ++col) {
            if (col == SELECT_ICON_COLUMN) continue;
            const dbi::DBTFieldInfo &fieldInf = dbi::fieldByNameIndex(tableName(), col - 1);
            qDebug() << "FIND SIMPLE, col =" << col << ", field =" << fieldInf.m_nameInDB;
            if (dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_simple)) {
                dbi::DBTInfo *relTableInf = dbi::relatedDBT(fieldInf);
                setRelation( col,
                             QSqlRelation( relTableInf->m_nameInDB, relTableInf->m_fields.at(0).m_nameInDB, relTableInf->m_fields.at(1).m_nameInDB ) );
                qDebug() << "SIMPLE WAS FOUND, col =" << col << ", field =" << fieldInf.m_nameInDB;
            }
        }

        qDebug() << "after slotRefreshTheModel()::insertColumn(0), column count =" << columnCount();
        printHeader(Qt::DisplayRole);
        printDataBase(Qt::DisplayRole);

        m_genDataStorage->flushFieldIndex(); // reset field index
        fillTheStorage(); // populate the storage
        m_genDataStorage->flushFieldIndex(); // reset field index
//        printDataBase(Qt::EditRole); // TODO: delete later



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
     * that is foreign and related with the complex DBT, sets to generated string (copy from the mapped widgets)
     * and there are no ability to get the foreign key values for this fields.
     */
//    QSqlRelationalDelegate::setModelData(editor, model, index);

    qDebug() << "delegate, [" << index.row() << "," << index.column() << "], data:" << model->data(index);
    CustomSqlTableModel *sqlModel = qobject_cast<CustomSqlTableModel *>(model);
    if (!sqlModel) return;

    // convert the model index of the custom model to the model index of the base model
    const QModelIndex &baseIdx = RegCI.convModelIndex(index, sqlModel, RegisterCI::cd_customToBase);

    const auto &fieldInf = dbi::fieldByNameIndex(sqlModel->tableName(), baseIdx.column());

//    // set data for all field, except the fields that is foreign keys to the complex DBT
    if (!fieldInf.isForeign() || dbi::isRelatedWithDBTType(fieldInf, dbi::DBTInfo::ttype_simple))
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

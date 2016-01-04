#include <QSqlQuery>
#include <QDebug>
#include "custom_sql_table_model.h"
#include "generator_dbt_data.h"
#include "storage_gen_data.h"
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
 * Turn on the run of this operations performs by calling the public method switchSpike1(). Turn off the run of this operations
 * performs without assistance from outside in the restoreData_spike1() method.
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
    if (m_genDataStorage->isEmpty()) {
        defineSimpleDBTAndComplexIndex();
        slotRefreshTheModel();
    }
//    connect(this, SIGNAL(dataChanged(QModelIndex,QModelIndex)), this, SLOT(slotDataChanged(QModelIndex,QModelIndex))); // TODO: for debugging, delete later
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

QVariant CustomSqlTableModel::data(const QModelIndex &item, int role) const
{
    // TODO: use try-catch
    QVariant data;
    int storageDataIndex = -1;
    if ( (role == Qt::EditRole || role == Qt::DisplayRole) && m_genDataStorage->isComplexDBTField(item.column(), storageDataIndex) ) {
        try {
            // TODO: after debugging, move this code to the another method and call it in this "try" section
            int idPrim = cmmn::safeQVariantToInt( QSqlRelationalTableModel::data( this->index(item.row(), 0), Qt::DisplayRole ) ); // primary id
            if (m_genDataStorage->isStorageContainsId(idPrim))
                data = m_genDataStorage->data(idPrim, storageDataIndex);
//            qDebug() << "data(), [" << item.row() << "," << item.column() << "], role:" << role << ", data:" << data.toString();
        }
        catch (const cmmn::MessageException &me) {
            // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
            QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
            qDebug() << "[ERROR custom] Title:" << me.title() << "\nMessage:" << me.message() << "\n" << strPlace;
            return false;
        }
        catch (const std::exception &ex) {
            // TODO: generate a message box with critical error
            qDebug() << "[ERROR standard] Message: " << ex.what();
            return false;
        }
    }
    else if ( role == Qt::UserRole ) {
        data = QSqlRelationalTableModel::data(item, Qt::DisplayRole); // take data from the display role
//        qDebug() << "data(), [" << item.row() << "," << item.column() << "], role:" << role << ", data:" << data.toString();
    }
    else data = QSqlRelationalTableModel::data(item, role);
    qDebug() << "data(), [" << item.row() << "," << item.column() << "], role:" << role << ", data:" << data.toString();
    return data;
}

bool CustomSqlTableModel::setData(const QModelIndex &item, const QVariant &value, int role)
{
    /*
     * You cannot call the QSqlRelationalTableModel::setData() method with the Qt::UserRole to save some data in the existent model.
     * In this case the QSqlRelationalTableModel::setData() method return false. The setData() method successfully works only with Qt::EditRole.
     * Saving data with the Qt::UserRole (ot UserRole + 1, +2, ...) can be achieved by means of data saving in a some custom storage.
     */
    if (!item.isValid()) return false;
    bool bSetted = false;
    int storageComplexIndex = -1;
    if (role == Qt::EditRole && m_genDataStorage->isComplexDBTField(item.column(), storageComplexIndex) ) {

        qDebug() << "setData(), before set, has dirty items:" << isDirty();
        printData(Qt::EditRole);
        if (m_bNeedSave_spike1) saveData_spike1(item); // Spike #1
        /*
         * TODO: figure out what a fucking shit happen in the QSqlRelationalTableModel::setData() and the QSqlTableModel::setData() methods!!!
         * The results of this shit is self-independent changes in the model data - foreign id's replaced to the generated data.
         */
        bSetted = QSqlRelationalTableModel::setData(item, value, role);

        qDebug() << "setData(), after set, has dirty items:" << isDirty();
        printData(Qt::EditRole);

        qDebug() << "setData(), Complex DBT, [" << item.row() << "," << item.column() << "], role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
        if (m_bNeedSave_spike1) restoreData_spike1(item.row()); // Spike #1

        try {
            updateDataInStorage(item, storageComplexIndex);
        }
        catch (const cmmn::MessageException &me) {
            // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
            QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
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
    else {
        bSetted = QSqlRelationalTableModel::setData(item, value, role);
        qDebug() << "setData(), NOT Complex DBT, [" << item.row() << "," << item.column() << "], role:" << role << ", set data:" << value.toString() << ", bSetted:" << bSetted;
    }
    return bSetted;
}

/* generate a data and update the storage */
void CustomSqlTableModel::updateDataInStorage(const QModelIndex &index, int storageComplexIndex)
{
    int idPrim = cmmn::safeQVariantToInt( QSqlRelationalTableModel::data( this->index(index.row(), 0), Qt::DisplayRole ) ); // primary key id
    int idFor = cmmn::safeQVariantToInt( QSqlRelationalTableModel::data( index, Qt::DisplayRole ) ); // foreign key id
    m_dataGenerator->setQueryGenerator( new QueryGenForeignOneId(idFor, idPrim) );
    m_dataGenerator->generate( dbi::fieldByNameIndex(tableName(), index.column()) );
    if (m_dataGenerator->hasNextResultData()) {
        const auto &resData = m_dataGenerator->nextResultData();
        m_genDataStorage->updateData(resData.idPrim, storageComplexIndex, resData.genData);
    }
    else {
        // TODO: generate error message
        qDebug() << "!--Error. Cannot set a data to the model storage. Data wasn't generated.\n"
                    "Please check the query for generating data for the item: [" << index.row() << "," << index.column() << "]";
        return;
    }
    if (m_dataGenerator->hasNextResultData()) {
        // TODO: generate error message
        qDebug() << "!--Error. Cannot set a data to the model storage. Too many data was generated.\n"
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
        qDebug() << "data restoring. col =" << it.key() << ", value =" << it.value().toString();
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

void CustomSqlTableModel::slotRefreshTheModel()
{
    try {
        flush(); // delete previous results.
        // TODO: maybe need to clear data in the model at here? For example, if there was added a new record (row) in a the DBT.
        select();
        fillTheStorage();
        m_genDataStorage->flushFieldIndex();
    }
    catch (const cmmn::MessageException &me) {
        // TODO: generate a message box with error depending on the cmmn::MessageException::MessageTypes
        QString strPlace = QObject::tr("Error placement") + ": " + me.codePlace();
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

void CustomSqlTableModel::slotInsertToTheStorage(int idPrim)
{
//    m_dataGen->setQueryGenerator( new QPMainTableOneId(idPrim, tableName()) );
}

void CustomSqlTableModel::slotUpdateTheStorage(int idPrim, int colNumb)
{
//    m_dataGen->setQueryGenerator( new QPMainTableOneId(idPrim, tableName()) );
//    int indexComplex = m_indexComplex.indexOf( cmmn::safeQVariantToInt(colNumb) ); // converting the column number to the storage index
    //    if (indexComplex == -1) return;
}

void CustomSqlTableModel::slotDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    qDebug() << "slotDataChanged(). from [" << topLeft.row() << "," << topLeft.column() << "] to [" << bottomRight.row() << "," << bottomRight.column() << "]";
    printData(Qt::EditRole);
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

    qDebug() << "delegate, [" << index.row() << "," << index.column() << "]";

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
    qDebug() << "set data to simple DBT - 1, [" << index.row() << "," << index.column() << "]";
    QSqlRelationalTableModel *sqlModel = qobject_cast<QSqlRelationalTableModel *>(model);
    QSqlTableModel *childModel = sqlModel ? sqlModel->relationModel(index.column()) : 0;
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (sqlModel && childModel && combo) {
        qDebug() << "set data to simple DBT - 2, [" << index.row() << "," << index.column() << "]";
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

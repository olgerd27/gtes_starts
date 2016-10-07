#include <QMap>
#include <QTableView>
#include <QLineEdit>
#include <QDebug>
#include "proxy_model.h"
#include "custom_sql_table_model.h"
#include "selection_allower.h"

/*
 * RowsChangesHolder - storage of rows changes
 */
class RowsChangesHolder
{
public:
    typedef int T_rowNumber;

    enum ChangesTypes {
          chtype_insert
        , chtype_delete
        , chtype_alter
        , chtype_invalid
    };

    bool addChange(T_rowNumber row, ChangesTypes change);
    bool deleteChange(T_rowNumber row);
    void clearChanges();
    bool getChangeForRow(T_rowNumber row, ChangesTypes *pChangeType) const; // get the change for the specified row
    bool hasRowChange(T_rowNumber row, ChangesTypes controlChangeType) const; // checking - has a row change controlChangeType
    bool hasChanges() const;
private:
    void print() const; // TODO: for debugging, delete later

    QMap<T_rowNumber, ChangesTypes> m_rowsChanges;
};

bool RowsChangesHolder::addChange(RowsChangesHolder::T_rowNumber row, ChangesTypes changeNew)
{
    ChangesTypes changeContain = chtype_invalid; // change, that has passed row
    bool hasChange = getChangeForRow(row, &changeContain);
    if (!hasChange)
        m_rowsChanges.insert(row, changeNew);
    hasChange = !hasChange; // if initially row contains - this make bool value false, if doesn't contain - insert row and make bool value true

//    qDebug() << "Add change, row #" << row << ", changes:";
//    print();

    return hasChange;
}

bool RowsChangesHolder::deleteChange(RowsChangesHolder::T_rowNumber row)
{
    return m_rowsChanges.remove(row);
}

void RowsChangesHolder::clearChanges()
{
    m_rowsChanges.clear();
}

bool RowsChangesHolder::getChangeForRow(RowsChangesHolder::T_rowNumber row, ChangesTypes *pChangeType) const
{
    if (!hasChanges()) return false;
    ChangesTypes chtypeDefault = chtype_invalid;
    *pChangeType = m_rowsChanges.value(row, chtypeDefault);
    return *pChangeType != chtypeDefault;
}

bool RowsChangesHolder::hasRowChange(RowsChangesHolder::T_rowNumber row, ChangesTypes controlChangeType) const
{
    ChangesTypes changeType = chtype_invalid;
    return getChangeForRow(row, &changeType) && changeType == controlChangeType;
}

bool RowsChangesHolder::hasChanges() const
{
    return !m_rowsChanges.isEmpty();
}

void RowsChangesHolder::print() const
{
    QString name;
    for (auto it = m_rowsChanges.cbegin(); it != m_rowsChanges.cend(); ++it) {
        if (it.value() == chtype_insert) name = "INSERT";
        else if (it.value() == chtype_delete) name = "DELETE";
        else if (it.value() == chtype_alter) name = "ALTER";
        else if (it.value() == chtype_invalid) name = "INVALID";
        else name = "UNKNOWN";
        qDebug() << "  " << it.key() << ":" << name;
    }
}

/*
 * IRDefiner - definer indexes of row.
 * Use for definition row index, that need for switching to, after performing change model operations.
 */
class IRDefiner
{
public:
    enum DefinerType
    {
          dtype_insert          // row index definer type, used after calling "insert" operation
        , dtype_deleteExistent  // row index definer type, used after calling "delete" operation of existent in the DB row
        , dtype_deleteInserted  // row index definer type, used after calling "delete" operation of new inserted row
        , dtype_refreshExistent // row index definer type, used after calling "refresh" model operation, when current row is existent in DB row
        , dtype_refreshInserted // row index definer type, used after calling "refresh" model operation, when current row is new inserted row
        , dtype_save            // row index definer type, used after calling "save" model's data operation
    };

    IRDefiner(const ProxyDecorModel *model) : m_model(model) { }
    inline void setModel(const ProxyDecorModel *model) { m_model = model; }
    virtual bool define(int *pRow = 0) const = 0;

protected:
    inline const std::unique_ptr<RowsChangesHolder> & changesHolder() const { return m_model->m_changedRows; }
    inline int modelRowsCount() const { return m_model->rowCount(); }

private:
    const ProxyDecorModel *m_model;
};

// Definer row index after performing insert operation
class IRD_Insert : public IRDefiner
{
public:
    IRD_Insert(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *) const override
    {
        return true; // return true - this mean defined row is a new inserted row
    }
};

// Definer row index after performing delete operation of just inserted row (not saved in the DB)
class IRD_DeleteExistent : public IRDefiner
{
private:
    typedef bool (IRD_DeleteExistent::*pFChangeRow)(int&) const;

    bool incr(int &row) const { ++row; return true; }
    bool decr(int &row) const { --row; return true; }
    bool stop(int &) const { return false; }

public:
    IRD_DeleteExistent(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *pRow) const override
    {
        int rowDef = *pRow; // copy for row definition
        int rowsCounter = 0;
        pFChangeRow fChangeRow = &IRD_DeleteExistent::decr; // set function, that decrement row index
        do {
            if (rowDef == 0) {
                fChangeRow = &IRD_DeleteExistent::incr; // set function, that increment row index
                rowDef = *pRow; // this allow don't repeat going by the same rows after change "change row" function to incr()
            }
            else if (rowDef == modelRowsCount())
                fChangeRow = &IRD_DeleteExistent::stop; // set function, that stop of row index changing
            if (++rowsCounter == modelRowsCount()) return false; // no one row can be defined -> exit from function with false
            if ( !(this->*fChangeRow)(rowDef) ) break; // make change of row index
        } while( changesHolder()->hasRowChange(rowDef, RowsChangesHolder::chtype_delete) );
        *pRow = rowDef;
        return true;
    }
};

// Definer row index after performing delete operation of existent row (saved in the DB)
class IRD_DeleteInserted : public IRDefiner
{
public:
    IRD_DeleteInserted(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *pRow) const override
    {
        do {
            if (*pRow == 0) return false;
            --(*pRow);
        } while ( changesHolder()->hasRowChange(*pRow, RowsChangesHolder::chtype_delete) );
        return true;
    }
};

// Definer row index after performing refresh model operation - case if current row is a new inserted row and doesn't saved in the DB.
// ER - current row is Existent Row in the DB
class IRD_RefreshER : public IRDefiner
{
public:
    IRD_RefreshER(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *) const override
    {
        return true; // don't change current row
    }
};

// Definer row index after performing refresh model operation - case if current row is existent in the DB row.
// IR - current row is Inserted Row
class IRD_RefreshIR : public IRDefiner
{
public:
    IRD_RefreshIR(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *pRow) const override
    {
//        *pRow = 0; // change current row to the first row
        *pRow = modelRowsCount() - 1; // change current row to the last existent in DB row
        return true;
    }
};

// Definer row index after performing save model's data operation
class IRD_Save : public IRDefiner
{
public:
    IRD_Save(const ProxyDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *) const override
    {
        return true; // return true - this mean defined row is current row -> after saving, the current row won't change
    }
};

// The Factory method for getting particular index of row definer
IRDefiner *getIRDefiner(IRDefiner::DefinerType dtype, const ProxyDecorModel * const model)
{
    IRDefiner *ird = 0;
    switch (dtype) {
    case IRDefiner::dtype_insert:
        ird = new IRD_Insert(model);
        break;
    case IRDefiner::dtype_deleteExistent:
        ird = new IRD_DeleteExistent(model);
        break;
    case IRDefiner::dtype_deleteInserted:
        ird = new IRD_DeleteInserted(model);
        break;
    case IRDefiner::dtype_refreshExistent:
        ird = new IRD_RefreshER(model);
        break;
    case IRDefiner::dtype_refreshInserted:
        ird = new IRD_RefreshIR(model);
        break;
    case IRDefiner::dtype_save:
        ird = new IRD_Save(model);
        break;
    default:
        ASSERT_DBG( 0, cmmn::MessageException::type_fatal, QObject::tr("Error index of row"),
                    QObject::tr("Cannot define index of row. Unknow model change operation."),
                    QString("getIRDefiner") );
        break;
    }
    return ird;
}

/*
 * ProxyDecorModel
 * Implemented with help of: http://www.qtcentre.org/threads/58307-Problem-when-adding-a-column-to-a-QAbstractProxyModel
 */
ProxyDecorModel::ProxyDecorModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , m_selectedId(NOT_SETTED)
    , m_selectIcon(":/images/ok.png")
    , m_changedRows(new RowsChangesHolder)
{
    /*
     * NOTE: For insertion a new virtual column in this model you cannot use the QAbstractProxyModel::insertColumn(),
     * because this method insert new column in the source model too. Probably, the proper decision for insertion
     * new column is reimplement some virtual methods of the QAbstractProxyModel class.
     */
    setSourceModel(new CustomSqlTableModel(this));
}

ProxyDecorModel::~ProxyDecorModel()
{ }

void ProxyDecorModel::setSqlTableName(const QString &tableName)
{
    customSourceModel()->setTable(tableName);
}

QString ProxyDecorModel::sqlTableName() const
{
    return customSourceModel()->tableName();
}

QVariant ProxyDecorModel::data(const QModelIndex &index, int role) const
{
    QVariant data;
    auto changeType = RowsChangesHolder::chtype_invalid;
    if (index.column() == SELECT_ICON_COLUMN && rowId(index.row()) == m_selectedId && role == Qt::DecorationRole )
        data = m_selectIcon;
    else if (index.column() == SELECT_ICON_COLUMN /*&& (role != Qt::DecorationRole)*/)
        data = QVariant();
    else if ( role == Qt::BackgroundColorRole && m_changedRows->getChangeForRow(index.row(), &changeType) ) {
        if (changeType == RowsChangesHolder::chtype_insert)
            data = QColor(0, 110, 0, 80); // set transperent green background
        else if (changeType == RowsChangesHolder::chtype_delete)
            data = QColor(255, 0, 0, 80); // set transperent red background
    }
    else if ( role == Qt::TextAlignmentRole && this->data(index, Qt::DisplayRole).convert(QMetaType::Float) )
        data = Qt::AlignCenter; // center alignment of the numerical values
    else
        data = sourceModel()->data(mapToSource(index), role);
    return data;
}

bool ProxyDecorModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool bSetted = false;
    if (role == Qt::DecorationRole && index.column() == SELECT_ICON_COLUMN) {
        m_selectedId = rowId(index.row());
//        qDebug() << "proxy model setData(), change selected id to" << m_selectedId << ", row =" << index.row();
    }
    else {
        bSetted = sourceModel()->setData( mapToSource(index), value, role );
//        qDebug() << "proxy model setData(), ELSE case: [" << index.row() << "," << index.column()
//                 << "] role =" << role << "data:" << value.toString() << ", bSetted:" << bSetted;
    }
    if (bSetted) emit dataChanged(index, index);
    return bSetted;
}

int ProxyDecorModel::columnCount(const QModelIndex &/*parent*/) const
{
    return sourceModel()->columnCount() + COUNT_ADDED_COLUMNS;
}

int ProxyDecorModel::rowCount(const QModelIndex &/*parent*/) const
{
    return sourceModel()->rowCount();
}

Qt::ItemFlags ProxyDecorModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return m_changedRows->hasRowChange(index.row(), RowsChangesHolder::chtype_delete)
            ? QAbstractProxyModel::flags(index) & ~Qt::ItemIsEnabled // current row deleted - make it disabled
            : Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ProxyDecorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant data;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        data = ( section == SELECT_ICON_COLUMN
                 ? QVariant("")
                 : sourceModel()->headerData(section - COUNT_ADDED_COLUMNS, orientation, role) ); // QAbstractProxyModel::headerData() don't work properly
    return data;
}

QModelIndex ProxyDecorModel::index(int row, int column, const QModelIndex &parent) const
{
    return (parent.isValid() || row < 0 || row >= rowCount(parent) || column < 0 || column >= columnCount(parent))
            ? QModelIndex() : createIndex( row, column );
}

QModelIndex ProxyDecorModel::parent(const QModelIndex &/*child*/) const
{
    return QModelIndex();
}

QModelIndex ProxyDecorModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) return QModelIndex();
    return index( sourceIndex.row(), sourceIndex.column() + COUNT_ADDED_COLUMNS ); // work version 2
}

QModelIndex ProxyDecorModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if ( !proxyIndex.isValid() || proxyIndex.column() == SELECT_ICON_COLUMN ) return QModelIndex();
    return sourceModel()->index( proxyIndex.row(), proxyIndex.column() - COUNT_ADDED_COLUMNS ); // work version 2
}

/* Get the pointer to the custom sql source model */
CustomSqlTableModel *ProxyDecorModel::customSourceModel() const
{
    CustomSqlTableModel *srcModel = qobject_cast<CustomSqlTableModel *>(sourceModel());
    ASSERT_DBG( srcModel,
                cmmn::MessageException::type_critical, QObject::tr("Error get the custom source model"),
                QObject::tr("Unknow source model was setted to the proxy model"),
                QString("ProxyDecorModel::customSourceModel") );
    return srcModel;
}

cmmn::T_id ProxyDecorModel::selectedId() const
{
    return m_selectedId;
}

cmmn::T_id ProxyDecorModel::rowId(int row) const
{
    cmmn::T_id id;
    const QVariant &varId = customSourceModel()->primaryIdInRow(row);
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, id), varId );
    return id;
}

bool ProxyDecorModel::isDirty() const
{
    return m_changedRows->hasChanges();
}

void ProxyDecorModel::clearDirtyChanges()
{
    m_changedRows->clearChanges();
}

#ifdef __linux__
QSize ProxyDecorModel::decorationSize() const
{
    auto sizes = m_selectIcon.availableSizes(QIcon::Normal, QIcon::Off);
    return sizes.empty() ? QSize() : sizes.first();
}
#endif

void ProxyDecorModel::slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected)
{
    QItemSelectionModel *selectModel = qobject_cast<QItemSelectionModel *>(sender());
    const QModelIndexList &deselectedList = deselected.indexes();

    // catch a deselection of the first left item in current row and setting icons decoration on it
    if (deselectedList.size() == COUNT_ADDED_COLUMNS) {
        // operate special case in Windows XP, Qt ver. 5.3.0. Normal case - in this place no one item must be selected.
        // TODO: use preprocessor declaration
        const QModelIndexList &selectedList = selected.indexes();
        if (!selectedList.isEmpty()) {
            selectModel->select(selectedList.first(), QItemSelectionModel::Deselect); // repeat deselection
            updatePrevDeselected(deselectedList);
            return;
        }
        setData( index( deselectedList.first().row(), SELECT_ICON_COLUMN ), QVariant(), Qt::DecorationRole );
        return;
    }
    // update the first left items in the previous selected row for clearing icons decoration
    if (deselectedList.size() > COUNT_ADDED_COLUMNS)
        updatePrevDeselected(deselectedList);
    selectModel->select(selected.indexes().first(), QItemSelectionModel::Deselect); // this make recursive calling of this slot
}

void ProxyDecorModel::updatePrevDeselected(const QModelIndexList &deselectList)
{
    // update the first left items in the previous selected row for clearing icons decoration
    const QModelIndex &someDeselected = deselectList.first();
    const QModelIndex &firstDeselected = someDeselected.model()->index(someDeselected.row(), SELECT_ICON_COLUMN);
    emit dataChanged(firstDeselected, firstDeselected); // clear remained icons decoration
}

// PRINTING
void ProxyDecorModel::printHeader(int role) const
{
    QString strData = "\n";
    for (int sect = 0; sect < columnCount(); ++sect) {
        strData += QString("|") += headerData(sect, Qt::Horizontal, role).toString() += QString("| ");
    }
    qDebug() << "Horizontal header data with role #" << role << ":" << strData;
}

// SLOTS
void ProxyDecorModel::slotAddRow()
{
    int rowInsert = this->rowCount();
    beginInsertRows(QModelIndex(), rowInsert, rowInsert);
    customSourceModel()->slotInsertToTheModel();
    // save current row insert change
    ASSERT_DBG( m_changedRows->addChange(rowInsert, RowsChangesHolder::chtype_insert),
                cmmn::MessageException::type_critical, QObject::tr("Error add row"),
                QObject::tr("Cannot add row change to the row changes storage"),
                QString("ProxyDecorModel::slotAddRow") );
    endInsertRows();
    qDebug() << "[proxy model] data ADDed to the model";
    changeRow(IRDefiner::dtype_insert, rowInsert);
}

void ProxyDecorModel::slotDeleteRow(int currentRow)
{
    ASSERT_DBG( canDeleteRow(currentRow), cmmn::MessageException::type_warning, QObject::tr("Error delete row"),
                QObject::tr("Cannot delete current row #%1").arg(currentRow),
                QString("ProxyDecorModel::slotDeleteRow") );
    customSourceModel()->slotDeleteRowRecord(currentRow);
    IRDefiner::DefinerType defType;
    if ( m_changedRows->hasRowChange(currentRow, RowsChangesHolder::chtype_insert) ) {
        // if row has "insert change" -> delete change of the last row in the model
        // (deletion the model's just inserted row in fact delete row from model completely)
        ASSERT_DBG( m_changedRows->deleteChange(rowCount()),
                    cmmn::MessageException::type_critical, QObject::tr("Error delete row"),
                    QObject::tr("Cannot delete row change from the row changes storage"),
                    QString("ProxyDecorModel::slotDeleteRow") );
        defType = IRDefiner::dtype_deleteInserted;
    }
    else {
        // if row is existent in the DB -> save it index as deleted and define appropriate type of row index definer
        m_changedRows->addChange(currentRow, RowsChangesHolder::chtype_delete); // save current row delete change
        defType = IRDefiner::dtype_deleteExistent;
    }
    qDebug() << "[proxy model] data DELETEd from the model";
    changeRow(defType, currentRow);
}

bool ProxyDecorModel::canDeleteRow(int row) const
{
    return row < rowCount() && !m_changedRows->hasRowChange(row, RowsChangesHolder::chtype_delete);
}

void ProxyDecorModel::slotRefreshModel(int currentRow)
{
    customSourceModel()->slotRefreshTheModel();
    // definition - is or not the current row a new inserted row
    IRDefiner::DefinerType defType = m_changedRows->hasRowChange(currentRow, RowsChangesHolder::chtype_insert)
            ? IRDefiner::dtype_refreshInserted : IRDefiner::dtype_refreshExistent;
    clearDirtyChanges(); // must be after definition - has current row the "insert change"
    qDebug() << "[proxy model] data REFRESHed in the model";
    changeRow(defType, currentRow);
}

void ProxyDecorModel::slotSaveDataToDB(int currentRow)
{
    customSourceModel()->slotSaveToDB();
    clearDirtyChanges();
    qDebug() << "[proxy model] data SAVEd in the DB";
    changeRow(IRDefiner::dtype_save, currentRow);
}

void ProxyDecorModel::changeRow(int defType, int row)
{
    std::unique_ptr<IRDefiner> rowIdxDef( getIRDefiner((IRDefiner::DefinerType)defType, this) ); // create definer of index row
    if (rowIdxDef->define(&row)) {
        emit sigChangeCurrentRow(row); // change row in connected view(-s)
        qDebug() << "change row definition - change to" << row;
    }
    emit dataChanged( this->index(0, 0), this->index(this->rowCount() - 1, this->columnCount() - 1) ); // update connected view(-s)
}

void ProxyDecorModel::printData(int role) const
{
    QString strData = "\n";
    for(int row = 0; row < rowCount(); ++row) {
        for(int col = 0; col < columnCount(); ++col ) {
            const QModelIndex &index = this->index(row, col);
            strData += QString("|%1|  ").arg(data(index, role).toString());
        }
        strData += "\n";
    }
    qDebug() << "The decor proxy model data with role #" << role << ":" << strData;
}

/*
 * ProxyFilterModel
 */
ProxyFilterModel::ProxyFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_selectAllow(new SelectionAllower(this)) // set default SelectionAllower
{ }

ProxyFilterModel::~ProxyFilterModel()
{ }

void ProxyFilterModel::setSelectionAllower(SelectionAllower *sa)
{
    m_selectAllow.reset(sa);
}

void ProxyFilterModel::slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected)
{
    if (!m_selectAllow->isSelectionAllowed()) return;

//    auto selIdxs = selected.indexes();
//    if (!selIdxs.isEmpty())
//        qDebug() << "FilterModel, row: filter proxy =" << selIdxs.first().row() << ", proxy =" << mapToSource(selIdxs.first()).row();

    QItemSelectionModel *selectModel = qobject_cast<QItemSelectionModel *>(sender());
    const QModelIndexList &deselectedList = deselected.indexes();

    // catch a deselection of the first left item in current row and setting icons decoration on it
    if (deselectedList.size() == ProxyDecorModel::COUNT_ADDED_COLUMNS) {
        // operate special case in Windows XP, Qt ver. 5.3.0. Normal case - in this place no one item must be selected.
        // TODO: use preprocessor declaration
        const QModelIndexList &selectedList = selected.indexes();
        if (!selectedList.isEmpty()) {
            selectModel->select(selectedList.first(), QItemSelectionModel::Deselect); // repeat deselection
            updatePrevDeselected(deselectedList);
            return;
        }
        setData( index( deselectedList.first().row(), ProxyDecorModel::SELECT_ICON_COLUMN ), QVariant(), Qt::DecorationRole );
        emit sigSelectionEnded(); // notification, that model ended its custom (with decoration icon) selection
        return;
    }
    // update the first left items in the previous selected row for clearing icons decoration
    if (deselectedList.size() > ProxyDecorModel::COUNT_ADDED_COLUMNS)
        updatePrevDeselected(deselectedList);
    selectModel->select(selected.indexes().first(), QItemSelectionModel::Deselect); // this make recursive calling of this slot
}

void ProxyFilterModel::updatePrevDeselected(const QModelIndexList &deselectList)
{
    // update the first left items in the previous selected row for clearing icons decoration
    const QModelIndex &someDeselected = deselectList.first();
    const QModelIndex &firstDeselected = someDeselected.model()->index(someDeselected.row(), ProxyDecorModel::SELECT_ICON_COLUMN);
    emit dataChanged(firstDeselected, firstDeselected); // clear remained icons decoration
}

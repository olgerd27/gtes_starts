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
    bool setRowChangeType(T_rowNumber row, ChangesTypes changeType);
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
//    else if (hasChange && changeNew != chtype_alter && changeContain == chtype_alter)
//        m_rowsChanges[row] = changeNew; // if passed row has "alter" change type -> replace it to the passed change type
    hasChange = !hasChange; // if initially row contains - this make bool value false, if doesn't contain - insert row and make bool value true

    qDebug() << "Add change, row #" << row << ", changes:";
    print();

    return hasChange;
}

bool RowsChangesHolder::deleteChange(RowsChangesHolder::T_rowNumber row)
{
    return m_rowsChanges.remove(row);

//    bool b = m_rowsChanges.remove(row);
//    qDebug() << "Remove row #" << row << ", changes:";
//    print();
//    return b;
}

void RowsChangesHolder::clearChanges()
{
    m_rowsChanges.clear();
}

bool RowsChangesHolder::getChangeForRow(RowsChangesHolder::T_rowNumber row, ChangesTypes *pChangeType) const
{
    ChangesTypes chtypeDefault = chtype_invalid;
    *pChangeType = m_rowsChanges.value(row, chtypeDefault);
    return *pChangeType != chtypeDefault;
}

bool RowsChangesHolder::hasRowChange(RowsChangesHolder::T_rowNumber row, ChangesTypes controlChangeType) const
{
    ChangesTypes changeType = chtype_invalid;
    return getChangeForRow(row, &changeType) && changeType == controlChangeType;
}

bool RowsChangesHolder::setRowChangeType(RowsChangesHolder::T_rowNumber row, RowsChangesHolder::ChangesTypes changeType)
{
    auto it = m_rowsChanges.find(row);
    bool isAbsent = (it != m_rowsChanges.end());
    if (isAbsent) it.value() = changeType;
    return isAbsent;
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
 * Use for definition row index, that need for switching to after performing change model operations.
 */
class IRDefiner
{
public:
    enum DefinerType
    {
          dtype_insert          // row index definer type, used after calling "insert" operation
        , dtype_deleteExistent  // row index definer type, used after calling "delete" operation of existent in the DB row
        , dtype_deleteInserted  // row index definer type, used after calling "delete" operation of just inserted row
        , dtype_refresh         // row index definer type, used after calling "refresh" model's data operation
        , dtype_save            // row index definer type, used after calling "save" model's data operation
    };

    IRDefiner(const ProxyChoiceDecorModel *model) : m_model(model) { }
    inline void setModel(const ProxyChoiceDecorModel *model) { m_model = model; }
    virtual bool define(int *pRow = 0) const = 0;

protected:
    inline RowsChangesHolder * changesHolder() const { return m_model->m_changedRows.get(); }
    const ProxyChoiceDecorModel *m_model;
};

// Definer row index after performing insert operation
class IRD_Insert : public IRDefiner
{
public:
    IRD_Insert(const ProxyChoiceDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *) const
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
    IRD_DeleteExistent(const ProxyChoiceDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *pRow) const
    {
        int rowDef = *pRow; // copy for row definition
        int rowsCounter = 0;
        pFChangeRow fChangeRow = &IRD_DeleteExistent::decr; // set function, that decrement row index
        do {
            if (rowDef == 0) {
                fChangeRow = &IRD_DeleteExistent::incr; // set function, that increment row index
                rowDef = *pRow; // this allow don't repeat going by the same rows after change "change row" function to incr()
            }
            else if (rowDef == m_model->rowCount()) {
                fChangeRow = &IRD_DeleteExistent::stop; // set function, that stop of row index changing
//                qDebug() << "  STOP";
            }
            if (++rowsCounter == m_model->rowCount()) return false; // no one row can be defined -> exit from function with false
            if ( !(this->*fChangeRow)(rowDef) ) break; // make change of row index
//            qDebug() << "  IRD_DeleteExistent, define row #" << rowDef;
        } while( changesHolder()->hasRowChange(rowDef, RowsChangesHolder::chtype_delete) );
        *pRow = rowDef;
//        qDebug() << "  IRD_DeleteExistent, row defined #" << rowDef << ", rows =" << m_model->rowCount();
        return true;
    }
};

// Definer row index after performing delete operation of existent row (saved in the DB)
class IRD_DeleteInserted : public IRDefiner
{
public:
    IRD_DeleteInserted(const ProxyChoiceDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *pRow) const
    {
        do {
            if (*pRow == 0) return false;
            --(*pRow);
//            qDebug() << "  IRD_DeleteInserted, define row #" << *pRow;
        } while ( changesHolder()->hasRowChange(*pRow, RowsChangesHolder::chtype_delete) );
//        qDebug() << "  IRD_DeleteInserted, row defined #" << *pRow;
        return true;
    }
};

// Definer row index after performing refresh model's data operation
class IRD_Refresh : public IRDefiner
{
public:
    IRD_Refresh(const ProxyChoiceDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *pRow) const
    {
        *pRow = 0; // return index of the first row
        return true;
    }
};

// Definer row index after performing save model's data operation
class IRD_Save : public IRDefiner
{
public:
    IRD_Save(const ProxyChoiceDecorModel *model) : IRDefiner(model) { }
    virtual bool define(int *) const
    {
        return true; // return true - this mean defined row is current row -> after saving, the current row won't change
    }
};

// The Factory method for getting particular index of row definer
IRDefiner *getIRDefiner(IRDefiner::DefinerType dtype, const ProxyChoiceDecorModel * const model)
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
    case IRDefiner::dtype_refresh:
        ird = new IRD_Refresh(model);
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
 * ProxyChoiceDecorModel
 * Implemented with help of: http://www.qtcentre.org/threads/58307-Problem-when-adding-a-column-to-a-QAbstractProxyModel
 */
ProxyChoiceDecorModel::ProxyChoiceDecorModel(QObject *parent)
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
//    connect(customSourceModel(), &CustomSqlTableModel::dataChanged, [this](const QModelIndex &topLeft, const QModelIndex &)
//    {
//        m_changedRows->addChange(topLeft.row(), RowsChangesHolder::chtype_alter); // add altering row change
//    } );
}

ProxyChoiceDecorModel::~ProxyChoiceDecorModel()
{ }

void ProxyChoiceDecorModel::setSqlTable(const QString &tableName)
{
    customSourceModel()->setTable(tableName);
//    printHeader();
    //    printData();
}



QVariant ProxyChoiceDecorModel::data(const QModelIndex &index, int role) const
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

bool ProxyChoiceDecorModel::setData(const QModelIndex &index, const QVariant &value, int role)
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

int ProxyChoiceDecorModel::columnCount(const QModelIndex &/*parent*/) const
{
    return sourceModel()->columnCount() + COUNT_ADDED_COLUMNS;
}

int ProxyChoiceDecorModel::rowCount(const QModelIndex &/*parent*/) const
{
    return sourceModel()->rowCount();
}

Qt::ItemFlags ProxyChoiceDecorModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    return m_changedRows->hasRowChange(index.row(), RowsChangesHolder::chtype_delete)
            ? QAbstractProxyModel::flags(index) & ~Qt::ItemIsEnabled // current row deleted - make it disabled
            : Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ProxyChoiceDecorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant data;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        data = ( section == SELECT_ICON_COLUMN
                 ? QVariant("")
                 : sourceModel()->headerData(section - COUNT_ADDED_COLUMNS, orientation, role) ); // QAbstractProxyModel::headerData() don't work properly
    return data;
}

QModelIndex ProxyChoiceDecorModel::index(int row, int column, const QModelIndex &parent) const
{
    return (parent.isValid() || row < 0 || row >= rowCount(parent) || column < 0 || column >= columnCount(parent))
            ? QModelIndex() : createIndex( row, column );
}

QModelIndex ProxyChoiceDecorModel::parent(const QModelIndex &/*child*/) const
{
    return QModelIndex();
}

QModelIndex ProxyChoiceDecorModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) return QModelIndex();
    return index( sourceIndex.row(), sourceIndex.column() + COUNT_ADDED_COLUMNS ); // work version 2
}

QModelIndex ProxyChoiceDecorModel::mapToSource(const QModelIndex &proxyIndex) const
{
    if ( !proxyIndex.isValid() || proxyIndex.column() == SELECT_ICON_COLUMN ) return QModelIndex();
    return sourceModel()->index( proxyIndex.row(), proxyIndex.column() - COUNT_ADDED_COLUMNS ); // work version 2
}

/* Get the pointer to the custom sql source model */
CustomSqlTableModel *ProxyChoiceDecorModel::customSourceModel() const
{
    CustomSqlTableModel *srcModel = qobject_cast<CustomSqlTableModel *>(sourceModel());
    ASSERT_DBG( srcModel,
                cmmn::MessageException::type_critical, QObject::tr("Error get the custom source model"),
                QObject::tr("Unknow source model was setted to the proxy model"),
                QString("ProxyChoiceDecorModel::customSourceModel()") );
    return srcModel;
}

cmmn::T_id ProxyChoiceDecorModel::selectedId() const
{
    return m_selectedId;
}

cmmn::T_id ProxyChoiceDecorModel::rowId(int row) const
{
    cmmn::T_id id;
    const QVariant &varId = customSourceModel()->primaryIdInRow(row);
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, id), varId );
    return id;
}

bool ProxyChoiceDecorModel::isDirty() const
{
    return m_changedRows->hasChanges();
}

#ifdef __linux__
QSize ProxyChoiceDecorModel::decorationSize() const
{
    auto sizes = m_selectIcon.availableSizes(QIcon::Normal, QIcon::Off);
    return sizes.empty() ? QSize() : sizes.first();
}
#endif

void ProxyChoiceDecorModel::slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected)
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

void ProxyChoiceDecorModel::updatePrevDeselected(const QModelIndexList &deselectList)
{
    // update the first left items in the previous selected row for clearing icons decoration
    const QModelIndex &someDeselected = deselectList.first();
    const QModelIndex &firstDeselected = someDeselected.model()->index(someDeselected.row(), SELECT_ICON_COLUMN);
    emit dataChanged(firstDeselected, firstDeselected); // clear remained icons decoration
}

void ProxyChoiceDecorModel::changeRow(int defType, int row)
{
    std::unique_ptr<IRDefiner> rowIdxDef( getIRDefiner((IRDefiner::DefinerType)defType, this) ); // create definer of index row
    if (rowIdxDef->define(&row)) {
        emit sigChangeCurrentRow(row); // change row in connected view(-s)
        qDebug() << "change row definition - change to" << row;
    }
    emit dataChanged( this->index(0, 0), this->index(this->rowCount() - 1, this->columnCount() - 1) ); // update connected view(-s)
}

bool ProxyChoiceDecorModel::canDeleteRow(int row) const
{
    return row < rowCount() && !m_changedRows->hasRowChange(row, RowsChangesHolder::chtype_delete);
}

// PRINTING
void ProxyChoiceDecorModel::printHeader(int role) const
{
    QString strData = "\n";
    for (int sect = 0; sect < columnCount(); ++sect) {
        strData += QString("|") += headerData(sect, Qt::Horizontal, role).toString() += QString("| ");
    }
    qDebug() << "Horizontal header data with role #" << role << ":" << strData;
}

void ProxyChoiceDecorModel::slotAddRow()
{
    int rowInsert = this->rowCount();
    beginInsertRows(QModelIndex(), rowInsert, rowInsert);
    customSourceModel()->slotInsertToTheModel();
    // save current row insert change
    ASSERT_DBG( m_changedRows->addChange(rowInsert, RowsChangesHolder::chtype_insert),
                cmmn::MessageException::type_critical, QObject::tr("Error add row"),
                QObject::tr("Cannot add row change to the row changes storage"),
                QString("ProxyChoiceDecorModel::slotAddRow()") );
    endInsertRows();
    qDebug() << "[proxy model] data added to the model";
    changeRow(IRDefiner::dtype_insert, rowInsert);
}

void ProxyChoiceDecorModel::slotDeleteRow(int row)
{
    ASSERT_DBG( canDeleteRow(row), cmmn::MessageException::type_warning, QObject::tr("Error delete row"),
                QObject::tr("Cannot delete current row #%1").arg(row),
                QString("ProxyChoiceDecorModel::slotDeleteRow") );
    customSourceModel()->slotDeleteRowRecord(row);
    IRDefiner::DefinerType defType;
    if ( m_changedRows->hasRowChange(row, RowsChangesHolder::chtype_insert) ) {
        // if row has "insert change" -> delete change of the last row in the model
        // (deletion the model's just inserted row in fact delete row from model completely)
        ASSERT_DBG( m_changedRows->deleteChange(rowCount()),
                    cmmn::MessageException::type_critical, QObject::tr("Error delete row"),
                    QObject::tr("Cannot delete row change from the row changes storage"),
                    QString("ProxyChoiceDecorModel::slotDeleteRow()") );
        defType = IRDefiner::dtype_deleteInserted;
    }
    else {
        // if row is existent in the DB -> save it index as deleted and define appropriate type of row index definer
        m_changedRows->addChange(row, RowsChangesHolder::chtype_delete); // save current row delete change
        defType = IRDefiner::dtype_deleteExistent;
    }
    qDebug() << "[proxy model] data deleted from the model";
    changeRow(defType, row);
}

void ProxyChoiceDecorModel::slotRefreshModel()
{
    customSourceModel()->slotRefreshTheModel();
    m_changedRows->clearChanges();
    qDebug() << "[proxy model] data refreshed in the model";
    changeRow(IRDefiner::dtype_refresh);
}

void ProxyChoiceDecorModel::slotSaveDataToDB(int currentRow)
{
    customSourceModel()->slotSaveToDB();
    m_changedRows->clearChanges();
    qDebug() << "[proxy model] data saved in the DB";
    changeRow(IRDefiner::dtype_save, currentRow);
}

void ProxyChoiceDecorModel::printData(int role) const
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
    if (deselectedList.size() == ProxyChoiceDecorModel::COUNT_ADDED_COLUMNS) {
        // operate special case in Windows XP, Qt ver. 5.3.0. Normal case - in this place no one item must be selected.
        // TODO: use preprocessor declaration
        const QModelIndexList &selectedList = selected.indexes();
        if (!selectedList.isEmpty()) {
            selectModel->select(selectedList.first(), QItemSelectionModel::Deselect); // repeat deselection
            updatePrevDeselected(deselectedList);
            return;
        }
        setData( index( deselectedList.first().row(), ProxyChoiceDecorModel::SELECT_ICON_COLUMN ), QVariant(), Qt::DecorationRole );
        emit sigSelectionEnded(); // notification, that model ended its custom (with decoration icon) selection
        return;
    }
    // update the first left items in the previous selected row for clearing icons decoration
    if (deselectedList.size() > ProxyChoiceDecorModel::COUNT_ADDED_COLUMNS)
        updatePrevDeselected(deselectedList);
    selectModel->select(selected.indexes().first(), QItemSelectionModel::Deselect); // this make recursive calling of this slot
}

void ProxyFilterModel::updatePrevDeselected(const QModelIndexList &deselectList)
{
    // update the first left items in the previous selected row for clearing icons decoration
    const QModelIndex &someDeselected = deselectList.first();
    const QModelIndex &firstDeselected = someDeselected.model()->index(someDeselected.row(), ProxyChoiceDecorModel::SELECT_ICON_COLUMN);
    emit dataChanged(firstDeselected, firstDeselected); // clear remained icons decoration
}

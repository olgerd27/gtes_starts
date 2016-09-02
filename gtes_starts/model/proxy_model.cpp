#include <QMap>
#include <QDebug>
#include "proxy_model.h"
#include "custom_sql_table_model.h"

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
        , chtype_invalid
    };

    bool addRow(T_rowNumber row, ChangesTypes change);
    bool deleteRow(T_rowNumber row);
    void clearRows();
    bool findChangeRow(T_rowNumber row, ChangesTypes *pChangeType) const;
    bool hasRowChange(T_rowNumber row, ChangesTypes controlChangeType) const; // checking - has a row change controlChangeType
    bool setRowChangeType(T_rowNumber row, ChangesTypes changeType);
private:
    QMap<T_rowNumber, ChangesTypes> m_rowsChanges;
};

bool RowsChangesHolder::addRow(RowsChangesHolder::T_rowNumber row, ChangesTypes change)
{
    bool isContain = m_rowsChanges.contains(row);
    if (!isContain)
        m_rowsChanges.insert(row, change);
    isContain = !isContain; // if initially row contains - this make bool value false, if doesn't contain - insert row and make bool value true

    qDebug() << "Add row #" << row << ", changes:";
    for (auto it = m_rowsChanges.cbegin(); it != m_rowsChanges.cend(); ++it)
        qDebug() << "  " << it.key() << ":" << it.value();

    return isContain;
}

bool RowsChangesHolder::deleteRow(RowsChangesHolder::T_rowNumber row)
{
//    return m_rowsChanges.remove(row);

    bool b = m_rowsChanges.remove(row);

    qDebug() << "Remove row #" << row << ", changes:";
    for (auto it = m_rowsChanges.cbegin(); it != m_rowsChanges.cend(); ++it)
        qDebug() << "  " << it.key() << ":" << it.value();

    return b;
}

void RowsChangesHolder::clearRows()
{
    m_rowsChanges.clear();
}

bool RowsChangesHolder::findChangeRow(RowsChangesHolder::T_rowNumber row, ChangesTypes *pChangeType) const
{
    ChangesTypes chtype = chtype_invalid;
    *pChangeType = m_rowsChanges.value(row, chtype);
    return *pChangeType != chtype;
}

bool RowsChangesHolder::hasRowChange(RowsChangesHolder::T_rowNumber row, ChangesTypes controlChangeType) const
{
    auto changeType = RowsChangesHolder::chtype_invalid;
    return findChangeRow(row, &changeType) && changeType == controlChangeType;
}

bool RowsChangesHolder::setRowChangeType(RowsChangesHolder::T_rowNumber row, RowsChangesHolder::ChangesTypes changeType)
{
    auto it = m_rowsChanges.find(row);
    bool isAbsent = (it != m_rowsChanges.end());
    if (isAbsent) it.value() = changeType;
    return isAbsent;
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
          dtype_insert
        , dtype_delete
        , dtype_refresh
    };

    IRDefiner(const ProxyChoiceDecorModel *model) : m_model(model) { }
    inline void setModel(const ProxyChoiceDecorModel *model) { m_model = model; }
    virtual int define(int row) const = 0;

protected:
    inline RowsChangesHolder * changesHolder() const { return m_model->m_changedRows.get(); }
    const ProxyChoiceDecorModel *m_model;
};

// Definer row index after performing insert operation
class InsertIRDefiner : public IRDefiner
{
public:
    InsertIRDefiner(const ProxyChoiceDecorModel *model) : IRDefiner(model) { }
    virtual int define(int row) const
    {
        return row; // return index of a new inserted row
    }
};

// Definer row index after performing delete operation
class DeleteIRDefiner : public IRDefiner
{
private:
    enum Direction { UP, DOWN }; // direction of row index definition. UP - decrease row, DOWN - increase row
    typedef bool (DeleteIRDefiner::*pFChangeRow)(int&) const;

    bool incr(int &row) const { ++row; return true; }
    bool decr(int &row) const { --row; return true; }
    bool invl(int &) const { return false; }

public:
    DeleteIRDefiner(const ProxyChoiceDecorModel *model) : IRDefiner(model) { }
    virtual int define(int row) const
    {
        int rowDef = row;
        pFChangeRow fChangeRow = &DeleteIRDefiner::decr; // set function, that decrement row index
        do {
            if (rowDef == 0) {
                fChangeRow = &DeleteIRDefiner::incr; // set function, that increment row index
                rowDef = row; // this allow don't repeat going by the same rows after change "change row" function to incr()
            }
            else if (rowDef == m_model->rowCount())
                fChangeRow = &DeleteIRDefiner::invl; // set function, that stop of row index changing
            if ( !(this->*fChangeRow)(rowDef) ) break; // make change of row index
        } while( changesHolder()->hasRowChange(rowDef, RowsChangesHolder::chtype_delete) );
        return rowDef;
    }
};

// Definer row index after performing refresh model operation
class RefreshIRDefiner : public IRDefiner
{
public:
    RefreshIRDefiner(const ProxyChoiceDecorModel *model) : IRDefiner(model) { }
    virtual int define(int) const
    {
        return 0; // return index of the first row
    }
};

// The Factory method for getting particular index of row definer
IRDefiner *getIRDefiner(IRDefiner::DefinerType dtype, const ProxyChoiceDecorModel * const model)
{
    IRDefiner *ird = 0;
    switch (dtype) {
    case IRDefiner::dtype_insert:
        ird = new InsertIRDefiner(model);
        break;
    case IRDefiner::dtype_delete:
        ird = new DeleteIRDefiner(model);
        break;
    case IRDefiner::dtype_refresh:
        ird = new RefreshIRDefiner(model);
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
 * Implemented with help of: http://www.qtcentre.org/threads/58307-Problem-when-adding-a-column-to-a-QAbstractProxyModel
 */
ProxyChoiceDecorModel::ProxyChoiceDecorModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , m_selectedRow(NOT_SETTED)
    , m_selectIcon(":/images/ok.png")
    , m_changedRows(new RowsChangesHolder)
{
    setSourceModel(new CustomSqlTableModel(this));
    /*
     * For insertion a new virtual column in this model you cannot use the QAbstractProxyModel::insertColumn(),
     * because this method insert new column in the source model too. Probably, the proper decision for insertion
     * new column is reimplement some virtual methods of the QAbstractProxyModel class.
     */
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
    if (index.column() == SELECT_ICON_COLUMN && index.row() == m_selectedRow && role == Qt::DecorationRole )
        data = m_selectIcon;
    else if (index.column() == SELECT_ICON_COLUMN /*&& (role != Qt::DecorationRole)*/)
        data = QVariant();
    else if ( role == Qt::BackgroundColorRole && m_changedRows->findChangeRow(index.row(), &changeType) ) {
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
    if (role == Qt::DecorationRole && index.column() == SELECT_ICON_COLUMN)
        m_selectedRow = index.row();
    else
        bSetted = sourceModel()->setData( mapToSource(index), value, role );
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
    cmmn::T_id id;
    const QVariant &varId = customSourceModel()->primaryIdInRow(m_selectedRow);
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, id), varId );
    return id;
}

QSize ProxyChoiceDecorModel::decorationSize() const
{
    auto sizes = m_selectIcon.availableSizes(QIcon::Normal, QIcon::Off);
    return sizes.empty() ? QSize() : sizes.first();
}

void ProxyChoiceDecorModel::slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected)
{
    QItemSelectionModel *selectModel = qobject_cast<QItemSelectionModel *>(sender());
    const QModelIndexList &deselectedList = deselected.indexes();

    // catch a deselection of the first left item in current row and setting icons decoration on it
    if (deselectedList.size() == 1) {
        // operate special case in Windows XP, Qt ver. 5.3.0. Normal case - in this place no one item must be selected.
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
    if (deselectedList.size() > 1)
        updatePrevDeselected(deselectedList);
    selectModel->select(selected.indexes().first(), QItemSelectionModel::Deselect); // this make recursive calling of this slot
}

void ProxyChoiceDecorModel::updatePrevDeselected(const QModelIndexList &deselectList)
{
    // update the first left items in the previous selected row for clearing icons decoration
    QModelIndex someDeselected = deselectList.first();
    QModelIndex firstDeselected = someDeselected.model()->index(someDeselected.row(), SELECT_ICON_COLUMN);
    emit sigNeedUpdateView(firstDeselected); // clear remained icons decoration
}

void ProxyChoiceDecorModel::changeRow(int defType, int row)
{
    std::unique_ptr<IRDefiner> rowIdxDef( getIRDefiner((IRDefiner::DefinerType)defType, this) ); // create definer of index row
    emit sigChangeCurrentRow( rowIdxDef->define(row) ); // change row in connected view(-s)
    emit dataChanged( this->index(0, 0), this->index(this->rowCount() - 1, this->columnCount() - 1) ); // update connected view(-s)
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
    ASSERT_DBG( m_changedRows->addRow(rowInsert, RowsChangesHolder::chtype_insert),
                cmmn::MessageException::type_critical, QObject::tr("Error add row"),
                QObject::tr("Cannot add row change to the row changes storage"),
                QString("ProxyChoiceDecorModel::slotAddRow()") );
    changeRow(IRDefiner::dtype_insert, rowInsert);
    endInsertRows();
}

void ProxyChoiceDecorModel::slotDeleteRow()
{
    qDebug() << "DELETE row #" << m_selectedRow;
    int rowDelete = m_selectedRow;
    customSourceModel()->slotDeleteRowRecord(rowDelete);
    if ( m_changedRows->hasRowChange(rowDelete, RowsChangesHolder::chtype_insert) ) {
        // if current row has "insert change" -> delete change of the last row in the model
        // (deletion the model's just inserted row in fact delete row from model completely)
        ASSERT_DBG( m_changedRows->deleteRow(rowCount()),
                    cmmn::MessageException::type_critical, QObject::tr("Error delete row"),
                    QObject::tr("Cannot delete row change from the row changes storage"),
                    QString("ProxyChoiceDecorModel::slotDeleteRow()") );
    }
    else
        m_changedRows->addRow(rowDelete, RowsChangesHolder::chtype_delete); // save current row delete change
    changeRow(IRDefiner::dtype_delete, rowDelete);
}

void ProxyChoiceDecorModel::slotRefreshModel()
{
    customSourceModel()->slotRefreshTheModel();
    m_changedRows->clearRows();
    changeRow(IRDefiner::dtype_refresh);
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
    qDebug() << "The proxy model data with role #" << role << ":" << strData;
}

#include <QTableView>
#include <QLineEdit>
#include <QDebug>
#include "proxy_decor_model.h"
#include "../src_sql/custom_sql_table_model.h"
#include "changes_model.h"

/*
 * ProxyDecorModel
 * Implemented with help of: http://www.qtcentre.org/threads/58307-Problem-when-adding-a-column-to-a-QAbstractProxyModel
 */
ProxyDecorModel::ProxyDecorModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , m_selectedId(NOT_SETTED)
    , m_selectIcon(":/images/ok.png")
    , m_changedRows(new RowsChangesHolder)
    , m_colors { QColor(0, 110, 0, 120), /* transperent green */
                 QColor(255, 0, 0, 120)  /* transperent red */ }
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
            data = m_colors[clrIdx_insert];
        else if (changeType == RowsChangesHolder::chtype_delete)
            data = m_colors[clrIdx_delete];
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
//        qDebug() << "proxy model setData(), ELSE case: [" << index.row() << "," << index.column()
//                 << "], role =" << role << ", data:" << value.toString();
        customSourceModel()->spike1_turnOn();
        bSetted = sourceModel()->setData( mapToSource(index), value, role );
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
    customSourceModel()->slotInsertRecord();
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
    customSourceModel()->slotDeleteRecord(currentRow);
    IRDefiner::DefinerType defType;
    if ( m_changedRows->hasRowChange(currentRow, RowsChangesHolder::chtype_insert) ) {
        // if row has "insert change" -> delete change of the last row in the model
        // (deletion the model's just inserted row in fact delete row from model completely - Qt MVC-pattern behaviour)
        ASSERT_DBG( m_changedRows->deleteChange(rowCount()),
                    cmmn::MessageException::type_critical, QObject::tr("Error delete row"),
                    QObject::tr("Cannot delete row change from the row changes storage"),
                    QString("ProxyDecorModel::slotDeleteRow") );
        defType = IRDefiner::dtype_deleteInserted;
    }
    else {
        // if row is existent in the DB (isn't just inserted) -> save it index as deleted and define appropriate type of row index definer
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
    customSourceModel()->slotRefreshModel();
    // definition - is or not the current row a new inserted row
    IRDefiner::DefinerType defType = m_changedRows->hasRowChange(currentRow, RowsChangesHolder::chtype_insert)
            ? IRDefiner::dtype_refreshInserted : IRDefiner::dtype_refreshExistent;
    clearDirtyChanges(); // must be after definition - has current row the "insert change"
    qDebug() << "[proxy model] data REFRESHed in the model";
    changeRow(defType, currentRow);
}

void ProxyDecorModel::slotSaveDataToDB(int currentRow)
{
    customSourceModel()->slotSaveInDB();
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


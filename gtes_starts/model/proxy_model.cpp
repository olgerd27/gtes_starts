#include <QSet>
#include <QDebug>
#include "proxy_model.h"
#include "custom_sql_table_model.h"

/*
 * RowsNumbers - the rows numbers storage
 */
class RowsNumbers
{
public:
    typedef int T_rowNumber;
    bool addRow(T_rowNumber row);
    bool deleteRow(T_rowNumber row);
    void clearRows();
    bool hasRow(T_rowNumber row) const;
private:
    QSet<T_rowNumber> m_deletedRows;
};

bool RowsNumbers::addRow(RowsNumbers::T_rowNumber row)
{
    bool isDeleted = hasRow(row);
    if (!isDeleted) m_deletedRows.insert(row);
    return isDeleted;
}

bool RowsNumbers::deleteRow(RowsNumbers::T_rowNumber row)
{
    return m_deletedRows.remove(row);
}

void RowsNumbers::clearRows()
{
    m_deletedRows.clear();
}

bool RowsNumbers::hasRow(RowsNumbers::T_rowNumber row) const
{
    return m_deletedRows.contains(row);
}

/*
 * Implemented with help of: http://www.qtcentre.org/threads/58307-Problem-when-adding-a-column-to-a-QAbstractProxyModel
 */
ProxyChoiceDecorModel::ProxyChoiceDecorModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , m_selectedRow(NOT_SETTED)
    , m_selectIcon(":/images/ok.png")
    , m_deletedRows(new RowsNumbers)
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
    if (index.column() == SELECT_ICON_COLUMN && index.row() == m_selectedRow && role == Qt::DecorationRole )
        data = m_selectIcon;
    else if (index.column() == SELECT_ICON_COLUMN /*&& (role != Qt::DecorationRole)*/)
        data = QVariant();
    else if ( role == Qt::BackgroundColorRole && m_deletedRows->hasRow(index.row()) )
        data = QColor(230, 183, 172, 255); // light red background
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
    return m_deletedRows->hasRow(index.row())
            ? QAbstractProxyModel::flags(index) & ~Qt::ItemIsEnabled // delete row - make it disabled
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
    int rowInsert = this->columnCount();
    beginInsertRows(QModelIndex(), rowInsert, rowInsert);
    this->customSourceModel()->slotInsertToTheModel();
    endInsertRows();
}

void ProxyChoiceDecorModel::slotDeleteRow()
{
    int deletedRow = m_selectedRow;
    this->customSourceModel()->slotDeleteFromTheModel(deletedRow);
    m_deletedRows->addRow(deletedRow);
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

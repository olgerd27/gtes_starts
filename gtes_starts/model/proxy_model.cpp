#include <QDebug>
#include "proxy_model.h"
#include "custom_sql_table_model.h"

ProxySqlModel::ProxySqlModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_selectedRow(NOT_SETTED)
    , m_selectIcon(":/images/ok.png")
{
    setSourceModel(new CustomSqlTableModel(this));
    /*
     * For insertion a new virtual column in this model you cannot use the QSortFilterProxyModel::insertColumn(),
     * because this method insert new column in the source model too. Probably, the proper decision for insertion
     * new column is reimplement some virtual methods of the QSortFilterProxyModel class.
     */
}

void ProxySqlModel::setSqlTable(const QString &tableName)
{
    customSourceModel()->setTable(tableName);
    printBaseHeader();
    printHeader();
    printBaseData();
    printData();
}

QVariant ProxySqlModel::data(const QModelIndex &index, int role) const
{
    QVariant data;
    if (index.column() == SELECT_ICON_COLUMN && index.row() == m_selectedRow && role == Qt::DecorationRole ) {
        data = m_selectIcon;
//        qDebug() << "data(), SELECT_ICON_COLUMN, decoration, [" << index.row() << "," << index.column() << "], selected row =" << m_selectedRow;
    }
    else if (index.column() == SELECT_ICON_COLUMN /*&& (role != Qt::DecorationRole)*/) {
        data = QVariant();
//        qDebug() << "data(), SELECT_ICON_COLUMN, NOT decoration, [" << index.row() << "," << index.column() << "], role =" << role;
    }
//    else if (index.column() == SELECT_ICON_COLUMN && (role != Qt::DisplayRole && role != Qt::EditRole)) {
        // TODO: maybe delete this case? Maybe this case works in the "else" section below?
//        data = QSortFilterProxyModel::data(index, role);
//        qDebug() << "data(), SELECT_ICON_COLUMN, NOT display AND edit, [" << index.row() << "," << index.column()
//                 << "], role =" << role << ", data:" << data;
//    }
    else if ( role == Qt::TextAlignmentRole && this->data(index, Qt::DisplayRole).convert(QMetaType::Float) ) {
        data = Qt::AlignCenter; // center alignment of the numerical values
    }
    else if (index.column() > SELECT_ICON_COLUMN) {
        data = QSortFilterProxyModel::data( this->index(index.row(), index.column() - 1), role );
//        data = sourceModel()->data( mapToSource(index), role ); // using sourceModel::data() for getting data is like spike))) - use QSortFilterProxyModel::data()
        if (role == Qt::DisplayRole && index.column() >= 6)
            qDebug() << "data(), NOT SELECT_ICON_COLUMN, [" << index.row() << "," << index.column() << "], role =" << role << ", data:" << data.toString();
    }
    else {
        data = QSortFilterProxyModel::data( this->index(index.row(), index.column() - COUNT_ADDED_COLUMNS), role);
//        data = QSortFilterProxyModel::index(index.row(), index.column() + COUNT_ADDED_COLUMNS).data(role);
//        qDebug() << "data(), else, [" << index.row() << "," << index.column() << "], role =" << role << ", data:" << data;
    }
    return data;
}

bool ProxySqlModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool bSetted = false;
    if (role == Qt::DecorationRole && index.column() == SELECT_ICON_COLUMN) {
        m_selectedRow = index.row();
        qDebug() << "setData(), decoration role, [" << index.row() << "," << index.column() << "], selected row =" << m_selectedRow;
    }
    else {
        bSetted = QSortFilterProxyModel::setData( this->index( index.row(), index.column() - COUNT_ADDED_COLUMNS ), value, role );
    }
    return bSetted;
}

int ProxySqlModel::columnCount(const QModelIndex &parent) const
{
    return QSortFilterProxyModel::columnCount(parent) + COUNT_ADDED_COLUMNS;
}

Qt::ItemFlags ProxySqlModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
//    qDebug() << "flags to [" << index.row() << "," << index.column() << "]";
//    return index.column() == SELECT_ICON_COLUMN
//            ? Qt::ItemIsEnabled | Qt::ItemIsSelectable
//            : QSortFilterProxyModel::flags(
//                    QSortFilterProxyModel::index(index.row(), index.column() - COUNT_ADDED_COLUMNS) )
//                | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ProxySqlModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant data;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        data = ( section == SELECT_ICON_COLUMN
                 ? QVariant("SELECT")
//                 : sourceModel()->headerData(section - 1, orientation, role) ); // QSortFilterProxyModel::headerData() don't work properly
                 : QSortFilterProxyModel::headerData(section - COUNT_ADDED_COLUMNS, orientation, role) );
//        qDebug() << "headerData(), section:" << section << ", orientation: Horizontal, role:" << role
//                 << ", source model data:" << sourceModel()->headerData(section, orientation, role).toString()
//                 << ", proxy data:" << data.toString();
    }
    return data;
}

//QModelIndex ProxySqlModel::index(int row, int column, const QModelIndex &parent) const
//{
//    if (row < 0 || row > rowCount(parent) || column < 0 || column > columnCount(parent))
//        return QModelIndex();
//    qDebug() << "index() [" << row << "," << column << "]";
//    return column == SELECT_ICON_COLUMN
//            ? createIndex(row, column)
//            : QSortFilterProxyModel::index(row, column - COUNT_ADDED_COLUMNS);
////    return QSortFilterProxyModel::index(row, column, parent);
//}

//QModelIndex ProxySqlModel::parent(const QModelIndex &child) const
//{
//    return QModelIndex();
//}

//QModelIndex ProxySqlModel::mapToSource(const QModelIndex &proxyIndex) const
//{
//    if (!proxyIndex.isValid()) return QModelIndex();
////    QModelIndex srcIndex = (proxyIndex.column() == SELECT_ICON_COLUMN
////                            ? QModelIndex()
////                            : sourceModel()->index( proxyIndex.row(), proxyIndex.column() - COUNT_ADDED_COLUMNS ) );
////    qDebug() << "mapToSource: proxy: [" << proxyIndex.row() << "," << proxyIndex.column() << "] data:" << proxyIndex.data(Qt::DisplayRole)
////             << ", source: [" << srcIndex.row() << "," << srcIndex.column() << "] data:" << srcIndex.data(Qt::DisplayRole);

////    if (proxyIndex.column() >= 6)
////        qDebug() << "mapToSource(), col =" << proxyIndex.column()
////                 << ", columnCount base =" << QSortFilterProxyModel::columnCount(proxyIndex.parent())
////                 << " derived =" << columnCount(proxyIndex.parent());

//    return proxyIndex.column() == SELECT_ICON_COLUMN
//            ? QModelIndex()
//            : sourceModel()->index( proxyIndex.row(), proxyIndex.column() ); // TODO: replace to sourceModel() method
//}

//QModelIndex ProxySqlModel::mapFromSource(const QModelIndex &sourceIndex) const
//{
//    if (!sourceIndex.isValid()) return QModelIndex();
////    if (sourceIndex.column() >= 6) qDebug() << "mapFromSource(), col =" << sourceIndex.column();
//    return index( sourceIndex.row(), sourceIndex.column() + COUNT_ADDED_COLUMNS );
//}

/* Get the pointer to the custom sql source model */
CustomSqlTableModel *ProxySqlModel::customSourceModel() const
{
    CustomSqlTableModel *srcModel = qobject_cast<CustomSqlTableModel *>(sourceModel());
    ASSERT_DBG( srcModel,
                cmmn::MessageException::type_critical, QObject::tr("The error with getting the custom source model"),
                QObject::tr("Unknow source model was setted to the proxy model"),
                QString("ProxySqlModel::customSourceModel()") );
    return srcModel;
}

cmmn::T_id ProxySqlModel::selectedId() const
{
    cmmn::T_id id;
    const QVariant &varId = customSourceModel()->primaryIdInRow(m_selectedRow);
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, id), varId );
    return id;
}

void ProxySqlModel::slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected)
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
    selectModel->select(selected.indexes().first(), QItemSelectionModel::Deselect); // this make recursive calling of this slot
}

// PRINTING
void ProxySqlModel::printBaseHeader(int role) const
{
    QString strData = "\n";
    for (int sect = 0; sect < QSortFilterProxyModel::columnCount(); ++sect) {
        strData += QString("|") += QSortFilterProxyModel::headerData(sect, Qt::Horizontal, role).toString() += QString("| ");
    }
    qDebug() << "Horizontal base header data with role #" << role << ":" << strData;
}

void ProxySqlModel::printHeader(int role) const
{
    QString strData = "\n";
    for (int sect = 0; sect < columnCount(); ++sect) {
        strData += QString("|") += headerData(sect, Qt::Horizontal, role).toString() += QString("| ");
    }
    qDebug() << "Horizontal header data with role #" << role << ":" << strData;
}

void ProxySqlModel::printBaseData(int role) const
{
    QString strData = "\n";
    for(int row = 0; row < QSortFilterProxyModel::rowCount(); ++row) {
        for(int col = 0; col < QSortFilterProxyModel::columnCount(); ++col ) {
            const QModelIndex &index = QSortFilterProxyModel::index(row, col);
            strData += QString("|%1|  ").arg(QSortFilterProxyModel::data(index, role).toString());
        }
        strData += "\n";
    }
    qDebug() << "The base proxy model data with role #" << role << ":" << strData;
}

void ProxySqlModel::printData(int role) const
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

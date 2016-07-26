#include <QDebug>
#include "proxy_model.h"
#include "custom_sql_table_model.h"

/*
 * Implemented with help of: http://www.qtcentre.org/threads/58307-Problem-when-adding-a-column-to-a-QAbstractProxyModel
 */
ProxySqlModel::ProxySqlModel(QObject *parent)
    : QAbstractProxyModel(parent)
    , m_selectedRow(NOT_SETTED)
    , m_selectIcon(":/images/ok.png")
{
    setSourceModel(new CustomSqlTableModel(this));
    /*
     * For insertion a new virtual column in this model you cannot use the QAbstractProxyModel::insertColumn(),
     * because this method insert new column in the source model too. Probably, the proper decision for insertion
     * new column is reimplement some virtual methods of the QAbstractProxyModel class.
     */
}

void ProxySqlModel::setSqlTable(const QString &tableName)
{
    customSourceModel()->setTable(tableName);
//    printBaseHeader();
    printHeader();
//    printBaseData();
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
//        data = QAbstractProxyModel::data(index, role);
//        qDebug() << "data(), SELECT_ICON_COLUMN, NOT display AND edit, [" << index.row() << "," << index.column()
//                 << "], role =" << role << ", data:" << data;
//    }
    else if ( role == Qt::TextAlignmentRole && this->data(index, Qt::DisplayRole).convert(QMetaType::Float) ) {
        data = Qt::AlignCenter; // center alignment of the numerical values
    }
    else if (index.column() > SELECT_ICON_COLUMN) {
        data = sourceModel()->data(mapToSource(index), role);
//        data = QAbstractProxyModel::data( this->index(index.row(), index.column() - COUNT_ADDED_COLUMNS), role );
//        data = sourceModel()->data( mapToSource(index), role ); // using sourceModel::data() for getting data is like spike))) - use QAbstractProxyModel::data()
//        if (role == Qt::DisplayRole && index.column() >= 6)
//            qDebug() << "data(), NOT SELECT_ICON_COLUMN, [" << index.row() << "," << index.column() << "], role =" << role << ", data:" << data.toString();
    }
    else {
        data = sourceModel()->data(mapToSource(index), role);
//        data = QAbstractProxyModel::index(index.row(), index.column() - COUNT_ADDED_COLUMNS).data(role);
//        qDebug() << "data(), else, [" << index.row() << "," << index.column() << "], role =" << role << ", data:" << data;
    }
    return data;
}

bool ProxySqlModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool bSetted = false;
    if (role == Qt::DecorationRole && index.column() == SELECT_ICON_COLUMN) {
        m_selectedRow = index.row();
        qDebug() << "proxy setData(), decoration role, [" << index.row() << "," << index.column() << "], selected row =" << m_selectedRow;
    }
    else {
        bSetted = sourceModel()->setData( mapToSource(index), value, role );
    }
    if (bSetted) emit dataChanged(index, index);
    return bSetted;
}

int ProxySqlModel::columnCount(const QModelIndex &/*parent*/) const
{
    return sourceModel()->columnCount() + COUNT_ADDED_COLUMNS;
}

int ProxySqlModel::rowCount(const QModelIndex &/*parent*/) const
{
    return sourceModel()->rowCount();
}

Qt::ItemFlags ProxySqlModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
//    qDebug() << "flags to [" << index.row() << "," << index.column() << "]";
//    return index.column() == SELECT_ICON_COLUMN
//            ? Qt::ItemIsEnabled | Qt::ItemIsSelectable
//            : QAbstractProxyModel::flags(
//                    QAbstractProxyModel::index(index.row(), index.column() - COUNT_ADDED_COLUMNS) )
//                | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant ProxySqlModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant data;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        data = ( section == SELECT_ICON_COLUMN
                 ? /*QVariant("SELECT")*/ QVariant("")
                 : sourceModel()->headerData(section - COUNT_ADDED_COLUMNS, orientation, role) ); // QAbstractProxyModel::headerData() don't work properly
//                 : QAbstractProxyModel::headerData(section - COUNT_ADDED_COLUMNS, orientation, role) );
//        qDebug() << "headerData(), section:" << section << ", orientation: Horizontal, role:" << role
//                 << ", source model data:" << sourceModel()->headerData(section, orientation, role).toString()
//                 << ", proxy data:" << data.toString();
    }
    return data;
}

QModelIndex ProxySqlModel::index(int row, int column, const QModelIndex &parent) const
{
    // version 1
    return (parent.isValid() || row < 0 || row >= rowCount(parent) || column < 0 || column >= columnCount(parent))
            ? QModelIndex() : createIndex( row, column );

    // version 2
//    return column == SELECT_ICON_COLUMN
//            ? createIndex(row, column)
//            : QAbstractProxyModel::index(row, column - COUNT_ADDED_COLUMNS);
//    return QAbstractProxyModel::index(row, column, parent);

    // version 3 - from inet - http://www.qtcentre.org/threads/58307-Problem-when-adding-a-column-to-a-QAbstractProxyModel
//    QModelIndex idxRes;
//    if (!parent.isValid())
//        idxRes = createIndex(row, column);
//    return idxRes;
}

QModelIndex ProxySqlModel::parent(const QModelIndex &/*child*/) const
{
    return QModelIndex();
}

QModelIndex ProxySqlModel::mapFromSource(const QModelIndex &sourceIndex) const
{
    if (!sourceIndex.isValid()) return QModelIndex();
//    if (sourceIndex.column() >= 6) qDebug() << "mapFromSource(), col =" << sourceIndex.column();
//    return index( sourceIndex.row(), sourceIndex.column() + COUNT_ADDED_COLUMNS );

    //    return createIndex( sourceIndex.row(), sourceIndex.column(), sourceIndex.internalId() ); // work version 1
    return index( sourceIndex.row(), sourceIndex.column() + COUNT_ADDED_COLUMNS ); // work version 2
}

QModelIndex ProxySqlModel::mapToSource(const QModelIndex &proxyIndex) const
{
    // version 1 - work is not correct, header: |SELECT| |0| |Id| ...
//    if (!proxyIndex.isValid() || proxyIndex.column() == SELECT_ICON_COLUMN) return QModelIndex();

//    const QModelIndex &sourceIdx = sourceModel()->index(proxyIndex.row(), proxyIndex.column() - COUNT_ADDED_COLUMNS);
//    qDebug() << "mapToSource(), proxy: [" << proxyIndex.row() << "," << proxyIndex.column() << "] data:" << proxyIndex.data(Qt::DisplayRole).toString()
//             << "| source: [" << sourceIdx.row() << "," << sourceIdx.column() << "] data:" << sourceIdx.data(Qt::DisplayRole).toString();
//    return sourceIdx;

    // version 2
    if ( !proxyIndex.isValid() || proxyIndex.column() == SELECT_ICON_COLUMN ) return QModelIndex();
    return sourceModel()->index( proxyIndex.row(), proxyIndex.column() - COUNT_ADDED_COLUMNS ); // work version 2
//    return createIndex( proxyIndex.row(), proxyIndex.column() - COUNT_ADDED_COLUMNS, sourceIdx.internalId() );
}

QItemSelection ProxySqlModel::mapSelectionFromSource(const QItemSelection &selection) const
{
    const QModelIndexList &srcSelectIndexes = selection.indexes();
    qDebug() << "mapSelectionFromSource(), indexes:";
    for (const QModelIndex &idx : srcSelectIndexes)
        qDebug() << "[" << idx.row() << "," << idx.column() << "] data:" << idx.data(Qt::DisplayRole).toString();
    return selection;

    //    return QItemSelection();
}

QItemSelection ProxySqlModel::mapSelectionToSource(const QItemSelection &selection) const
{
    qDebug() << "mapSelectionToSource()";
    return selection;
}

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
    const QModelIndexList &deselectedList = deselected.indexes();

    qDebug() << "1";

    // catch a deselection of the first left item in current row and setting icons decoration on it
    if (deselectedList.size() == 1) {

        qDebug() << "2";

        const QModelIndex &indexDecor = index( deselectedList.first().row(), SELECT_ICON_COLUMN );
        qDebug() << "|DEselected items list(" << deselectedList.size() << "):";
        foreach (auto item, deselectedList) {
            qDebug() << "- [" << item.row() << "," << item.column() << "]";
        }
        qDebug() << "|selected items list (" << selected.indexes().size() << "):";
        foreach (auto item, selected.indexes()) {
            qDebug() << "- [" << item.row() << "," << item.column() << "]";
        }

        qDebug() << "set decoration to [" << indexDecor.row() << "," << indexDecor.column() << "]";
        setData(indexDecor, QVariant(), Qt::DecorationRole);

        qDebug() << "3";

//        setData( index(deselectedList.first().row(), SELECT_ICON_COLUMN), QVariant(), Qt::DecorationRole );
        return;
    }

    qDebug() << "4";

    // update the first left items in the previous selected row for clearing icons decoration
    if (deselectedList.size() > 1) {

        qDebug() << "5";

        qDebug() << "Selected items list SIZE > 1";
        QModelIndex someDeselected = deselectedList.first();
        QModelIndex firstDeselected = someDeselected.model()->index(someDeselected.row(), SELECT_ICON_COLUMN);

        qDebug() << "6";

        emit sigNeedUpdateView(firstDeselected); // clear remained icons decoration

        qDebug() << "7";
    }

    qDebug() << "8";

    const QModelIndexList &selectedList = selected.indexes();
    qDebug() << "Selected items list:";
    foreach (auto item, selectedList) {
        qDebug() << "- [" << item.row() << "," << item.column() << "]";
    }

    qDebug() << "9";

    selectModel->select(selected.indexes().first(), QItemSelectionModel::Deselect); // this make recursive calling of this slot

    qDebug() << "10";
}

// PRINTING
//void ProxySqlModel::printBaseHeader(int role) const
//{
//    QString strData = "\n";
//    for (int sect = 0; sect < QAbstractProxyModel::columnCount(); ++sect) {
//        strData += QString("|") += QAbstractProxyModel::headerData(sect, Qt::Horizontal, role).toString() += QString("| ");
//    }
//    qDebug() << "Horizontal base header data with role #" << role << ":" << strData;
//}

void ProxySqlModel::printHeader(int role) const
{
    QString strData = "\n";
    for (int sect = 0; sect < columnCount(); ++sect) {
        strData += QString("|") += headerData(sect, Qt::Horizontal, role).toString() += QString("| ");
    }
    qDebug() << "Horizontal header data with role #" << role << ":" << strData;
}

//void ProxySqlModel::printBaseData(int role) const
//{
//    QString strData = "\n";
//    for(int row = 0; row < QAbstractProxyModel::rowCount(); ++row) {
//        for(int col = 0; col < QAbstractProxyModel::columnCount(); ++col ) {
//            const QModelIndex &index = QAbstractProxyModel::index(row, col);
//            strData += QString("|%1|  ").arg(QAbstractProxyModel::data(index, role).toString());
//        }
//        strData += "\n";
//    }
//    qDebug() << "The base proxy model data with role #" << role << ":" << strData;
//}

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

/*
 * From: http://www.sql.ru/forum/963842/kak-dobavit-dopolnitelnuu-stroku-v-qsortfilterproxymodel
 */
//TreeModelProxy::TreeModelProxy(QObject *parent):QAbstractProxyModel(parent)
//{

//}
//QModelIndex TreeModelProxy::mapFromSource( const QModelIndex & sourceIndex ) const
//{
//  if ( !sourceIndex.isValid())
//  {
//	  return QModelIndex();
//  }
//  return createIndex( sourceIndex.row(), sourceIndex.column(), sourceIndex.internalPointer() );
//}
//QModelIndex TreeModelProxy::mapToSource( const QModelIndex &proxyIndex ) const
//{
//  return sourceModel()->index(proxyIndex.row(),proxyIndex.column());
//}
//QModelIndex TreeModelProxy::index( int row, int col, const QModelIndex& index ) const
//{
//	if(index.isValid())
//	{
//		int count = sourceModel()->rowCount(index);
//		if (row==count)
//		{
//			return createIndex( index.row(), index.column(), index.internalPointer() );
//		}
//		else
//		{
//			//return sourceModel()->index( row, col, mapToSource(index.parent()) );???
//		}
//	}
//    return mapFromSource(sourceModel()->index( row, col, mapToSource(index) ));
//}

//QModelIndex TreeModelProxy::parent( const QModelIndex& index ) const
//{
//    return mapFromSource(sourceModel()->parent( mapToSource(index) ));
//}

//int TreeModelProxy::rowCount(const QModelIndex &parent) const
//{
//	int count = sourceModel()->rowCount(parent);
//	if(parent.isValid())
//		count+=1;
//	return count;
//}
//int TreeModelProxy::columnCount(const QModelIndex &parent) const
//{
//	return sourceModel()->columnCount(parent);
//}

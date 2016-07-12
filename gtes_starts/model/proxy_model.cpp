#include <QDebug>
#include "proxy_model.h"
#include "custom_sql_table_model.h"

ProxySqlModel::ProxySqlModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_selectedRow(NOT_SETTED)
    , m_selectIcon(":/images/ok.png")
{
    setSourceModel(new CustomSqlTableModel(this));
}

QVariant ProxySqlModel::data(const QModelIndex &index, int role) const
{
    QVariant data;
    if (index.column() == SELECT_ICON_COLUMN && (role == Qt::DisplayRole || role == Qt::EditRole)) {
        data = QVariant();
//        qDebug() << "data(), SELECT_ICON_COLUMN, display OR edit, [" << index.row() << "," << index.column() << "], role =" << role;
    }
    else if (index.column() == SELECT_ICON_COLUMN && (role != Qt::DisplayRole && role != Qt::EditRole)) {
        data = QSortFilterProxyModel::data(index, role);
//        qDebug() << "data(), SELECT_ICON_COLUMN, NOT display AND edit, [" << index.row() << "," << index.column()
//                 << "], role =" << role << ", data:" << data;
    }
    else if (index.column() != SELECT_ICON_COLUMN) {
        data = QSortFilterProxyModel::data( this->index( index.row(), baseColumn(index.column()) ), role );
//        qDebug() << "data(), NOT SELECT_ICON_COLUMN, [" << index.row() << "," << index.column() << "], role =" << role << ", data:" << data;
    }
    else if (index.column() == SELECT_ICON_COLUMN && index.row() == m_selectedRow && role == Qt::DecorationRole ) {
        data = m_selectIcon;
//        qDebug() << "data(), decoration role, [" << index.row() << "," << index.column() << "], selected row =" << m_selectedRow;
    }
    else if (role == Qt::TextAlignmentRole) {
        if ( this->data(index, Qt::DisplayRole).convert(QMetaType::Float) )
            data = Qt::AlignCenter; // center alignment of the numerical values
    }
    else {
        data = QSortFilterProxyModel::data( this->index(index.row(), baseColumn(index.column()) ), role);
//        qDebug() << "data(), else, [" << index.row() << "," << index.column() << "], role =" << role << ", data:" << data;
    }
    return data;
}

bool ProxySqlModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    bool bSetted = false;
    if (role == Qt::DecorationRole && index.column() == SELECT_ICON_COLUMN) {
        m_selectedRow = index.row();
//        qDebug() << "setData(), decoration role, [" << index.row() << "," << index.column() << "], selected row =" << m_selectedRow;
    }
    else
        bSetted = QSortFilterProxyModel::setData( this->index( index.row(), baseColumn(index.column()) ), value, role );
    return bSetted;
}

int ProxySqlModel::columnCount(const QModelIndex &parent) const
{
    return QSortFilterProxyModel::columnCount(parent) + 1;
//    return QSortFilterProxyModel::columnCount(parent);
}

QVariant ProxySqlModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant data;
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        data = ( section == SELECT_ICON_COLUMN ? QVariant("SELECT") : QSortFilterProxyModel::headerData(baseColumn(section), orientation, role) );
//        qDebug() << "headerData(), section:" << section << ", orientation: Horizontal"
//                 << ", base data:" << QSortFilterProxyModel::headerData(section, orientation, role).toString()
//                 << ", source model data:" << sourceModel()->headerData(section, orientation, role).toString()
//                 << ", data:" << data.toString();
    }
    return data;
//    return (orientation == Qt::Horizontal && section != SELECT_ICON_COLUMN)
//            ? QSortFilterProxyModel::headerData(baseColumn(section), orientation, role)
//            : QVariant();
//        return QSortFilterProxyModel::headerData(section, orientation, role);
}

Qt::ItemFlags ProxySqlModel::flags(const QModelIndex &index) const
{
//    return QSortFilterProxyModel::flags(index);
    return ( index.column() == SELECT_ICON_COLUMN
             ? Qt::ItemIsEnabled | Qt::ItemIsSelectable
             : QSortFilterProxyModel::flags( this->index( index.row(), baseColumn(index.column()) ) ) | Qt::ItemIsEnabled | Qt::ItemIsSelectable );
}

QModelIndex ProxySqlModel::parent(const QModelIndex &child) const
{
    return QModelIndex();
}

QModelIndex ProxySqlModel::index(int row, int column, const QModelIndex &parent) const
{
//    return ( column == SELECT_ICON_COLUMN
//             ? this->createIndex(row, column)
//             : QSortFilterProxyModel::index(row, column) );

    // this make crash with help of the ASSERT calling
//    if (row < rowCount() && column == SELECT_ICON_COLUMN)
//        return createIndex(row, column);

    return QSortFilterProxyModel::index(row, baseColumn(column), parent);
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

int ProxySqlModel::baseColumn(int col) const
{
    return col - COUNT_ADDED_COLUMNS;
}

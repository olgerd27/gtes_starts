#include "proxy_filter_model.h"
#include "proxy_decor_model.h"
#include "custom_sql_table_model.h"
#include "selection_allower.h"

/*
 * ProxyFilterModel
 */
ProxyFilterModel::ProxyFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , m_decorPrxModel(new ProxyDecorModel(parent))
    , m_selectAllow(new SelectionAllower(this)) // set default SelectionAllower
{
    setSourceModel(m_decorPrxModel);
    connect(m_decorPrxModel, SIGNAL(sigChangeCurrentRow(int)), this, SIGNAL(sigChangeCurrentRow(int)));
}

ProxyFilterModel::~ProxyFilterModel()
{
    delete m_decorPrxModel;
}

void ProxyFilterModel::setSqlTableName(const QString &tableName)
{
    m_decorPrxModel->setSqlTableName(tableName);
}

void ProxyFilterModel::setSelectionAllower(SelectionAllower *sa)
{
    m_selectAllow.reset(sa);
}

CustomSqlTableModel *ProxyFilterModel::customSourceModel() const
{
    CustomSqlTableModel *srcModel = qobject_cast<CustomSqlTableModel *>(m_decorPrxModel->sourceModel());
    ASSERT_DBG( srcModel,
                cmmn::MessageException::type_critical, QObject::tr("Error get the custom source model"),
                QObject::tr("Unknow source model was setted to the proxy filter model"),
                QString("ProxyFilterModel::customSourceModel") );
    return srcModel;
}

cmmn::T_id ProxyFilterModel::selectedId() const
{
    return m_decorPrxModel->selectedId();
}

void ProxyFilterModel::slotAddRow()
{
    m_decorPrxModel->slotAddRow();
}

void ProxyFilterModel::slotDeleteRow(int currentRow)
{
    m_decorPrxModel->slotDeleteRow(currentRow);
}

void ProxyFilterModel::slotRefreshModel(int currentRow)
{
    m_decorPrxModel->slotRefreshModel(currentRow);
}

void ProxyFilterModel::slotSaveDataToDB(int currentRow)
{
    m_decorPrxModel->slotSaveDataToDB(currentRow);
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

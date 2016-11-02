#ifndef PROXY_FILTER_MODEL_H
#define PROXY_FILTER_MODEL_H

#include <memory>
#include <QSortFilterProxyModel>
#include "../common/common_defines.h"

/*
 * The proxy model that performs filtering of model data.
 * This proxy model contains and operates the ProxyDecorModel instance.
 * If it was created this class instance, it wasn't need create the ProxyDecorModel instance.
 */
class ProxyDecorModel;
class SelectionAllower;
class CustomSqlTableModel;
class ProxyFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ProxyFilterModel(QObject *parent = 0);
    ~ProxyFilterModel();
    void setSqlTableName(const QString &tableName);
    void setSelectionAllower(SelectionAllower *sa);

    CustomSqlTableModel * customSourceModel() const; // get pointer to the custom sql source model
    cmmn::T_id selectedId() const;

signals:
    void sigSelectionEnded();
    void sigChangeCurrentRow(int row) const;

public slots:
    void slotAddRow();
    void slotDeleteRow(int currentRow);
    void slotRefreshModel(int currentRow);
    void slotSaveDataToDB(int currentRow);
    void slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected);

private:
    void updatePrevDeselected(const QModelIndexList &deselectList);

    ProxyDecorModel *m_decorPrxModel;
    std::unique_ptr<SelectionAllower> m_selectAllow;
};

#endif // PROXY_FILTER_MODEL_H

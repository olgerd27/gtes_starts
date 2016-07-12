#ifndef PROXY_MODEL_H
#define PROXY_MODEL_H

#include <QSortFilterProxyModel>
#include <QIcon>
#include "../common/common_defines.h"

class CustomSqlTableModel;

class ProxySqlModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    enum {
        SELECT_ICON_COLUMN = 0,
        COUNT_ADDED_COLUMNS // count of added columns
    };

    explicit ProxySqlModel(QObject *parent = 0);
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    int columnCount(const QModelIndex &parent) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QModelIndex parent(const QModelIndex &child) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    CustomSqlTableModel * customSourceModel() const;
    cmmn::T_id selectedId() const;

signals:
    void sigNeedUpdateView(const QModelIndex &index);

public slots:
    void slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected);

private:
    enum { NOT_SETTED = -1 };

    int baseColumn(int col) const;

    int m_selectedRow;
    QIcon m_selectIcon;
};

#endif // PROXY_MODEL_H

#ifndef PROXY_MODEL_H
#define PROXY_MODEL_H

#include <QAbstractProxyModel>
#include <QIcon>
#include "../common/common_defines.h"

class CustomSqlTableModel;

class ProxySqlModel : public QAbstractProxyModel
{
    Q_OBJECT
public:
    enum {
        SELECT_ICON_COLUMN = 0, // index of the inserted column
        COUNT_ADDED_COLUMNS // count of added columns
    };

    explicit ProxySqlModel(QObject *parent = 0);
    void setSqlTable(const QString &tableName);

    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    QModelIndex mapFromSource(const QModelIndex &sourceIndex) const;
    QModelIndex mapToSource(const QModelIndex &proxyIndex) const;
    QItemSelection mapSelectionFromSource(const QItemSelection &selection) const;
    QItemSelection mapSelectionToSource(const QItemSelection &selection) const;

    CustomSqlTableModel * customSourceModel() const;
    cmmn::T_id selectedId() const;

//    void printBaseData(int role = Qt::DisplayRole) const; // TODO: temporary function, delete later
    void printData(int role = Qt::DisplayRole) const; // TODO: temporary function, delete later
//    void printBaseHeader(int role = Qt::DisplayRole) const; // TODO: temporary function, delete later
    void printHeader(int role = Qt::DisplayRole) const; // TODO: temporary function, delete later

signals:
    void sigNeedUpdateView(const QModelIndex &index);

public slots:
    void slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected);

private:
    enum { NOT_SETTED = -1 };

    int m_selectedRow;
    QIcon m_selectIcon;
};

#endif // PROXY_MODEL_H

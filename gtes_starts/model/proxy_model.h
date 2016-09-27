#ifndef PROXY_MODEL_H
#define PROXY_MODEL_H

#include <memory>
#include <QAbstractProxyModel>
#include <QIcon>
#include <QSortFilterProxyModel>
#include "../common/common_defines.h"

/*
 * The proxy model that add the choice decoration (Ok icon) to the data model.
 */
class CustomSqlTableModel;
class RowsChangesHolder;
class ProxyChoiceDecorModel : public QAbstractProxyModel
{
    Q_OBJECT
    friend class IRDefiner;
public:
    enum {
        SELECT_ICON_COLUMN = 0, // index of the inserted column
        COUNT_ADDED_COLUMNS // count of added columns
    };

    explicit ProxyChoiceDecorModel(QObject *parent = 0);
    ~ProxyChoiceDecorModel();
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

    CustomSqlTableModel * customSourceModel() const;
    cmmn::T_id selectedId() const;
    cmmn::T_id rowId(int row) const;
    bool isDirty() const; // check - has the model any changes, don't saved in the DB
#ifdef __linux__
    QSize decorationSize() const;
#endif

    void printData(int role = Qt::DisplayRole) const; // TODO: temporary function, delete later
    void printHeader(int role = Qt::DisplayRole) const; // TODO: temporary function, delete later

signals:
    void sigChangeCurrentRow(int row) const;

public slots:
    void slotAddRow();
    void slotDeleteRow(int currentRow);
    void slotRefreshModel(int currentRow);
    void slotSaveDataToDB(int currentRow);
    void slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected);

private:
    enum { NOT_SETTED = -1 };

    void updatePrevDeselected(const QModelIndexList &deselectList);
    void changeRow(int defType, int row = NOT_SETTED);
    bool canDeleteRow(int row) const;

    int m_selectedId; // TODO: using selected Id value (instead of selected row) may allow don't use (maybe partial) the IRDefiner and it childs classes. Test it!
    QIcon m_selectIcon;
    std::unique_ptr<RowsChangesHolder> m_changedRows;
};

/*
 * ProxyFilterModel
 */
class SelectionAllower;
class ProxyFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ProxyFilterModel(QObject *parent = 0);
    ~ProxyFilterModel();
    void setSelectionAllower(SelectionAllower *sa);

signals:
    void sigSelectionEnded();

public slots:
    void slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected);

private:
    void updatePrevDeselected(const QModelIndexList &deselectList);

    std::unique_ptr<SelectionAllower> m_selectAllow;
};

#endif // PROXY_MODEL_H

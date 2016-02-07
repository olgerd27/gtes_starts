#ifndef DBT_EDITOR_H
#define DBT_EDITOR_H

#include <memory>
#include <QSqlTableModel>
#include <QIcon>
#include <QDialog>
#include <QItemSelection>
#include "../common/common_defines.h"

class QAbstractItemView;
class CustomSqlTableModel;
namespace Ui {
    class ComplexDBTEditor;
}
namespace dbi {
    class DBTInfo;
}

/*
 * The custom QSqlTableModel class, that add a new first column and show choice icon there for rows choosing.
 */
 // TODO: delete this model and not use anymore
class RowsChooseSqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    enum { SELECT_ICON_COLUMN = 0 };

    RowsChooseSqlTableModel(QObject *parent = 0);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &idx, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

    bool findPrimaryIdRow(const QVariant &idPrim, int &rRowValue);
    cmmn::T_id selectedId() const;

    void printData(int role = Qt::DisplayRole) const;

signals:
    void sigNeedUpdateView(const QModelIndex &index);

public slots:
    void slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected);

private:
    enum { DONT_SELECTED_ROW = -1 };

    int m_selectedRow;
    QIcon m_selectIcon;
};

/*
 * The base class for editing database tables (DBT)
 */
class DBTEditor : public QDialog
{
    Q_OBJECT
public:
    explicit DBTEditor(dbi::DBTInfo *dbTable, QWidget *parent = 0);
    virtual ~DBTEditor();

    bool selectInitial(const QVariant &idPrim);
    cmmn::T_id selectedId() const;

protected: // TODO: make private
    void setContentsUI();
    void setEditingUI();
    void setModel();
    void setWindowName();
    void setHeaderData();
    void makeSelect(int row);

    dbi::DBTInfo *m_DBTInfo;
    std::unique_ptr<Ui::ComplexDBTEditor> m_ui;
//    std::unique_ptr<RowsChooseSqlTableModel> m_model;
    std::unique_ptr<CustomSqlTableModel> m_model;
};

#endif // DBT_EDITOR_H

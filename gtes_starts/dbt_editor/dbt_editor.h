#ifndef DBT_EDITOR_H
#define DBT_EDITOR_H

#include <QSqlTableModel>
#include <QIcon>
#include <QDialog>

namespace dbi {
    class DBTInfo;
}

/*
 * The custom QSqlTableModel class, that add a new first column and show choice icon there for rows choosing.
 */
class RowsChooseSqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    RowsChooseSqlTableModel(QObject *parent = 0);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &idx, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    int columnCount(const QModelIndex &parent) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;

private:
    enum { DONT_SELECTED_ROW = -1 };
    enum { SELECT_ICON_COLUMN = 0 };

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
    typedef int T_id;

    enum ColumnNumbers {
          col_empty = 0
        , col_id
        , col_firstWithData
    };

    explicit DBTEditor(dbi::DBTInfo *dbTable, QWidget *parent = 0);
    virtual ~DBTEditor();

    bool selectInitial(const QVariant &value, DBTEditor::ColumnNumbers compareCol);
    T_id selectedId() const;

protected:
    void setWindowName();
    virtual void makeSelect(int row) = 0;

    dbi::DBTInfo *m_DBTInfo;
    RowsChooseSqlTableModel *m_model;
    T_id m_selectedId;
};

#endif // DBT_EDITOR_H
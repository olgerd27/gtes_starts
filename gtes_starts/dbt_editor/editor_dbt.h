#ifndef EDITER_DBT_H
#define EDITER_DBT_H

#include <QSqlTableModel>
#include <QIcon>
#include <QDialog>

class DBTInfo;

/*
 * The custom QSqlTableModel class, that remove unnecessary characters from the end of every data item.
 * (Return "trimmed" data).
 */
class CustomSqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    CustomSqlTableModel(QObject *parent = 0);
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
    explicit DBTEditor(DBTInfo *dbTable, QWidget *parent = 0);
    virtual ~DBTEditor();

    QString identificationData() const;

protected:
    void setWindowName();

    DBTInfo *m_DBTInfo;
    CustomSqlTableModel *m_model;
    QString m_identificationData;
};

#endif // EDITER_DBT_H

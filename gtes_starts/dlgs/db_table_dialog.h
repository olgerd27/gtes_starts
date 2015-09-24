#ifndef DB_TABLE_DIALOG_H
#define DB_TABLE_DIALOG_H

#include <QDialog>
#include <QSqlTableModel>
#include <QIcon>

class DBTableInfo;

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
 * Base class for dialogs, used for editing database tables data.
 */
class DBTableDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DBTableDialog(DBTableInfo *dbTable, QWidget *parent = 0);
    virtual ~DBTableDialog();

    QString identityString() const;

protected:
    void setWidgetTitle();

    DBTableInfo *m_dbTableInfo;
    CustomSqlTableModel *m_model;
    QString m_identityData;
};

#endif // DB_TABLE_DIALOG_H

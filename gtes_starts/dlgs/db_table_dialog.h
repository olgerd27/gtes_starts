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
    CustomSqlTableModel(QObject *parent);
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &idx, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);

private:
    enum { NO_CHECKED_ROW = -1 };

    int m_checkedRow;
    QIcon m_checkIcon;
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

protected:
    DBTableInfo *m_dbTableInfo;
    CustomSqlTableModel *m_model;
};

#endif // DB_TABLE_DIALOG_H

#ifndef DB_TABLE_DIALOG_H
#define DB_TABLE_DIALOG_H

#include <QDialog>
#include <QSqlTableModel>

class DBTableInfo;

/*
 * The custom QSqlTableModel class, that remove unnecessary characters from the end of every data item.
 * (Return "trimmed" data).
 */
class TrimmedSqlTableModel : public QSqlTableModel
{
    Q_OBJECT
public:
    TrimmedSqlTableModel(QObject *parent);
    QVariant data(const QModelIndex &idx, int role) const;
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
    TrimmedSqlTableModel *m_model;
};

#endif // DB_TABLE_DIALOG_H

#ifndef DB_TABLE_DIALOG_H
#define DB_TABLE_DIALOG_H

#include <QDialog>

class QSqlTableModel;
class DBTableInfo;
class DBTableDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DBTableDialog(DBTableInfo *dbTable, QWidget *parent = 0);
    virtual ~DBTableDialog();

private:
    DBTableInfo *m_dbTableInfo;
    QSqlTableModel *m_model;
};

#endif // DB_TABLE_DIALOG_H

#ifndef SIMPLE_DB_TABLE_DIALOG_H
#define SIMPLE_DB_TABLE_DIALOG_H

#include "db_table_dialog.h"

namespace Ui {
    class SimpleDBTableDialog;
}
class QStringListModel;

struct ItemQuery
{
    long m_id;          // item id
    QString m_query;    // item query
};

class SimpleDBTableDialog : public DBTableDialog
{
    Q_OBJECT

public:
    explicit SimpleDBTableDialog(DBTableInfo *dbTable, QWidget *parent = 0);
    virtual ~SimpleDBTableDialog();

    void setData(const QStringList &data);
    QStringList SQLstatements() const;

private:
    void setDBdataView();

    Ui::SimpleDBTableDialog *ui;
    // TODO: create the class for forming any types of queries. This must allow move the queries forming code from the UI classes, like this class.
    QStringListModel *m_sourceModel;
    QList<ItemQuery> m_queries;
};

#endif // SIMPLE_DB_TABLE_DIALOG_H

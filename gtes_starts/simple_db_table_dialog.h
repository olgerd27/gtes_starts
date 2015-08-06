#ifndef SIMPLE_DB_TABLE_DIALOG_H
#define SIMPLE_DB_TABLE_DIALOG_H

#include <QDialog>

namespace Ui {
    class SimpleDBTableDialog;
}
class QStringListModel;

struct ItemQuery
{
    long m_id;          // item id
    QString m_query;    // item query
};

class SimpleDBTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SimpleDBTableDialog(QWidget *parent = 0);
    ~SimpleDBTableDialog();

    void setData(const QStringList &data);
    QStringList queries() const;

private:
    void setDBdataView();

    Ui::SimpleDBTableDialog *ui;
    // TODO: create the class for forming any types of queries. This must allow move the queries forming code from the UI classes, like this class.
    QStringListModel *m_sourceModel;
    QList<ItemQuery> m_queries;
};

#endif // SIMPLE_DB_TABLE_DIALOG_H

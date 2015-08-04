#ifndef SIMPLE_TABLE_DIALOG_H
#define SIMPLE_TABLE_DIALOG_H

#include <QDialog>

namespace Ui {
    class SimpleTableDialog;
}

struct ItemQuery
{
    long m_id;          // item id
    QString m_query;    // item query
};

class SimpleTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SimpleTableDialog(QWidget *parent = 0);
    ~SimpleTableDialog();

    void setData(const QStringList &data);
    QStringList queries() const;

private:
    Ui::SimpleTableDialog *ui;
    // TODO: create the class for forming any types of queries. This must allow move the queries forming code from the UI classes, like this class.
    QList<ItemQuery> m_queries;
};

#endif // SIMPLE_TABLE_DIALOG_H

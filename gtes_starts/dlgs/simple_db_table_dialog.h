#ifndef SIMPLE_DB_TABLE_DIALOG_H
#define SIMPLE_DB_TABLE_DIALOG_H

#include "db_table_dialog.h"

namespace Ui {
    class SimpleDBTableDialog;
}

class SimpleDBTableDialog : public DBTableDialog
{
    Q_OBJECT

public:
    explicit SimpleDBTableDialog(DBTableInfo *dbTable, QWidget *parent = 0);
    virtual ~SimpleDBTableDialog();

private slots:
    void slotSetFilter(const QString &strFilter);

private:
    void setDBdataView();

    Ui::SimpleDBTableDialog *ui;
};

#endif // SIMPLE_DB_TABLE_DIALOG_H

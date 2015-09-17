#ifndef COMPLEX_DB_TABLE_DIALOG_H
#define COMPLEX_DB_TABLE_DIALOG_H

#include "db_table_dialog.h"

class QItemSelection;
namespace Ui {
    class ComplexDBTableDialog;
}

class ComplexDBTableDialog : public DBTableDialog
{
    Q_OBJECT

public:
    explicit ComplexDBTableDialog(DBTableInfo *dbTable, QWidget *parent = 0);
    virtual ~ComplexDBTableDialog();

private slots:
    void slotSelectionTemp(const QItemSelection &selected, const QItemSelection &deselected);
    void slotChooseRow(const QModelIndex &currIndex, const QModelIndex &prevIndex);
    void slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected);

private:
    Ui::ComplexDBTableDialog *ui;
};

#endif // COMPLEX_DB_TABLE_DIALOG_H

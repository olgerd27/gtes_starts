#ifndef COMPLEX_DB_TABLE_DIALOG_H
#define COMPLEX_DB_TABLE_DIALOG_H

#include <QDialog>

namespace Ui {
    class ComplexDBTableDialog;
}

class ComplexDBTableDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ComplexDBTableDialog(QWidget *parent = 0);
    ~ComplexDBTableDialog();

private:
    Ui::ComplexDBTableDialog *ui;
};

#endif // COMPLEX_DB_TABLE_DIALOG_H

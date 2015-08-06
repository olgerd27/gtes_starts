#include "complex_db_table_dialog.h"
#include "ui_complex_db_table_dialog.h"

ComplexDBTableDialog::ComplexDBTableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ComplexDBTableDialog)
{
    ui->setupUi(this);
}

ComplexDBTableDialog::~ComplexDBTableDialog()
{
    delete ui;
}

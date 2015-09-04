#include "complex_db_table_dialog.h"
#include "ui_complex_db_table_dialog.h"

ComplexDBTableDialog::ComplexDBTableDialog(DBTableInfo *dbTable, QWidget *parent)
    : DBTableDialog(dbTable, parent)
    , ui(new Ui::ComplexDBTableDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
}

ComplexDBTableDialog::~ComplexDBTableDialog()
{
    delete ui;
}

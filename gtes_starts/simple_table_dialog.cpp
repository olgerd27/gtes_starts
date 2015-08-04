#include "simple_table_dialog.h"
#include "ui_simple_table_dialog.h"

SimpleTableDialog::SimpleTableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SimpleTableDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
}

SimpleTableDialog::~SimpleTableDialog()
{
    delete ui;
}

void SimpleTableDialog::setData(const QStringList &data)
{
    QListWidget *listWgt = ui->m_listwData;
    foreach (QString str, data) {
        new QListWidgetItem(str, listWgt);
    }
}

QStringList SimpleTableDialog::queries() const
{
    // TODO: forming queries with help of some QueryCreator class
}

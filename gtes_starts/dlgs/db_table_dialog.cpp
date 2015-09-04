#include <QSqlTableModel>
#include "db_table_dialog.h"
#include "db_info.h"

DBTableDialog::DBTableDialog(DBTableInfo *dbTable, QWidget *parent)
    : QDialog(parent)
    , m_dbTableInfo(dbTable)
    , m_model(new QSqlTableModel(this))
{
    m_model->setTable(dbTable->m_nameInDB);
}

DBTableDialog::~DBTableDialog()
{
    delete m_model;
}

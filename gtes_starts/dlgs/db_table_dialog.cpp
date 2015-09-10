#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include "db_table_dialog.h"
#include "db_info.h"

/*
 * TrimmedSqlTableModel
 */
TrimmedSqlTableModel::TrimmedSqlTableModel(QObject *parent)
    : QSqlTableModel(parent)
{
}

QVariant TrimmedSqlTableModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid()) return QVariant();
    QVariant data;
    if (role == Qt::DisplayRole) {
        data = QSqlTableModel::data(idx, Qt::DisplayRole).toString().trimmed();
    }
    return data;
}

/*
 * DBTableDialog
 */
DBTableDialog::DBTableDialog(DBTableInfo *dbTable, QWidget *parent)
    : QDialog(parent)
    , m_dbTableInfo(dbTable)
    , m_model(new TrimmedSqlTableModel(this))
{
    m_model->setTable(dbTable->m_nameInDB);
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!m_model->select()) {
        qDebug() << "Error populating data from the table. The reason are:" + m_model->lastError().text();
        return;
    }
    for (int field = 0; field < dbTable->tableDegree(); ++field) {
        DBTableFieldInfo fieldInfo = dbTable->fieldByIndex(field);
        if (!fieldInfo.isValid()) {
            qDebug() << "Error #XXX. Cannot forming the header of the database table \"" + dbTable->m_nameInDB << "\"";
            return;
        }
        m_model->setHeaderData(field, Qt::Horizontal, fieldInfo.m_nameInUI);
    }
}

DBTableDialog::~DBTableDialog()
{
    delete m_model;
}

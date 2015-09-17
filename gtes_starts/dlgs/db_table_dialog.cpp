#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include "db_table_dialog.h"
#include "db_info.h"

/*
 * TrimmedSqlTableModel
 */
CustomSqlTableModel::CustomSqlTableModel(QObject *parent)
    : QSqlTableModel(parent)
    , m_checkedRow(NO_CHECKED_ROW)
    , m_checkIcon(":/images/ok.png")
{
}

QVariant CustomSqlTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant data;
    if (orientation == Qt::Horizontal && section == 0 && role == Qt::DisplayRole)
        data = QVariant();
    else
        data = QSqlTableModel::headerData(section, orientation, role);
    return data;
}

QVariant CustomSqlTableModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid()) return QVariant();
    QVariant data;
    if (role == Qt::DisplayRole)
        data = QSqlTableModel::data(idx, Qt::DisplayRole).toString().trimmed();
    if (idx.column() == 0) {
        if (role == Qt::DisplayRole) data = QVariant();
        if (role == Qt::DecorationRole && idx.row() == m_checkedRow) data = m_checkIcon;
    }
    return data;
}

bool CustomSqlTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value)
    if ( index.column() == 0 && role == Qt::DecorationRole ) {
        m_checkedRow = index.row();
        qDebug() << "set decoration in the first column";
    }
    emit dataChanged(index, index);
    return true;
}

void checkItemFlags(Qt::ItemFlags f)
{
    if (f & Qt::NoItemFlags)
        qDebug() << "NoItemFlags";
    if (f & Qt::ItemIsSelectable)
        qDebug() << "ItemIsSelectable";
    if (f & Qt::ItemIsEditable)
        qDebug() << "ItemIsEditable";
    if (f & Qt::ItemIsDragEnabled)
        qDebug() << "ItemIsDragEnabled";
    if (f & Qt::ItemIsDropEnabled)
        qDebug() << "ItemIsDropEnabled";
    if (f & Qt::ItemIsUserCheckable)
        qDebug() << "ItemIsUserCheckable";
    if (f & Qt::ItemIsEnabled)
        qDebug() << "ItemIsEnabled";
    if (f & Qt::ItemIsTristate)
        qDebug() << "ItemIsTristate";
    if (f & Qt::ItemNeverHasChildren)
        qDebug() << "ItemNeverHasChildren";
    qDebug() << "********";
}

Qt::ItemFlags CustomSqlTableModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags itemFlags = QSqlTableModel::flags(index);
    if (!index.isValid()) return itemFlags;
//    if (index.column() == 0) {
//        checkItemFlags(itemFlags);
//        itemFlags &= ~Qt::ItemIsSelectable;
//        checkItemFlags(itemFlags);
//    }
//    if (index.column() == 0)
//        itemFlags &= ~Qt::ItemIsEnabled;
    return itemFlags;
}

/*
 * DBTableDialog
 */
DBTableDialog::DBTableDialog(DBTableInfo *dbTable, QWidget *parent)
    : QDialog(parent)
    , m_dbTableInfo(dbTable)
    , m_model(new CustomSqlTableModel(this))
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

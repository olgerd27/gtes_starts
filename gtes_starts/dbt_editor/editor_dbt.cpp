#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include "editor_dbt.h"
#include "db_info.h"

/*
 * TrimmedSqlTableModel
 */
CustomSqlTableModel::CustomSqlTableModel(QObject *parent)
    : QSqlTableModel(parent)
    , m_selectedRow(DONT_SELECTED_ROW)
    , m_selectIcon(":/images/ok.png")
{
}

QVariant CustomSqlTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant data;
    if (orientation == Qt::Horizontal)
        data = section > SELECT_ICON_COLUMN
               ? QSqlTableModel::headerData(section - 1, orientation, role)
               : QVariant();
    return data;
}

QVariant CustomSqlTableModel::data(const QModelIndex &idx, int role) const
{
    QVariant data;
    if (!idx.isValid()) return data;

    if (idx.column() > SELECT_ICON_COLUMN) {
        QModelIndex indexLeftCell = QSqlTableModel::index(idx.row(), idx.column() - 1);
        data = QSqlTableModel::data(indexLeftCell, role); // set a data, getted from the neighbor's left cell
        if (role == Qt::DisplayRole) data = data.toString().trimmed();
    }
    else {
        if (role == Qt::DisplayRole) data = QVariant();
        if (role == Qt::DecorationRole && idx.row() == m_selectedRow) data = m_selectIcon; // set selection icon
    }

    if (role == Qt::TextAlignmentRole) {
        if ( this->data(idx, Qt::DisplayRole).convert(QMetaType::Float) )
            data = Qt::AlignCenter; // center alignment of the numerical values
    }
    return data;
}

bool CustomSqlTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value)
    if ( index.column() == SELECT_ICON_COLUMN && role == Qt::DecorationRole )
        m_selectedRow = index.row();
    emit dataChanged(index, index);
    return true;
}

int CustomSqlTableModel::columnCount(const QModelIndex &parent) const
{
    return QSqlTableModel::columnCount(parent) + 1;
}

Qt::ItemFlags CustomSqlTableModel::flags(const QModelIndex &index) const
{
    return index.column() == columnCount(index.parent()) - 1
            ? QSqlTableModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable
            : QSqlTableModel::flags(index) & ~Qt::ItemIsEditable;
}

/*
 * DBTEditor
 */
DBTEditor::DBTEditor(DBTInfo *dbTable, QWidget *parent)
    : QDialog(parent)
    , m_DBTInfo(dbTable)
    , m_model(new CustomSqlTableModel)
{
    m_model->setTable(dbTable->m_nameInDB);
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!m_model->select()) {
        qDebug() << "Error populating data from the table. The reason are:" + m_model->lastError().text();
        return;
    }
    for (int field = 0; field < dbTable->tableDegree(); ++field) {
        DBTFieldInfo fieldInfo = dbTable->fieldByIndex(field);
        if (!fieldInfo.isValid()) {
            qDebug() << "Error #XXX. Cannot forming the header of the database table \"" + dbTable->m_nameInDB << "\"";
            return;
        }
        m_model->setHeaderData(field, Qt::Horizontal, fieldInfo.m_nameInUI);
    }
}

DBTEditor::~DBTEditor()
{
    delete m_model;
}

QString DBTEditor::identificationData() const
{
    return m_identificationData;
}

void DBTEditor::setWindowName()
{
    setWindowTitle( tr("Editing the table: ") + m_DBTInfo->m_nameInUI );
}

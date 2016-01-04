#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QMessageBox>
#include <QDebug>
#include "dbt_editor.h"
#include "common/db_info.h"

/*
 * IconedSqlTableModel
 */
RowsChooseSqlTableModel::RowsChooseSqlTableModel(QObject *parent)
    : QSqlTableModel(parent)
    , m_selectedRow(DONT_SELECTED_ROW)
    , m_selectIcon(":/images/ok.png")
{
}

QVariant RowsChooseSqlTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QVariant data;
    if (orientation == Qt::Horizontal)
        data = section > SELECT_ICON_COLUMN
               ? QSqlTableModel::headerData(section - 1, orientation, role)
               : QVariant();
    return data;
}

QVariant RowsChooseSqlTableModel::data(const QModelIndex &idx, int role) const
{
    QVariant data;
    if (!idx.isValid()) return data;

    if (idx.column() > SELECT_ICON_COLUMN) {
        QModelIndex indexLeftCell = QSqlTableModel::index(idx.row(), idx.column() - 1);
        data = QSqlTableModel::data(indexLeftCell, role); // set a data, getted from the neighbor's left cell
//        if (role == Qt::DisplayRole) data = data.toString().trimmed(); // TODO: maybe delete, real inputed data don't need calling trimmed() function
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

bool RowsChooseSqlTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Q_UNUSED(value)
    if ( index.column() == SELECT_ICON_COLUMN && role == Qt::DecorationRole )
        m_selectedRow = index.row();
    emit dataChanged(index, index);
    return true;
}

int RowsChooseSqlTableModel::columnCount(const QModelIndex &parent) const
{
    return QSqlTableModel::columnCount(parent) + 1;
}

Qt::ItemFlags RowsChooseSqlTableModel::flags(const QModelIndex &index) const
{
    return index.column() == columnCount(index.parent()) - 1
            ? QSqlTableModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsSelectable
            : QSqlTableModel::flags(index) & ~Qt::ItemIsEditable;
}

/*
 * DBTEditor
 */
DBTEditor::DBTEditor(dbi::DBTInfo *dbTable, QWidget *parent)
    : QDialog(parent)
    , m_DBTInfo(dbTable)
    , m_model(new RowsChooseSqlTableModel)
    , m_selectedId(-1)
{
    setWindowName();
    m_model->setTable(dbTable->m_nameInDB);
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!m_model->select()) {
        qDebug() << "Error populating data from the table. The reason is:" + m_model->lastError().text();
        // TODO: send this error to the some class, inherited from the QWidget, for creating error message box
        return;
    }
    for (int field = 0; field < dbTable->tableDegree(); ++field) {
        dbi::DBTFieldInfo fieldInfo = dbTable->fieldByIndex(field);
        if (!fieldInfo.isValid()) {
            qDebug() << "Error #XXX. Cannot forming the header of the database table \"" + dbTable->m_nameInDB << "\"";
            // TODO: send this error to the some class, inherited from the QWidget, for creating error message box
            return;
        }
        m_model->setHeaderData(field, Qt::Horizontal, fieldInfo.m_nameInUI);
    }
}

DBTEditor::~DBTEditor()
{
    delete m_model;
}

DBTEditor::T_id DBTEditor::selectedId() const
{
    return m_selectedId;
}

void DBTEditor::setWindowName()
{
    setWindowTitle( tr("Editing the table: ") + m_DBTInfo->m_nameInUI );
}

bool DBTEditor::selectInitial(const QVariant &value, DBTEditor::ColumnNumbers compareCol)
{
//    qDebug() << "select initial. value:" << value;
    int selectedRow = -1;
    for (int row = 0; row < m_model->rowCount(); ++row) {
//        qDebug() << "row:" << row << ", display data:" << m_model->index(row, compareCol).data(Qt::DisplayRole);
        if ( m_model->index(row, compareCol).data(Qt::DisplayRole) == value ) {
            selectedRow = row;
            break;
        }
    }
    if (selectedRow != -1) {
        makeSelect(selectedRow); // the virtual function calling that select the found row
//        qDebug() << "selected row #" << selectedRow;
    }
    else {
        QMessageBox::warning( this, tr("Selection error"),
                              tr("Cannot select an item in the list.\n"
                                 "The reason is: cannot find in the database table \"%1\" the item with id = %2")
                              .arg(m_DBTInfo->m_nameInUI).arg(value.toString()) );
        return false;
    }
    return true;
}

//void DBTEditor::selectInitial(const QVariant &value, ColumnNumbers compareCol)
//{
//    QModelIndex selectRowIndex;
//    for (int row = 0; row < m_model->rowCount(); ++row)
//        if ( m_model->index(row, compareCol).data() == value )
//            selectRowIndex = m_model->index(row, compareCol);
//    if (selectRowIndex.isValid())
//        makeSelect(selectRowIndex.row()); // calling the virtual function for selection the found row
//    else
//        QMessageBox::warning( this, tr("Selection error"),
//                              tr("Cannot select an item in the list.\n"
//                                 "The reason is: cannot find in the database table \"%1\" the item: \"%2\" in the column #%3")
//                              .arg( m_DBTInfo->m_nameInUI ).arg( value.toString() ).arg((int)compareCol) );
//}

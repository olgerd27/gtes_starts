#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QDebug>
#include "dbt_editor.h"
#include "../common/db_info.h"

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
        data = QSqlTableModel::data( index(idx.row(), idx.column() - 1), role); // set a data, getted from the neighbor's left cell
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

cmmn::T_id RowsChooseSqlTableModel::selectedId() const
{
    return cmmn::safeQVariantToIdType( data(index(m_selectedRow, 1), Qt::DisplayRole) );
}

void RowsChooseSqlTableModel::slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected)
{
    QItemSelectionModel *selectModel = qobject_cast<QItemSelectionModel *>(sender());
    QModelIndexList deselectedList = deselected.indexes();

    // catch a deselection of the first left item in current row and setting icons decoration on it
    if (deselectedList.size() == 1) {
        setData(deselectedList.first(), QVariant(), Qt::DecorationRole);
        return;
    }

    // update the first left items in the previous selected row for clearing icons decoration
    if (deselectedList.size() > 1) {
        QModelIndex someDeselected = deselectedList.first();
        QModelIndex firstDeselected = someDeselected.model()->index(someDeselected.row(), SELECT_ICON_COLUMN);
        emit sigNeedUpdateView(firstDeselected); // clear remained icons decoration
    }

    QModelIndex firstSelected = selected.indexes().first();
    selectModel->select(firstSelected, QItemSelectionModel::Deselect); // this make recursive calling of this slot
}

/*
 * DBTEditor
 */
DBTEditor::DBTEditor(dbi::DBTInfo *dbTable, QWidget *parent)
    : QDialog(parent)
    , m_DBTInfo(dbTable)
    , m_model(new RowsChooseSqlTableModel)
{
    setModel();
    setWindowName();
    setHeaderData();
}

DBTEditor::~DBTEditor()
{ }

cmmn::T_id DBTEditor::selectedId() const
{
    return m_model->selectedId();
}

void DBTEditor::setModel()
{
    m_model->setTable(m_DBTInfo->m_nameInDB);
    m_model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    if (!m_model->select()) {
        // TODO: generate the error
        qDebug() << "Cannot populating the model by a data from the database.\nThe DB error text: " + m_model->lastError().text();
        return;
    }
}

void DBTEditor::setWindowName()
{
    setWindowTitle( tr("Editing the table: ") + m_DBTInfo->m_nameInUI );
}

void DBTEditor::setHeaderData()
{
    for (int field = 0; field < m_DBTInfo->tableDegree(); ++field) {
        const dbi::DBTFieldInfo &fieldInfo = m_DBTInfo->fieldByIndex(field);
        if (!fieldInfo.isValid()) {
            // TODO: generate the error
            qDebug() << "Invalid field info. Cannot forming the header of the database table \"" + m_DBTInfo->m_nameInDB << "\", field #" << field << "\n.";
            return;
        }
        m_model->setHeaderData(field, Qt::Horizontal, fieldInfo.m_nameInUI);
    }
}

void DBTEditor::setSelection(QAbstractItemView *view)
{
    if (!view) return;
    connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            m_model.get(), SLOT(slotChooseRow(QItemSelection,QItemSelection)));
    connect(m_model.get(), SIGNAL(sigNeedUpdateView(QModelIndex)), view, SLOT(update(QModelIndex)));
}

bool DBTEditor::selectInitial(const QVariant &value, DBTEditor::ColumnNumbers compareCol)
{
    qDebug() << "select initial. value:" << value;
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

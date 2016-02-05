#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QAbstractItemView>
#include <QMessageBox>
#include <QDebug>
#include "dbt_editor.h"
//#include "../datagen/custom_sql_table_model.h"
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

bool RowsChooseSqlTableModel::findPrimaryIdRow(const QVariant &id, int &rRowId)
{
    return findValueRow(id, 0, rRowId);
}

bool RowsChooseSqlTableModel::findValueRow(const QVariant &value, int column, int &rRowValue)
{
    for (int row = 0; row < rowCount(); ++row) {
        if (data(index(row, column), Qt::DisplayRole) == value) {
            rRowValue = row;
            return true;
        }
    }
    return false;
}

cmmn::T_id RowsChooseSqlTableModel::selectedId() const
{
    cmmn::T_id id;
    const QVariant &varId = data(index(m_selectedRow, 1), Qt::DisplayRole);
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(varId, id), varId );
    return id;
}

void RowsChooseSqlTableModel::printData(int role) const
{
    QString strData;
    for(int row = 0; row < rowCount(); ++row) {
        for(int col = 0; col < columnCount(); ++col ) {
            QModelIndex index = QSqlTableModel::index(row, col);
            strData += (QSqlTableModel::data(index, role).toString() + "  ");
        }
        strData += "\n";
    }
    qDebug() << "Table model data with role #" << role << ":\n" << strData;
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
//    , m_model(new CustomSqlTableModel(this))
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
    // TODO: move the select() calling to the RowsChooseSqlTableModel::setTable() reimplemented method
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

bool DBTEditor::selectInitial(const QVariant &compareValue, ColumnNumbers compareCol)
{
//    qDebug() << "select initial. primary id value:" << idPrim;
//    m_model->printData(Qt::DisplayRole);
    int selectedRow = -1;
    if (m_model->findValueRow(compareValue, compareCol, selectedRow))
        makeSelect(selectedRow); // the virtual function that select the found row
    else {
        QMessageBox::warning( this, tr("Table row selection error"),
                              tr("Cannot select an item in the view.\n"
                                 "Cannot find in the internal model of the database table \"%1\" the item with id = %2")
                              .arg(m_DBTInfo->m_nameInUI).arg(compareValue.toString()) );
        return false;
    }
    return true;
}

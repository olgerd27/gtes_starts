#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTableView>
#include <QHeaderView>
#include <QItemSelection>
#include <QMessageBox>
#include <QDebug>
#include "dbt_editor.h"
#include "ui_dbt_editor.h"
#include "../model/custom_sql_table_model.h"
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

bool RowsChooseSqlTableModel::findPrimaryIdRow(const QVariant &idPrim, int &rRowValue)
{
    for (int row = 0; row < rowCount(); ++row) {
        if (data(index(row, SELECT_ICON_COLUMN + 1), Qt::DisplayRole) == idPrim) {
            rRowValue = row;
            return true;
        }
    }
    return false;
}

cmmn::T_id RowsChooseSqlTableModel::selectedId() const
{
    int colPrimId = SELECT_ICON_COLUMN + 1; // TODO: use the ConvMDBI.convColumn() method
    cmmn::T_id id;
    const QVariant &varId = this->data(index(m_selectedRow, colPrimId), Qt::DisplayRole);
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
 * HighlightTableRowsDelegate
 */
class HighlightTableRowsDelegate : public QStyledItemDelegate
{
public:
    enum { DONT_HIGHLIGHTED = -1 }; // number of a table row that is not highlighted (need for highlighting removing)

    HighlightTableRowsDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        static int hRow = DONT_HIGHLIGHTED; /* initialization of the highlighted row number.
                                             * Use static variable, bacause this is const method and it cannot to change any class data.
                                             * Also you may use global variable, defined outside of current class.
                                             */
        QTableView *tableView = qobject_cast<QTableView *>(this->parent());

        /* Definition the fact of necessity of highlighting removing */
        if ( isMouseOutTable(tableView) )
            hRow = DONT_HIGHLIGHTED;

        /* Definition the table row number for highlighting, and initiate highlighting of the current row
         * and highlighting removing of the previous highlighted row
         */
        if (option.state & QStyle::State_MouseOver) {
            if (index.row() != hRow) {
                // mouse is over item, that placed in another row, than previous "mouse over" item
                const QAbstractItemModel *model = index.model();
                for (int col = 0; col < model->columnCount(); ++col) {
                    tableView->update(model->index(index.row(), col)); // update items from current row for painting visual row highlighting
                    tableView->update(model->index(hRow, col)); // update items from previous row for removing visual row highlighting
                }
                hRow = index.row();
            }
        }

        /* Creation a visual view of the highlighted row items */
        if (index.row() == hRow && hRow != DONT_HIGHLIGHTED) {
            // color variant #1 - horizontal linear gradient. Highlighting is the same as in simple DB table dialog
            QRect rectView = tableView->rect();
            QPoint topLeft = rectView.topLeft();
            QLinearGradient gradient( topLeft.x(), topLeft.y(),
                                      topLeft.x() + tableView->horizontalHeader()->length(), rectView.topRight().y() );
            gradient.setColorAt(0, Qt::white);
            gradient.setColorAt(0.8, Qt::lightGray);
            gradient.setColorAt(1, Qt::gray);

            // color variant #2 - vertical linear gradient
//            QRect rectItem = option.rect;
//            QLinearGradient gradient(rectItem.topLeft(), rectItem.bottomLeft());
//            gradient.setColorAt(0, Qt::white);
//            gradient.setColorAt(0.5, Qt::lightGray);
//            gradient.setColorAt(1, Qt::white);

            // painter paint a rectangle with setted gradient
            painter->setBrush(gradient);
            painter->setPen(Qt::NoPen);
            painter->drawRect(option.rect);
        }
        QStyledItemDelegate::paint(painter, option, index);
    }

private:
    bool isMouseOutTable(const QTableView * const table) const
    {
        /*  Checking - is a mouse out of the viewport of a table view or not */
        QPoint viewportCursorPos = table->viewport()->mapFromGlobal(QCursor::pos());
        return !table->indexAt(viewportCursorPos).isValid();
    }
};

/*
 * DBTEditor
 * TODO: add the apply and revert push buttons on this window, as in example "Cached table"
 */
DBTEditor::DBTEditor(dbi::DBTInfo *dbTable, QWidget *parent)
    : QDialog(parent)
    , m_DBTInfo(dbTable)
    , m_ui(new Ui::DBTEditor)
//    , m_model(new RowsChooseSqlTableModel)
    , m_model(new CustomSqlTableModel(this))
{
    m_ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    setModel();
    setWindowName();
    setContentsUI();
    setEditingUI();
}

DBTEditor::~DBTEditor()
{ }

cmmn::T_id DBTEditor::selectedId() const
{
    return m_model->selectedId();
}

void DBTEditor::setContentsUI()
{
    QTableView *view = m_ui->m_tableContents;
    view->setModel(m_model.get());
    view->setItemDelegate(new HighlightTableRowsDelegate(view));
    view->viewport()->setAttribute(Qt::WA_Hover);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    view->setAlternatingRowColors(true);

    // set selection
    connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            m_model.get(), SLOT(slotChooseRow(QItemSelection,QItemSelection)));
    connect(m_model.get(), SIGNAL(sigNeedUpdateView(QModelIndex)), view, SLOT(update(QModelIndex)));
}

void DBTEditor::setEditingUI()
{
    // TODO: code here
}

void DBTEditor::setModel()
{
    m_model->setTable(m_DBTInfo->m_nameInDB);
}

void DBTEditor::setWindowName()
{
    setWindowTitle( tr("Editing the table: ") + m_DBTInfo->m_nameInUI );
}

void DBTEditor::selectInitial(const QVariant &idPrim)
{
    int selectedRow = -1;
    ASSERT_DBG( m_model->findPrimaryIdRow(idPrim, selectedRow),
                cmmn::MessageException::type_warning, tr("Selection error"),
                tr("Cannot select the row in the table \"%1\". Cannot find the item with id: %2")
                .arg(m_DBTInfo->m_nameInUI).arg(idPrim.toString()),
                QString("FormDataInput::slotEditDBT()") );
    m_ui->m_tableContents->selectRow(selectedRow);
}

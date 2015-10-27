#include <QSqlTableModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTableView>
#include <QHeaderView>
#include <QItemSelection>
#include <QDebug>
#include "complex_dbt_editor.h"
#include "ui_complex_dbt_editor.h"
#include "db_info.h"

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
 * ComplexDBTEditor
 * TODO: add the apply and revert push buttons on this window, as in example "Cached table"
 */
ComplexDBTEditor::ComplexDBTEditor(dbi::DBTInfo *dbtInfo, QWidget *parent)
    : DBTEditor(dbtInfo, parent)
    , ui(new Ui::ComplexDBTEditor)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    setContentsUI();
    setEditingUI();
}

void ComplexDBTEditor::setContentsUI()
{
    QTableView *view = ui->m_tableContents;
    view->setModel(m_model);
    view->setItemDelegate(new HighlightTableRowsDelegate(view));
    view->viewport()->setAttribute(Qt::WA_Hover);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotChooseRow(QItemSelection,QItemSelection)));
}

void ComplexDBTEditor::setEditingUI()
{
    // TODO: code here
}

ComplexDBTEditor::~ComplexDBTEditor()
{
    delete ui;
}

void ComplexDBTEditor::makeSelect(int row)
{
    ui->m_tableContents->selectRow(row);
}

/* perform choosing by a user some row of a table view */
void ComplexDBTEditor::slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList deselectedList = deselected.indexes();

    // catch a deselection of the first left item in current row and setting icons decoration on it
    if (deselectedList.size() == 1) {
        ui->m_tableContents->model()->setData(deselectedList.first(), QVariant(), Qt::DecorationRole);
        return;
    }

    // update the first left items in the previous selected row for clearing icons decoration
    if (deselectedList.size() > 1) {
        QModelIndex someDeselected = deselectedList.first();
        QModelIndex firstDeselected = someDeselected.model()->index(someDeselected.row(), 0);
        ui->m_tableContents->update(firstDeselected); // clear remained icons decoration
    }

    QModelIndex firstSelected = selected.indexes().first();
    ui->m_tableContents->selectionModel()->select(firstSelected, QItemSelectionModel::Deselect); // this make recursive calling of this slot
    setSelectedId(firstSelected.row()); /* NOTE: sometimes the selected.indexes() list has only 1 item, because of it there are
                                           used only a row number of the first item in the selected.indexes() list */
    setIdentificationData(firstSelected); // TODO: maybe delete
}

void ComplexDBTEditor::setSelectedId(int selectedRow)
{
    m_selectedId = m_model->index(selectedRow, 1).data().toInt();
}

void ComplexDBTEditor::setIdentificationData(const QModelIndex &indexInSelectRow)
{
    const QAbstractItemModel *model = indexInSelectRow.model();
    m_identificationData.clear();
    for (unsigned i = 0; i < m_DBTInfo->m_idnFields.size(); ++i) {
        const dbi::DBTInfo::IdentityInfo &info = m_DBTInfo->m_idnFields.at(i);
        m_identificationData += info.m_strBefore + model->index(indexInSelectRow.row(), info.m_NField + 1).data().toString();
    }
}

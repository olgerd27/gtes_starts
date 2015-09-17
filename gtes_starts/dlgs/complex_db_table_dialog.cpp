#include <QSqlTableModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTableView>
#include <QHeaderView>
#include <QItemSelection>
#include <QDebug>
#include "complex_db_table_dialog.h"
#include "ui_complex_db_table_dialog.h"
#include "db_info.h"

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
        /*  Check - is a mouse out of the viewport of a table view or not */
        QPoint viewportCursorPos = table->viewport()->mapFromGlobal(QCursor::pos());
        return !table->indexAt(viewportCursorPos).isValid();
    }
};

ComplexDBTableDialog::ComplexDBTableDialog(DBTableInfo *dbTable, QWidget *parent)
    : DBTableDialog(dbTable, parent)
    , ui(new Ui::ComplexDBTableDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);

    // table view settings
    QTableView *view = ui->m_tableContents;
    view->setModel(m_model);
    view->setItemDelegate(new HighlightTableRowsDelegate(view));
    view->viewport()->setAttribute(Qt::WA_Hover);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // TODO: resize to contents OR NOT?

//    connect(view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
//            this, SLOT(slotChooseRow(QModelIndex,QModelIndex)));
//    connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//            this, SLOT(slotSelectionTemp(QItemSelection,QItemSelection)));

    connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            this, SLOT(slotChooseRow(QItemSelection,QItemSelection)));
}

ComplexDBTableDialog::~ComplexDBTableDialog()
{
    delete ui;
}

#include <QMessageBox>
void ComplexDBTableDialog::slotChooseRow(const QModelIndex &currIndex, const QModelIndex &prevIndex)
{
    if (!prevIndex.isValid()) return;
    qDebug() << "1";
    QModelIndex firstItemIndex = currIndex.model()->index(currIndex.row(), 0, currIndex.parent());
    qDebug() << "2";
    qDebug() << "firstItemIndex: is valid =" << firstItemIndex.isValid() << ", col =" << firstItemIndex.column() << ", row =" << firstItemIndex.row();
    ui->m_tableContents->selectionModel()->select(firstItemIndex, QItemSelectionModel::Deselect);

    QMessageBox::information(this, "title", "after deselection");

    QModelIndex firstItemIndex2 = currIndex.model()->index(currIndex.row() + 1, currIndex.column() + 1);
    ui->m_tableContents->selectionModel()->select(firstItemIndex2, QItemSelectionModel::Select);

    QMessageBox::information(this, "title", "after selection");

    qDebug() << "3";
    ui->m_tableContents->model()->setData(firstItemIndex, QVariant(), Qt::DecorationRole);
    qDebug() << "------------";
}

void showList(const QModelIndexList &list, const QString &listName)
{
    qDebug() << listName << "count =" << list.size() << ":";
    int counter = 0;
    if (list.size())
        foreach (QModelIndex index, list) {
            qDebug() << "   index #" << counter++ << ": row =" << index.row() << ", col =" << index.column();
        }
    else
        qDebug() << "   NONE";
}

void ComplexDBTableDialog::slotSelectionTemp(const QItemSelection &selected, const QItemSelection &deselected)
{
    QModelIndexList selectedList = selected.indexes();
    showList(selectedList, "selected");
    QModelIndexList deselectedList = deselected.indexes();
    showList(deselectedList, "deselected");
}

// rows selection - bad behaviour - not updated previous selected rows
void ComplexDBTableDialog::slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected)
{
    qDebug() << "1";
    QModelIndexList selectedList = selected.indexes();
    showList(selectedList, "selected");
    QModelIndexList deselectedList = deselected.indexes();
    showList(deselectedList, "deselected");

    qDebug() << "2";
    if (deselectedList.size() == 1) {
        // there are recursive slot calling for deselection first item in current row
        qDebug() << "selection is empty";
        ui->m_tableContents->model()->setData(deselectedList.first(), QVariant(), Qt::DecorationRole);
        return;
    }
//    else {
        // forced deselection of the first item in current row
//        qDebug() << "not empty - forced deselection of the first item in current row";
//        QModelIndex someDeselected = deselected.indexes().first();
//        QModelIndex needDeselect = someDeselected.model()->index(someDeselected.row(), 0);
//        ui->m_tableContents->selectionModel()->select(needDeselect, QItemSelectionModel::Deselect);
//        return;
//    }
    qDebug() << "3";

    // update first row items in the previous selected row
    if (deselectedList.size() > 1) {
        QModelIndex someSelected = deselectedList.first();
        QModelIndex firstDeselectedIndex = someSelected.model()->index(someSelected.row(), 0);
        ui->m_tableContents->update(firstDeselectedIndex);
        qDebug() << "update deselected item: row =" << firstDeselectedIndex.row() << ", col =" << firstDeselectedIndex.column();
    }

    QModelIndex firstSelectedIndex = selectedList.first();
    qDebug() << "4";

//    QMessageBox::information(this, "title", "before deselect");
//    ui->m_tableContents->model()->setData(firstItemIndex, QVariant(), Qt::DecorationRole);


//    QMessageBox::information(this, "title", "after deselect");
    qDebug() << "5";
    ui->m_tableContents->selectionModel()->select(firstSelectedIndex, QItemSelectionModel::Deselect);
//    QMessageBox::information(this, "title", "after setData");

    qDebug() << "----";
}

// items selection - bad behaviour - not updated previous selected rows
//void ComplexDBTableDialog::slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected)
//{
//    QModelIndexList selectedList = selected.indexes();
//    if (selectedList.size() == 1) {
//        QModelIndex selectedIndex = selectedList.first();
//        const QAbstractItemModel *model = selectedIndex.model();
//        int selectedRow = selectedIndex.row();
//        QItemSelection selection( model->index(selectedRow, 1), model->index(selectedRow, model->columnCount() - 1) );
//        ui->m_tableContents->selectionModel()->select(selection, QItemSelectionModel::Select);
//        QModelIndex firstIndex = model->index(selectedRow, 0);
//        ui->m_tableContents->model()->setData(firstIndex, QVariant(), Qt::DecorationRole);
//    }
//    qDebug() << "----";
//}

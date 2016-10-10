#include <QStyledItemDelegate>
#include <QPainter>
#include <QMouseEvent>
#include <QDebug>
#include "table_view_ds.h"
#include "../model/proxy_model.h"

// HighlightTableRowsDelegate
class HighlightTableRowsDelegate : public QStyledItemDelegate
{
public:
    enum { DONT_HIGHLIGHTED = -1 }; // number of a table row that is not highlighted (need for highlighting removing)

    HighlightTableRowsDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
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

// TableView_DS
TableView_DS::TableView_DS(QWidget *parent)
    : QTableView(parent)
{
    setItemDelegate(new HighlightTableRowsDelegate(this));
    viewport()->setAttribute(Qt::WA_Hover);
    setVertHeader();
}

void TableView_DS::setVertHeader()
{
    auto vHeader = verticalHeader();
    vHeader->setDefaultSectionSize( vHeader->defaultSectionSize() * 0.75 ); // reduce rows high - need for the QHeaderView::Fixed resize mode
    vHeader->setSectionResizeMode(QHeaderView::Fixed);
}

void TableView_DS::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
    setHorizHeader(); // set horizontal header must be only after model setting
}

void TableView_DS::setHorizHeader()
{
    auto hHeader = horizontalHeader();
    setHorizSectionResizeMode(hHeader);
    hHeader->setStretchLastSection(true);

    // increase view width, that vertical scroll widget do not cover data in the last table column
    int minWidthIncreasing = 0;
#ifdef __linux__
    minWidthIncreasing = 0;
#else
    minWidthIncreasing = 30;
#endif
    setMinimumWidth(hHeader->length() + minWidthIncreasing);
}

void TableView_DS::setHorizSectionResizeMode(QHeaderView *header)
{
#ifdef __linux__
    // resize decoration field to fit size
    ProxyFilterModel *modelFilter = qobject_cast<ProxyFilterModel *>(this->model());
    if (!modelFilter) return;
    ProxyDecorModel *modelDecor = qobject_cast<ProxyDecorModel *>(modelFilter->sourceModel());
    if (!modelDecor) return;
    for (int i = 0; i < header->count(); ++i) {
        if (i == 0) {
            header->setSectionResizeMode(i, QHeaderView::Fixed);
            header->resizeSection(i, modelDecor->decorationSize().width());
        }
        else header->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
#else
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
}

void TableView_DS::mousePressEvent(QMouseEvent *event)
{
    if ( indexAt(event->pos()).isValid() ) // if model index on the mouse current position is valid -> mouse pressed over the table field
        emit sigMousePressedOverTable();
    QTableView::mousePressEvent(event);
}

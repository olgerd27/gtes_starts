#include <QSqlTableModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTableView>
#include <QDebug>
#include "complex_db_table_dialog.h"
#include "ui_complex_db_table_dialog.h"
#include "db_info.h"

class HighlightTableDelegate : public QStyledItemDelegate
{
public:
    enum { DONT_HIGHLIGHTED = -1 }; // number of a table row that is not highlighted (need for highlighting removing)

    HighlightTableDelegate(QObject *parent = 0)
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
            QRect rect = option.rect;
            QLinearGradient gradient(0, 0, rect.width(), rect.height());

            // color variant #1
            gradient.setColorAt(0, Qt::white);
            gradient.setColorAt(0.8, Qt::lightGray);
            gradient.setColorAt(1, Qt::gray);

            // color varian #2
//            gradient.setColorAt(0, Qt::white);
//            gradient.setColorAt(0.9, QColor(190, 200, 250));
//            gradient.setColorAt(1.0, QColor(135, 150, 250));

            // painter paint a rectangle with setted gradient
            painter->setBrush(gradient);
            painter->setPen(Qt::NoPen);
            painter->drawRect(rect);
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
    view->setItemDelegate(new HighlightTableDelegate(view));
    view->viewport()->setAttribute(Qt::WA_Hover);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // TODO: resize to contents OR NOT?
//    view->horizontalHeader()->resizeSection(
    view->setColumnHidden(m_model->fieldIndex("id"), true); // hide "id"
}

ComplexDBTableDialog::~ComplexDBTableDialog()
{
    delete ui;
}

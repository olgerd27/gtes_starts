#include <QSqlTableModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QDebug>
#include "complex_db_table_dialog.h"
#include "ui_complex_db_table_dialog.h"

class HighlightTableDelegate : public QStyledItemDelegate
{
public:
    enum { DONT_HIGHLIGHTED = -1 };

    HighlightTableDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        // variant 3 - The issue:
        QTableView *tableView = qobject_cast<QTableView *>(this->parent());
        static int prevRow = DONT_HIGHLIGHTED;
        if (option.state & QStyle::State_MouseOver) {
            if (index.row() != prevRow) {
                // mouse is over item, placed in another row, than previous "mouse over" item
                const QAbstractItemModel *model = index.model();
                for (int col = 0; col < model->columnCount(); ++col) {
                    tableView->update(model->index(index.row(), col)); // update items from current row for painting visual highlighting
                    tableView->update(model->index(prevRow, col)); // update items from previous row for removing visual highlighting
                }
                prevRow = index.row();
            }
        }
        if (index.row() == prevRow) {
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
};

ComplexDBTableDialog::ComplexDBTableDialog(DBTableInfo *dbTable, QWidget *parent)
    : DBTableDialog(dbTable, parent)
    , ui(new Ui::ComplexDBTableDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    QTableView *view = ui->m_tableContents;
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents); // TODO: resize to contents OR NOT?
//    view->horizontalHeader()->resizeSection(
    view->setModel(m_model);
    view->setItemDelegate(new HighlightTableDelegate(view));
    view->viewport()->setAttribute(Qt::WA_Hover);
}

ComplexDBTableDialog::~ComplexDBTableDialog()
{
    delete ui;
}

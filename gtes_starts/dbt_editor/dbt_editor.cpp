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
#include "../model/proxy_model.h"
#include "../common/db_info.h"

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
    , m_proxyModel(new ProxySqlModel(this))
{
    m_ui->setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowMaximizeButtonHint);
    setModel();
    setWindowName();
    setContentsUI();
    setEditingUI();
}

DBTEditor::~DBTEditor()
{
    delete m_ui;
    delete m_proxyModel;
}

cmmn::T_id DBTEditor::selectedId() const
{
    return m_proxyModel->selectedId();
}

void DBTEditor::setContentsUI()
{
    QTableView *view = m_ui->m_tableContents;
    view->setModel(m_proxyModel); // TODO: use m_proxyModel.get()
    view->setItemDelegate(new HighlightTableRowsDelegate(view));
    view->viewport()->setAttribute(Qt::WA_Hover);
    view->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    view->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    view->setAlternatingRowColors(true);

    // set selection
    connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            m_proxyModel, SLOT(slotChooseRow(QItemSelection,QItemSelection))); // TODO: use m_proxyModel.get()
    connect(m_proxyModel, SIGNAL(sigNeedUpdateView(QModelIndex)), view, SLOT(update(QModelIndex))); // TODO: use m_proxyModel.get()
}

void DBTEditor::setEditingUI()
{
    // TODO: code here
}

void DBTEditor::setModel()
{
    m_proxyModel->customSourceModel()->setTable(m_DBTInfo->m_nameInDB);
}

void DBTEditor::setWindowName()
{
    setWindowTitle( tr("Editing the table:") + " " + m_DBTInfo->m_nameInUI );
}

void DBTEditor::selectInitial(const QVariant &idPrim)
{
    int selectedRow = -1;
    ASSERT_DBG( m_proxyModel->customSourceModel()->findPrimaryIdRow(idPrim, selectedRow),
                cmmn::MessageException::type_warning, tr("Selection error"),
                tr("Cannot select the row in the table \"%1\". Cannot find the item with id: %2")
                .arg(m_DBTInfo->m_nameInUI).arg(idPrim.toString()),
                QString("FormDataInput::slotEditDBT()") );
    m_ui->m_tableContents->selectRow(selectedRow);
}

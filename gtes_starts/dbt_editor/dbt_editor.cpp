#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QTableView>
#include <QHeaderView>
#include <QItemSelection>
#include <QMessageBox>
#include <QLabel>
#include <QDebug>
#include "dbt_editor.h"
#include "ui_dbt_editor.h"
#include "../model/custom_sql_table_model.h"
#include "../model/proxy_model.h"
#include "../common/db_info.h"
#include "../common/fl_widgets.h"

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
DBTEditor::DBTEditor(const dbi::DBTInfo *dbTable, QWidget *parent)
    : QDialog(parent)
    , m_DBTInfo(dbTable)
    , m_ui(new Ui::DBTEditor)
    , m_proxyModel(new ProxyChoiceDecorModel(this))
    , m_mapper(new QDataWidgetMapper(this))
{
    m_ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowName();
    setWindowSize();
    setModel();
    setMapper();
    setSelectUI();
    setEditingUI();
    setDataNavigation();
}

DBTEditor::~DBTEditor()
{
    delete m_ui;
    delete m_proxyModel;
    delete m_mapper;
}

void DBTEditor::setWindowName()
{
    setWindowTitle( tr("Editing the table:") + " " + m_DBTInfo->m_nameInUI );
}

void DBTEditor::setWindowSize()
{
    // TODO: set window size depending on the DB table size
}

void DBTEditor::setModel()
{
    m_proxyModel->setSqlTable(m_DBTInfo->m_nameInDB);
}

void DBTEditor::setMapper()
{
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    m_mapper->setModel(m_proxyModel); // TODO: use m_proxyModel.get()
}

void DBTEditor::setSelectUI()
{
    QTableView *view = m_ui->m_tableContents;
    view->setModel(m_proxyModel); // TODO: use m_proxyModel.get()
    view->setItemDelegate(new HighlightTableRowsDelegate(view));
    view->viewport()->setAttribute(Qt::WA_Hover);
    // set headers - make select view of big data in the last column
    auto vHeader = view->verticalHeader();
    vHeader->setDefaultSectionSize( vHeader->defaultSectionSize() * 0.75 ); // reduce rows high - need for the QHeaderView::Fixed resize mode
    vHeader->setSectionResizeMode(QHeaderView::Fixed);
    auto hHeader = view->horizontalHeader();
    hHeader->setSectionResizeMode(QHeaderView::ResizeToContents);
    hHeader->setStretchLastSection(true);
    view->setMinimumWidth(hHeader->length() + 30); // increase view width, that vertical scroll widget do not cover data in the last table column

    /*
     * It is not properly to use the currentRowChanged signal for choose row, because in time of the
     * currentRowChanged signal calling, items is still not selected. Selecting items performs after changing
     * current row (or column). Because of this there are need to use only selectionChanged signal for choose some row.
     */
    connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            m_proxyModel, SLOT(slotChooseRow(QItemSelection,QItemSelection))); // TODO: use m_proxyModel.get()
    connect(m_proxyModel, SIGNAL(sigNeedUpdateView(QModelIndex)), view, SLOT(update(QModelIndex))); // TODO: use m_proxyModel.get()
}

void DBTEditor::setEditingUI()
{
    // TODO: maybe move this to the external builder class. This class can use the FormDataInput class too.
    int rowWgtSpan = 1;
    QWidget *wgt = 0;
    QGridLayout *layout = new QGridLayout(m_ui->m_gboxEditingData);
    for (int i = 0; i < m_DBTInfo->tableDegree(); ++i) {
        const auto &dbtField = m_DBTInfo->fieldByIndex(i);

        if (dbtField.m_widgetType == dbi::DBTFieldInfo::wtype_plainTextEdit) {
            rowWgtSpan = 2;
            layout->addItem(new QSpacerItem(10, 70), i + 1, 0);
        }
        else rowWgtSpan = 1;

        wgt = createFieldWidget( dbtField.m_widgetType, dbtField.isKey(), this, SLOT(slotFocusLost_DataSet(QString)) );
        m_mapper->addMapping(wgt, i + 1);

        // put widgets to the layout
        layout->addWidget( createFieldDescLabel(dbtField.m_nameInUI), i, 0 );
        layout->addWidget( wgt, i, 1, rowWgtSpan, 1 ); // set read only status if it is a key
        if (dbtField.isForeign()) {
            layout->addWidget(createSelEdPButton(), i, 3);
        }
    }
}

QLabel * DBTEditor::createFieldDescLabel(const QString &text) const
{
    // create field description label
    return new QLabel(text + ":"); // TODO: use smart pointer
}

QPushButton *DBTEditor::createSelEdPButton() const
{
    // create select/edit push button
    QPushButton *cmd = new QPushButton(QIcon(":/images/edit.png"), tr("Select/Edit")); // TODO: use smart pointer
    cmd->setStyleSheet("text-align: left");
    return cmd;
}

void DBTEditor::setDataNavigation()
{
    connect(m_ui->m_tableContents->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            m_mapper, SLOT(setCurrentModelIndex(QModelIndex)));
}

void DBTEditor::selectInitial(const QVariant &idPrim)
{
    int selectedRow = -1;
    ASSERT_DBG( m_proxyModel->customSourceModel()->findRowWithId(idPrim, selectedRow),
                cmmn::MessageException::type_warning, tr("Selection error"),
                tr("Cannot select the row in the table \"%1\". Cannot find the item with id: %2")
                .arg(m_DBTInfo->m_nameInUI).arg(idPrim.toString()),
                QString("FormDataInput::slotEditChildDBT()") );
    m_ui->m_tableContents->selectRow(selectedRow);
    m_mapper->setCurrentIndex(selectedRow);
}

cmmn::T_id DBTEditor::selectedId() const
{
    return m_proxyModel->selectedId();
}

void DBTEditor::slotFocusLost_DataSet(const QString &data)
{
    QWidget *wgt = qobject_cast<QWidget*>(sender());
    ASSERT_DBG( wgt, cmmn::MessageException::type_warning, tr("Error getting a widget"),
                tr("Cannot get the widget from sender"), QString("FormDataInput::slotFocusLost_DataSet()") );
    const QModelIndex &currIndex = m_proxyModel->index( m_mapper->currentIndex(), m_mapper->mappedSection(wgt) );
    m_proxyModel->setData(currIndex, data, Qt::EditRole);
}

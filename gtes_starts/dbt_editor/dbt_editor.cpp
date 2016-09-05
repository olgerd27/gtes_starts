#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QItemSelection>
#include <QMessageBox>
#include <QLabel>
#include <QSplitter> // TODO: temp, delete later
#include <QDebug>
#include "dbt_editor.h"
#include "ui_dbt_editor.h"
#include "../model/custom_sql_table_model.h"
#include "../model/proxy_model.h"
#include "edit_ui_creator.h"
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
DBTEditor::DBTEditor(const dbi::DBTInfo *dbtInfo, QWidget *parent)
    : QDialog(parent)
    , m_DBTInfo(dbtInfo)
    , m_ui(new Ui::DBTEditor)
    , m_proxyModel(new ProxyChoiceDecorModel(this))
    , m_mapper(new QDataWidgetMapper(this))
    , m_editUICreator(new EditUICreator(m_DBTInfo, m_mapper, this))
{
    m_ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setWindowPosition();
    setWindowName();
    setModel();
    setMapper();
    setSelectUI();
    setEditingUI();
    setControl();
    setDataNavigation();

//    // --- The test vizualization of the model data ---
//    // The proxy model
//    QTableView *tablePrx = new QTableView;
//    tablePrx->setWindowTitle( QString("Proxy model for debugging. Use the \"%1\" DB table")
//                              .arg(m_proxyModel->customSourceModel()->tableName()) );
//    tablePrx->setSelectionBehavior(QAbstractItemView::SelectRows);
//    tablePrx->setSelectionMode(QAbstractItemView::SingleSelection);
//    tablePrx->setModel(m_proxyModel); // TODO: use m_proxyModel.get()
//    tablePrx->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    tablePrx->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    connect(m_mapper, SIGNAL(currentIndexChanged(int)), tablePrx, SLOT(selectRow(int))); // TODO: use m_mapper.get()
//    // selection setting - testing
//    connect(tablePrx->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//            m_proxyModel, SLOT(slotChooseRow(QItemSelection,QItemSelection))); // TODO: use m_proxyModel.get()

//    // The source model
//    QTableView *tableSrc = new QTableView;
//    tableSrc->setWindowTitle( QString("Source model for debugging. Use the \"%1\" DB table")
//                              .arg(m_proxyModel->customSourceModel()->tableName()) );
//    tableSrc->setSelectionBehavior(QAbstractItemView::SelectRows);
//    tableSrc->setSelectionMode(QAbstractItemView::SingleSelection);
//    tableSrc->setModel(m_proxyModel->customSourceModel());
//    tableSrc->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    tableSrc->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    //    connect(m_mapper, SIGNAL(currentIndexChanged(int)), tableSrc, SLOT(selectRow(int))); // TODO: use m_mapper.get()
//    // selection setting - testing
//    connect(tablePrx->selectionModel(), &QItemSelectionModel::selectionChanged,
//            [tableSrc](const QItemSelection &selected, const QItemSelection &)
//    {
//        const QModelIndexList &selectedList = selected.indexes();
//        if (selectedList.size() > 0)
//            tableSrc->selectRow(selectedList.at(0).row());
//    } );

//    QSplitter *sp = new QSplitter;
//    sp->addWidget(tableSrc);
//    sp->addWidget(tablePrx);
//    sp->setWindowTitle( QString("Source and Proxy models of the table: %1").arg(m_proxyModel->customSourceModel()->tableName()) );
//    sp->move(10, 10);
//    sp->show();
}

DBTEditor::~DBTEditor()
{
    delete m_ui;
    delete m_proxyModel;
    delete m_mapper;
}

void DBTEditor::setWindowPosition()
{
    // if parent widget is not dialog, this dialog appears in the center of parent widget by default
    if (parentWidget()->windowFlags() & Qt::Dialog)
        this->move( parentWidget()->pos() + QPoint(shiftCEByX, shiftCEByY) );
}

void DBTEditor::setWindowName()
{
    setWindowTitle( tr("Editing the table:") + " " + m_DBTInfo->m_nameInUI );
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
    hHeader->setStretchLastSection(true);
    setHorizSectionResizeMode(hHeader);
    view->setMinimumWidth(hHeader->length() + 30); // increase view width, that vertical scroll widget do not cover data in the last table column

    /*
     * NOTE: It is not properly to use the currentRowChanged signal for choose row, because in time of the
     * currentRowChanged signal calling, items is still not selected. Selecting items performs after changing
     * current row (or column). Because of this there are need to use only selectionChanged signal for choose some row.
     */
    connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            m_proxyModel, SLOT(slotChooseRow(QItemSelection,QItemSelection))); // TODO: use m_proxyModel.get()
}

void DBTEditor::setHorizSectionResizeMode(QHeaderView *header)
{
#ifdef __linux__
    // resize decoration field to fit size
    for (int i = 0; i < header->count(); ++i) {
        if (i == 0) {
            header->setSectionResizeMode(i, QHeaderView::Fixed);
            header->resizeSection(i, m_proxyModel->decorationSize().width());
        }
        else header->setSectionResizeMode(i, QHeaderView::ResizeToContents);
    }
#else
    header->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
}

void DBTEditor::setEditingUI()
{
    m_editUICreator->createUI(m_ui->m_gboxEditingData);
    connect(m_editUICreator.get(), SIGNAL(sigWidgetFocusLost(QWidget*,QString)),
            this, SLOT(slotFocusLost_DataSet(QWidget*,QString))); // set data to the model when widget lose the focus
    connect(m_editUICreator.get(), SIGNAL(sigSEPBClicked(const dbi::DBTInfo*,int)),
            this, SLOT(slotEditChildDBT(const dbi::DBTInfo*,int))); // open child DBT edit dialog - provide recursive opening of dialog
}

void DBTEditor::setControl()
{
    connect(m_ui->m_pbAdd, SIGNAL(clicked()), m_proxyModel, SLOT(slotAddRow()));
    connect(m_ui->m_pbDelete, SIGNAL(clicked()), m_proxyModel, SLOT(slotDeleteRow()));
    connect(m_ui->m_pbRefresh, SIGNAL(clicked()), m_proxyModel, SLOT(slotRefreshModel()));
}

void DBTEditor::setDataNavigation()
{
    connect(m_proxyModel, SIGNAL(sigChangeCurrentRow(int)), m_mapper, SLOT(setCurrentIndex(int)));
    connect(m_ui->m_tableContents->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            m_mapper, SLOT(setCurrentModelIndex(QModelIndex))); // select rows in the view -> show data in the mapped widgets
    connect(m_mapper, SIGNAL(currentIndexChanged(int)), m_ui->m_tableContents, SLOT(selectRow(int))); // the aim: add a new row -> select it
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

void DBTEditor::slotEditChildDBT(const dbi::DBTInfo *dbtInfo, int fieldNo)
{
    const QModelIndex &currIndex = m_proxyModel->index( m_mapper->currentIndex(), fieldNo + ProxyChoiceDecorModel::COUNT_ADDED_COLUMNS );
    const QVariant &forId = m_proxyModel->data(currIndex, Qt::UserRole);
    DBTEditor childEditor(dbtInfo, this);
    if ( !forId.isNull() ) // if data is NULL -> don't select any row in the editor view
        childEditor.selectInitial(forId);
    if ( childEditor.exec() == QDialog::Accepted ) {
        m_proxyModel->customSourceModel()->spike1_turnOn(true); /* Switch ON the Spike #1 */
        ASSERT_DBG( m_proxyModel->setData( currIndex, childEditor.selectedId(), Qt::EditRole ),
                    cmmn::MessageException::type_critical, tr("Error data setting"),
                    tr("Cannot set data: \"%1\" to the model").arg(childEditor.selectedId()),
                    QString("DBTEditor::slotEditChildDBT()") );
        qDebug() << "The id value: \"" << childEditor.selectedId() << "\" was successfully setted to the model";
    }
}

void DBTEditor::slotFocusLost_DataSet(QWidget *w, const QString &data)
{
    const QModelIndex &currIndex = m_proxyModel->index( m_mapper->currentIndex(), m_mapper->mappedSection(w) );
    m_proxyModel->setData(currIndex, data, Qt::EditRole);
}

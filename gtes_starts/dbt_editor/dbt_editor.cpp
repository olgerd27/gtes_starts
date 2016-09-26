#include <QSqlTableModel>
#include <QSqlError>
#include <QSqlQuery>
#include <QItemSelection>
#include <QMessageBox>
#include <QLabel>
#include <QSplitter> // TODO: temp, delete later
#include <QDebug>
#include "dbt_editor.h"
#include "ui_dbt_editor.h"
#include "../model/custom_sql_table_model.h"
#include "../model/proxy_model.h"
#include "../model/selection_allower.h"
#include "edit_ui_creator.h"
#include "../common/db_info.h"
#include "../common/fl_widgets.h"
#include "../common/reimplemented_widgets.h"

/*
 * DBTEditor
 * TODO: add the apply and revert push buttons on this window, as in example "Cached table"
 */
DBTEditor::DBTEditor(const dbi::DBTInfo *dbtInfo, QWidget *parent)
    : QDialog(parent)
    , m_DBTInfo(dbtInfo)
    , m_ui(new Ui::DBTEditor)
    , m_proxyModel(new ProxyChoiceDecorModel(nullptr))
    , m_sfProxyModel(new ProxyFilterModel(nullptr))
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
//    tablePrx->setModel(m_proxyModel);
//    tablePrx->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    tablePrx->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    connect(m_mapper, SIGNAL(currentIndexChanged(int)), tablePrx, SLOT(selectRow(int)));
//    // selection setting - testing
//    connect(tablePrx->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//            m_proxyModel, SLOT(slotChooseRow(QItemSelection,QItemSelection)));

//    // The source model
//    QTableView *tableSrc = new QTableView;
//    tableSrc->setWindowTitle( QString("Source model for debugging. Use the \"%1\" DB table")
//                              .arg(m_proxyModel->customSourceModel()->tableName()) );
//    tableSrc->setSelectionBehavior(QAbstractItemView::SelectRows);
//    tableSrc->setSelectionMode(QAbstractItemView::SingleSelection);
//    tableSrc->setModel(m_proxyModel->customSourceModel());
//    tableSrc->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    tableSrc->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    //    connect(m_mapper, SIGNAL(currentIndexChanged(int)), tableSrc, SLOT(selectRow(int)));
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

    // --- Experiments with using QSortProxyFilterModel ---
//    QTableView *tablePrx = new QTableView;
//    tablePrx->setWindowTitle( QString("Proxy sort/filter model for debugging. Use the \"%1\" DB table")
//                              .arg(m_proxyModel->customSourceModel()->tableName()) );
//    tablePrx->setSelectionBehavior(QAbstractItemView::SelectRows);
//    tablePrx->setSelectionMode(QAbstractItemView::SingleSelection);

//    QSortFilterProxyModel *sfPrxModel = new QSortFilterProxyModel(tablePrx);
//    sfPrxModel->setSourceModel(m_proxyModel);
//    sfPrxModel->setFilterKeyColumn(-1);
//    tablePrx->setModel(sfPrxModel);

//    tablePrx->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    tablePrx->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    connect(m_mapper, SIGNAL(currentIndexChanged(int)), tablePrx, SLOT(selectRow(int)));

//    QLineEdit *leFilter = new QLineEdit;
//    connect(leFilter, SIGNAL(textChanged(QString)), sfPrxModel, SLOT(setFilterFixedString(QString)));

//    m_ui->verticalLayout->addWidget(leFilter);
//    m_ui->verticalLayout->addWidget(tablePrx);
}

DBTEditor::~DBTEditor()
{
    delete m_ui;
    /*
     * if parent of proxy models is not nullptr, do not delete model memory here.
     * It must be deleted when performs deletion of parent.
     * Even if parent of model is DBTEditor, do not delete model here.
     */
    delete m_sfProxyModel;
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
    setWindowTitle( tr("The DB table:") + " " + m_DBTInfo->m_nameInUI );
}

void DBTEditor::setModel()
{
    m_proxyModel->setSqlTable(m_DBTInfo->m_nameInDB);
    m_sfProxyModel->setSourceModel(m_proxyModel);
    m_sfProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_sfProxyModel->setFilterKeyColumn(-1);
}

void DBTEditor::setMapper()
{
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
//    m_mapper->setModel(m_proxyModel);
    m_mapper->setModel(m_sfProxyModel);
}

void DBTEditor::setSelectUI()
{
    auto view = m_ui->m_tableContents; // get the table view
    m_ui->m_tableContents->setModel(m_sfProxyModel);
//    m_ui->m_tableContents->setModel(m_proxyModel);

    // set horizontal header - this header settings must be here (after model setting)
    auto hHeader = view->horizontalHeader();
    hHeader->setStretchLastSection(true);
    view->setMinimumWidth(hHeader->length() + 30); // increase view width, that vertical scroll widget do not cover data in the last table column

    setFilter();

    /*
     * NOTE: It is not properly to use the currentRowChanged signal for choose row, because in time of the
     * currentRowChanged signal calling, items is still not selected. Selecting items performs after changing
     * current row (or column). Because of this there are need to use only selectionChanged signal for choose some row.
     */
//    connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//            m_proxyModel, SLOT(slotChooseRow(QItemSelection,QItemSelection)));
    connect(view->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            m_sfProxyModel, SLOT(slotChooseRow(QItemSelection,QItemSelection)));
}

void DBTEditor::setFilter()
{
    // NOTE: creation of the SelectionAllower_IC must perform before connection the selectionModel::selectionChanged with the slotChooseRow().
    // Also, before creating the SelectionAllower_IC model must be setted to the tableView.
    SelectionAllower_IC *sa = new SelectionAllower_IC(m_ui->m_leFilter, m_ui->m_tableContents, m_sfProxyModel);
    m_sfProxyModel->setSelectionAllower(sa);
    connect(m_sfProxyModel, SIGNAL(sigSelectionEnded()), sa, SLOT(slotSelectionEnded()));
    connect(m_ui->m_leFilter, SIGNAL(textChanged(QString)), m_sfProxyModel, SLOT(setFilterFixedString(QString)));
}

void DBTEditor::setEditingUI()
{
    m_editUICreator->createUI(m_ui->m_gboxEditingData);
    connect(m_editUICreator.get(), SIGNAL(sigWidgetFocusLost(QWidget*,QString)),
            this, SLOT(slotFocusLost_DataSet(QWidget*,QString))); // set data to the model when widget lose the focus
    connect(m_editUICreator.get(), SIGNAL(sigSEPBClicked(const dbi::DBTInfo*,int)),
            this, SLOT(slotEditChildDBT(const dbi::DBTInfo*,int))); // open child DBT edit dialog - perform recursive opening of dialog
}

void DBTEditor::setControl()
{
    connect(m_ui->m_pbAdd, SIGNAL(clicked()), m_proxyModel, SLOT(slotAddRow()));
    connect(m_ui->m_pbDelete, &QPushButton::clicked, [this]()
    {
        m_proxyModel->slotDeleteRow( m_mapper->currentIndex() );
    } );
    connect(m_ui->m_pbSave, &QPushButton::clicked, [this]()
    {
        m_proxyModel->slotSaveDataToDB( m_mapper->currentIndex() );
    } );
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
    m_initSelectRow = -1;
    ASSERT_DBG( m_proxyModel->customSourceModel()->getIdRow(idPrim, m_initSelectRow),
                cmmn::MessageException::type_warning, tr("Selection error"),
                tr("Cannot select the row in the table \"%1\". Cannot find the item with id: %2")
                .arg(m_DBTInfo->m_nameInUI).arg(idPrim.toString()),
                QString("DBTEditor::selectInitial") );
    m_ui->m_tableContents->selectRow(m_initSelectRow);
}

cmmn::T_id DBTEditor::selectedId() const
{
    return m_proxyModel->selectedId();
}

void DBTEditor::accept()
{
    if (m_proxyModel->customSourceModel()->isDirty()) {
        /*
         * TODO: add application settings - "Automatic save by clicking "Ok"" (checkbox).
         * IF this setting is setted - make data autosaving when clicking "Ok" push button, ELSE - ask confirmation in user
         */
        bool autoSave = false; // test, delete later
        if (autoSave)
            m_proxyModel->slotSaveDataToDB( m_mapper->currentIndex() );
        else {
            auto btnChoosed =
                    QMessageBox::question( this, tr("Save changes"),
                                           QString("<font size=+1><b>") +
                                           tr("The data of the \"%1\" DB table has been modified.").arg(m_DBTInfo->m_nameInUI) +
                                           QString("</b></font><br><br>") +
                                           tr("Do you want to save your changes?"),
                                           QMessageBox::Save | QMessageBox::Discard | QMessageBox::Close, QMessageBox::Save);
            switch (btnChoosed) {
            case QMessageBox::Save:
                m_proxyModel->slotSaveDataToDB( m_mapper->currentIndex() ); // save data to the DB
                break;
            case QMessageBox::Discard:
                m_ui->m_tableContents->selectRow(m_initSelectRow); // restore initial row selection
                break;
            case QMessageBox::Close:
                return; // just close this dialog window
            default:
                ASSERT_DBG( false, cmmn::MessageException::type_warning, tr("Unknow button clicked"),
                            tr("There was clicked unknown button: %1").arg((int)btnChoosed), QString("DBTEditor::accept"))
                break;
            }
        }
    }
    QDialog::accept();
}

void DBTEditor::slotEditChildDBT(const dbi::DBTInfo *dbtInfo, int fieldNo)
{
    const QModelIndex &currIndex = m_proxyModel->index( m_mapper->currentIndex(), fieldNo + ProxyChoiceDecorModel::COUNT_ADDED_COLUMNS );
    const QVariant &forId = m_proxyModel->data(currIndex, Qt::UserRole);
    DBTEditor childEditor(dbtInfo, this);
    connect(childEditor.m_proxyModel->customSourceModel(), SIGNAL(sigSavedInDB()),
            m_proxyModel->customSourceModel(), SLOT(slotRefreshTheModel())); // save changes in the child model -> refresh the parent (current) model
    if ( !forId.isNull() ) // if data is NULL -> don't select any row in the view
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

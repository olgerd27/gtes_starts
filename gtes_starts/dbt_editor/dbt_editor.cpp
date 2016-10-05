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
#include "../widgets/fl_widgets.h"
#include "../widgets/reimplemented_widgets.h"

/*
 * DBTEditor
 * TODO: add the apply and revert push buttons on this window, as in example "Cached table"
 */
DBTEditor::DBTEditor(const dbi::DBTInfo *dbtInfo, QWidget *parent)
    : QDialog(parent)
    , m_DBTInfo(dbtInfo)
    , m_ui(new Ui::DBTEditor)
    , m_prxDecorMdl_1(new ProxyDecorModel(nullptr))
    , m_prxFilterMdl_2(new ProxyFilterModel(nullptr))
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
    setEditUI();
    setControl();
    setDataNavigation();
}

DBTEditor::~DBTEditor()
{
    delete m_ui;
    /*
     * If parent of proxy models is not nullptr (even if parent of model is DBTEditor), do not free memory of model at here.
     * In other words, if parent of proxy models is not nullptr, it must be deleted when performs deletion of parent.
     * Free model's memory here only if parent is nullptr.
     */
    delete m_prxFilterMdl_2;
    delete m_prxDecorMdl_1;
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
    m_prxDecorMdl_1->setSqlTableName(m_DBTInfo->m_nameInDB);
    m_prxFilterMdl_2->setSourceModel(m_prxDecorMdl_1);
    m_prxFilterMdl_2->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_prxFilterMdl_2->setFilterKeyColumn(-1);
}

void DBTEditor::setMapper()
{
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
//    m_mapper->setModel(m_prxDecorMdl_1);
    m_mapper->setModel(m_prxFilterMdl_2);
}

void DBTEditor::setSelectUI()
{
    m_ui->m_tableContents->setModel(m_prxFilterMdl_2);
//    m_ui->m_tableContents->setModel(m_prxDecorMdl_1);

    setFilter();

    /*
     * NOTE: It is not properly to use the currentRowChanged signal for choose row, because in time of the
     * currentRowChanged signal calling, items is still not selected. Selecting items performs after changing
     * current row (or column). Because of this there are need to use only selectionChanged signal for choose some row.
     */
//    connect(m_ui->m_tableContents->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//            m_prxDecorMdl_1, SLOT(slotChooseRow(QItemSelection,QItemSelection)));
    connect(m_ui->m_tableContents->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            m_prxFilterMdl_2, SLOT(slotChooseRow(QItemSelection,QItemSelection)));
}

void DBTEditor::setFilter()
{
    // NOTE: creation of the SelectionAllower_IC must perform before connection the selectionModel::selectionChanged with the slotChooseRow().
    // Also, before creating the SelectionAllower_IC model must be setted to the tableView.
    SelectionAllower_IC *sa = new SelectionAllower_IC(m_ui->m_leFilter, m_ui->m_tableContents, m_prxFilterMdl_2);
    m_prxFilterMdl_2->setSelectionAllower(sa);
    connect(m_prxFilterMdl_2, SIGNAL(sigSelectionEnded()), sa, SLOT(slotSelectionEnded()));
    connect(m_ui->m_leFilter, SIGNAL(textChanged(QString)), m_prxFilterMdl_2, SLOT(setFilterFixedString(QString)));
}

void DBTEditor::setEditUI()
{
    m_editUICreator->createUI(m_ui->m_gboxEditingData);
    connect(m_editUICreator.get(), SIGNAL(sigWidgetFocusLost(QWidget*,QString)),
            this, SLOT(slotFocusLost_DataSet(QWidget*,QString))); // set data to the model when widget lose the focus
    connect(m_editUICreator.get(), SIGNAL(sigSEPBClicked(const dbi::DBTInfo*,int)),
            this, SLOT(slotEditChildDBT(const dbi::DBTInfo*,int))); // open child DBT edit dialog - perform recursive opening of dialog
}

void DBTEditor::setControl()
{
    connect(m_ui->m_pbAdd, SIGNAL(clicked()), m_prxDecorMdl_1, SLOT(slotAddRow()));
    connect(m_ui->m_pbDelete, &QPushButton::clicked, [this]()
    {
        m_prxDecorMdl_1->slotDeleteRow( m_mapper->currentIndex() );
    } );
    connect(m_ui->m_pbSave, &QPushButton::clicked, [this]()
    {
        m_prxDecorMdl_1->slotSaveDataToDB( m_mapper->currentIndex() );
    } );
    connect(m_ui->m_pbRefresh, &QPushButton::clicked, [this]()
    {
        m_prxDecorMdl_1->slotRefreshModel( m_mapper->currentIndex() );
    } );
}

void DBTEditor::setDataNavigation()
{
    connect(m_prxDecorMdl_1, SIGNAL(sigChangeCurrentRow(int)), m_mapper, SLOT(setCurrentIndex(int)));
    connect(m_ui->m_tableContents->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            m_mapper, SLOT(setCurrentModelIndex(QModelIndex))); // select rows in the view -> show data in the mapped widgets
    connect(m_mapper, SIGNAL(currentIndexChanged(int)), m_ui->m_tableContents, SLOT(selectRow(int))); // the aim: add a new row -> select it
}

void DBTEditor::selectInitial(const QVariant &idPrim)
{
    m_initSelectRow = -1;
    ASSERT_DBG( m_prxDecorMdl_1->customSourceModel()->getIdRow(idPrim, m_initSelectRow),
                cmmn::MessageException::type_warning, tr("Selection error"),
                tr("Cannot select the row in the table \"%1\". Cannot find the item with id: %2")
                .arg(m_DBTInfo->m_nameInUI).arg(idPrim.toString()),
                QString("DBTEditor::selectInitial") );
    m_ui->m_tableContents->selectRow(m_initSelectRow);
}

cmmn::T_id DBTEditor::selectedId() const
{
    return m_prxDecorMdl_1->selectedId();
}

void DBTEditor::accept()
{
    if (m_prxDecorMdl_1->customSourceModel()->isDirty()) {
        /*
         * TODO: add application settings - "Automatic save by clicking "Ok"" (checkbox).
         * IF this setting is setted - make data autosaving when clicking "Ok" push button, ELSE - ask confirmation in user
         */
        bool autoSave = false; // test, delete when will be added setting "Autosave"
        if (autoSave) m_prxDecorMdl_1->slotSaveDataToDB( m_mapper->currentIndex() );
        else askSaving();
    }
    QDialog::accept();
}

void DBTEditor::askSaving()
{
    auto btnChoosed =
            QMessageBox::question( this, tr("Save changes"),
                                   QString("<font size=+1><b>") +
                                   tr("The data of the \"%1\" DB table has been modified.").arg(m_DBTInfo->m_nameInUI) +
                                   QString("</b></font><br><br>") +
                                   tr("Do you want to save your changes?"),
                                   QMessageBox::Save | QMessageBox::Discard, QMessageBox::Save);
    switch (btnChoosed) {
    case QMessageBox::Save:
        m_prxDecorMdl_1->slotSaveDataToDB( m_mapper->currentIndex() ); // save data to the DB
        break;
    case QMessageBox::Discard:
        // Delete all changes and return initial Id value
        m_prxDecorMdl_1->clearDirtyChanges(); // this allow select initial row (below) if it is marked "deleted" and not saved in the DB
        m_ui->m_tableContents->selectRow(m_initSelectRow); // restore initial row selection, that will return to parent DB table (returns Id value)
        break;  // just close this dialog window without changes saving
    default:
        ASSERT_DBG( false, cmmn::MessageException::type_warning, tr("Unknow button clicked"),
                    tr("There was clicked unknown button: %1").arg((int)btnChoosed), QString("DBTEditor::askSaving"))
        break;
    }
}

void DBTEditor::slotEditChildDBT(const dbi::DBTInfo *dbtInfo, int fieldNo)
{
    const QModelIndex &currIndex = m_prxDecorMdl_1->index( m_mapper->currentIndex(), fieldNo + ProxyDecorModel::COUNT_ADDED_COLUMNS );
    const QVariant &forId = m_prxDecorMdl_1->data(currIndex, Qt::UserRole);
    DBTEditor childEditor(dbtInfo, this);
    connect(childEditor.m_prxDecorMdl_1->customSourceModel(), SIGNAL(sigSavedInDB()),
            m_prxDecorMdl_1->customSourceModel(), SLOT(slotRefreshTheModel())); // save changes in the child model -> refresh the parent (current) model
    if ( !forId.isNull() ) // if data is NULL -> don't select any row in the view
        childEditor.selectInitial(forId);
    if ( childEditor.exec() == QDialog::Accepted ) {
        m_prxDecorMdl_1->customSourceModel()->spike1_turnOn(true); /* Switch ON the Spike #1 */
        ASSERT_DBG( m_prxDecorMdl_1->setData( currIndex, childEditor.selectedId(), Qt::EditRole ),
                    cmmn::MessageException::type_critical, tr("Error data setting"),
                    tr("Cannot set data: \"%1\" to the model").arg(childEditor.selectedId()),
                    QString("DBTEditor::slotEditChildDBT") );
        qDebug() << "The id value: \"" << childEditor.selectedId() << "\" was successfully setted to the model";
    }
}

void DBTEditor::slotFocusLost_DataSet(QWidget *w, const QString &data)
{
    const QModelIndex &currIndex = m_prxDecorMdl_1->index( m_mapper->currentIndex(), m_mapper->mappedSection(w) );
    m_prxDecorMdl_1->setData(currIndex, data, Qt::EditRole);
}

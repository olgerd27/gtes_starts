#include <QItemSelection>
#include <QMessageBox>
#include <QDebug>
#include "dbt_editor.h"
#include "ui_dbt_editor.h"
#include "../model/custom_sql_table_model.h"
#include "../model/proxy_filter_model.h"
#include "../model/selection_allower.h"
#include "edit_ui_creator.h"
#include "../common/db_info.h"
#include "../widgets/fl_widgets.h"
#include "../widgets/reimplemented_widgets.h"
#include "../widgets/widget_mapper.h"

/*
 * DBTEditor
 * TODO: add the apply and revert push buttons on this window, as in example "Cached table"
 */
DBTEditor::DBTEditor(const dbi::DBTInfo *dbtInfo, QWidget *parent)
    : QDialog(parent)
    , m_ui(new Ui::DBTEditor)
    , m_DBTInfo(dbtInfo)
    , m_filterPrxModel(new ProxyFilterModel(nullptr))
    , m_mapper(new WidgetMapper(this))
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
    setMainControl();
    setDataNavigation();
}

DBTEditor::~DBTEditor()
{
    delete m_ui;
    delete m_filterPrxModel; // free model's memory here only if it's parent is nullptr
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
    m_filterPrxModel->setSqlTableName(m_DBTInfo->m_nameInDB);
    m_filterPrxModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_filterPrxModel->setFilterKeyColumn(-1);
    connect(m_filterPrxModel->customSourceModel(), SIGNAL(sigSavedInDB()), this, SIGNAL(sigDataSavedInDB())); // transmit save data signal to outside
}

void DBTEditor::setMapper()
{
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    m_mapper->setModel(m_filterPrxModel);
}

void DBTEditor::setSelectUI()
{
    m_ui->m_tableContents->setModel(m_filterPrxModel);

    setFilter();

    /*
     * NOTE: It is not properly to use the currentRowChanged signal for choose row, because in time of the
     * currentRowChanged signal calling, items is still not selected. Selecting items performs after changing
     * current row (or column). Because of this there are need to use only selectionChanged signal for choose some row.
     */
    connect(m_ui->m_tableContents->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            m_filterPrxModel, SLOT(slotChooseRow(QItemSelection,QItemSelection)));
}

void DBTEditor::setFilter()
{
    // NOTE: creation of the SelectionAllower_IC must perform before connection the selectionModel::selectionChanged with the slotChooseRow().
    // Also, before creating the SelectionAllower_IC model must be setted to the tableView.
    SelectionAllower_IC *sa = new SelectionAllower_IC(m_ui->m_leFilter, m_ui->m_tableContents, m_filterPrxModel);
    m_filterPrxModel->setSelectionAllower(sa);
    connect(m_filterPrxModel, SIGNAL(sigSelectionEnded()), sa, SLOT(slotSelectionEnded()));
    connect(m_ui->m_leFilter, SIGNAL(textChanged(QString)), m_filterPrxModel, SLOT(setFilterFixedString(QString)));
}

void DBTEditor::setEditUI()
{
    m_editUICreator->createUI(m_ui->m_gboxEditingData);
    connect(m_editUICreator.get(), SIGNAL(sigSEPBClicked(const dbi::DBTInfo*,int)),
            this, SLOT(slotEditChildDBT(const dbi::DBTInfo*,int))); // open child DBT edit dialog - perform recursive opening of dialog
}

void DBTEditor::setMainControl()
{
    connect(m_ui->m_pbAdd, SIGNAL(clicked()), m_filterPrxModel, SLOT(slotAddRow()));
    connect(m_ui->m_pbDelete, &QPushButton::clicked, [this]()
    {
        m_filterPrxModel->slotDeleteRow( m_mapper->currentIndex() );
    } );
    connect(m_ui->m_pbSave, &QPushButton::clicked, [this]()
    {
        m_filterPrxModel->slotSaveDataToDB( m_mapper->currentIndex() );
    } );
    connect(m_ui->m_pbRefresh, &QPushButton::clicked, [this]()
    {
        m_filterPrxModel->slotRefreshModel( m_mapper->currentIndex() );
    } );
}

void DBTEditor::setDataNavigation()
{
    connect(m_filterPrxModel, SIGNAL(sigChangeCurrentRow(int)), m_mapper, SLOT(setCurrentIndex(int)));
    connect(m_ui->m_tableContents->selectionModel(), SIGNAL(currentRowChanged(QModelIndex,QModelIndex)),
            m_mapper, SLOT(setCurrentModelIndex(QModelIndex))); // select rows in the view -> show data in the mapped widgets
    connect(m_mapper, SIGNAL(currentIndexChanged(int)), m_ui->m_tableContents, SLOT(selectRow(int))); // the aim: add a new row -> select it
}

void DBTEditor::selectInitial(const QVariant &idPrim)
{
    m_initSelectRow = -1;
    ASSERT_DBG( m_filterPrxModel->customSourceModel()->getIdRow(idPrim, m_initSelectRow),
                cmmn::MessageException::type_warning, tr("Selection error"),
                tr("Cannot select the row in the table \"%1\". Cannot find the item with id: %2")
                .arg(m_DBTInfo->m_nameInUI).arg(idPrim.toString()),
                QString("DBTEditor::selectInitial") );
    m_ui->m_tableContents->selectRow(m_initSelectRow);
}

cmmn::T_id DBTEditor::selectedId() const
{
    return m_filterPrxModel->selectedId();
}

void DBTEditor::accept()
{
    if (m_filterPrxModel->customSourceModel()->isDirty()) {
        /*
         * TODO: add application settings - "Automatic save by clicking "Ok"" (checkbox).
         * IF this setting is setted - make data autosaving when clicking "Ok" push button, ELSE - ask confirmation in user
         */
        bool autoSave = false; // test, delete when will be added setting "Autosave"
        if (autoSave) m_filterPrxModel->slotSaveDataToDB( m_mapper->currentIndex() );
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
        m_filterPrxModel->slotSaveDataToDB( m_mapper->currentIndex() ); // save data to the DB
        break;
    case QMessageBox::Discard:
        break;
    default:
        ASSERT_DBG( false, cmmn::MessageException::type_warning, tr("Unknow button clicked"),
                    tr("There was clicked unknown button: %1").arg((int)btnChoosed), QString("DBTEditor::askSaving"))
        break;
    }
}

void DBTEditor::slotEditChildDBT(const dbi::DBTInfo *dbtInfo, int fieldNo)
{
    const QModelIndex &srcIndex = m_filterPrxModel->customSourceModel()->index( m_mapper->currentIndex(), fieldNo );
    const QVariant &foreignId = m_filterPrxModel->customSourceModel()->data(srcIndex, Qt::UserRole);
    DBTEditor childEditor(dbtInfo, this);
    connect(&childEditor, SIGNAL(sigDataSavedInDB()),
            m_filterPrxModel->customSourceModel(), SLOT(slotRefreshTheModel())); // save changes in the child model -> refresh the parent (current) model
    if ( !foreignId.isNull() ) // if data is NULL (this is a new record) -> don't select any row in the view
        childEditor.selectInitial(foreignId);
    if ( childEditor.exec() == QDialog::Accepted ) {
        m_filterPrxModel->customSourceModel()->spike1_turnOn(); // switch ON the Spike #1
        ASSERT_DBG( m_filterPrxModel->customSourceModel()->setData( srcIndex, childEditor.selectedId(), Qt::EditRole ),
                    cmmn::MessageException::type_critical, tr("Error data setting"),
                    tr("Cannot set data: \"%1\" to the model").arg(childEditor.selectedId()),
                    QString("DBTEditor::slotEditChildDBT") );
        qDebug() << "The id value: \"" << childEditor.selectedId() << "\" was successfully setted to the model";
    }
}

﻿#include <QSqlRelationalTableModel>
#include <QSqlRelationalDelegate>
#include <QDataWidgetMapper>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QMessageBox>
#include <QDebug>
#include <memory> /* smart pointer */

#include "form_data_input.h"
#include "ui_form_data_input.h"
#include "../dbt_editor/dbt_editor.h"
#include "../datagen/custom_sql_table_model.h"
#include "../common/db_info.h"

/*
 * ChangerMChTypeImpl - private section of the ChangerMChType class
 */
class ChangerMChTypeImpl
{
public:
    typedef cmmn::T_id T_id;
    typedef QMap<T_id, MChTypeLabel::ChangeTypes> T_modelChanges;
    inline void updateModelChange(T_id idPrim, MChTypeLabel::ChangeTypes changeType) { m_changes[idPrim] = changeType; }
    inline void clearChanges() { m_changes.clear(); }
    void checkModelChanges(T_id idPrim, MChTypeLabel::ChangeTypes &changeType, bool *isDeleted);
private:
    T_modelChanges m_changes;
};

void ChangerMChTypeImpl::checkModelChanges(T_id idPrim, MChTypeLabel::ChangeTypes &changeType, bool *isDeleted)
{
    auto it = m_changes.find(idPrim);
    if (it != m_changes.end()) changeType = it.value();
    *isDeleted = (changeType == MChTypeLabel::ctype_deleted);
}

/*
 * ChangerMChType
 */
ChangerMChType::ChangerMChType(QObject *parent)
    : QObject(parent)
    , m_pImpl(new ChangerMChTypeImpl)
{ }

ChangerMChType::~ChangerMChType()
{ }

void ChangerMChType::updateModelChange(const QVariant &idPrimary, int changeType)
{
    if (!idPrimary.isValid()) {
        // TODO: generate the error
        qCritical() << "[CRITICAL ERROR] Cannot update the model change storage. The primary id value is invalid";
    }
    cmmn::T_id id;
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(idPrimary, id), idPrimary );
    m_pImpl->updateModelChange( id, (MChTypeLabel::ChangeTypes)changeType );
}

void ChangerMChType::clearChanges()
{
    m_pImpl->clearChanges();
}

void ChangerMChType::slotCheckModelChanges(const QVariant &idPrimary)
{
    if (!idPrimary.isValid()) {
        // TODO: generate the error
        qCritical() << "[CRITICAL ERROR] Cannot check the model changes. The primary id value is invalid";
    }
    MChTypeLabel::ChangeTypes ctype = MChTypeLabel::ctype_noChange;
    bool isDeleted = false;
    cmmn::T_id id;
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(idPrimary, id), idPrimary );
    m_pImpl->checkModelChanges( id, ctype, &isDeleted );
    emit sigChangeChangedType(ctype);
    emit sigChangeChangedType(isDeleted);
}

/*
 * FormDataInput
 */
FormDataInput::FormDataInput(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::FormDataInput)
    , m_enginesModel(new CustomSqlTableModel(this))
    , m_mapper(new QDataWidgetMapper(this))
    , m_mchTChanger(new ChangerMChType(this))
{
    m_ui->setupUi(this);
    setMainControls();
    setEditDBTPushButtons();
    setDataOperating();
    setDataNavigation();
    setModelChange();
}

/* set the main commands for data manipulation */
void FormDataInput::setMainControls()
{
    // Insert data
    connect(this, SIGNAL(sigInsertNew()), m_enginesModel.get(), SLOT(slotInsertToTheModel())); // insert data to the model
    // update model changes - this must take place before the connection that update the current index
    connect(m_enginesModel.get(), &CustomSqlTableModel::sigNewRecordInserted,
            [this](int row, cmmn::T_id primId) { Q_UNUSED(row); m_mchTChanger->updateModelChange( primId, MChTypeLabel::ctype_inserted ); } );
    connect(m_enginesModel.get(), SIGNAL(sigNewRecordInserted(int,cmmn::T_id)), m_mapper.get(), SLOT(setCurrentIndex(int))); // go to the inserted row

    // Delete data
    connect(this, SIGNAL(sigDeleteRow(int)), m_enginesModel.get(), SLOT(slotDeleteFromTheModel(int))); // delete row from the model
    // update model changes - this must take place before the connection that update the current index
    connect(m_enginesModel.get(), &CustomSqlTableModel::sigRecordDeleted,
            [this](int row, cmmn::T_id primId) { Q_UNUSED(row); m_mchTChanger->updateModelChange( primId, MChTypeLabel::ctype_deleted ); } );

    /* Behaviour after deletion a some record (row) in the model. The last decision is: after calling delete operation - go to the previous record.
     * This implementation also excellent suit for the case when user insert a new record, don't save that and call the delete operation (from the model).
     * After deletion this record, the mapper toPrevious() slot doesn't work and the ability go to the previous record is calling
     * the setCurrentIndex(int) method with passing previous value of the current index (row).
     */
    connect(m_enginesModel.get(), &CustomSqlTableModel::sigRecordDeleted,
            [this](int row){ if (row > 0) --row; m_mapper->setCurrentIndex(row); } );

    // Save data
    connect(this, SIGNAL(sigSaveAll()), this, SLOT(slotSubmit())); // submit changes from the "engines" model to the DB
    connect(this, &FormDataInput::sigChangesSubmitted, [this](){ m_mchTChanger->clearChanges(); } ); // clearing changes after data saving
    connect(this, SIGNAL(sigChangesSubmitted(int)), m_mapper.get(), SLOT(setCurrentIndex(int)));

    // Refresh data
    connect(this, SIGNAL(sigRefreshAll()), m_enginesModel.get(), SLOT(slotRefreshTheModel())); // refresh all data in the "engines" model
    connect(m_enginesModel.get(), &CustomSqlTableModel::sigModelRefreshed, [this](){ m_mchTChanger->clearChanges(); } ); // clearing changes after data refreshing
    connect(m_enginesModel.get(), SIGNAL(sigModelRefreshed()), m_ui->m_leRecordId, SIGNAL(returnPressed())); // restore the current index

    // Revert data
//    connect(this, SIGNAL(sigRevertChanges()), m_mapper, SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_enginesModel, SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_enginesModel, SLOT(revertAll()));
//    connect(this, SIGNAL(sigRevertChanges()), m_mapper, SLOT(revert()));
}

/* set push buttons, that call some widget for editing DB tables */
void FormDataInput::setEditDBTPushButtons()
{
    setEditDBTOnePB( m_ui->m_pbEditFullName, "full_names_engines", m_ui->m_leFullNameData );
    setEditDBTOnePB( m_ui->m_pbEditFuels, "fuels_types", m_ui->m_cboxFuel );
    setEditDBTOnePB( m_ui->m_pbEditChambers, "combustion_chambers", m_ui->m_leChamberData );
    setEditDBTOnePB( m_ui->m_pbEditStartDevices, "start_devices", m_ui->m_leStartDeviceData );
}

void FormDataInput::setEditDBTOnePB(PBtnForEditDBT *pb, const QString &pbname, QWidget *identWidget)
{
    pb->setDBTableName(pbname);
    pb->setIdentDataWidget(identWidget);
    connect(pb, SIGNAL(clicked()), this, SLOT(slotEditDBT()));
}

#include <QTableView>
void FormDataInput::setDataOperating()
{
    m_enginesModel->setTable("engines");

    // NOTE: for debugging. Delete later ************************************************
    QTableView *table = new QTableView(0);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setModel(m_enginesModel.get());
    table->resize(800, 500);
    table->move(10, 10);
//    table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    table->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    table->show();
    connect(m_mapper.get(), SIGNAL(currentIndexChanged(int)), table, SLOT(selectRow(int)));
    // selection setting - testing
    connect(table->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            m_enginesModel.get(), SLOT(slotChooseRow(QItemSelection,QItemSelection)));
    connect(m_enginesModel.get(), SIGNAL(sigNeedUpdateView(QModelIndex)), table, SLOT(update(QModelIndex)));
    // ***********************************************************************************

    //*************************************************************************************
    // Checking the table view header renaming after setting the relation with other table
//    QSqlRelationalTableModel *model = new QSqlRelationalTableModel(this);
//    model->setTable("engines");
//    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
//    model->setRelation(2, QSqlRelation("fuels_types", "id", "name"));
//    model->select();
//    QTableView *tview = new QTableView(0);
//    tview->setModel(model);
//    tview->setItemDelegate(new QSqlRelationalDelegate(tview));
//    tview->setWindowTitle("Test table view: the \"engines\" DB table");
//    tview->resize(800, 500);
//    tview->move(30, 50);
//    tview->show();
    //*************************************************************************************

    // set combo box
    m_ui->m_cboxFuel->setModel(m_enginesModel->relationModel(3));
    m_ui->m_cboxFuel->setModelColumn(1);
    // set mapper
    m_mapper->setItemDelegate(new CustomSqlRelationalDelegate(this)); // NOTE: is this need?
    m_mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    m_mapper->setModel(m_enginesModel.get());
    // indexes starts from 1, because in the 0-th section place the selection icon
    // TODO: insert new column
    m_mapper->addMapping(m_ui->m_leIdData, 1);
    m_mapper->addMapping(m_ui->m_leFullNameData, 2);
    m_mapper->addMapping(m_ui->m_cboxFuel, 3);
    m_mapper->addMapping(m_ui->m_leChamberData, 4);
    m_mapper->addMapping(m_ui->m_leStartDeviceData, 5);
    m_mapper->addMapping(m_ui->m_sboxStartDevicesQntyData, 6);
    m_mapper->addMapping(m_ui->m_pteComments, 7);
    m_mapper->toFirst();
}

void FormDataInput::setDataNavigation()
{
    m_ui->m_leRecordId->setValidator(new QIntValidator(0, 1e6, m_ui->m_leRecordId)); /* set validator that control inputing only
                                                                                    integer values in range between 0 and 1e6 */
    // navigation set
    connect(m_ui->m_tbRecordFirst, SIGNAL(clicked()), m_mapper.get(), SLOT(toFirst()));
    connect(m_ui->m_tbRecordLast, SIGNAL(clicked()), m_mapper.get(), SLOT(toLast()));
    connect(m_ui->m_tbRecordPrev, SIGNAL(clicked()), m_mapper.get(), SLOT(toPrevious()));
    connect(m_ui->m_tbRecordNext, SIGNAL(clicked()), m_mapper.get(), SLOT(toNext()));

    // set inputing of the "id" value in the line edit
    connect(m_ui->m_leRecordId, SIGNAL(sigReturnPressed(QString)), this, SLOT(slotNeedChangeMapperIndex(QString)));
    connect(this, SIGNAL(sigChangeMapperIndex(int)), m_mapper.get(), SLOT(setCurrentIndex(int))); // change mapper index by the id's line edit value
    connect(this, SIGNAL(sigWrongIdEntered()), m_ui->m_leRecordId, SLOT(clear())); // indicate that inputed value is wrong and there are need to input another
//    connect(this, SIGNAL(sigWrongIdEntered()), m_mapper, SLOT(revert())); // perform a clearing of the mapped widgets - TODO: maybe delete?

    // enable & disable navigation buttons
    connect(m_mapper.get(), SIGNAL(currentIndexChanged(int)), this, SLOT(slotRowIndexChanged(int)));
    connect(this, SIGNAL(sigFirstRowReached(bool)), m_ui->m_tbRecordFirst, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigFirstRowReached(bool)), m_ui->m_tbRecordPrev, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigLastRowReached(bool)), m_ui->m_tbRecordLast, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigLastRowReached(bool)), m_ui->m_tbRecordNext, SLOT(setDisabled(bool)));
    slotRowIndexChanged(m_mapper->currentIndex()); // the initial checking
}

void FormDataInput::setModelChange()
{
    // implementation the behaviour of window form when changes current index
    connect(m_mapper.get(), &QDataWidgetMapper::currentIndexChanged,
            [this](int index){ m_mchTChanger->slotCheckModelChanges( m_enginesModel->primaryIdInRow(index) ); } );
    connect(m_mchTChanger.get(), SIGNAL(sigChangeChangedType(bool)), m_ui->m_gboxEngineData, SLOT(setDisabled(bool)));
    connect(m_mchTChanger.get(), SIGNAL(sigChangeChangedType(int)), m_ui->m_lblModelChangeType, SLOT(slotChangeType(int)));
}

FormDataInput::~FormDataInput()
{ }

void FormDataInput::slotDeleteRow()
{
    qDebug() << "delete row, current mapper index =" << m_mapper->currentIndex();
    emit sigDeleteRow( m_mapper->currentIndex() );
}

void FormDataInput::slotNeedChangeMapperIndex(const QString &value)
{
    int row = -1;
    if (m_enginesModel->findPrimaryIdRow(value, row))
        emit sigChangeMapperIndex(row);
    else {
        QMessageBox::warning(this, tr("Error engine ""id"" value"),
                             tr("The engine with Id=%1 does not exists. Please enter the valid \"Id\" value of an existent engine").arg(value));
        emit sigWrongIdEntered();
    }
}

void FormDataInput::slotRowIndexChanged(int row)
{
//    qDebug() << "change mapper index, current index =" << m_mapper->currentIndex() << ", row =" << row;
    emit sigFirstRowReached(row <= 0);
    emit sigLastRowReached(row >= m_enginesModel->rowCount() - 1);
//    qDebug() << "slotRowIndexChanged(" << row << "), DisplayRole data:";
//    m_enginesModel->printData(Qt::DisplayRole);
}

void FormDataInput::slotSubmit()
{
//    qDebug() << "slotSubmit(), start";
    int currentIndex = m_mapper->currentIndex();
    if (!m_enginesModel->database().transaction()) {
        QMessageBox::critical(this, tr("Database transaction error"),
                              tr("The database driver do not support the transactions operations"));
    }

    if (m_enginesModel->submitAll()) {
//        qDebug() << "slotSubmit(), after submitAll(), before commit()";
//        m_enginesModel->printData(Qt::EditRole);
//        qDebug() << m_enginesModel->printRecords();

        // After submit all data, the mapper current index is -1
        m_enginesModel->database().commit();
    }
    else {
        m_enginesModel->database().rollback();
        QMessageBox::critical(this, tr("Error data submit to the database"),
                              tr("Cannot submit data to the database. The database report an error: %1")
                              .arg(m_enginesModel->lastError().text()));
        return;
    }
    emit sigChangesSubmitted(currentIndex);
//    qDebug() << "slotSubmit(), end";
}

void FormDataInput::slotEditDBT()
{
    PBtnForEditDBT *pbEditDBT = qobject_cast<PBtnForEditDBT *>(sender());
    if ( !pbEditDBT ) {
        /*
         * TODO: wrong error message text. There are need to say some like "cannot edit database table" and
         * not "cannot open the dialog", or maybe there are need rename this slot.
         * This also applies to the other error messageboxes in this slot.
         */
        QMessageBox::critical(this, tr("Invalid widget"),
                              tr("Cannot open the dialog.\n"
                                 "The reason is: clicked on an unexpected push button.\n\n"
                                 "Please consult with the application developer for fixing this problem."));
        return;
    }

    dbi::DBTInfo *tableInfo = DBINFO.tableByName(pbEditDBT->DBTableName());
    if ( !tableInfo ) {
        QMessageBox::critical(this, tr("Invalid push button"),
                              tr("Cannot open the dialog.\n"
                                 "The reason is: cannot define the database table - pressed unknown push button.\n\n"
                                 "Please consult with the application developer for fixing this problem."));
        return;
    }

    DBTEditor editor(tableInfo, this);
    const QModelIndex &currIndex = m_enginesModel->index(m_mapper->currentIndex(), m_mapper->mappedSection(pbEditDBT->identWidget()));
    qDebug() << "before selectInitial(), [" << currIndex.row() << "," << currIndex.column() << "]"
             << ", dataD =" << m_enginesModel->data( currIndex, Qt::DisplayRole).toString()
             << ", dataE =" << m_enginesModel->data( currIndex, Qt::EditRole).toString()
             << ", dataU =" << m_enginesModel->data( currIndex, Qt::UserRole).toString();
    const QVariant &forId = m_enginesModel->data(currIndex, Qt::UserRole);
    if ( !forId.isNull() && !editor.selectInitial(forId) ) // if data is NULL -> don't select any row
        return;

    if ( editor.exec() == QDialog::Accepted ) {
        m_enginesModel->spike1_turnOn(true); /* Switch ON the Spike #1 */
        if ( !m_enginesModel->setData( currIndex, editor.selectedId(), Qt::EditRole ) ) {
            // TODO: generate the error
            qCritical() << "[CRITICAL ERROR] Cannot set data: \"" << editor.selectedId() << "\" to the model";
            return;
        }
        qDebug() << "The id value: \"" << editor.selectedId() << "\" was successfully setted to the model";
    }
}

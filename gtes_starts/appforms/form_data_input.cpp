#include <QSqlRelationalTableModel>
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
#include "../dbt_editor/simple_dbt_editor.h"
#include "../dbt_editor/complex_dbt_editor.h"
#include "../datagen/custom_sql_table_model.h"
#include "../common/db_info.h"

/*
 * ChangerMChTypeImpl
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
{}

void ChangerMChType::updateModelChange(const QVariant &idPrimary, int changeType)
{
    if (!idPrimary.isValid()) {
        // TODO: generate the error
        qCritical() << "Cannot update the model change storage. The primary id value is invalid";
    }
    m_pImpl->updateModelChange( cmmn::safeQVariantToIdType(idPrimary), (MChTypeLabel::ChangeTypes)changeType );
}

void ChangerMChType::clearChanges()
{
    m_pImpl->clearChanges();
}

void ChangerMChType::slotCheckModelChanges(const QVariant &idPrimary)
{
    if (!idPrimary.isValid()) {
        // TODO: generate the error
        qCritical() << "Cannot check the model changes. The primary id value is invalid";
    }
    MChTypeLabel::ChangeTypes ctype = MChTypeLabel::ctype_noChange;
    bool isDeleted = false;
    m_pImpl->checkModelChanges( cmmn::safeQVariantToIdType(idPrimary), ctype, &isDeleted );
    emit sigChangeChangedType(ctype);
    emit sigChangeChangedType(isDeleted);
}

/*
 * FormDataInput
 */
FormDataInput::FormDataInput(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormDataInput)
    , m_enginesModel(new CustomSqlTableModel(this))
    , m_mapper(new QDataWidgetMapper(this))
    , m_mchTChanger(new ChangerMChType(this))
{
    ui->setupUi(this);
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
    connect(this, SIGNAL(sigInsertNew()), m_enginesModel, SLOT(slotInsertToTheModel())); // insert data to the model
    // update model changes - this must take place before the connection that update the current index
    connect(m_enginesModel, &CustomSqlTableModel::sigNewRecordInserted,
            [this](int row, cmmn::T_id primId) { Q_UNUSED(row); m_mchTChanger->updateModelChange( primId, MChTypeLabel::ctype_inserted ); } );
    connect(m_enginesModel, SIGNAL(sigNewRecordInserted(int,cmmn::T_id)), m_mapper, SLOT(setCurrentIndex(int))); // go to the inserted row

    // Delete data
    connect(this, SIGNAL(sigDeleteRow(int)), m_enginesModel, SLOT(slotDeleteFromTheModel(int))); // delete row from the model
    // update model changes - this must take place before the connection that update the current index
    connect(m_enginesModel, &CustomSqlTableModel::sigRecordDeleted,
            [this](int row, cmmn::T_id primId) { Q_UNUSED(row); m_mchTChanger->updateModelChange( primId, MChTypeLabel::ctype_deleted ); } );

    /* Behaviour after deletion a some record (row) in the model. The last decision is: after calling delete operation - go to the previous record.
     * This implementation also excellent suit for the case when user insert a new record, don't save that and call the delete operation (from the model).
     * After deletion this record, the mapper toPrevious() slot doesn't work and the ability go to the previous record is calling
     * the setCurrentIndex(int) method with passing previous value of the current index (row).
     */
    connect(m_enginesModel, &CustomSqlTableModel::sigRecordDeleted,
            [this](int row){ if (row > 0) --row; m_mapper->setCurrentIndex(row); } );

    // Save data
    connect(this, SIGNAL(sigSaveAll()), this, SLOT(slotSubmit())); // submit changes from the "engines" model to the DB
    connect(this, &FormDataInput::sigChangesSubmitted, [this](){ m_mchTChanger->clearChanges(); } ); // clearing changes after data saving
    connect(this, SIGNAL(sigChangesSubmitted(int)), m_mapper, SLOT(setCurrentIndex(int)));

    // Refresh data
    connect(this, SIGNAL(sigRefreshAll()), m_enginesModel, SLOT(slotRefreshTheModel())); // refresh all data in the "engines" model
    connect(m_enginesModel, &CustomSqlTableModel::sigModelRefreshed, [this](){ m_mchTChanger->clearChanges(); } ); // clearing changes after data refreshing
    connect(this, SIGNAL(sigRefreshAll()), this, SLOT(slotNeedChangeMapperIndex())); // restore the current index
    // TODO: emit the signal that imitate or cause return pressed in the line edit, that store the current "id" value and cause calling the slotNeedChangeMapperIndex();

    // Revert data
//    connect(this, SIGNAL(sigRevertChanges()), m_mapper, SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_enginesModel, SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_enginesModel, SLOT(revertAll()));
//    connect(this, SIGNAL(sigRevertChanges()), m_mapper, SLOT(revert()));
}

/* set push buttons, that call some widget for editing DB tables */
void FormDataInput::setEditDBTPushButtons()
{
    setEditDBTOnePB( ui->m_pbEditFullName, "full_names_engines", ui->m_leFullNameData );
    setEditDBTOnePB( ui->m_pbEditFuels, "fuels_types", ui->m_cboxFuel );
    setEditDBTOnePB( ui->m_pbEditChambers, "combustion_chambers", ui->m_leChamberData );
    setEditDBTOnePB( ui->m_pbEditStartDevices, "start_devices", ui->m_leStartDeviceData );
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

    // NOTE: for debugging. Delete later
    QTableView *table = new QTableView(0);
    table->setModel(m_enginesModel);
    table->resize(800, 500);
    table->move(10, 10);
    table->show();

    // set combo box
    ui->m_cboxFuel->setModel(m_enginesModel->relationModel(2));
    ui->m_cboxFuel->setModelColumn(1);
    // set mapper
    m_mapper->setItemDelegate(new CustomSqlRelationalDelegate(this));
    m_mapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);
    m_mapper->setModel(m_enginesModel);
    m_mapper->addMapping(ui->m_leIdData, 0);
    m_mapper->addMapping(ui->m_leFullNameData, 1);
    m_mapper->addMapping(ui->m_cboxFuel, 2);
    m_mapper->addMapping(ui->m_leChamberData, 3);
    m_mapper->addMapping(ui->m_leStartDeviceData, 4);
    m_mapper->addMapping(ui->m_sboxStartDevicesQntyData, 5);
    m_mapper->addMapping(ui->m_pteComments, 6);
    m_mapper->toFirst();
}

void FormDataInput::setDataNavigation()
{
    ui->m_leRecordId->setValidator(new QIntValidator(0, 1e6, ui->m_leRecordId)); /* set validator that control inputing only
                                                                                    integer values in range between 0 and 1e6 */
    // navigation set
    connect(ui->m_tbRecordFirst, SIGNAL(clicked()), m_mapper, SLOT(toFirst()));
    connect(ui->m_tbRecordLast, SIGNAL(clicked()), m_mapper, SLOT(toLast()));
    connect(ui->m_tbRecordPrev, SIGNAL(clicked()), m_mapper, SLOT(toPrevious()));
    connect(ui->m_tbRecordNext, SIGNAL(clicked()), m_mapper, SLOT(toNext()));

    // set inputing of the "id" value in the line edit
    connect(ui->m_leRecordId, SIGNAL(returnPressed()), this, SLOT(slotNeedChangeMapperIndex())); /* TODO: reimplement QLineEdit with
                                                                                                    adding the signal with QString
                                                                                                    argument, that store QLineEdit data */
    connect(this, SIGNAL(sigChangeMapperIndex(int)), m_mapper, SLOT(setCurrentIndex(int))); // change mapper index by the id's line edit value
    connect(this, SIGNAL(sigWrongIdEntered()), ui->m_leRecordId, SLOT(clear())); // indicate that inputed value is wrong and there are need to input another
//    connect(this, SIGNAL(sigWrongIdEntered()), m_mapper, SLOT(revert())); // perform a clearing of the mapped widgets - TODO: maybe delete?

    // enable & disable navigation buttons
    connect(m_mapper, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRowIndexChanged(int)));
    connect(this, SIGNAL(sigFirstRowReached(bool)), ui->m_tbRecordFirst, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigFirstRowReached(bool)), ui->m_tbRecordPrev, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigLastRowReached(bool)), ui->m_tbRecordLast, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigLastRowReached(bool)), ui->m_tbRecordNext, SLOT(setDisabled(bool)));
    slotRowIndexChanged(m_mapper->currentIndex()); // the initial checking
}

void FormDataInput::setModelChange()
{
    // implementation the behaviour of window form when changes current index
    connect(m_mapper, &QDataWidgetMapper::currentIndexChanged,
            [this](int index){ m_mchTChanger->slotCheckModelChanges( m_enginesModel->primaryIdInRow(index) ); } );
    connect(m_mchTChanger, SIGNAL(sigChangeChangedType(bool)), ui->m_gboxEngineData, SLOT(setDisabled(bool)));
    connect(m_mchTChanger, SIGNAL(sigChangeChangedType(int)), ui->m_lblModelChangeType, SLOT(slotChangeType(int)));
}

FormDataInput::~FormDataInput()
{
    delete ui;
    delete m_enginesModel;
    delete m_mapper;
    delete m_mchTChanger;
}

void FormDataInput::slotDeleteRow()
{
    qDebug() << "delete row, current mapper index =" << m_mapper->currentIndex();
    emit sigDeleteRow( m_mapper->currentIndex() );
}

void FormDataInput::slotNeedChangeMapperIndex()
{
    // TODO: the value getting from the ui->m_leRecordId move to the reimplemented QLineEdit class. This slot must receive the id value as argument.
    auto enteredId = ui->m_leRecordId->text().toLongLong();
    // TODO: the search of row, that contains id value perform in the model. Here maybe performs just calling this search function with argument - id value.
    for (int row = 0; row < m_enginesModel->rowCount(); ++row) {
        if (m_enginesModel->record(row).value(0).toLongLong() == enteredId) {
            emit sigChangeMapperIndex(row);
            return;
        }
    }
    QMessageBox::warning(this, tr("Error engine ""id"" value"),
                         tr("The engine with Id=%1 does not exists. Please enter the valid \"Id\" value of an existent engine").arg(enteredId));
    emit sigWrongIdEntered();
}

void FormDataInput::slotRowIndexChanged(int row)
{
    qDebug() << "change mapper index, current index =" << m_mapper->currentIndex() << ", row =" << row;
    emit sigFirstRowReached(row <= 0);
    emit sigLastRowReached(row >= m_enginesModel->rowCount() - 1);
//    qDebug() << "slotRowIndexChanged(" << row << "), DisplayRole data:";
//    m_enginesModel->printData(Qt::DisplayRole);
}

void FormDataInput::slotSubmit()
{
    qDebug() << "slotSubmit(), start";
    int currentIndex = m_mapper->currentIndex(); /* save the current index suit only if performs updating of table. If there are performs
                                                  * delete or insert rows in database operation, there are need to use another way to restore
                                                  * current index (e.g. saving "id" value with following searching current index by it)
                                                  */
//    QMessageBox::information(this, "info", QString("Before transaction, current index = %1").arg(currentIndex));
    if (!m_enginesModel->database().transaction()) {
        QMessageBox::critical(this, tr("Database transaction error"),
                              tr("The database driver do not support the transactions operations"));
    }

//    qDebug() << "slotSubmit(), before submitAll()";
//    m_enginesModel->printData(Qt::EditRole);
//    qDebug() << m_enginesModel->printRecords();
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
//    QMessageBox::information(this, "info", QString("Before set current index to %1").arg(currentIndex));
//    m_mapper->setCurrentIndex(currentIndex);
    emit sigChangesSubmitted(currentIndex);
    qDebug() << "slotSubmit(), end";
}

/* Factory method for creation some type of a database table dialog */
DBTEditor * FormDataInput::createDBTEditor(dbi::DBTInfo *info)
{
    DBTEditor *dlg = 0;
    switch (info->m_type) {
    case dbi::DBTInfo::ttype_simple:
        dlg = new SimpleDBTEditor(info, this);
        break;
    case dbi::DBTInfo::ttype_complex:
        dlg = new ComplexDBTEditor(info, this);
        break;
    default:
        break;
    }
    return dlg;
}

void FormDataInput::slotEditDBT()
{
    PBtnForEditDBT *pbEditDBT = qobject_cast<PBtnForEditDBT *>(sender());
    if ( !pbEditDBT ) {
        QMessageBox::critical(this, tr("Invalid widget"),
                              tr("Cannot open the dialog.\n"
                                 "The reason is: clicked on an unexpected widget.\n\n"
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

    std::shared_ptr<DBTEditor> editor(createDBTEditor(tableInfo));
    if ( !editor.operator bool() ) {
        QMessageBox::critical(this, tr("Invalid database table information"),
                              tr("Cannot open the dialog.\n"
                                 "The reason is: cannot define the created dialog type, database table information is incorrect.\n\n"
                                 "Please consult with the application developer for fixing this problem."));
        return;
    }

    /* define the column number, that used for defining the initial selection in dialog.
     * As the relational table model return a related DB table data (not a foreign key), then there are need to use the next (after "id")
     * column for definition the selection of a row in the table of an opened dialog */
    DBTEditor::ColumnNumbers columnTtype = ( tableInfo->m_type == dbi::DBTInfo::ttype_simple ? DBTEditor::col_firstWithData
                                                                                             : DBTEditor::col_id );
    int currentRow = m_mapper->currentIndex();
    int currentCol = m_mapper->mappedSection(pbEditDBT->identWidget());
    const QModelIndex &currIndex = m_enginesModel->index(currentRow, currentCol);
    qDebug() << "before selectInitial(), columnTtype =" << columnTtype << ", [" << currentRow << "," << currentCol << "]"
             << ", dataD =" << m_enginesModel->data( currIndex, Qt::DisplayRole).toString()
             << ", dataE =" << m_enginesModel->data( currIndex, Qt::EditRole).toString()
             << ", dataU =" << m_enginesModel->data( currIndex, Qt::UserRole).toString();
    const QVariant &userData = m_enginesModel->data(currIndex, Qt::UserRole);
    if ( !userData.isNull() && !editor->selectInitial(userData, columnTtype) ) // data is NULL -> it was not setted and there are not need to select any row
        return;

    if ( editor->exec() == QDialog::Accepted ) {
        m_enginesModel->spike1_turnOn(true); /* Switch ON the Spike #1 */
        m_enginesModel->setData( currIndex, editor->selectedId(), Qt::EditRole );
//        qDebug() << "choosed item with id =" << editor->selectedId();
    }

//    int row = 0;
//    for (int col = 3; col <= 4; ++col)
//        qDebug() << "after dialog, [" << row << "," << col << "]"
//                 << ", dataD =" << m_enginesModel->data( m_enginesModel->index(row, col), Qt::DisplayRole ).toString()
//                 << ", dataE =" << m_enginesModel->data( m_enginesModel->index(row, col), Qt::EditRole ).toString();
    //    qDebug() << "=========================";
}



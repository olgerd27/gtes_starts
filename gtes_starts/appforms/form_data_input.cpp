#include <QDataWidgetMapper>
#include <QSqlError>
#include <QMessageBox>
#include <QSplitter> // TODO: temp, delete later
#include <QDebug>

#include "../model/proxy_model.h"
#include <QTableView> // TODO: for testing, delete later

#include "form_data_input.h"
#include "ui_form_data_input.h"
#include "../dbt_editor/dbt_editor.h"
#include "../model/custom_sql_table_model.h"
#include "../model/proxy_model.h"
#include "../common/db_info.h"
#include "../common/fl_widgets.h"

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
    void checkModelChanges(T_id idPrim, MChTypeLabel::ChangeTypes *changeType, bool *isDeleted);
private:
    T_modelChanges m_changes;
};

void ChangerMChTypeImpl::checkModelChanges(T_id idPrim, MChTypeLabel::ChangeTypes *changeType, bool *isDeleted)
{
    auto it = m_changes.find(idPrim);
    if (it != m_changes.end()) *changeType = it.value();
    *isDeleted = (*changeType == MChTypeLabel::ctype_deleted);
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
    m_pImpl->checkModelChanges( id, &ctype, &isDeleted );
    emit sigChangeChangedType(ctype);
    emit sigChangeChangedType(isDeleted);
}

/*
 * FormDataInput
 */
FormDataInput::FormDataInput(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::FormDataInput)
    , m_proxyModel(new ProxyChoiceDecorModel(this))
    , m_mapper(new QDataWidgetMapper(this))
    , m_mchTChanger(new ChangerMChType(this))
{
    m_ui->setupUi(this);
    setMainControls();
    setEditDBTPushButtons();
    setDataOperating();
    setDataNavigation();
    setModelChange();

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

/* set the main commands for data manipulation */
void FormDataInput::setMainControls()
{
    // Insert data
//    qDebug() << "source model:" << (void*)m_proxyModel->customSourceModel();
    connect(this, SIGNAL(sigInsertNew()),
            m_proxyModel->customSourceModel(), SLOT(slotInsertToTheModel())); // insert data to the model
    // update model changes - this must take place before the connection that update the current index
    connect(m_proxyModel->customSourceModel(), &CustomSqlTableModel::sigNewRecordInserted,
            [this](int row, cmmn::T_id primId)
            {
                Q_UNUSED(row);
                m_mchTChanger->updateModelChange( primId, MChTypeLabel::ctype_inserted );
            } );
    connect(m_proxyModel->customSourceModel(), SIGNAL(sigNewRecordInserted(int, cmmn::T_id)),
            m_mapper, SLOT(setCurrentIndex(int))); // mapper go to the inserted record (row)  --- TODO: use m_mapper.get()

    // Delete data
    connect(this, &FormDataInput::sigDeleteRow, [this]()
    {
        m_proxyModel->customSourceModel()->slotDeleteFromTheModel( m_mapper->currentIndex() ); // delete row from the model
    });

    // update model changes - this must take place before the connection that update the current index
    connect(m_proxyModel->customSourceModel(), &CustomSqlTableModel::sigRecordDeleted,
            [this](int row, cmmn::T_id primId)
            {
                Q_UNUSED(row);
                m_mchTChanger->updateModelChange( primId, MChTypeLabel::ctype_deleted );
            } );
    /*
     * Behaviour after deletion a some record (row) in the model.
     * The last decision is: after calling delete operation - go to the previous record.
     * This implementation also excellent suit for the case when user insert a new record, don't save that and
     * call the delete operation (from the model).
     * After deletion this record from the model, the mapper's toPrevious() slot doesn't work and
     * the ability go to the previous record is calling the setCurrentIndex(int) method with passing decremented value
     * of the current index (row).
     */
    connect(m_proxyModel->customSourceModel(), &CustomSqlTableModel::sigRecordDeleted,
            [this](int row){ if (row > 0) --row; m_mapper->setCurrentIndex(row); } );

    // Save data
    connect(this, SIGNAL(sigSaveAll()), this, SLOT(slotSubmit())); // submit changes from the "engines" model to the DB
    connect(this, &FormDataInput::sigChangesSubmitted, [this](){ m_mchTChanger->clearChanges(); } ); // clearing changes after data saving
    connect(this, SIGNAL(sigChangesSubmitted(int)), m_mapper, SLOT(setCurrentIndex(int))); // TODO: use m_mapper.get()

    // Refresh data
    connect(this, SIGNAL(sigRefreshAll()),
            m_proxyModel->customSourceModel(), SLOT(slotRefreshTheModel())); // refresh all data in the "engines" model
    connect(m_proxyModel->customSourceModel(), &CustomSqlTableModel::sigModelRefreshed,
            [this](){ m_mchTChanger->clearChanges(); } ); // clearing changes after data refreshing
    // set current index after refresh data in the model
//    connect(m_proxyModel->customSourceModel(), SIGNAL(sigModelRefreshed()),
//            m_ui->m_leRecordId, SIGNAL(returnPressed())); // generate error if current index (value in the record ID LineEdit) doesn't exist in the model
    connect(m_proxyModel->customSourceModel(), &CustomSqlTableModel::sigModelRefreshed, [this](){ m_mapper->toFirst(); }); // go to the first index

    // Revert data
//    connect(this, SIGNAL(sigRevertChanges()), m_mapper, SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_proxyModel->customSourceModel(), SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_proxyModel->customSourceModel(), SLOT(revertAll()));
//    connect(this, SIGNAL(sigRevertChanges()), m_mapper, SLOT(revert()));
}

/* set push buttons, that call some widget for editing DB tables */
void FormDataInput::setEditDBTPushButtons()
{
    setEditDBTOnePB( m_ui->m_pbEditFullName, "full_names_engines", m_ui->m_leFullNameData );
    setEditDBTOnePB( m_ui->m_pbEditFuels, "fuels_types", m_ui->m_leFuel );
    setEditDBTOnePB( m_ui->m_pbEditChambers, "combustion_chambers", m_ui->m_leChamberData );
    setEditDBTOnePB( m_ui->m_pbEditStartDevices, "start_devices", m_ui->m_leStartDeviceData );
}

void FormDataInput::setEditDBTOnePB(SelectEditPB *pb, const QString &pbname, QWidget *identWidget)
{
    pb->setDBTableName(pbname);
    pb->setIdentDataWidget(identWidget);
    connect(pb, SIGNAL(clicked()), this, SLOT(slotEditChildDBT()));
}

void FormDataInput::setDataOperating()
{
    m_proxyModel->setSqlTable("engines");

    // set mapper
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    m_mapper->setModel(m_proxyModel); // TODO: use m_proxyModel.get()
    // indexes starts from 1, because in the 0-th section place the selection icon
    m_mapper->addMapping(m_ui->m_leIdData, 1);
    m_mapper->addMapping(m_ui->m_leFullNameData, 2);
    m_mapper->addMapping(m_ui->m_leFuel, 3);
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
    connect(m_ui->m_tbRecordFirst, SIGNAL(clicked()), m_mapper, SLOT(toFirst())); // TODO: use m_mapper.get()
    connect(m_ui->m_tbRecordLast, SIGNAL(clicked()), m_mapper, SLOT(toLast())); // TODO: use m_mapper.get()
    connect(m_ui->m_tbRecordPrev, SIGNAL(clicked()), m_mapper, SLOT(toPrevious())); // TODO: use m_mapper.get()
    connect(m_ui->m_tbRecordNext, SIGNAL(clicked()), m_mapper, SLOT(toNext())); // TODO: use m_mapper.get()

    // set inputing of the "id" value in the line edit
    connect(m_ui->m_leRecordId, SIGNAL(sigReturnPressed(QString)),
            this, SLOT(slotNeedChangeMapperIndex(QString)));
    connect(this, SIGNAL(sigChangeMapperIndex(int)),
            m_mapper, SLOT(setCurrentIndex(int))); // change mapper index by the id's line edit value --- // TODO: use m_mapper.get()
    connect(this, SIGNAL(sigWrongIdEntered()),
            m_ui->m_leRecordId, SLOT(clear())); // indicate that inputed value is wrong and there are need to input another
//    connect(this, SIGNAL(sigWrongIdEntered()),
//              m_mapper, SLOT(revert())); // perform a clearing of the mapped widgets - TODO: maybe delete?

    // enable & disable navigation buttons
    connect(m_mapper, SIGNAL(currentIndexChanged(int)), this, SLOT(slotRowIndexChanged(int))); // TODO: use m_mapper.get()
    connect(this, SIGNAL(sigFirstRowReached(bool)), m_ui->m_tbRecordFirst, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigFirstRowReached(bool)), m_ui->m_tbRecordPrev, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigLastRowReached(bool)), m_ui->m_tbRecordLast, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigLastRowReached(bool)), m_ui->m_tbRecordNext, SLOT(setDisabled(bool)));
    slotRowIndexChanged(m_mapper->currentIndex()); // the initial checking
}

void FormDataInput::setModelChange()
{
    // implementation the behaviour of window form when current index changes
    connect(m_mapper, &QDataWidgetMapper::currentIndexChanged,
            [this](int index)
    {
        m_mchTChanger->slotCheckModelChanges( m_proxyModel->customSourceModel()->primaryIdInRow(index) );
    } );  // TODO: use m_mapper.get()
    connect(m_mchTChanger, SIGNAL(sigChangeChangedType(bool)), m_ui->m_gboxEngineData, SLOT(setDisabled(bool))); // TODO: use m_mchTChanger.get()
    connect(m_mchTChanger, SIGNAL(sigChangeChangedType(int)), m_ui->m_lblModelChangeType, SLOT(slotChangeType(int)));  // TODO: use m_mchTChanger.get()
}

FormDataInput::~FormDataInput()
{
    delete m_ui;
    delete m_proxyModel;
    delete m_mapper;
    delete m_mchTChanger;
}

void FormDataInput::slotNeedChangeMapperIndex(const QString &value)
{
    int row = -1;
    if (m_proxyModel->customSourceModel()->findRowWithId(value, row))
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
    emit sigLastRowReached(row >= m_proxyModel->rowCount() - 1);
//    qDebug() << "slotRowIndexChanged(" << row << "), DisplayRole data:";
//    m_proxyModel->customSourceModel()->printData(Qt::DisplayRole);
}

void FormDataInput::slotSubmit()
{
//    qDebug() << "slotSubmit(), start";
    int currentIndex = m_mapper->currentIndex();
    if (!m_proxyModel->customSourceModel()->database().transaction()) {
        QMessageBox::critical(this, tr("Database transaction error"),
                              tr("The database driver do not support the transactions operations"));
    }

    if (m_proxyModel->customSourceModel()->submitAll()) {
//        qDebug() << "slotSubmit(), after submitAll(), before commit()";
//        m_proxyModel->customSourceModel()->printData(Qt::EditRole);

        // After submit all data, the mapper current index is -1
        m_proxyModel->customSourceModel()->database().commit();
    }
    else {
        m_proxyModel->customSourceModel()->database().rollback();
        QMessageBox::critical(this, tr("Error data submit to the database"),
                              tr("Cannot submit data to the database. The database report an error: %1")
                              .arg(m_proxyModel->customSourceModel()->lastError().text()));
        return;
    }
    emit sigChangesSubmitted(currentIndex);
//    qDebug() << "slotSubmit(), end";
}

void FormDataInput::slotEditChildDBT()
{
    // TODO: use try-catch
    SelectEditPB *pbSEDBT = qobject_cast<SelectEditPB *>(sender());
    if ( !pbSEDBT ) {
        /*
         * TODO: wrong error message text. There are need to say some like "cannot edit database table" and
         * not "cannot open the dialog", or maybe there are need rename this slot.
         * This also applies to the other error messageboxes in this slot.
         */
        QMessageBox::critical(this, tr("Invalid widget"),
                              tr("Error of editing database table\n"
                                 "Cannot open the dialog for database table editing.\n"
                                 "The reason is: clicked on an unexpected push button.\n\n"
                                 "Please consult with the application developer for fixing this problem."));
        return;
    }

    dbi::DBTInfo *tableInfo = DBINFO.tableByName(pbSEDBT->DBTableName());
    if ( !tableInfo ) {
        QMessageBox::critical(this, tr("Invalid push button"),
                              tr("Error of editing database table\n"
                                 "Cannot open the dialog for database table editing.\n"
                                 "The reason is: cannot define the database table - pressed unknown push button.\n\n"
                                 "Please consult with the application developer for fixing this problem."));
        return;
    }

    DBTEditor editor(tableInfo, this);
    // TODO: about next line: in the EditUICreator use pbSEDBT->fieldNo() instead of the m_mapper->mappedSection(pbSEDBT->identWidget())
    const QModelIndex &currIndex = m_proxyModel->index(m_mapper->currentIndex(), m_mapper->mappedSection(pbSEDBT->identWidget()));
    qDebug() << "before selectInitial(), [" << currIndex.row() << "," << currIndex.column() << "]"
             << ", dataD =" << m_proxyModel->data( currIndex, Qt::DisplayRole).toString()
             << ", dataE =" << m_proxyModel->data( currIndex, Qt::EditRole).toString()
             << ", dataU =" << m_proxyModel->data( currIndex, Qt::UserRole).toString();
    const QVariant &forId = m_proxyModel->data(currIndex, Qt::UserRole);
    if ( !forId.isNull() ) // if data is NULL -> don't select any row in the editor view
        editor.selectInitial(forId);

    if ( editor.exec() == QDialog::Accepted ) {
        m_proxyModel->customSourceModel()->spike1_turnOn(true); /* Switch ON the Spike #1 */
        ASSERT_DBG( m_proxyModel->setData( currIndex, editor.selectedId(), Qt::EditRole ),
                    cmmn::MessageException::type_critical, tr("Error data setting"),
                    tr("Cannot set data: \"%1\" to the model").arg(editor.selectedId()),
                    QString("FormDataInput::slotEditChildDBT()") );
        qDebug() << "The id value: \"" << editor.selectedId() << "\" was successfully setted to the model";
    }
}

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
#include "../widgets/fl_widgets.h"

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

void ChangerMChType::slotClearChanges()
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
    , m_prxDecorMdl_1(new ProxyDecorModel(this))
    , m_mapper(new QDataWidgetMapper(this))
    , m_mchTChanger(new ChangerMChType(this))
{
    m_ui->setupUi(this);
    setMainControls();
    setEditDBTPushButtons();
    setDataOperating();
    setDataNavigation();
    setModelChange();
    setEngineName();

//    // --- The test vizualization of the model data ---
//    // The proxy model
//    QTableView *tablePrx = new QTableView;
//    tablePrx->setWindowTitle( QString("Proxy model for debugging. Use the \"%1\" DB table")
//                              .arg(m_prxDecorMdl_1->customSourceModel()->tableName()) );
//    tablePrx->setSelectionBehavior(QAbstractItemView::SelectRows);
//    tablePrx->setSelectionMode(QAbstractItemView::SingleSelection);
//    tablePrx->setModel(m_prxDecorMdl_1);
//    tablePrx->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    tablePrx->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    connect(m_mapper, SIGNAL(currentIndexChanged(int)), tablePrx, SLOT(selectRow(int)));
//    // selection setting - testing
//    connect(tablePrx->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//            m_prxDecorMdl_1, SLOT(slotChooseRow(QItemSelection,QItemSelection)));

//    // The source model
//    QTableView *tableSrc = new QTableView;
//    tableSrc->setWindowTitle( QString("Source model for debugging. Use the \"%1\" DB table")
//                              .arg(m_prxDecorMdl_1->customSourceModel()->tableName()) );
//    tableSrc->setSelectionBehavior(QAbstractItemView::SelectRows);
//    tableSrc->setSelectionMode(QAbstractItemView::SingleSelection);
//    tableSrc->setModel(m_prxDecorMdl_1->customSourceModel());
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
//    sp->setWindowTitle( QString("Source and Proxy models of the table: %1").arg(m_prxDecorMdl_1->customSourceModel()->tableName()) );
//    sp->move(10, 10);
//    sp->show();
}

/* set the main commands for data manipulation */
void FormDataInput::setMainControls()
{
    // Insert data
    connect(this, SIGNAL(sigInsertNew()),
            m_prxDecorMdl_1->customSourceModel(), SLOT(slotInsertToTheModel())); // insert data to the model
    // update model changes - this must take place before the connection that update the current index
    connect(m_prxDecorMdl_1->customSourceModel(), &CustomSqlTableModel::sigNewRecordInserted,
            [this](int /*row*/, cmmn::T_id primId)
            {
                m_mchTChanger->updateModelChange( primId, MChTypeLabel::ctype_inserted );
            } );
    connect(m_prxDecorMdl_1->customSourceModel(), SIGNAL(sigNewRecordInserted(int, cmmn::T_id)),
            m_mapper, SLOT(setCurrentIndex(int))); // mapper go to the inserted record (row)

    // Delete data
    connect(this, &FormDataInput::sigDeleteRow, [this]()
    {
        m_prxDecorMdl_1->customSourceModel()->slotDeleteRowRecord( m_mapper->currentIndex() ); // delete row from the model
    });

    // update model changes - this must take place before the connection that update the current index
    connect(m_prxDecorMdl_1->customSourceModel(), &CustomSqlTableModel::sigRecordDeleted,
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
    connect(m_prxDecorMdl_1->customSourceModel(), &CustomSqlTableModel::sigRecordDeleted,
            [this](int row){ if (row > 0) --row; m_mapper->setCurrentIndex(row); } );

    // Save data
    connect(this, SIGNAL(sigSaveAll()), m_prxDecorMdl_1->customSourceModel(), SLOT(slotSaveToDB())); // save model's data to the DB
    connect(m_prxDecorMdl_1->customSourceModel(), SIGNAL(sigSavedInDB()), m_mchTChanger, SLOT(slotClearChanges())); // clearing changes after data saving
//    connect(m_prxDecorMdl_1->customSourceModel(), SIGNAL(sigSavedInDB()), m_mapper // TODO: IMPLEMENT switching to the index, that was before saving

    // Refresh data
    connect(this, SIGNAL(sigRefreshAll()),
            m_prxDecorMdl_1->customSourceModel(), SLOT(slotRefreshTheModel())); // refresh all data in the "engines" model
    connect(m_prxDecorMdl_1->customSourceModel(), SIGNAL(sigModelRefreshed()), m_mchTChanger, SLOT(slotClearChanges())); // clearing changes after data refreshing
    // set current index after refresh data in the model
//    connect(m_prxDecorMdl_1->customSourceModel(), SIGNAL(sigModelRefreshed()),
//            m_ui->m_leRecordId, SIGNAL(returnPressed())); // generate error if current index (value in the record ID LineEdit) doesn't exist in the model
    connect(m_prxDecorMdl_1->customSourceModel(), &CustomSqlTableModel::sigModelRefreshed, [this](){ m_mapper->toFirst(); }); // go to the first index

    // Revert data
//    connect(this, SIGNAL(sigRevertChanges()), m_mapper, SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_prxDecorMdl_1->customSourceModel(), SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_prxDecorMdl_1->customSourceModel(), SLOT(revertAll()));
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
    m_prxDecorMdl_1->setSqlTableName("engines");

    // set mapper
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit); // use manual, because auto perform setting earlier generated data from widgets to model
    m_mapper->setModel(m_prxDecorMdl_1);
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
    connect(m_ui->m_tbRecordFirst, SIGNAL(clicked()), m_mapper, SLOT(toFirst()));
    connect(m_ui->m_tbRecordLast, SIGNAL(clicked()), m_mapper, SLOT(toLast()));
    connect(m_ui->m_tbRecordPrev, SIGNAL(clicked()), m_mapper, SLOT(toPrevious()));
    connect(m_ui->m_tbRecordNext, SIGNAL(clicked()), m_mapper, SLOT(toNext()));

    // set inputing of the "id" value in the line edit
    connect(m_ui->m_leRecordId, SIGNAL(sigReturnPressed(QString)),
            this, SLOT(slotNeedChangeMapperIndex(QString)));
    connect(this, SIGNAL(sigChangeMapperIndex(int)),
            m_mapper, SLOT(setCurrentIndex(int))); // change mapper index by the id's line edit value
    connect(this, SIGNAL(sigWrongIdEntered()),
            m_ui->m_leRecordId, SLOT(clear())); // indicate that inputed value is wrong and there are need to input another

    // checking boundaries of DB data records
    connect(m_mapper, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCheckRowIndex(int)));
    connect(this, SIGNAL(sigFirstRowReached(bool)), m_ui->m_tbRecordFirst, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigFirstRowReached(bool)), m_ui->m_tbRecordPrev, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigLastRowReached(bool)), m_ui->m_tbRecordLast, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigLastRowReached(bool)), m_ui->m_tbRecordNext, SLOT(setDisabled(bool)));
    slotCheckRowIndex(m_mapper->currentIndex()); // the initial checking
}

void FormDataInput::setModelChange()
{
    // implementation the behaviour of window form when current index changes
    connect(m_mapper, &QDataWidgetMapper::currentIndexChanged,
            [this](int index)
    {
        m_mchTChanger->slotCheckModelChanges( m_prxDecorMdl_1->rowId(index) );
    } );
    connect(m_mchTChanger, SIGNAL(sigChangeChangedType(bool)), m_ui->m_gboxEngineData, SLOT(setDisabled(bool)));
    connect(m_mchTChanger, SIGNAL(sigChangeChangedType(int)), m_ui->m_lblModelChangeType, SLOT(slotChangeType(int)));
}

void FormDataInput::setEngineName()
{
    connect(m_mapper, SIGNAL(currentIndexChanged(int)), this, SLOT(slotGenEngineName(int))); // generate engine name when mapper's index changes
    connect(m_prxDecorMdl_1, &QAbstractItemModel::dataChanged,
            [this](const QModelIndex &topLeft, const QModelIndex &/*bottomRight*/)
    {
        if (topLeft.column() != ProxyDecorModel::SELECT_ICON_COLUMN)
            slotGenEngineName(topLeft.row());
    }); // generate engine name when model's data changes
    connect(this, SIGNAL(sigEngineNameGenerated(QString)), m_ui->m_lblEngineName, SLOT(setText(QString)));
    slotGenEngineName(m_mapper->currentIndex()); // generate initial engine name
}

FormDataInput::~FormDataInput()
{
    delete m_ui;
    delete m_prxDecorMdl_1;
    delete m_mapper;
    delete m_mchTChanger;
}

void FormDataInput::slotNeedChangeMapperIndex(const QString &value)
{
    int row = -1;
    if (m_prxDecorMdl_1->customSourceModel()->getIdRow(value, row))
        emit sigChangeMapperIndex(row);
    else {
        QMessageBox::warning(this, tr("Error engine ""id"" value"),
                             tr("The engine with Id=%1 does not exists. Please enter the valid \"Id\" value of an existent engine").arg(value));
        emit sigWrongIdEntered();
    }
}

void FormDataInput::slotCheckRowIndex(int row)
{
//    qDebug() << "change mapper index, current index =" << m_mapper->currentIndex() << ", row =" << row;
    emit sigFirstRowReached(row <= 0);
    emit sigLastRowReached(row >= m_prxDecorMdl_1->rowCount() - 1);
//    qDebug() << "slotCheckRowIndex(" << row << "), DisplayRole data:";
//    m_prxDecorMdl_1->customSourceModel()->printData(Qt::DisplayRole);
}

void FormDataInput::slotGenEngineName(int row)
{
    const dbi::DBTInfo *tableInfo = DBINFO.tableByName( m_prxDecorMdl_1->sqlTableName() );
    ASSERT_DBG(tableInfo, cmmn::MessageException::type_critical, tr("Error engine name"),
               tr("Cannot set engine name to the label. Unknown engine: ") + m_prxDecorMdl_1->sqlTableName(),
               QString("FormDataInput::slotGenEngineName"));
    QString engineName;
    for (const auto &identInf : tableInfo->m_idnFields)
        engineName += ( identInf.m_strBefore +
                    m_prxDecorMdl_1->index( row, identInf.m_NField + ProxyDecorModel::COUNT_ADDED_COLUMNS ).data(Qt::DisplayRole).toString() );
    emit sigEngineNameGenerated(engineName);
}

void FormDataInput::slotEditChildDBT()
{
    // TODO: use try-catch
    SelectEditPB *pbSEDBT = qobject_cast<SelectEditPB *>(sender());
    if ( !pbSEDBT ) {
        /*
         * TODO: wrong error message text. There are need to say something like "cannot edit database table" and
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
    const QModelIndex &currIndex = m_prxDecorMdl_1->index(m_mapper->currentIndex(), m_mapper->mappedSection(pbSEDBT->identWidget()));
    qDebug() << "before selectInitial(), [" << currIndex.row() << "," << currIndex.column() << "]"
             << ", dataD =" << m_prxDecorMdl_1->data( currIndex, Qt::DisplayRole).toString()
             << ", dataE =" << m_prxDecorMdl_1->data( currIndex, Qt::EditRole).toString()
             << ", dataU =" << m_prxDecorMdl_1->data( currIndex, Qt::UserRole).toString();
    const QVariant &forId = m_prxDecorMdl_1->data(currIndex, Qt::UserRole);
    if ( !forId.isNull() ) // if data is NULL -> don't select any row in the editor view
        editor.selectInitial(forId);

    if ( editor.exec() == QDialog::Accepted ) {
        m_prxDecorMdl_1->customSourceModel()->spike1_turnOn(true); /* Switch ON the Spike #1 */
        ASSERT_DBG( m_prxDecorMdl_1->setData( currIndex, editor.selectedId(), Qt::EditRole ),
                    cmmn::MessageException::type_critical, tr("Error data setting"),
                    tr("Cannot set data: \"%1\" to the model").arg(editor.selectedId()),
                    QString("FormDataInput::slotEditChildDBT") );
        qDebug() << "The id value: \"" << editor.selectedId() << "\" was successfully setted to the model";
    }
}

#include <QSqlError>
#include <QMessageBox>
#include <QSplitter> // TODO: temp, delete later
#include <QDebug>  // TODO: temp, delete later
#include <QTableView> // TODO: for testing, delete later

#include "form_data_input.h"
#include "ui_form_data_input.h"
#include "../dbt_editor/dbt_editor.h"
#include "../model/custom_sql_table_model.h"
#include "../model/proxy_decor_model.h"
#include "../common/db_info.h"
#include "../widgets/fl_widgets.h"
#include "../widgets/widget_mapper.h"
#include "../dbt_editor/edit_ui_creator.h" // TODO: maybe move this to the "widgets" directory?

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
    , m_DBTInfo(DBINFO.tableByName("engines"))
    , m_prxDecorMdl(new ProxyDecorModel(this))
    , m_mapper(new WidgetMapper(this))
    , m_editUICreator(new EditUICreator(m_DBTInfo, m_mapper, this))
    , m_mchTChanger(new ChangerMChType(this))
{
    m_ui->setupUi(this);
    setModel();
    setMapper();
    setEditUI();
    setMainControls();
    setDataNavigation();
    setModelChange();
    setEngineName();
    m_mapper->toFirst(); // set init data - must placing after performing all settings

//    // --- The test vizualization of the model data ---
//    // The proxy model
//    QTableView *tablePrx = new QTableView;
//    tablePrx->setWindowTitle( QString("Proxy model for debugging. Use the \"%1\" DB table")
//                              .arg(m_prxDecorMdl->customSourceModel()->tableName()) );
//    tablePrx->setSelectionBehavior(QAbstractItemView::SelectRows);
//    tablePrx->setSelectionMode(QAbstractItemView::SingleSelection);
//    tablePrx->setModel(m_prxDecorMdl);
//    tablePrx->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    tablePrx->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
//    connect(m_mapper, SIGNAL(currentIndexChanged(int)), tablePrx, SLOT(selectRow(int)));
//    // selection setting - testing
//    connect(tablePrx->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
//            m_prxDecorMdl, SLOT(slotChooseRow(QItemSelection,QItemSelection)));

//    // The source model
//    QTableView *tableSrc = new QTableView;
//    tableSrc->setWindowTitle( QString("Source model for debugging. Use the \"%1\" DB table")
//                              .arg(m_prxDecorMdl->customSourceModel()->tableName()) );
//    tableSrc->setSelectionBehavior(QAbstractItemView::SelectRows);
//    tableSrc->setSelectionMode(QAbstractItemView::SingleSelection);
//    tableSrc->setModel(m_prxDecorMdl->customSourceModel());
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
//    sp->setWindowTitle( QString("Source and Proxy models of the table: %1").arg(m_prxDecorMdl->customSourceModel()->tableName()) );
//    sp->move(10, 10);
//    sp->show();
}

void FormDataInput::setModel()
{
    m_prxDecorMdl->setSqlTableName(m_DBTInfo->m_nameInDB);
}

void FormDataInput::setMapper()
{
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit); // use manual, because auto perform setting earlier generated data from widgets to model
    m_mapper->setModel(m_prxDecorMdl);
}

void FormDataInput::setEditUI()
{
    m_editUICreator->createUI(m_ui->m_gboxEngineData);
    connect(m_editUICreator, SIGNAL(sigSEPBClicked(const dbi::DBTInfo*,int)),
            this, SLOT(slotEditChildDBT(const dbi::DBTInfo*,int))); // open child DBT edit dialog
}

/* set the main commands for data manipulation */
void FormDataInput::setMainControls()
{
    // Insert data
    connect(this, SIGNAL(sigInsertNew()),
            m_prxDecorMdl->customSourceModel(), SLOT(slotInsertToTheModel())); // insert data to the model
    // update model changes - this must take place before the connection that update the current index
    connect(m_prxDecorMdl->customSourceModel(), &CustomSqlTableModel::sigNewRecordInserted,
            [this](int /*row*/, cmmn::T_id primId)
            {
                m_mchTChanger->updateModelChange( primId, MChTypeLabel::ctype_inserted );
            } );
    connect(m_prxDecorMdl->customSourceModel(), SIGNAL(sigNewRecordInserted(int, cmmn::T_id)),
            m_mapper, SLOT(setCurrentIndex(int))); // mapper go to the inserted record (row)

    // Delete data
    connect(this, &FormDataInput::sigDeleteRow, [this]()
    {
        m_prxDecorMdl->customSourceModel()->slotDeleteRowRecord( m_mapper->currentIndex() ); // delete row from the model
    });

    // update model changes - this must take place before the connection that update the current index
    connect(m_prxDecorMdl->customSourceModel(), &CustomSqlTableModel::sigRecordDeleted,
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
    connect(m_prxDecorMdl->customSourceModel(), &CustomSqlTableModel::sigRecordDeleted,
            [this](int row){ if (row > 0) --row; m_mapper->setCurrentIndex(row); } );

    // Save data
    connect(this, SIGNAL(sigSaveAll()), m_prxDecorMdl->customSourceModel(), SLOT(slotSaveToDB())); // save model's data to the DB
    connect(m_prxDecorMdl->customSourceModel(), SIGNAL(sigSavedInDB()), m_mchTChanger, SLOT(slotClearChanges())); // clearing changes after data saving
//    connect(m_prxDecorMdl->customSourceModel(), SIGNAL(sigSavedInDB()), m_mapper // TODO: IMPLEMENT switching to the index, that was before saving

    // Refresh data
    connect(this, SIGNAL(sigRefreshAll()),
            m_prxDecorMdl->customSourceModel(), SLOT(slotRefreshTheModel())); // refresh all data in the "engines" model
    connect(m_prxDecorMdl->customSourceModel(), SIGNAL(sigModelRefreshed()), m_mchTChanger, SLOT(slotClearChanges())); // clearing changes after data refreshing
    // set current index after refresh data in the model
//    connect(m_prxDecorMdl->customSourceModel(), SIGNAL(sigModelRefreshed()),
//            m_ui->m_leRecordId, SIGNAL(returnPressed())); // generate error if current index (value in the record ID LineEdit) doesn't exist in the model
    connect(m_prxDecorMdl->customSourceModel(), &CustomSqlTableModel::sigModelRefreshed, [this](){ m_mapper->toFirst(); }); // go to the first index

    // Revert data
//    connect(this, SIGNAL(sigRevertChanges()), m_mapper, SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_prxDecorMdl->customSourceModel(), SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_prxDecorMdl->customSourceModel(), SLOT(revertAll()));
//    connect(this, SIGNAL(sigRevertChanges()), m_mapper, SLOT(revert()));
}

void FormDataInput::setDataNavigation()
{
    // navigation set
    connect(m_ui->m_tbRecordFirst, SIGNAL(clicked()), m_mapper, SLOT(toFirst()));
    connect(m_ui->m_tbRecordLast, SIGNAL(clicked()), m_mapper, SLOT(toLast()));
    connect(m_ui->m_tbRecordPrev, SIGNAL(clicked()), m_mapper, SLOT(toPrevious()));
    connect(m_ui->m_tbRecordNext, SIGNAL(clicked()), m_mapper, SLOT(toNext()));

    // TODO: place here the signal/slot connection, that force to set current mapper's index from decor proxy model (like in the DBTEditor::setDataNavigation method)
    setLEid();

    // checking boundaries of DB data records
    connect(m_mapper, &QDataWidgetMapper::currentIndexChanged, [this](int row)
    {
        emit sigFirstRowReached(row <= 0);
        emit sigLastRowReached(row >= m_prxDecorMdl->rowCount() - 1);
    });
    connect(this, SIGNAL(sigFirstRowReached(bool)), m_ui->m_tbRecordFirst, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigFirstRowReached(bool)), m_ui->m_tbRecordPrev, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigLastRowReached(bool)), m_ui->m_tbRecordLast, SLOT(setDisabled(bool)));
    connect(this, SIGNAL(sigLastRowReached(bool)), m_ui->m_tbRecordNext, SLOT(setDisabled(bool)));
}

void FormDataInput::setLEid()
{
    m_ui->m_leRecordId->setValidator(new QIntValidator(0, 1e6, m_ui->m_leRecordId)); // control input integer values in range between 0 and 1e6

    // behaviour of Id's LineEdit after changing current mapper index
    connect(m_mapper, &QDataWidgetMapper::currentIndexChanged, [this](int row)
    {
        // transmit signal from mapper to LineEdit in appropriate view - convert row number value to Id value in QString-type format
        emit sigCurrentIdChanged( QString::number( this->m_prxDecorMdl->rowId(row) ) );
    });
    connect(this, SIGNAL(sigCurrentIdChanged(QString)), m_ui->m_leRecordId, SLOT(setText(QString)));

    // behaviour after pressing Enter in the LineEdit with records Id
    connect(m_ui->m_leRecordId, SIGNAL(sigReturnPressed(QString)),
            this, SLOT(slotNeedChangeMapperIndex(QString)));
    connect(this, SIGNAL(sigChangeMapperIndex(int)),
            m_mapper, SLOT(setCurrentIndex(int))); // change mapper index by the id's line edit value
    connect(this, SIGNAL(sigWrongIdEntered()),
            m_ui->m_leRecordId, SLOT(clear())); // indicate that inputed value is wrong and there are need to input another
}

void FormDataInput::setModelChange()
{
    // implementation the behaviour of window form when current index changes
    connect(m_mapper, &QDataWidgetMapper::currentIndexChanged,
            [this](int index)
    {
        m_mchTChanger->slotCheckModelChanges( m_prxDecorMdl->rowId(index) );
    } );
    connect(m_mchTChanger, SIGNAL(sigChangeChangedType(bool)), m_ui->m_gboxEngineData, SLOT(setDisabled(bool)));
    connect(m_mchTChanger, SIGNAL(sigChangeChangedType(int)), m_ui->m_lblModelChangeType, SLOT(slotChangeType(int)));
}

void FormDataInput::setEngineName()
{
    // Generate engine name in two case: when mapper's index changes and when model's data changes
    connect(m_mapper, SIGNAL(currentIndexChanged(int)), this, SLOT(slotGenEngineName(int))); // generate engine name when mapper's index changes
    connect(m_prxDecorMdl, &QAbstractItemModel::dataChanged,
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
    delete m_prxDecorMdl;
    delete m_mapper;
    delete m_editUICreator;
    delete m_mchTChanger;
}

void FormDataInput::slotNeedChangeMapperIndex(const QString &value)
{
    int row = -1;
    if (m_prxDecorMdl->customSourceModel()->getIdRow(value, row))
        emit sigChangeMapperIndex(row);
    else {
        QMessageBox::warning(this, tr("Error engine ""id"" value"),
                             tr("The engine with Id=%1 does not exists. Please enter the valid \"Id\" value of an existent engine")
                             .arg(value));
        emit sigWrongIdEntered();
    }
}

void FormDataInput::slotGenEngineName(int row)
{
    QString engineName;
    for (const auto &identInf : m_DBTInfo->m_idnFields)
        engineName += ( identInf.m_strBefore +
                        m_prxDecorMdl->index( row, identInf.m_NField + ProxyDecorModel::COUNT_ADDED_COLUMNS ).data().toString() );
    emit sigEngineNameGenerated(engineName);
}

void FormDataInput::slotEditChildDBT(const dbi::DBTInfo *dbtInfo, int fieldNo)
{
    const QModelIndex &currIndex = m_prxDecorMdl->index( m_mapper->currentIndex(), fieldNo + ProxyDecorModel::COUNT_ADDED_COLUMNS );
    const QVariant &forId = m_prxDecorMdl->data(currIndex, Qt::UserRole);
    DBTEditor childEditor(dbtInfo, this);
    connect(&childEditor, SIGNAL(sigDataSavedInDB()),
            m_prxDecorMdl->customSourceModel(), SLOT(slotRefreshTheModel())); // save changes in the child model -> refresh the parent (current) model
    if ( !forId.isNull() ) // if data is NULL -> don't select any row in the view
        childEditor.selectInitial(forId);
    if ( childEditor.exec() == QDialog::Accepted ) {
        m_prxDecorMdl->customSourceModel()->spike1_turnOn(); // switch ON the Spike #1
        ASSERT_DBG( m_prxDecorMdl->setData( currIndex, childEditor.selectedId(), Qt::EditRole ),
                    cmmn::MessageException::type_critical, tr("Error data setting"),
                    tr("Cannot set data: \"%1\" to the model").arg(childEditor.selectedId()),
                    QString("DBTEditor::slotEditChildDBT") );
        qDebug() << "The id value: \"" << childEditor.selectedId() << "\" was successfully setted to the model";
    }
}

#include <QMessageBox>
#include <QDebug> // TODO: delete
#include "engine_widget.h"
#include "../common/db_info.h"
#include "../models/src_sql/custom_sql_table_model.h"
#include "../models/prx_decor/proxy_decor_model.h"
#include "../widgets/widget_mapper.h"
#include "../dbt_editor/dbt_editor.h"
#include "../dbt_editor/edit_ui_creator.h" // TODO: maybe move this to the "widgets" directory?
#include "../widgets/fl_widgets.h"
#include "../widgets/reimplemented_widgets.h"

/*
 * ChangerMChTypeImpl - private section of the ChangerMChType class
 */
class ChangerMChTypeImpl
{
public:
    void updateModelChange(cmmn::T_id idPrim, MChTypeLabel::ChangeTypes changeType);
    inline void clearChanges() { m_changes.clear(); }
    MChTypeLabel::ChangeTypes getChange(cmmn::T_id idPrim);
private:
    typedef QMap<cmmn::T_id, MChTypeLabel::ChangeTypes> T_modelChanges;

    T_modelChanges m_changes;
    MChTypeLabel *m_lblDisplay;
};

void ChangerMChTypeImpl::updateModelChange(cmmn::T_id idPrim, MChTypeLabel::ChangeTypes changeType)
{
    m_changes[idPrim] = changeType;
}

MChTypeLabel::ChangeTypes ChangerMChTypeImpl::getChange(cmmn::T_id idPrim)
{
    auto it = m_changes.find(idPrim);
    return it != m_changes.end() ? it.value() : MChTypeLabel::ctype_noChange;
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

void ChangerMChType::slotUpdateChange(const cmmn::T_id &idPrimary, int chType)
{
    m_pImpl->updateModelChange(idPrimary, (MChTypeLabel::ChangeTypes)chType);
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
    cmmn::T_id id;
    CHECK_ERROR_CONVERT_ID( cmmn::safeQVariantToIdType(idPrimary, id), idPrimary );
    emit sigChangeChType( m_pImpl->getChange(id) );
}

/*
 * EDGenerator
 */
EDGenerator::EDGenerator(const QAbstractTableModel *m, const dbi::DBTInfo *info)
    : m_model(m)
    , m_dbtInfo(info)
    , m_isDataEmpty(false)
{}

void EDGenerator::slotGenerate(int row)
{
    QString genData;
    if (!m_isDataEmpty)
        for (const auto &identInf : m_dbtInfo->m_idnFields)
            genData += ( identInf.m_strBefore +
                         m_model->data(m_model->index(row, identInf.m_NField)).toString() );
    emit sigGenerated(genData);
}

/*
 * EngineWidget
 */
EngineWidget::EngineWidget(QWidget *parent)
    : QWidget(parent)
    , m_DBTInfo( DBINFO.tableByName("engines") )
    , m_prxDecorMdl( new ProxyDecorModel(this) )
    , m_mapper( new WidgetMapper(this) )
    , m_grBoxAll( new QGroupBox(tr("Engine's data")) )
    , m_editUICreator( new EditUICreator(m_DBTInfo, m_mapper, this) )
    , m_mchTChanger( new ChangerMChType(this) )
    , m_engNameGen( new EDGenerator(m_prxDecorMdl->customSourceModel(), m_DBTInfo) )
{
    setModel();
    setMapper();
    setUI();
    setMainControls();
    setDataNavigation();
    setModelChange();
    setEngineName();
}

void EngineWidget::init()
{
    m_mapper->toFirst();
    m_engNameGen->slotGenerate(m_mapper->currentIndex()); // generate initial engine name
}

void EngineWidget::setModel()
{
    m_prxDecorMdl->setSqlTableName(m_DBTInfo->m_nameInDB);
}

void EngineWidget::setMapper()
{
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit); // use manual, because auto perform setting earlier generated data from widgets to model
    m_mapper->setModel(m_prxDecorMdl);
}

void EngineWidget::setUI()
{
    (new QVBoxLayout(this))->addWidget(m_grBoxAll);
    m_editUICreator->createUI(m_grBoxAll);
    connect(m_editUICreator, SIGNAL(sigSEPBClicked(const dbi::DBTInfo*,int)),
            this, SLOT(slotEditChildDBT(const dbi::DBTInfo*,int))); // open child DBT edit dialog
}

void EngineWidget::setMainControls()
{
    // Insert data
    connect(this, SIGNAL(sigInsertNew()), m_prxDecorMdl->customSourceModel(), SLOT(slotInsertRecord())); // insert data to the model
    connect(m_prxDecorMdl->customSourceModel(), &CustomSqlTableModel::sigRecordInserted,
            [this](int row, cmmn::T_id primId)
    {
//        The first - update model changes, the second - set current mapper index
        m_mchTChanger->slotUpdateChange(primId, MChTypeLabel::ctype_inserted); // update - mark as inserted after insertion
        emit this->sigRecordInserted(primId); // transmittung signal that a new record inserted
        m_mapper->setCurrentIndex(row); // mapper go to the inserted record (row)
    } );

    // Delete data
    connect(this, &EngineWidget::sigDeleteRow, [this]()
    {
        m_prxDecorMdl->customSourceModel()->slotDeleteRecord( m_mapper->currentIndex() ); // delete row from the model
    } );
    connect(m_prxDecorMdl->customSourceModel(), &CustomSqlTableModel::sigRecordDeleted,
            [this](int row, cmmn::T_id primId)
    {
//        The first - update model changes, the second - set current mapper index
        m_mchTChanger->slotUpdateChange(primId, MChTypeLabel::ctype_deleted); // update - mark as deleted after deletion
        emit this->sigRecordDeleted(primId); // transmittung signal that record deleted
        /*
         * Behaviour after deletion a some record (row) in the model.
         * The last decision is: after calling delete operation - go to the previous record.
         * This implementation also excellent suit for the case when user insert a new record, don't save that and
         * call the delete operation (from the model).
         * After deletion this record from the model, the mapper's toPrevious() slot doesn't work and
         * the ability go to the previous record is calling the setCurrentIndex(int) method with passing decremented value
         * of the current index (row).
         */
        m_mapper->setCurrentIndex( row > 0 ? --row : row );
    } );

    // Save data
    connect(this, SIGNAL(sigSaveAll()), m_prxDecorMdl->customSourceModel(), SLOT(slotSaveInDB())); // save model's data to the DB
    connect(m_prxDecorMdl->customSourceModel(), SIGNAL(sigSavedInDB()), m_mchTChanger, SLOT(slotClearChanges())); // clearing changes after data saving
    connect(m_prxDecorMdl->customSourceModel(), SIGNAL(sigSavedInDB()), this, SIGNAL(sigDataSaved())); // transmitting signal that data saved
//    connect(m_prxDecorMdl->customSourceModel(), SIGNAL(sigSavedInDB()), m_mapper // TODO: IMPLEMENT switching to the index, that was before saving

    // Refresh data
    connect(this, SIGNAL(sigRefreshAll()), m_prxDecorMdl->customSourceModel(), SLOT(slotRefreshModel())); // refresh all data in the model
    connect(m_prxDecorMdl->customSourceModel(), SIGNAL(sigModelRefreshed()), m_mchTChanger, SLOT(slotClearChanges())); // clearing changes after data saving
    connect(m_prxDecorMdl->customSourceModel(), SIGNAL(sigModelRefreshed()), this, SIGNAL(sigDataRefreshed())); // transmitting signal that data refreshed
    // TODO: need to set current index after refresh data in the model (if the record with this Id still exists). If record doesn't exists - set the first index.
    connect(m_prxDecorMdl->customSourceModel(), &CustomSqlTableModel::sigModelRefreshed, [this](){ m_mapper->toFirst(); }); // go to the first index

    // Revert data TODO: implement and debug here and in the DBTEditor too.
//    connect(this, SIGNAL(sigRevertChanges()), m_mapper, SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_prxDecorMdl->customSourceModel(), SLOT(revert()));
//    connect(this, SIGNAL(sigRevertChanges()), m_prxDecorMdl->customSourceModel(), SLOT(revertAll()));
//    connect(this, SIGNAL(sigRevertChanges()), m_mapper, SLOT(revert()));
}

void EngineWidget::setDataNavigation()
{
    // navigation set
    connect(this, SIGNAL(sigToFirstRec()), m_mapper, SLOT(toFirst()));
    connect(this, SIGNAL(sigToLastRec()), m_mapper, SLOT(toLast()));
    connect(this, SIGNAL(sigToPreviousRec()), m_mapper, SLOT(toPrevious()));
    connect(this, SIGNAL(sigToNextRec()), m_mapper, SLOT(toNext()));

    // TODO: place here the signal/slot connection, that force to set current mapper's index from decor proxy model (like in the DBTEditor::setDataNavigation())
    connect(m_mapper, &QDataWidgetMapper::currentIndexChanged, [this](int row)
    {
        // transmit signal from mapper to LineEdit in appropriate view - convert row number value to Id value in QString-type format
        emit sigCurrentIdChanged( QString::number( this->m_prxDecorMdl->rowId(row) ) );
    });

    // transmit signals - checking boundaries of DB data records
    connect(m_mapper, SIGNAL(sigFirstRowReached(bool)), this, SIGNAL(sigFirstRecReached(bool)));
    connect(m_mapper, SIGNAL(sigLastRowReached(bool)), this, SIGNAL(sigLastRecReached(bool)));
}

void EngineWidget::setModelChange()
{
    // implementation the behaviour of window when current index changes
    connect(m_mapper, &QDataWidgetMapper::currentIndexChanged,
            [this](int index)
    {
        m_mchTChanger->slotCheckModelChanges( m_prxDecorMdl->rowId(index) );
    } );
    connect(m_mchTChanger, SIGNAL(sigChangeChType(int)), this, SIGNAL(sigChTypeChanged(int)));
    connect(m_mchTChanger, &ChangerMChType::sigChangeChType, [this](int changeType)
    {
        emit sigChTypeToDeletedChanged( changeType == MChTypeLabel::ctype_deleted );
        // TODO: there are need to show disabled engines name when it was deleted. Now it is showing empty string instead of the name of deleted engine.
        // To achieve this there are need to get generated engines name before its record deletion and save it to the storage (maybe to the EDGenerator).
    });
    connect(this, SIGNAL(sigChTypeToDeletedChanged(bool)), m_engNameGen, SLOT(slotSetDataEmpty(bool)));
    connect(this, SIGNAL(sigChTypeToDeletedChanged(bool)), m_grBoxAll, SLOT(setDisabled(bool)));
}

void EngineWidget::setEngineName()
{
    // Generate engine name in two cases: when mapper's index changes and when model's data changes
    connect(m_mapper, SIGNAL(currentIndexChanged(int)), m_engNameGen, SLOT(slotGenerate(int))); // generate engine name when mapper's index changes
    connect(m_prxDecorMdl, &QAbstractItemModel::dataChanged,
            [this](const QModelIndex &topLeft, const QModelIndex &/*bottomRight*/)
    {
        if (topLeft.column() != ProxyDecorModel::SELECT_ICON_COLUMN)
            m_engNameGen->slotGenerate(topLeft.row());
    }); // generate engine name when model's data changes
    connect(m_engNameGen, SIGNAL(sigGenerated(QString)), this, SIGNAL(sigEngineNameGenerated(QString))); // transmit signal
}

void EngineWidget::slotTryChangeEngine(const QString &id)
{
    int row = -1;
    if (m_prxDecorMdl->customSourceModel()->getIdRow(id, row))
        m_mapper->setCurrentIndex(row);
    else {
        QMessageBox::warning(this, tr("Error engine ""id"" value"),
                             tr("The engine with Id=%1 does not exists. Please enter the valid \"Id\" value of an existent engine").arg(id));
        emit sigWrongEngineId();
    }
}

void EngineWidget::slotEditChildDBT(const dbi::DBTInfo *dbtInfo, int fieldNo)
{
    const QModelIndex &currIndex = m_prxDecorMdl->index( m_mapper->currentIndex(), fieldNo + ProxyDecorModel::COUNT_ADDED_COLUMNS );
    const QVariant &forId = m_prxDecorMdl->data(currIndex, Qt::UserRole);
    DBTEditor editor(dbtInfo, this);
    connect(&editor, SIGNAL(sigDataSavedInDB()),
            m_prxDecorMdl->customSourceModel(), SLOT(slotRefreshModel())); // save changes in the child model -> refresh the parent (current) model
    if ( !forId.isNull() ) // if data is NULL -> don't select any row in the view
        editor.selectInitial(forId);
    if ( editor.exec() == QDialog::Accepted ) {
        m_prxDecorMdl->customSourceModel()->spike1_turnOn(); // switch ON the Spike #1
        ASSERT_DBG( m_prxDecorMdl->setData( currIndex, editor.selectedId(), Qt::EditRole ),
                    cmmn::MessageException::type_critical, tr("Error data setting"),
                    tr("Cannot set data: \"%1\" to the model").arg(editor.selectedId()),
                    QString("DBTEditor::slotEditChildDBT") );
        qDebug() << "The id value: \"" << editor.selectedId() << "\" was successfully setted to the model";
    }
}

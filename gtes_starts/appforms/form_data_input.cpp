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

FormDataInput::FormDataInput(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormDataInput)
    , m_enginesModel(new CustomSqlTableModel(parent))
    , m_mapper(new QDataWidgetMapper(this))
{
    ui->setupUi(this);
    setEditDBTPushButtons();
    setDataOperating();
    setDataNavigation();

    // command signals & slots connections
    connect(this, SIGNAL(sigInsertNew()), m_enginesModel, SLOT(slotInsertToTheModel()));
    connect(this, SIGNAL(sigSaveAll()), m_mapper, SLOT(submit())); // submit changes from the mapped widgets to the "engines" model
    connect(this, SIGNAL(sigSaveAll()), this, SLOT(slotSubmit())); // submit changes from the "engines" model to the DB
    connect(this, SIGNAL(sigRefreshAll()), m_enginesModel, SLOT(slotRefreshTheModel())); // refresh all data in the "engines" model
    connect(this, SIGNAL(sigRefreshAll()), this, SLOT(slotNeedChangeMapperIndex())); // updating data on the panel and mapper
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

void FormDataInput::setDataOperating()
{
    setModel();
    // set combo box
    ui->m_cboxFuel->setModel(m_enginesModel->relationModel(2));
    ui->m_cboxFuel->setModelColumn(1);
    // set mapper
    m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    m_mapper->setModel(m_enginesModel);
    m_mapper->setItemDelegate(new CustomSqlRelationalDelegate(m_mapper));
    m_mapper->addMapping(ui->m_leIdData, 0);
    m_mapper->addMapping(ui->m_leFullNameData, 1);
    m_mapper->addMapping(ui->m_cboxFuel, 2);
    m_mapper->addMapping(ui->m_leChamberData, 3);
    m_mapper->addMapping(ui->m_leStartDeviceData, 4);
    m_mapper->addMapping(ui->m_sboxStartDevicesQntyData, 5);
    m_mapper->addMapping(ui->m_pteComments, 6);
    m_mapper->toFirst();
}

void FormDataInput::setModel()
{
    m_enginesModel->setTable("engines");
    connect(m_enginesModel, SIGNAL(sigNewRecordInserted(int)), m_mapper, SLOT(setCurrentIndex(int)));
}

void FormDataInput::setDataNavigation()
{
    ui->m_leRecordId->setValidator(new QIntValidator(0, 1e6, ui->m_leRecordId)); /* set validator that control inputing only
                                                                                    integer values in range between 0 and 1e6 */
    connect(ui->m_tbRecordFirst, SIGNAL(clicked()), m_mapper, SLOT(toFirst()));
    connect(ui->m_tbRecordLast, SIGNAL(clicked()), m_mapper, SLOT(toLast()));
    connect(ui->m_tbRecordPrev, SIGNAL(clicked()), m_mapper, SLOT(toPrevious()));
    connect(ui->m_tbRecordNext, SIGNAL(clicked()), m_mapper, SLOT(toNext()));

    // set inputing of the "id" value in the line edit
    connect(ui->m_leRecordId, SIGNAL(returnPressed()), this, SLOT(slotNeedChangeMapperIndex())); /* TODO: reimplement QLineEdit with
                                                                                                    adding the signal with QString
                                                                                                    argument, that store QLineEdit data */
    connect(this, SIGNAL(sigChangeMapperIndex(int)), m_mapper, SLOT(setCurrentIndex(int)));
    connect(this, SIGNAL(sigWrongIdEntered()), ui->m_leRecordId, SLOT(clear()));
    connect(this, SIGNAL(sigWrongIdEntered()), m_mapper, SLOT(revert())); // perform a clearing of the mapped widgets
}

FormDataInput::~FormDataInput()
{
    delete ui;
}

void FormDataInput::slotNeedChangeMapperIndex()
{
    auto enteredId = ui->m_leRecordId->text().toLongLong();
    for (int row = 0; row < m_enginesModel->rowCount(); ++row) {
        if (m_enginesModel->record(row).value(0).toLongLong() == enteredId) {
            emit sigChangeMapperIndex(row);
            return;
        }
    }
    QMessageBox::warning(this, tr("Error engine ""id"" value"),
                         tr("The engine with id=%1 does not exists. Please enter id value of an existent engine").arg(enteredId));
    emit sigWrongIdEntered();
}

void FormDataInput::slotSubmit()
{
//    qDebug() << "slotSubmit(), start";
    int currentIndex = m_mapper->currentIndex(); /* NOTE: save the current index suit only if performs updating of table. If there are performs
                                                  * delete or insert rows in database operation, there are need to use another way to restore
                                                  * current index (e.g. saving "id" value with following searching current index by it)
                                                  */
    if (!m_enginesModel->database().transaction()) {
        QMessageBox::critical(this, tr("Database transaction error"),
                              tr("The database driver do not support the transactions operations"));
    }
    if (m_enginesModel->submitAll()) {
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
    m_mapper->setCurrentIndex(currentIndex);
//    qDebug() << "slotSubmit(), end";
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
//    qDebug() << "before selectInitial(), columnTtype =" << columnTtype << ", [" << currentRow << "," << currentCol << "]"
//             << ", dataD =" << m_enginesModel->data( currIndex, Qt::DisplayRole).toString()
//             << ", dataE =" << m_enginesModel->data( currIndex, Qt::EditRole).toString()
//             << ", dataU =" << m_enginesModel->data( currIndex, Qt::UserRole).toString();
    const QVariant &userData = m_enginesModel->data(currIndex, Qt::UserRole);
    if ( userData != CustomSqlTableModel::NOT_SETTED && !editor->selectInitial( userData, columnTtype ) )
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

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
#include "dbt_editor/simple_dbt_editor.h"
#include "dbt_editor/complex_dbt_editor.h"
#include "db_info.h"
#include "custom_sql_table_model.h"

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

    connect(this, SIGNAL(sigSaveData()), m_mapper, SLOT(submit())); // submit changes from the mapped widgets to the model
    connect(this, SIGNAL(sigSaveData()), this, SLOT(slotSubmit())); // submit changes from the model to the database
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
    m_enginesModel->setTable("engines");
    m_enginesModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
//    m_enginesModel->setRelation(1, QSqlRelation("full_name_engines", "id", "name_modif_id"));
    m_enginesModel->setRelation(2, QSqlRelation("fuels_types", "id", "id"));
//    m_enginesModel->setRelation(3, QSqlRelation("combustion_chambers", "id", "draft_number"));

    // set combo box
    ui->m_cboxFuel->setModel(m_enginesModel->relationModel(2));
    ui->m_cboxFuel->setModelColumn(0);

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

    m_enginesModel->select();
    m_mapper->toFirst();
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
}

FormDataInput::~FormDataInput()
{
    delete ui;
}

void FormDataInput::slotNeedChangeMapperIndex()
{
    int enteredId = ui->m_leRecordId->text().toInt();
    for (int row = 0; row < m_enginesModel->rowCount(); ++row) {
        if (m_enginesModel->record(row).value(0).toInt() == enteredId) {
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
    qDebug() << "slotSubmit(); start";
    int currentIndex = m_mapper->currentIndex(); /* NOTE: save the current index suit only if performs updating of table. If there are performs
                                                  * delete or insert rows in database operation, there are need to use another way to restore
                                                  * current index (e.g. saving "id" value with following searching current index by it)
                                                  */
    if (!m_enginesModel->database().transaction())
        QMessageBox::critical(this, tr("Database transaction error"),
                              tr("The database driver do not support the transactions operations"));
    if (m_enginesModel->submitAll()) {
        // In this line the mapper current index is -1 (after submitAll() calling)
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
    qDebug() << "slotSubmit(); end";
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

    dbi::DBTInfo *dbtInfo = DBINFO.tableByName(pbEditDBT->DBTableName());
    if ( !dbtInfo ) {
        QMessageBox::critical(this, tr("Invalid push button"),
                              tr("Cannot open the dialog.\n"
                                 "The reason is: cannot define the database table - pressed unknown push button.\n\n"
                                 "Please consult with the application developer for fixing this problem."));
        return;
    }

    std::shared_ptr<DBTEditor> editor(createDBTEditor(dbtInfo));
    if ( !editor.operator bool() ) {
        QMessageBox::critical(this, tr("Invalid database table information"),
                              tr("Cannot open the dialog.\n"
                                 "The reason is: cannot define the created dialog type, database table information is incorrect.\n\n"
                                 "Please consult with the application developer for fixing this problem."));
        return;
    }

    int id = 0;
    if ( !getCurrentIdForSelection(id, pbEditDBT->identWidget()) ) return;
    editor->selectInitial(id);
    if ( editor->exec() == QDialog::Accepted ) {
//        QString identifData = editor->identificationData(); // TODO: maybe delete?
//        if (!identifData.isEmpty()) pbEditDBT->setIdentData( identifData ); // set identification data in appropriate widget - TODO: maybe delete?
        m_enginesModel->setData( m_enginesModel->index( m_mapper->currentIndex(), m_mapper->mappedSection(pbEditDBT->identWidget()) ),
                                 editor->selectedId(), Qt::EditRole );
    }
}

bool FormDataInput::getCurrentIdForSelection(int &id, QWidget *identWidget)
{
    QVariant vId = m_enginesModel->index( m_mapper->currentIndex(), m_mapper->mappedSection(identWidget) ).data(Qt::DisplayRole);
    bool b = false;
    id = vId.toInt(&b);
    if (!b) {
        QMessageBox::critical( this, tr("Error id"),
                               tr("Cannot get the valid integer id value for select row with it.\n"
                                  "The reason is: cannot convert the variant type value \"%1\" to the integer type value")
                               .arg(vId.toString()) );
    }
    return b;
}

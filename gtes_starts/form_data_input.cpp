#include <QSqlQuery>
#include <QDataWidgetMapper>
#include <QSqlRecord> // TODO: delete?
#include <QDebug>
#include <memory>

#include "form_data_input.h"
#include "ui_form_data_input.h"
#include "dlgs/simple_db_table_dialog.h"
#include "dlgs/complex_db_table_dialog.h"
#include "db_info.h"

FormDataInput::FormDataInput(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FormDataInput)
    , m_enginesModel(new QSqlQueryModel(this))
    , m_mapper(new QDataWidgetMapper(this))
{
    ui->setupUi(this);
    setPushBtnsForEditDBTables();
    populateData();
    setRecordsNavigation();
    // TODO: set int validator for ui->m_leRecordId line edit
}

/* set push buttons, that call some widget for editing DB tables */
void FormDataInput::setPushBtnsForEditDBTables()
{
    const char *sigCommon = SIGNAL(clicked()); // the common signal for all push buttons
    const char *slotCommon = SLOT(slotOpenDBTableDialog()); // the common slot for all push buttons

    ui->m_pbEditIdentif->setDBTableName("identification_data_engines");
    ui->m_pbEditIdentif->setDataLabel( ui->m_lblIdentifData );
    connect(ui->m_pbEditIdentif, sigCommon, slotCommon);

    ui->m_pbEditFuels->setDBTableName("fuels_types");
    connect(ui->m_pbEditFuels, sigCommon, slotCommon);

    ui->m_pbEditChambers->setDBTableName("combustion_chambers");
    ui->m_pbEditChambers->setDataLabel( ui->m_lblChamberData );
    connect(ui->m_pbEditChambers, sigCommon, slotCommon);

    ui->m_pbEditStartDevices->setDBTableName("start_devices");
    ui->m_pbEditStartDevices->setDataLabel( ui->m_lblStartDeviceData );
    connect(ui->m_pbEditStartDevices, sigCommon, slotCommon);
}

void FormDataInput::populateData()
{
    m_enginesModel->setQuery("SELECT * FROM engines");
    m_mapper->setModel(m_enginesModel);
    m_mapper->addMapping(ui->m_lblIdData, 0);
    m_mapper->addMapping(ui->m_lblIdentifData, 1);
    m_mapper->addMapping(ui->m_cboxFuel, 2);
    m_mapper->addMapping(ui->m_lblChamberData, 3);
    m_mapper->addMapping(ui->m_lblStartDeviceData, 4);
    m_mapper->addMapping(ui->m_sboxStartDevicesQntyData, 5);
    m_mapper->addMapping(ui->m_teComments, 6);
    m_mapper->toFirst();
}

void FormDataInput::setRecordsNavigation()
{
    connect(ui->m_tbRecordFirst, SIGNAL(clicked()), m_mapper, SLOT(toFirst()));
    connect(ui->m_tbRecordLast, SIGNAL(clicked()), m_mapper, SLOT(toLast()));
    connect(ui->m_tbRecordPrev, SIGNAL(clicked()), m_mapper, SLOT(toPrevious()));
    connect(ui->m_tbRecordNext, SIGNAL(clicked()), m_mapper, SLOT(toNext()));
    connect(ui->m_leRecordId, SIGNAL(returnPressed()), this, SLOT(slotNeedChangeMapperIndex()));
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
    // TODO: implement showing the error message box
    qDebug() << tr("The engine with id=%1 does not exists. Please enter id value of an existent engine").arg(enteredId);
    emit sigWrongIdEntered();
}

/* Factory method for creation some type of a database table dialog */
DBTableDialog * FormDataInput::createDBTableDialog(DBTableInfo *info)
{
    DBTableDialog *dlg = 0;
    switch (info->m_type) {
    case DBTableInfo::ttype_simple:
        dlg = new SimpleDBTableDialog(info, this);
        break;
    case DBTableInfo::ttype_complex:
        dlg = new ComplexDBTableDialog(info, this);
        break;
    default:
        break;
    }
    return dlg;
}

void FormDataInput::slotOpenDBTableDialog()
{
    QSqlDatabase::database().isOpen(); // FIXME: the connection loses without this checking
    PushButtonForEditDBTable *pbKDBT = qobject_cast<PushButtonForEditDBTable *>(sender());
    if ( !pbKDBT ) {
        // TODO: Generate the error #XXX: Invalid push button. Consult with a application developer.
        qDebug() << "Generate the error #1: Invalid push button. Consult with a application developer.";
        return;
    }

//    QStringList names;
//    QSqlQuery query(QString("SELECT * FROM gtes_starts.%1;").arg(pbKDBT->DBTableName()));
//    while (query.next()) {
//        names.push_back(query.value(1).toString().trimmed());
//    }

    DBTableInfo *dbTableInfo = DBINFO.findTable(pbKDBT->DBTableName());
    if ( !dbTableInfo ) {
        // TODO: Generate the error #XXX: Invalid push button. Cannot define the database table. Consult with a application developer.
        qDebug() << "Generate the error #2: Invalid push button. Cannot define the database table. Consult with a application developer.";
        return;
    }

    std::shared_ptr<DBTableDialog> dialog(createDBTableDialog(dbTableInfo));
    if ( !dialog.operator bool() ) {
        // TODO: Generate the error #XXX: Invalid push button. Cannot define the created dialog type. Consult with a application developer.
        qDebug() << "Generate the error #3: Invalid push button. Cannot define the created dialog type. Consult with a application developer.";
        return;
    }

    if (dialog->exec()) {
        QString identStr = dialog->identityString();
        if (!identStr.isEmpty()) pbKDBT->dataLabel()->setText( identStr );
    }
}

void FormDataInput::slotTemp()
{
    qDebug() << "calling temp slot";
}

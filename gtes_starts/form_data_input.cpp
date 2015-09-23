#include <QSqlQuery>
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
{
    ui->setupUi(this);
    setPBtnsForEditDBTables();
}

void FormDataInput::setPBtnsForEditDBTables()
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

FormDataInput::~FormDataInput()
{
    delete ui;
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

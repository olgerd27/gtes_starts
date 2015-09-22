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
    setDBTableNames();

    const char *slotMethod = SLOT(slotOpenDBTableDialog());
    connect(ui->m_pbEditNames, SIGNAL(clicked()), this, slotMethod);
    connect(ui->m_pbEditFuels, SIGNAL(clicked()), this, slotMethod);
    connect(ui->m_pbEditChambers, SIGNAL(clicked()), this, slotMethod);
    connect(ui->m_pbEditStartDevices, SIGNAL(clicked()), this, slotMethod);
}

void FormDataInput::setDBTableNames()
{
    ui->m_pbEditNames->setDBTableName("engines_names");
    ui->m_pbEditFuels->setDBTableName("fuels_types");
    ui->m_pbEditChambers->setDBTableName("combustion_chambers");
    ui->m_pbEditChambers->setDataLabel( ui->m_lblChamberData );
    ui->m_pbEditStartDevices->setDBTableName("start_devices");
    ui->m_pbEditStartDevices->setDataLabel( ui->m_lblStartDeviceData );
}

FormDataInput::~FormDataInput()
{
    delete ui;
}

/* Factory method for creation some type of a database table dialog */
DBTableDialog * FormDataInput::createDBTableDialog(DBTableInfo *info)
{
    int degree = info->tableDegree();
    bool fieldWasFound = info->fieldByName("id").isValid();
    if ( fieldWasFound && degree == 2 )
        return new SimpleDBTableDialog(info, this);
    else if ( fieldWasFound && degree > 2 )
        return new ComplexDBTableDialog(info, this);
    else
        return 0;
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

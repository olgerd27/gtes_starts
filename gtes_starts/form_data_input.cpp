#include <QSqlQuery>
#include <QDebug>

#include "form_data_input.h"
#include "ui_form_data_input.h"
#include "simple_db_table_dialog.h"

FormDataInput::FormDataInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormDataInput)
{
    ui->setupUi(this);
    setDBTableNames();

    connect(ui->m_pbEditNames, SIGNAL(clicked()), this, SLOT(slotEditSimplyDBTable()));
    connect(ui->m_pbEditFuels, SIGNAL(clicked()), this, SLOT(slotEditSimplyDBTable()));
}

void FormDataInput::setDBTableNames()
{
    ui->m_pbEditNames->setDBTableName("engines_names");
    ui->m_pbEditFuels->setDBTableName("fuels_types");
    ui->m_pbEditChambers->setDBTableName("combustion_chambers");
    ui->m_pbEditStartDevices->setDBTableName("start_devices");
}

FormDataInput::~FormDataInput()
{
    delete ui;
}

void FormDataInput::slotEditSimplyDBTable()
{
    qDebug() << "SimpleDBTableDialog: Is DB open? " << QSqlDatabase::database().isOpen(); // FIXME: the connection loses without this checking
    PushButtonKnowsDBTable *pbKDBT = 0;
    if ( (pbKDBT = qobject_cast<PushButtonKnowsDBTable *>(sender())) == 0 )
        return;
    QStringList names;
    QSqlQuery query(QString("SELECT * FROM gtes_starts.%1;").arg(pbKDBT->DBTableName()));
    while (query.next()) {
        names.push_back(query.value(1).toString().trimmed());
    }

    SimpleDBTableDialog dlgEngineNames(this);
    dlgEngineNames.setData(names);
    if (dlgEngineNames.exec()) {
        // TODO: exec INSERT, UPDATE and/or DELETE SQL-statement, returned by the SimpleDBTableDialog class object
    }
}

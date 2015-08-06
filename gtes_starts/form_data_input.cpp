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

    connect(ui->m_pbEditEnginesNames, SIGNAL(clicked()), this, SLOT(slotEditEnginesNames()));
    connect(ui->m_pbEditFuels, SIGNAL(clicked()), this, SLOT(slotEditFuels()));
}

FormDataInput::~FormDataInput()
{
    delete ui;
}

void FormDataInput::slotEditEnginesNames()
{
    editSimplyDBTable("engines_names");
}

void FormDataInput::slotEditFuels()
{
    editSimplyDBTable("fuels_types");
}

void FormDataInput::editSimplyDBTable(const QString &tablename)
{
    qDebug() << "SimpleDBTableDialog: Is DB open? " << QSqlDatabase::database().isOpen(); // FIXME: the connection loses without this checking
    QStringList names;
    QSqlQuery query(QString("SELECT * FROM gtes_starts.%1;").arg(tablename));
    while (query.next()) {
        names.push_back(query.value(1).toString().trimmed());
    }

    SimpleDBTableDialog dlgEngineNames(this);
    dlgEngineNames.setData(names);
    if (dlgEngineNames.exec()) {
        // TODO: exec INSERT, UPDATE and/or DELETE returned by queries of the SimpleDBTableDialog class
    }
}

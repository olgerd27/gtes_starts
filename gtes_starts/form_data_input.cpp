#include <QDebug>

#include "form_data_input.h"
#include "ui_form_data_input.h"
#include "simple_table_dialog.h"

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
    QStringList names = { "DG90", "DN80", "DA91" }; // TODO: exec SELECT query and extract all names (without id values) for passing to dialog
    SimpleTableDialog dlgEngineNames(this);
    dlgEngineNames.setData(names);
    if (dlgEngineNames.exec()) {
        // TODO: exec INSERT, UPDATE and/or DELETE returned queries
    }
}

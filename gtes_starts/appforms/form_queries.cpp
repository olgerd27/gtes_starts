#include "form_queries.h"
#include "ui_form_queries.h"

FormQueries::FormQueries(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormQueries)
{
    ui->setupUi(this);
}

FormQueries::~FormQueries()
{
    delete ui;
}

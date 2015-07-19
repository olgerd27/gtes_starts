#include "form_graphs.h"
#include "ui_form_graphs.h"

FormGraphs::FormGraphs(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormGraphs)
{
    ui->setupUi(this);
}

FormGraphs::~FormGraphs()
{
    delete ui;
}

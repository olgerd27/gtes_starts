#include "form_options.h"
#include "ui_form_options.h"

FormOptions::FormOptions(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormOptions)
{
    ui->setupUi(this);
}

FormOptions::~FormOptions()
{
    delete ui;
}

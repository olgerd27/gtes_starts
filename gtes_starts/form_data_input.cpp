#include "form_data_input.h"
#include "ui_form_data_input.h"

FormDataInput::FormDataInput(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormDataInput)
{
    ui->setupUi(this);
}

FormDataInput::~FormDataInput()
{
    delete ui;
}

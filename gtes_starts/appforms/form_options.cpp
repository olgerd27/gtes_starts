#include "form_options.h"
#include "ui_form_options.h"

FormOptions::FormOptions(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::FormOptions)
{
    m_ui->setupUi(this);
}

FormOptions::~FormOptions()
{ }

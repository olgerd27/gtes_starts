#include "form_queries.h"
#include "ui_form_queries.h"

FormQueries::FormQueries(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::FormQueries)
{
    m_ui->setupUi(this);
}

FormQueries::~FormQueries()
{ }

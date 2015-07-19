#include "main_window.h"
#include "ui_main_window.h"
#include "form_data_input.h"
#include "form_queries.h"
#include "form_graphs.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QStackedWidget *sw = ui->m_stackForms;
    sw->addWidget(new FormDataInput(sw));
    sw->addWidget(new FormQueries(sw));
    sw->addWidget(new FormGraphs(sw));

    connect(ui->m_listChoice, SIGNAL(currentRowChanged(int)), sw, SLOT(setCurrentIndex(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

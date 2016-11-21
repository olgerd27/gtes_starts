#include <QSqlError>
#include <QDebug>  // TODO: temp, delete later
#include "form_data_input.h"
#include "ui_form_data_input.h"
#include "engine_widget.h"

// TODO: temp, delete later
#include "../models/prx_decor/proxy_decor_model.h"
#include "../models/src_sql/custom_sql_table_model.h"

/*
 * FormDataInput
 */
FormDataInput::FormDataInput(QWidget *parent)
    : QWidget(parent)
    , m_ui(new Ui::FormDataInput)
{
    m_ui->setupUi(this);
    setMainControls();
    setDataNavigation();
    connect(m_ui->tabEngines, SIGNAL(sigChTypeChanged(int)), m_ui->m_lblModelChangeType, SLOT(slotChangeType(int)));
    connect(m_ui->tabEngines, SIGNAL(sigChTypeToDeletedChanged(bool)), m_ui->m_lblEngineName, SLOT(setDisabled(bool)));
    connect(m_ui->tabEngines, SIGNAL(sigEngineNameGenerated(QString)), m_ui->m_lblEngineName, SLOT(setText(QString)));
    m_ui->tabEngines->init();

    /*
     * TODO: when data was saved in the "engines" DB tale, there are need to refresh data in all composite DB tables.
     * Or, maybe do not refresh, but save flag that "engines" DB changes saved. And when user click to a some tab
     * with editor of a some DB table, data will be refreshed in this particular DB table and displays already refreshed on this tab.
     */

    // test of composite DB table editor
    qDebug() << "********** engines_bypasses **********";
    ProxyDecorModel *model = new ProxyDecorModel(this);
    model->setSqlTableName("engines_bypasses");
//    model->customSourceModel()->setFilter("engine_id='1'");
    m_ui->m_tvDBTContent->setModel(model);
    qDebug() << "********** **********";
}

void FormDataInput::setMainControls()
{
    // Transmitting signals
    connect(this, SIGNAL(sigInsertNew()), m_ui->tabEngines, SIGNAL(sigInsertNew())); // insert data
    connect(this, SIGNAL(sigDeleteRow()), m_ui->tabEngines, SIGNAL(sigDeleteRow())); // delete data
    connect(this, SIGNAL(sigSaveAll()), m_ui->tabEngines, SIGNAL(sigSaveAll())); // save data
    connect(this, SIGNAL(sigRefreshAll()), m_ui->tabEngines, SIGNAL(sigRefreshAll())); // refresh data
}

void FormDataInput::setDataNavigation()
{
    // navigation set - signals transmitting
    connect(m_ui->m_tbEngineFirst, SIGNAL(clicked()), m_ui->tabEngines, SIGNAL(sigToFirstRec()));
    connect(m_ui->m_tbEngineLast, SIGNAL(clicked()), m_ui->tabEngines, SIGNAL(sigToLastRec()));
    connect(m_ui->m_tbEnginePrev, SIGNAL(clicked()), m_ui->tabEngines, SIGNAL(sigToPreviousRec()));
    connect(m_ui->m_tbEngineNext, SIGNAL(clicked()), m_ui->tabEngines, SIGNAL(sigToNextRec()));
    setLEid();
    // checking boundaries of DB data records
    connect(m_ui->tabEngines, SIGNAL(sigFirstRecReached(bool)), m_ui->m_tbEngineFirst, SLOT(setDisabled(bool)));
    connect(m_ui->tabEngines, SIGNAL(sigFirstRecReached(bool)), m_ui->m_tbEnginePrev, SLOT(setDisabled(bool)));
    connect(m_ui->tabEngines, SIGNAL(sigLastRecReached(bool)), m_ui->m_tbEngineLast, SLOT(setDisabled(bool)));
    connect(m_ui->tabEngines, SIGNAL(sigLastRecReached(bool)), m_ui->m_tbEngineNext, SLOT(setDisabled(bool)));
}

void FormDataInput::setLEid()
{
    m_ui->m_leEngineId->setValidator(new QIntValidator(0, 1e6, m_ui->m_leEngineId)); // control input integer values in range between 0 and 1e6
    connect(m_ui->tabEngines, SIGNAL(sigCurrentIdChanged(QString)), m_ui->m_leEngineId, SLOT(setText(QString))); // set a changed new Id to the LineEdit

    // behaviour after pressing Enter in the Engine's id LineEdit
    connect(m_ui->m_leEngineId, SIGNAL(sigReturnPressed(QString)),
            m_ui->tabEngines, SLOT(slotTryChangeEngine(QString)));
    connect(m_ui->tabEngines, SIGNAL(sigWrongEngineId()),
            m_ui->m_leEngineId, SLOT(clear())); // indicate that inputed value is wrong and there are need to input another
}

FormDataInput::~FormDataInput()
{
    delete m_ui;
}

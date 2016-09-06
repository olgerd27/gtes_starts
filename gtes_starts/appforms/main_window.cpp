#include <QStyledItemDelegate>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QScrollBar>
#include <QDesktopWidget>
#include <QDebug>

#include "main_window.h"
#include "ui_main_window.h"
#include "form_data_input.h"
#include "form_queries.h"
#include "form_options.h"

class DelegateListIconsLeft : public QStyledItemDelegate
{
public:
    DelegateListIconsLeft(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    { }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem optionLeftAlign = option;
        optionLeftAlign.decorationAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        QStyledItemDelegate::paint(painter, optionLeftAlign, index);
    }
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    m_ui->m_listChoice->setItemDelegate(new DelegateListIconsLeft(m_ui->m_listChoice));
    /*
     * TODO: in time of the first running app on particular computer, move window to the center of the desktop with help of:  \
     * widget.move(QApplication::desktop()->screen()->rect().center() - widget.rect().center());
     * In time of MainWindow closing it will need save current geometry in settings file and restore it by next app running.
     */
    setWindowIcon(QIcon(":/images/window_icon.png")); // TODO: use another icon
    setWindowTitle( tr("Control application") );
    setPosition();

    // TODO: delete later
    m_ui->dockWidget->hide();
    adjustSize();

    /*
     * Open database connection
     * TODO: move to the external class for manipulation with DB
     */
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName("localhost"); // local: localhost, LAN: 190.91.112.211
    db.setPort(3306);
    db.setUserName("root");
    db.setPassword("optimus27");
    db.setDatabaseName("gtes_starts");
    if (!db.open()) {
        QMessageBox::critical( 0, tr("Database connection error"),
                               QString("%1:\n\t%2: '%3'\n\t%4: '%5'\n\t%6: '%7'\n\t%8: '%9'\n\n%10: %11")
                               .arg(tr("Cannot connect to the database with the next settings"))
                               .arg(tr("hostname"), db.hostName(), tr("port")).arg(db.port())
                               .arg(tr("username"), db.userName(), tr("password"), db.password())
                               .arg(tr("The database error message"), db.lastError().text()) );
        exit(1);
    }
//    qDebug() << "main: Is DB open? " << db.isOpen();

//    QSqlQuery query("SELECT * FROM gtes_starts.graphs_parameters_type;");
//    while (query.next()) {
//        int id = query.value(0).toInt();
//        QString symbol = query.value(1).toString();
//        QString fullname = query.value(2).toString();
//        QString units = query.value(3).toString().trimmed();
//        qDebug() << id << "\t" << symbol << "\t" << fullname << "\t" << units;
//    }

//    QSqlQuery query("SELECT * FROM gtes_starts.graphs_parameters_values;");
//    while (query.next()) {
//        int id = query.value(0).toInt();
//        int par_type_id = query.value(1).toInt();
//        QByteArray par_values = query.value(2).toByteArray().trimmed();
//        qDebug() << id << "\t" << par_type_id << "\t" << par_values;
//    }
//    qDebug() << "Last error:" << db.lastError().text();

    QStackedWidget *sw = m_ui->m_stackForms;
    FormDataInput *formDInput = new FormDataInput(this);
    sw->insertWidget(index_data_input, formDInput);
    sw->insertWidget(index_queries, new FormQueries(this));
    sw->insertWidget(index_options, new FormOptions(this));

    // actions connections
    connect(m_ui->m_actCreateEngine, SIGNAL(triggered()), formDInput, SIGNAL(sigInsertNew()));
    connect(m_ui->m_actDeleteEngine, SIGNAL(triggered()), formDInput, SLOT(slotDeleteRecord())); // TODO: implement and use SIGNAL, as for the others connections here
    connect(m_ui->m_actSave, SIGNAL(triggered()), formDInput, SIGNAL(sigSaveAll()));
    connect(m_ui->m_actRefresh, SIGNAL(triggered()), formDInput, SIGNAL(sigRefreshAll()));
//    connect(ui->m_actConnectToDB, SIGNAL(triggered()), formDInput, SIGNAL(sigRevertChanges())); // TODO: replace the action to the appropriate one
    connect(m_ui->m_actAboutApp, SIGNAL(triggered()), this, SLOT(slotAboutApp()));
    connect(m_ui->m_actAboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(m_ui->m_actExit, SIGNAL(triggered()), qApp, SLOT(quit()));

    // another signals & slots connections
    connect(m_ui->m_listChoice, SIGNAL(currentRowChanged(int)), sw, SLOT(setCurrentIndex(int)));
}

void MainWindow::setPosition()
{
    // The main window appears in the center of desktop by default
//    this->move( QApplication::desktop()->screenGeometry().center() - this->rect().center() );
}

MainWindow::~MainWindow()
{
    QSqlDatabase::database().close(); // TODO: move to the external class for manipulation with DB
}

void MainWindow::debugOutput(const QString &msg)
{
    m_ui->m_dbgOut->appendPlainText(msg);
    QScrollBar *vScrollBar = m_ui->m_dbgOut->verticalScrollBar();
    vScrollBar->setValue(vScrollBar->maximum());
}

void MainWindow::slotAboutApp()
{
    // TODO: figure out how to use QObject::tr() function and QString::arg() for inserting some data in a string dynamically.
    QString winTitle = windowTitle();
    // TODO: write correct "About application" text
    QString text = QString("The <b>") + winTitle + "</b> "
                   "performs input, delete, update and query data, that stores in the GTE's starts database.<br><br>"
                   "<b>Version 1.0</b> (freeware).<br><br>"
                   "The programm is provided \"AS IS\" with no warranty of any kind, "
                   "including the warranty of design, merchantability and "
                   "fitness for a particular purpose.<br><br>"
                   "Developed by the group of GTE's starts characteristics, dep.19.<br>"
                   "Developer: Matiyuk O.I., olmati@zorya.com.<br><br>"
                   "Nikolaev, Ukraine - 2014-2016.";
    QMessageBox::about(this, tr("About") + " " + winTitle, tr(text.toUtf8()));
}

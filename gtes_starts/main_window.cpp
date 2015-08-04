#include "main_window.h"
#include "ui_main_window.h"
#include "form_data_input.h"
#include "form_queries.h"
#include "form_options.h"
#include "core_app.h"

#include <QStyledItemDelegate>
#include <QDebug>

class DelegateListIconsLeft : public QStyledItemDelegate
{
public:
    DelegateListIconsLeft(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QStyleOptionViewItem optionLeftAlign = option;
        optionLeftAlign.decorationAlignment = Qt::AlignLeft | Qt::AlignVCenter;
        QStyledItemDelegate::paint(painter, optionLeftAlign, index);
    }
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->m_listChoice->setItemDelegate(new DelegateListIconsLeft(ui->m_listChoice));

    QStackedWidget *sw = ui->m_stackForms;
    sw->insertWidget(index_data_input, new FormDataInput(sw));
    sw->insertWidget(index_queries, new FormQueries(sw));
    sw->insertWidget(index_options, new FormOptions(sw));

    // signals & slots connections
    connect(ui->m_listChoice, SIGNAL(currentRowChanged(int)), sw, SLOT(setCurrentIndex(int)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

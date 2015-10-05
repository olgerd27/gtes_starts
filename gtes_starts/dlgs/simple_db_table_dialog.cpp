#include <QListView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QSqlTableModel>
#include <QDebug>

#include "simple_db_table_dialog.h"
#include "ui_simple_db_table_dialog.h"
#include "db_info.h"

/*
 * HighlightListDelegate
 */
class HighlightListDelegate : public QStyledItemDelegate
{
public:
    HighlightListDelegate(QObject *parent = 0)
        : QStyledItemDelegate(parent)
    {
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex          &index) const
    {
        if(option.state & QStyle::State_MouseOver) {
            QRect rect = option.rect;
            QLinearGradient gradient(0, 0, rect.width(), rect.height());
            gradient.setColorAt(0, Qt::white);
            gradient.setColorAt(0.8, Qt::lightGray);
            gradient.setColorAt(1, Qt::gray);
            // painter paint a rectangle with setted gradient
            painter->setBrush(gradient);
            painter->setPen(Qt::NoPen);
            painter->drawRect(rect);
        }
        QStyledItemDelegate::paint(painter, option, index); // paint part of the base class
    }
};

/*
 * SimpleDBTableDialog
 */
SimpleDBTableDialog::SimpleDBTableDialog(DBTableInfo *dbTable, QWidget *parent)
    : DBTableDialog(dbTable, parent)
    , ui(new Ui::SimpleDBTableDialog)
{
    ui->setupUi(this);
    setWidgetTitle();
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setDBdataView();
}

void SimpleDBTableDialog::setDBdataView()
{
    // ListView settings
    QListView *view = ui->m_listvData;
    view->setItemDelegate(new HighlightListDelegate(view));
    view->viewport()->setAttribute(Qt::WA_Hover);
    view->setModel(m_model);
    view->setModelColumn(2); // column number is 2, as the 0-th column is check icon and 1-th is "id"
    connect(ui->m_leSearchMask, SIGNAL(textChanged(QString)), this, SLOT(slotSetFilter(QString)));
}

SimpleDBTableDialog::~SimpleDBTableDialog()
{
    delete ui;
}

void SimpleDBTableDialog::slotSetFilter(const QString &strFilter)
{
    m_model->setFilter("name LIKE '%" + strFilter + "%'");
}

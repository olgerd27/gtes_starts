#include <QListView>
#include <QStyledItemDelegate>
#include <QPainter>
#include <QSqlTableModel>
#include <QStringListModel> // TODO: delete?
#include <QSortFilterProxyModel> // TODO: delete?
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
    view->setModelColumn(1);

//    m_sourceModel = new QStringListModel(this);
//    QSortFilterProxyModel *filterModel = new QSortFilterProxyModel(this);
//    filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
//    filterModel->setSourceModel(m_sourceModel);
//    view->setModel(filterModel);
//    connect(ui->m_leSearchMask, SIGNAL(textChanged(QString)), filterModel, SLOT(setFilterFixedString(QString)));
}

SimpleDBTableDialog::~SimpleDBTableDialog()
{
    delete ui;
}

void SimpleDBTableDialog::setData(const QStringList &data)
{
    m_sourceModel->setStringList(data);
}

QStringList SimpleDBTableDialog::SQLstatements() const
{
    // TODO: forming queries with help of some QueryCreator class
    return QStringList();
}

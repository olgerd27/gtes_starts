#include <QItemDelegate>
#include <QPainter>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QDebug>

#include "simple_db_table_dialog.h"
#include "ui_simple_db_table_dialog.h"

/*
 * HighlightDelegate
 */
class HighlightDelegate : public QItemDelegate
{
public:
    HighlightDelegate(QObject *parent = 0)
        : QItemDelegate(parent)
    {
    }

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex          &index) const {
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
        QItemDelegate::paint(painter, option, index); // paint part of the base class
    }
};

/*
 * SimpleDBTableDialog
 */
SimpleDBTableDialog::SimpleDBTableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SimpleDBTableDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setDBdataView();
}

void SimpleDBTableDialog::setDBdataView()
{
    // ListView settings
    QListView *view = ui->m_listvData;
    view->setItemDelegate(new HighlightDelegate(view));
    view->viewport()->setAttribute(Qt::WA_Hover);

    // ListView model setting
    m_sourceModel = new QStringListModel(this);
    QSortFilterProxyModel *filterModel = new QSortFilterProxyModel(this);
    filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    filterModel->setSourceModel(m_sourceModel);
    view->setModel(filterModel);
    connect(ui->m_leSearchMask, SIGNAL(textChanged(QString)), filterModel, SLOT(setFilterFixedString(QString)));
}

SimpleDBTableDialog::~SimpleDBTableDialog()
{
    delete ui;
}

void SimpleDBTableDialog::setData(const QStringList &data)
{
    m_sourceModel->setStringList(data);
}

QStringList SimpleDBTableDialog::queries() const
{
    // TODO: forming queries with help of some QueryCreator class
    return QStringList();
}

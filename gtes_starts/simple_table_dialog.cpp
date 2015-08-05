#include <QItemDelegate>
#include <QPainter>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QDebug>
#include "simple_table_dialog.h"
#include "ui_simple_table_dialog.h"

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
 * SimpleTableDialog
 */
SimpleTableDialog::SimpleTableDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SimpleTableDialog)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setDBdataView();
}

void SimpleTableDialog::setDBdataView()
{
    // ListView settings
    QListView *lv = ui->m_listvData;
    lv->setItemDelegate(new HighlightDelegate(lv));
    lv->viewport()->setAttribute(Qt::WA_Hover);

    // ListView model setting
    m_sourceModel = new QStringListModel(this);
    QSortFilterProxyModel *filterModel = new QSortFilterProxyModel(this);
    filterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    filterModel->setSourceModel(m_sourceModel);
    lv->setModel(filterModel);
    connect(ui->m_leSearchMask, SIGNAL(textChanged(QString)), filterModel, SLOT(setFilterFixedString(QString)));
}

SimpleTableDialog::~SimpleTableDialog()
{
    delete ui;
}

void SimpleTableDialog::setData(const QStringList &data)
{
    m_sourceModel->setStringList(data);
}

QStringList SimpleTableDialog::queries() const
{
    // TODO: forming queries with help of some QueryCreator class
}

void SimpleTableDialog::slotFilterList(const QString &mask)
{
//    QList<QListWidgetItem *> findedItems = ui->m_listwData->findItems(mask, Qt::MatchContains);
//    foreach (QListWidgetItem *item, findedItems) {
//        qDebug() << item->text();
//    }
    qDebug() << "------";
}


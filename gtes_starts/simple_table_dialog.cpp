#include <QItemDelegate>
#include <QPainter>
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

    // ListWidget settings
    QListWidget *lw = ui->m_listwData;
    lw->setItemDelegate(new HighlightDelegate(lw));
    lw->viewport()->setAttribute(Qt::WA_Hover);

    connect(ui->m_leSearchMask, SIGNAL(textChanged(QString)), this, SLOT(slotFilterList(QString)));
}

SimpleTableDialog::~SimpleTableDialog()
{
    delete ui;
}

void SimpleTableDialog::setData(const QStringList &data)
{
    QListWidget *listWgt = ui->m_listwData;
    foreach (QString str, data) {
        new QListWidgetItem(str, listWgt);
    }
}

QStringList SimpleTableDialog::queries() const
{
    // TODO: forming queries with help of some QueryCreator class
}

void SimpleTableDialog::slotFilterList(const QString &mask)
{
    QList<QListWidgetItem *> findedItems = ui->m_listwData->findItems(mask, Qt::MatchContains);
    foreach (QListWidgetItem *item, findedItems) {
        qDebug() << item->text();
    }
    qDebug() << "------";
}

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QDebug>
#include "reimplemented_widgets.h"

/*
 * ResizableWidgetList
 */
ResizableWidgetList::ResizableWidgetList(QWidget *parent)
    : QListWidget(parent)
{
}

QSize ResizableWidgetList::sizeHint() const
{
    QSize sizeContent = QListWidget::contentsSize();
    sizeContent.rwidth() += 5;
    return sizeContent;
}

/*
 * PBtnForEditDBT
 */
PBtnForEditDBT::PBtnForEditDBT(QWidget *parent)
    : QPushButton(parent)
    , m_identWgt(0)
{
}

void PBtnForEditDBT::setDBTableName(const QString &name)
{
    m_DBTableName = name;
}

QString PBtnForEditDBT::DBTableName() const
{
    return m_DBTableName;
}

void PBtnForEditDBT::setIdentDataWidget(QWidget *wgt)
{
    m_identWgt = wgt;
}

QWidget *PBtnForEditDBT::identWidget() const
{
    return m_identWgt;
}


/*
 * MChTypeLabel
 */
MChTypeLabel::MChTypeLabel(QWidget *parent)
    : QLabel(parent)
{ }

void MChTypeLabel::slotChangeType(int ctype)
{
    QString strType;
    QPalette pal = palette();
    QPalette::ColorRole crWinText = QPalette::WindowText;
    switch (ctype) {
    case ctype_inserted:
        strType = tr("inserted");
        pal.setColor(crWinText, Qt::darkGreen);
        show();
        break;
    case ctype_deleted:
        strType = tr("deleted");
        pal.setColor(crWinText, Qt::red);
        show();
        break;
    case ctype_noChange:
        hide();
        break;
    default:
        // TODO: generate the message error
        qWarning() << tr("Unknown change model type - ") + QString::number((int)ctype);
        break;
    }
    setText(strType);
    setPalette(pal);
}

/*
 * DataSendLineEdit
 */
DataSendLineEdit::DataSendLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    connect(this, &DataSendLineEdit::returnPressed, [this]{ emit sigReturnPressed(text()); } );
}

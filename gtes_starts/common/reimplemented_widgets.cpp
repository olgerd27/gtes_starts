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
SelectEditPB::SelectEditPB(QWidget *parent)
    : QPushButton(QIcon(":/images/edit.png"), tr("Select/Edit"), parent)
    , m_identWgt(0)
    , m_fieldNo(-1)
{
    setStyleSheet("text-align: left");
}

void SelectEditPB::setDBTableName(const QString &name)
{
    m_DBTableName = name;
}

QString SelectEditPB::DBTableName() const
{
    return m_DBTableName;
}

void SelectEditPB::setIdentDataWidget(QWidget *wgt)
{
    m_identWgt = wgt;
}

QWidget *SelectEditPB::identWidget() const
{
    return m_identWgt;
}

void SelectEditPB::setFieldNumber(int fn)
{
    m_fieldNo = fn;
}

int SelectEditPB::fieldNo() const
{
    return m_fieldNo;
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
 * LE_DataSend
 */
LE_DataSend::LE_DataSend(QWidget *parent)
    : QLineEdit(parent)
{
    connect(this, &LE_DataSend::returnPressed, [this]{ emit sigReturnPressed(text()); } );
}

/*
 * LE_definerFLCh
 */
LE_DefinerFLCh::LE_DefinerFLCh(QWidget *parent)
    : QLineEdit(parent)
    , m_isPrevEmpty(true)
{
    connect( this, &LE_DefinerFLCh::textChanged, [this]()
    {
        if (text().isEmpty()) { // catch the case, when deleted a some last character
            emit sigChangesExistenceCh(true);
            m_isPrevEmpty = true;
        }
        else if (!text().isEmpty() && m_isPrevEmpty) { // catch the case, when inputed a some first character
            emit sigChangesExistenceCh(false);
            m_isPrevEmpty = false;
        }
    } );
}

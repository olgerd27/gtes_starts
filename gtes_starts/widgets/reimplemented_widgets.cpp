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
    , m_defaultPalette(palette())
{
//    setAutoFillBackground(true);
}

void MChTypeLabel::slotChangeType(int ctype)
{
    QString strType;
    QPalette pal = palette();
    bool isAFBG = false; // is AutoFillBackGround. Changing the label's background color is possible when this parameter is true, if false - use inherited color.
    switch (ctype) {
    case ctype_inserted:
    {
        strType = tr("inserted");
        setCustomPalette(pal, Qt::darkGreen);
        isAFBG = true;
        break;
    }
    case ctype_deleted:
        strType = tr("deleted");
        setCustomPalette(pal, Qt::red);
        isAFBG = true;
        break;
    case ctype_noChange:
        strType = "";
        pal = m_defaultPalette;
        isAFBG = false;
        break;
    default:
        // TODO: generate the message error
        qWarning() << tr("Unknown change model type - ") + QString::number((int)ctype);
        return;
    }
    setText(strType);
    setPalette(pal);
    setAutoFillBackground(isAFBG);
}

void MChTypeLabel::setCustomPalette(QPalette &pal, Qt::GlobalColor glColorBack)
{
    QColor colorBack(glColorBack);
    colorBack.setAlpha(150);
    pal.setColor(QPalette::Window, colorBack);
    pal.setColor(QPalette::WindowText, Qt::white);
}

/*
 * LE_DataSend
 */
LE_DataSend::LE_DataSend(QWidget *parent)
    : QLineEdit(parent)
{
    connect(this, &LE_DataSend::returnPressed, [this]{ emit sigReturnPressed(text()); } );
}

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

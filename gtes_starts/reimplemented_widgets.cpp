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

void PBtnForEditDBT::setIdentData(const QString &data)
{
    if (QLabel *lbl = qobject_cast<QLabel *>(m_identWgt))
        lbl->setText(data);
    else if (QLineEdit *le = qobject_cast<QLineEdit *>(m_identWgt))
        le->setText(data);
    else {
        Q_ASSERT_X( false, "PBtnForEditDBT::setIdentData()",
                    QString("Unknow identification data widget, that place near of the push button \"%1\"."
                            "Please set a known widget or add processing of a new widget to current function")
                    .arg(this->text()) );
    }
}

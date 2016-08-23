#include <stdexcept>
#include <limits>
#include "fl_widgets.h"
#include "db_info.h"

/*
 * The signal-slot connections in every widget constructor - is a spike. It must work when the mapper submit police was setted to the ManualSubmit.
 * The purpose of this spike is set data to the model when focus leaves the widget.
 * In the widget must be reimplemented the focusOutEvent() virtual method, and it must to emit the signal sigFocusOut().
 * Current functionality must be implemented for every widget, that is mapped with DB table field that is not foreign.
 */
/*
 * FL_LineEdit
 */
FL_LineEdit::FL_LineEdit(const WidgetDataSender *wdsender, bool isReadOnly, QWidget *parent)
    : QLineEdit(parent)
{
    setReadOnly(isReadOnly);
    if (!isReadOnly) connect(this, SIGNAL(sigFocusOut(QWidget*,QString)), wdsender, SIGNAL(sigSendLostFocusWidgetData(QWidget*,QString)));
}

void FL_LineEdit::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this, this->text());
    QLineEdit::focusOutEvent(fe);
}

/*
 * FL_SpinBox
 */
FL_SpinBox::FL_SpinBox(const WidgetDataSender *wdsender, bool isReadOnly, QWidget *parent)
    : QSpinBox(parent)
{
    setReadOnly(isReadOnly);
    setMaximum(INT_MAX);
    if (!isReadOnly) connect(this, SIGNAL(sigFocusOut(QWidget*,QString)), wdsender, SIGNAL(sigSendLostFocusWidgetData(QWidget*,QString)));
}

void FL_SpinBox::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this, this->text());
    QSpinBox::focusOutEvent(fe);
}

/*
 * FL_DoubleSpinBox
 */
FL_DoubleSpinBox::FL_DoubleSpinBox(const WidgetDataSender *wdsender, bool isReadOnly, QWidget *parent)
    : QDoubleSpinBox(parent)
{
    setReadOnly(isReadOnly);
    setMaximum((double)INT_MAX);
    if (!isReadOnly) connect(this, SIGNAL(sigFocusOut(QWidget*,QString)), wdsender, SIGNAL(sigSendLostFocusWidgetData(QWidget*,QString)));
}

void FL_DoubleSpinBox::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this, this->text());
    QDoubleSpinBox::focusOutEvent(fe);
}

/*
 * FL_PlainTextEdit
 */
FL_PlainTextEdit::FL_PlainTextEdit(const WidgetDataSender *wdsender, bool isReadOnly, QWidget *parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(isReadOnly);
    if (!isReadOnly) connect(this, SIGNAL(sigFocusOut(QWidget*,QString)), wdsender, SIGNAL(sigSendLostFocusWidgetData(QWidget*,QString)));
}

void FL_PlainTextEdit::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this, this->toPlainText());
    QPlainTextEdit::focusOutEvent(fe);
}

QWidget *createFieldWidget(int wgtType, bool isReadOnly, const WidgetDataSender *transmitter)
{
    QWidget *wgt = 0;
    switch (wgtType) {
    case dbi::DBTFieldInfo::wtype_lineEdit:
        wgt = new FL_LineEdit(transmitter, isReadOnly, 0);
        break;
    case dbi::DBTFieldInfo::wtype_spinBoxInt:
        wgt = new FL_SpinBox(transmitter, isReadOnly, 0);
        break;
    case dbi::DBTFieldInfo::wtype_spinBoxDouble:
        wgt = new FL_DoubleSpinBox(transmitter, isReadOnly, 0);
        break;
    case dbi::DBTFieldInfo::wtype_plainTextEdit:
        wgt = new FL_PlainTextEdit(transmitter, isReadOnly, 0);
        break;
    case dbi::DBTFieldInfo::wtype_not_show:
        wgt = 0;
        break;
    default:
        throw std::runtime_error("Cannot create the widget for DB table field - unknown widget type");
    }
    return wgt;
}

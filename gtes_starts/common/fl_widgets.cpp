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
FL_LineEdit::FL_LineEdit(QObject *sigReceiver, const char *slotMember, bool isReadOnly, QWidget *parent)
    : QLineEdit(parent)
{
    setReadOnly(isReadOnly);
    if (!isReadOnly) connect(this, SIGNAL(sigFocusOut(QString)), sigReceiver, slotMember);
}

void FL_LineEdit::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->text());
    QLineEdit::focusOutEvent(fe);
}

/*
 * FL_SpinBox
 */
FL_SpinBox::FL_SpinBox(QObject *sigReceiver, const char *slotMember, bool isReadOnly, QWidget *parent)
    : QSpinBox(parent)
{
    setReadOnly(isReadOnly);
    setMaximum(INT_MAX);
    if (!isReadOnly) connect(this, SIGNAL(sigFocusOut(QString)), sigReceiver, slotMember);
}

void FL_SpinBox::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->text());
    QSpinBox::focusOutEvent(fe);
}

/*
 * FL_DoubleSpinBox
 */
FL_DoubleSpinBox::FL_DoubleSpinBox(QObject *sigReceiver, const char *slotMember, bool isReadOnly, QWidget *parent)
    : QDoubleSpinBox(parent)
{
    setReadOnly(isReadOnly);
    setMaximum((double)INT_MAX);
    if (!isReadOnly) connect(this, SIGNAL(sigFocusOut(QString)), sigReceiver, slotMember);
}

void FL_DoubleSpinBox::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->text());
    QDoubleSpinBox::focusOutEvent(fe);
}

/*
 * FL_PlainTextEdit
 */
FL_PlainTextEdit::FL_PlainTextEdit(QObject *sigReceiver, const char *slotMember, bool isReadOnly, QWidget *parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(isReadOnly);
    if (!isReadOnly) connect(this, SIGNAL(sigFocusOut(QString)), sigReceiver, slotMember);
}

void FL_PlainTextEdit::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->toPlainText());
    QPlainTextEdit::focusOutEvent(fe);
}

QWidget *createFieldWidget(int wgtType, bool isReadOnly, QObject *sigReceiver, const char *slotMember)
{
    QWidget *wgt = 0;
    switch (wgtType) {
    case dbi::DBTFieldInfo::wtype_lineEdit:
        wgt = new FL_LineEdit(sigReceiver, slotMember, isReadOnly, 0);
        break;
    case dbi::DBTFieldInfo::wtype_spinBoxInt:
        wgt = new FL_SpinBox(sigReceiver, slotMember, isReadOnly, 0);
        break;
    case dbi::DBTFieldInfo::wtype_spinBoxDouble:
        wgt = new FL_DoubleSpinBox(sigReceiver, slotMember, isReadOnly, 0);
        break;
    case dbi::DBTFieldInfo::wtype_plainTextEdit:
        wgt = new FL_PlainTextEdit(sigReceiver, slotMember, isReadOnly, 0);
        break;
    case dbi::DBTFieldInfo::wtype_not_show:
        wgt = 0;
        break;
    default:
        throw std::runtime_error("Cannot create the widget for DB table field - unknown widget type");
    }
    return wgt;
}

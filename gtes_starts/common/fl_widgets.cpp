#include <stdexcept>
#include <limits>
#include "fl_widgets.h"
#include "db_info.h"

/*
 * FL_LineEdit
 */
FL_LineEdit::FL_LineEdit(bool isReadOnly, QWidget *parent)
    : QLineEdit(parent)
{
    setReadOnly(isReadOnly);
}

void FL_LineEdit::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->text());
    QLineEdit::focusOutEvent(fe);
}

/*
 * FL_SpinBox
 */
FL_SpinBox::FL_SpinBox(bool isReadOnly, QWidget *parent)
    : QSpinBox(parent)
{
    setReadOnly(isReadOnly);
    setMaximum(INT_MAX);
}

void FL_SpinBox::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->text());
    QSpinBox::focusOutEvent(fe);
}

/*
 * FL_DoubleSpinBox
 */
FL_DoubleSpinBox::FL_DoubleSpinBox(bool isReadOnly, QWidget *parent)
    : QDoubleSpinBox(parent)
{
    setReadOnly(isReadOnly);
    setMaximum((double)INT_MAX);
}

void FL_DoubleSpinBox::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->text());
    QDoubleSpinBox::focusOutEvent(fe);
}

/*
 * FL_PlainTextEdit
 */
FL_PlainTextEdit::FL_PlainTextEdit(bool isReadOnly, QWidget *parent)
    : QPlainTextEdit(parent)
{
    setReadOnly(isReadOnly);
}

void FL_PlainTextEdit::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->toPlainText());
    QPlainTextEdit::focusOutEvent(fe);
}

QWidget *createFieldWidget(int wgtType, bool isReadOnly, QWidget *parent)
{
    QWidget *wgt = 0;
    switch (wgtType) {
    case dbi::DBTFieldInfo::wtype_lineEdit:
        wgt = new FL_LineEdit(isReadOnly, parent);
        break;
    case dbi::DBTFieldInfo::wtype_spinBoxInt:
        wgt = new FL_SpinBox(isReadOnly, parent);
        break;
    case dbi::DBTFieldInfo::wtype_spinBoxDouble:
        wgt = new FL_DoubleSpinBox(isReadOnly, parent);
        break;
    case dbi::DBTFieldInfo::wtype_plainTextEdit:
        wgt = new FL_PlainTextEdit(isReadOnly, parent);
        break;
    case dbi::DBTFieldInfo::wtype_not_show:
        wgt = 0;
        break;
    default:
        throw std::runtime_error("Cannot create the widget for DB table field - unknown widget type");
    }
    return wgt;
}

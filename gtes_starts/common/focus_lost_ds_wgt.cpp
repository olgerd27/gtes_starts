#include <stdexcept>
#include "focus_lost_ds_wgt.h"
#include "db_info.h"

/*
 * FL_LineEdit
 */
FL_LineEdit::FL_LineEdit(QWidget *parent)
    : QLineEdit(parent)
{
}

void FL_LineEdit::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->text());
    QLineEdit::focusOutEvent(fe);
}

/*
 * FL_ComboBox
 */
FL_ComboBox::FL_ComboBox(QWidget *parent)
    : QComboBox(parent)
{
}

void FL_ComboBox::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->currentText()); // current text or current index?
    QComboBox::focusOutEvent(fe);
}

/*
 * FL_SpinBox
 */
FL_SpinBox::FL_SpinBox(QWidget *parent)
    : QSpinBox(parent)
{
}

void FL_SpinBox::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->text());
    QSpinBox::focusOutEvent(fe);
}

/*
 * FL_DoubleSpinBox
 */
FL_DoubleSpinBox::FL_DoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
{
}

void FL_DoubleSpinBox::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->text());
    QDoubleSpinBox::focusOutEvent(fe);
}

/*
 * FL_PlainTextEdit
 */
FL_PlainTextEdit::FL_PlainTextEdit(QWidget *parent)
    : QPlainTextEdit(parent)
{
}

void FL_PlainTextEdit::focusOutEvent(QFocusEvent *fe)
{
    emit sigFocusOut(this->toPlainText());
    QPlainTextEdit::focusOutEvent(fe);
}


QWidget *createFieldWidget(int type, QWidget *parent)
{
    QWidget *wgt = 0;
    switch (type) {
    case dbi::DBTFieldInfo::wtype_lineEdit:
        wgt = new FL_LineEdit(parent);
        break;
    case dbi::DBTFieldInfo::wtype_comboBox:
        wgt = new FL_ComboBox(parent);
        break;
    case dbi::DBTFieldInfo::wtype_spinBoxInt:
        wgt = new FL_SpinBox(parent);
        break;
    case dbi::DBTFieldInfo::wtype_spinBoxDouble:
        wgt = new FL_DoubleSpinBox(parent);
        break;
    case dbi::DBTFieldInfo::wtype_plainTextEdit:
        wgt = new FL_PlainTextEdit(parent);
        break;
    case dbi::DBTFieldInfo::wtype_not_show:
        wgt = 0;
        break;
    default:
        throw std::runtime_error("Cannot create the widget for DB table field - unknown widget type");
    }
    return wgt;
}

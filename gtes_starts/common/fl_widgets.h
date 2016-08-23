#ifndef FOCUSLOST_DS_WGT_H
#define FOCUSLOST_DS_WGT_H

#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPlainTextEdit>

/*
 * Sender QString widget data
 */
class WidgetDataSender : public QObject
{
    Q_OBJECT
public:
    WidgetDataSender(QObject *parent) : QObject(parent) { }
signals:
    void sigSendLostFocusWidgetData(QWidget *wgt, const QString &text);
};

/*
 * The widgets that emit signal with current data in time of lost the focus (FL - focus lost)
 */
class FL_LineEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit FL_LineEdit(const WidgetDataSender *wdsender = 0, bool isReadOnly = false, QWidget *parent = 0);
signals:
    void sigFocusOut(QWidget *w, const QString &text);
protected:
    void focusOutEvent(QFocusEvent *fe);
};

class FL_SpinBox : public QSpinBox
{
    Q_OBJECT
public:
    explicit FL_SpinBox(const WidgetDataSender *wdsender = 0, bool isReadOnly = false, QWidget *parent = 0);
signals:
    void sigFocusOut(QWidget *w, const QString &text);
protected:
    void focusOutEvent(QFocusEvent *fe);
};

class FL_DoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT
public:
    explicit FL_DoubleSpinBox(const WidgetDataSender *wdsender = 0, bool isReadOnly = false, QWidget *parent = 0);
signals:
    void sigFocusOut(QWidget *w, const QString &text);
protected:
    void focusOutEvent(QFocusEvent *fe);
};

class FL_PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit FL_PlainTextEdit(const WidgetDataSender *wdsender = 0, bool isReadOnly = false, QWidget *parent = 0);
signals:
    void sigFocusOut(QWidget *w, const QString &text);
protected:
    void focusOutEvent(QFocusEvent *fe);
};

/*
 * Factory method for creation the DB table widget
 */
QWidget * createFieldWidget(int wgtType, bool isReadOnly, const WidgetDataSender *transmitter);

#endif // FOCUSLOST_DS_WGT_H

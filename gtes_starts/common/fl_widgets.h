#ifndef FOCUSLOST_DS_WGT_H
#define FOCUSLOST_DS_WGT_H

#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPlainTextEdit>

/*
 * The widgets that emit signal with current data in time of lost the focus
 */
class FL_LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit FL_LineEdit(QObject *sigReceiver, const char *slotMember, bool isReadOnly = false, QWidget *parent = 0);

signals:
    void sigFocusOut(const QString &text);

protected:
    void focusOutEvent(QFocusEvent *fe);
};

class FL_SpinBox : public QSpinBox
{
    Q_OBJECT

public:
    explicit FL_SpinBox(QObject *sigReceiver, const char *slotMember, bool isReadOnly = false, QWidget *parent = 0);

signals:
    void sigFocusOut(const QString &text);

protected:
    void focusOutEvent(QFocusEvent *fe);
};

class FL_DoubleSpinBox : public QDoubleSpinBox
{
    Q_OBJECT

public:
    explicit FL_DoubleSpinBox(QObject *sigReceiver, const char *slotMember, bool isReadOnly = false, QWidget *parent = 0);

signals:
    void sigFocusOut(const QString &text);

protected:
    void focusOutEvent(QFocusEvent *fe);
};

class FL_PlainTextEdit : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit FL_PlainTextEdit(QObject *sigReceiver, const char *slotMember, bool isReadOnly = false, QWidget *parent = 0);

signals:
    void sigFocusOut(const QString &text);

protected:
    void focusOutEvent(QFocusEvent *fe);
};

/*
 * Factory method for creation the DB table widget
 */
QWidget * createFieldWidget(int wgtType, bool isReadOnly, QObject *sigReceiver, const char *slotMember);

#endif // FOCUSLOST_DS_WGT_H

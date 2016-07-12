#ifndef REIMPLEMENTED_WIDGETS_H
#define REIMPLEMENTED_WIDGETS_H

#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

/*
 * The custom QListWidget class, that resize itself for making all content data visible.
 */
class ResizableWidgetList : public QListWidget
{
public:
    explicit ResizableWidgetList(QWidget *parent = 0);
    QSize sizeHint() const;
};

/*
 * The custom QPushButton class, that store name of the database table, this button related with.
 */
class PBtnForEditDBT : public QPushButton
{
    Q_OBJECT
public:
    explicit PBtnForEditDBT(QWidget *parent = 0);

    void setDBTableName(const QString &name);
    QString DBTableName() const;

    void setIdentDataWidget(QWidget *wgt);
    QWidget * identWidget() const;

private:
    QString m_DBTableName;
    QWidget *m_identWgt;
};

/*
 * The custom label for showing a type of a model change
 */
class MChTypeLabel : public QLabel
{
    Q_OBJECT

public:
    enum ChangeTypes {
        ctype_inserted,
        ctype_deleted,
        ctype_noChange
    };

    explicit MChTypeLabel(QWidget *parent = 0);

public slots:
    void slotChangeType(int ctype);
};

/*
 * The custom line edit that send the inputed value when user press return
 */
class DataSendLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit DataSendLineEdit(QWidget *parent = 0);

signals:
    void sigReturnPressed(const QString &value);
};

#endif // REIMPLEMENTED_WIDGETS_H

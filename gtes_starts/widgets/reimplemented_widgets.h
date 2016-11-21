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
 * The custom Select/Edit QPushButton, that store name of the database table, this button related with.
 */
class SelectEditPB : public QPushButton
{
    Q_OBJECT
public:
    explicit SelectEditPB(QWidget *parent = 0);

    void setDBTableName(const QString &name);
    QString DBTableName() const;

    void setIdentDataWidget(QWidget *wgt);
    QWidget * identWidget() const;

    void setFieldNumber(int fn);
    int fieldNo() const;

private:
    QString m_DBTableName;
    QWidget *m_identWgt;
    int m_fieldNo;
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

private:
    void setCustomPalette(QPalette &pal, QColor colorBack);

    QPalette m_defaultPalette;
};

/*
 * The custom line edit, that send the inputed value when user press Enter
 */
class LE_DataSend : public QLineEdit
{
    Q_OBJECT

public:
    explicit LE_DataSend(QWidget *parent = 0);

signals:
    void sigReturnPressed(const QString &value);
};

#endif // REIMPLEMENTED_WIDGETS_H

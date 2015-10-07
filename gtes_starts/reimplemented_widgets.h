#ifndef REIMPLEMENTED_WIDGETS_H
#define REIMPLEMENTED_WIDGETS_H

#include <QListWidget>
#include <QPushButton>

/*
 * The custom QListWidget class, that resize itself for making all content data visible.
 */
class ResizableWidgetList : public QListWidget
{
public:
    ResizableWidgetList(QWidget *parent = 0);
    QSize sizeHint() const;
};

/*
 * The custom QPushButton class, that store name of the database table, with which this button related.
 */
class PBtnForEditDBT : public QPushButton
{
    Q_OBJECT
public:
    PBtnForEditDBT(QWidget *parent = 0);

    void setDBTableName(const QString &name);
    QString DBTableName() const;

    void setIdentDataWidget(QWidget *wgt);
    void setIdentData(const QString &data);

private:
    QString m_DBTableName;
    QWidget *m_identWgt;
};

#endif // REIMPLEMENTED_WIDGETS_H

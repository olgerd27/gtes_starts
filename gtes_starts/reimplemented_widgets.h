#ifndef REIMPLEMENTED_WIDGETS_H
#define REIMPLEMENTED_WIDGETS_H

#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QDebug>

/*
 * The custom QListWidget class, that resize itself for making all content data visible.
 */
class ResizableWidgetList : public QListWidget
{
public:
    ResizableWidgetList(QWidget *parent = 0)
        : QListWidget(parent)
    {
    }

    QSize sizeHint() const
    {
        QSize sizeContent = QListWidget::contentsSize();
        sizeContent.rwidth() += 5;
        return sizeContent;
    }
};

/*
 * The custom QPushButton class, that store name of the database table, with which this button related.
 */
class PushButtonForEditDBTable : public QPushButton
{
    Q_OBJECT
public:
    PushButtonForEditDBTable(QWidget *parent)
        : QPushButton(parent)
        , m_lblData(0)
    {
    }

    void setDBTableName(const QString &name) { m_DBTableName = name; }
    QString DBTableName() const { return m_DBTableName; }

    void setDataLabel(QLabel *label) { m_lblData = label; }
    QLabel * dataLabel() const { return m_lblData; }

private:
    QString m_DBTableName;
    QLabel *m_lblData;
};

#endif // REIMPLEMENTED_WIDGETS_H

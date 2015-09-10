#ifndef REIMPLEMENTED_WIDGETS_H
#define REIMPLEMENTED_WIDGETS_H

#include <QListWidget>
#include <QPushButton>
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
class PushButtonKnowsDBTable : public QPushButton
{
    Q_OBJECT
public:
    PushButtonKnowsDBTable(QWidget *parent)
        : QPushButton(parent)
    {
    }

    void setDBTableName(const QString &name) { m_DBTableName = name; }
    QString DBTableName() const { return m_DBTableName; }

private:
    QString m_DBTableName;
};

#endif // REIMPLEMENTED_WIDGETS_H

#ifndef REIMPLEMENTED_WIDGETS_H
#define REIMPLEMENTED_WIDGETS_H

#include <QListWidget>
#include <QPushButton>
#include <QDebug>

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

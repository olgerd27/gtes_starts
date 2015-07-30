#ifndef REIMPLEMENTED_WIDGETS_H
#define REIMPLEMENTED_WIDGETS_H

#include <QListWidget>
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

#endif // REIMPLEMENTED_WIDGETS_H

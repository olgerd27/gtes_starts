#ifndef WIDGET_MAPPER_H
#define WIDGET_MAPPER_H

#include <QDataWidgetMapper>

class WidgetMapper : public QDataWidgetMapper
{
    Q_OBJECT
public:
    explicit WidgetMapper(QObject *parent = 0);
    virtual ~WidgetMapper();

signals:

public slots:
    void slotSetWidgetData(QWidget *w, const QString &data);
};

#endif // WIDGET_MAPPER_H

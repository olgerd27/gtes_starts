#include <QAbstractItemModel>
#include "widget_mapper.h"

WidgetMapper::WidgetMapper(QObject *parent)
    : QDataWidgetMapper(parent)
{ }

WidgetMapper::~WidgetMapper()
{ }

void WidgetMapper::slotSetWidgetData(QWidget *w, const QString &data)
{
    const QModelIndex &currIndex = model()->index( currentIndex(), mappedSection(w) );
    model()->setData(currIndex, data, Qt::EditRole);
}

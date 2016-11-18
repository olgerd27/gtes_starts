#include <QAbstractItemModel>
#include "widget_mapper.h"

WidgetMapper::WidgetMapper(QObject *parent)
    : QDataWidgetMapper(parent)
{
    // checking boundaries of DB data records
    connect(this, &QDataWidgetMapper::currentIndexChanged, [this](int row)
    {
        emit sigFirstRowReached(row <= 0);
        emit sigLastRowReached(row >= model()->rowCount() - 1);
    } );
}

WidgetMapper::~WidgetMapper()
{ }

void WidgetMapper::slotSetWidgetData(QWidget *w, const QString &data)
{
    const QModelIndex &currIndex = model()->index( currentIndex(), mappedSection(w) );
    model()->setData(currIndex, data, Qt::EditRole);
}

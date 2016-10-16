#ifndef TABLE_VIEW_DS_H
#define TABLE_VIEW_DS_H

#include <QTableView>
#include <QHeaderView>

/*
 * The custom table view for data selection (DS).
 * This table notify about mouse pressing inside any cell (MPN - mouse press notify).
 */
class TableView_DS : public QTableView
{
    Q_OBJECT

public:
    explicit TableView_DS(QWidget *parent = 0);
    void setModel(QAbstractItemModel *model);

signals:
    void sigMousePressedOverTable();

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    void setVertHeader();
    void setHorizHeader();
    void setHorizSectionResizeMode(QHeaderView *header);
};

#endif // TABLE_VIEW_DS_H

#ifndef TABLE_VIEW_DS_H
#define TABLE_VIEW_DS_H

#include <QTableView>
#include <QHeaderView>

/*
 * The custom table view for data selection (SD).
 * This table notify about mouse pressing inside any cell (MPN - mouse press notify).
 */
class TableView_DS : public QTableView
{
    Q_OBJECT

public:
    explicit TableView_DS(QWidget *parent = 0);

signals:
    void sigMousePressedOverTable();

protected:
    void mousePressEvent(QMouseEvent *event);

private:
    void setHeaders();
    void setHorizSectionResizeMode(QHeaderView *header);
};

#endif // TABLE_VIEW_DS_H

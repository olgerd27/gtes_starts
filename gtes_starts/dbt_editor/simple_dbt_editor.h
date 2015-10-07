#ifndef SIMPLE_DBT_EDITOR_H
#define SIMPLE_DBT_EDITOR_H

#include "editor_dbt.h"

namespace Ui {
    class SimpleDBTEditor;
}
class QItemSelection;

class SimpleDBTEditor : public DBTEditor
{
    Q_OBJECT

public:
    explicit SimpleDBTEditor(DBTInfo *dbtInfo, QWidget *parent = 0);
    virtual ~SimpleDBTEditor();

private slots:
    void slotSetFilter(const QString &strFilter);

private:
    void setDBdataView();

    Ui::SimpleDBTEditor *ui;
};

#endif // SIMPLE_DBT_EDITOR_H

#ifndef SIMPLE_DBT_EDITOR_H
#define SIMPLE_DBT_EDITOR_H

#include "dbt_editor.h"

namespace Ui {
    class SimpleDBTEditor;
}
class QItemSelection;

class SimpleDBTEditor : public DBTEditor
{
    Q_OBJECT

public:
    explicit SimpleDBTEditor(dbi::DBTInfo *dbtInfo, QWidget *parent = 0);
    virtual ~SimpleDBTEditor();

private slots:
    void slotSetFilter(const QString &strFilter);

private:
    void setDBdataView();
    virtual void makeSelect(int row);

    Ui::SimpleDBTEditor *ui;
};

#endif // SIMPLE_DBT_EDITOR_H

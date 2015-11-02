#ifndef COMPLEX_DBT_EDITOR_H
#define COMPLEX_DBT_EDITOR_H

#include "dbt_editor.h"

namespace Ui {
    class ComplexDBTEditor;
}
class QItemSelection;

class ComplexDBTEditor : public DBTEditor
{
    Q_OBJECT
public:
    explicit ComplexDBTEditor(dbi::DBTInfo *dbtInfo, QWidget *parent = 0);
    virtual ~ComplexDBTEditor();

private slots:
    void slotChooseRow(const QItemSelection &selected, const QItemSelection &deselected);

private:
    void setContentsUI();
    void setEditingUI();
    virtual void makeSelect(int row);
    void setSelectedId(int selectedRow);

    Ui::ComplexDBTEditor *ui;
};

#endif // COMPLEX_DBT_EDITOR_H

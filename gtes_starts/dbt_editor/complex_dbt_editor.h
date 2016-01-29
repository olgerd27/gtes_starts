#ifndef COMPLEX_DBT_EDITOR_H
#define COMPLEX_DBT_EDITOR_H

#include <memory>
#include "dbt_editor.h"

namespace Ui {
    class ComplexDBTEditor;
}

class ComplexDBTEditor : public DBTEditor
{
    Q_OBJECT
public:
    explicit ComplexDBTEditor(dbi::DBTInfo *dbtInfo, QWidget *parent = 0);
    virtual ~ComplexDBTEditor();

private:
    void setContentsUI();
    void setEditingUI();
    virtual void makeSelect(int row);

    std::unique_ptr<Ui::ComplexDBTEditor> m_ui;
};

#endif // COMPLEX_DBT_EDITOR_H
